/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <libglom/spawn_with_feedback.h>
#include <libglom/dialog_progress_creating.h>
#include <libglom/glade_utils.h>
#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <glibmm/i18n.h>
#include <memory> //For auto_ptr.
#include <iostream>

#ifdef G_OS_WIN32
#define SAVE_DATADIR DATADIR
#undef DATADIR
#include <windows.h>
#define DATADIR SAVE_DATADIR
#endif


namespace Glom
{

namespace Spawn
{

class CommandLineThreadData
{
public:
  std::string m_command;
  Glib::Cond* m_cond;
  Glib::Mutex* m_mutex;
  bool* m_result;
};

// This is a simple process launching API wrapping g_spawn_async on linux and
// CreateProcess() on Windows. We need to use CreateProcess on Windows to be
// able to suppress the console window.
namespace Impl
{

static const unsigned int REDIRECT_STDOUT = 1;
static const unsigned int REDIRECT_STDERR = 2;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
class SpawnError: public std::runtime_error
{
public:
  SpawnError(const std::string& error_message): std::runtime_error(error_message) {}
};
#endif

class SpawnInfo: public sigc::trackable
{
private:
  // Private platform-dependant helper functions
#ifdef G_OS_WIN32
  static std::string win32_error_message()
  {
    gchar* message = g_win32_error_message(GetLastError());
    std::string msg = message;
    g_free(message);
    return msg;
  }
  // We redirect stdout and stderr output into a file, and read that file later.
  // I would prefer to read child stderr directly, without using a temporary
  // file, but that is probably not so easy since Glib::IOChannel does not
  // support Windows file HANDLEs, but we need to pass a HANDLE to
  // CreateProcess() for redirection.
  static HANDLE create_redirect_file(std::string& filename)
  {
    static unsigned int redirect_seq = 0;
    const gchar* appdata = getenv("APPDATA");
    filename = Glib::ustring::compose("%1\\glom_spawn_redirect_%2.txt", std::string(appdata), ++ redirect_seq);

    // Allow the redirect file to be inherited by the child process, so
    // it can write to it.
    SECURITY_ATTRIBUTES security_attr;
    security_attr.nLength = sizeof(security_attr);
    security_attr.lpSecurityDescriptor = NULL;
    security_attr.bInheritHandle = TRUE;

    HANDLE result = CreateFile(filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, &security_attr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    if(!result) throw SpawnError(win32_error_message());
#endif // GLIBMM_EXCEPTIONS_ENABLED
    return result;
  }
#else // G_OS_WIN32
  bool on_io(Glib::IOCondition cond, Glib::RefPtr<Glib::IOChannel> channel, std::string& result)
  {
    if(cond != Glib::IO_IN)
    {
      // Perhaps the pipe was closed or something. Ignore & Disconnect. If the
      // this was because the child exited, then the on_child_watch() callback
      // will be called anyway.
      return false;
    }
    else
    {
      char buffer[1024 + 1];
      gsize bytes_read;

      Glib::IOStatus status;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
      try
      {
        status = channel->read(buffer, 1024, bytes_read);
      }
      catch(const Glib::Exception& ex)
      {
        std::cerr << "Glom::Spawn::Impl::SpawnInfo::on_io: Error while reading from pipe: " << ex.what() << std::endl;
        return false;
      }
#else
      std::auto_ptr<Glib::Error> error;
      status = channel->read(buffer, 1024, bytes_read, error);
      if(error.get())
      {
        std::cerr << "Glom::Spawn::Impl::SpawnInfo::on_io: Error while reading from pipe: " << error->what() << std::endl;
        return false;
      }
#endif

      buffer[bytes_read] = '\0';
      result += buffer;

      return status == Glib::IO_STATUS_NORMAL || status == Glib::IO_STATUS_AGAIN;
    }
  }

  void redirect_to_string(int fd, std::string& string)
  {
    Glib::RefPtr<Glib::IOChannel> channel = Glib::IOChannel::create_from_fd(fd);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    channel->set_flags(Glib::IO_FLAG_NONBLOCK);
#else
    std::auto_ptr<Glib::Error> error;
    channel->set_flags(Glib::IO_FLAG_NONBLOCK, error);
#endif // !GLIBMM_EXCEPTIONS_ENABLED

    channel->set_encoding("");
    channel->set_buffered(false);

    Glib::signal_io().connect(sigc::bind(sigc::mem_fun(*this, &SpawnInfo::on_io), channel, sigc::ref(string)), channel, Glib::IO_IN);
  }
#endif // !G_OS_WIN32

public:
  typedef sigc::signal<void> SignalFinished;

  SpawnInfo(const Glib::ustring& command_line, int redirect):
    running(false), return_status(0)
  {
#ifdef G_OS_WIN32
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESTDHANDLES;
    startup_info.lpReserved = NULL;
    startup_info.lpDesktop = NULL;
    startup_info.lpTitle = NULL;
    startup_info.cbReserved2 = 0;
    startup_info.lpReserved2 = NULL;
    startup_info.hStdInput = INVALID_HANDLE_VALUE;

    if(redirect & REDIRECT_STDOUT)
      startup_info.hStdOutput = create_redirect_file(stdout_file);
    else
      startup_info.hStdOutput = INVALID_HANDLE_VALUE;

    if(redirect & REDIRECT_STDERR)
      startup_info.hStdError = create_redirect_file(stderr_file);
    else
      startup_info.hStdError = INVALID_HANDLE_VALUE;

    // CreateProcess needs a non-const string, so we copy command_line to provide one.
    std::vector<char> command(command_line.length() + 1);
    std::copy(command_line.data(), command_line.data() + command_line.length(), command.begin());
    command[command_line.length()] = '\0';

    if(!CreateProcess(NULL, &command[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &startup_info, &process_info))
    {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
      throw SpawnError(win32_error_message());
#endif // GLIBMM_EXCEPTIONS_ENABLED
    }
#else // G_OS_WIN32
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    try
#endif // GLIBMM_EXCEPTIONS_ENABLED
    {
      std::vector<std::string> arguments = Glib::shell_parse_argv(command_line);
      int child_stdout;
      int child_stderr;
      Glib::spawn_async_with_pipes(Glib::get_current_dir(), arguments, Glib::SPAWN_DO_NOT_REAP_CHILD, sigc::slot<void>(), &pid, NULL, redirect & REDIRECT_STDOUT ? &child_stdout : NULL, redirect & REDIRECT_STDERR ? &child_stderr : NULL);

      if(redirect & REDIRECT_STDOUT)
        redirect_to_string(child_stdout, stdout_text);
      if(redirect & REDIRECT_STDERR)
        redirect_to_string(child_stderr, stderr_text);
    }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    catch(Glib::Exception& ex)
    {
      throw SpawnError(ex.what());
    }
#endif // GLIBMM_EXCEPTIONS_ENABLED
#endif // !G_OS_WIN32

    Glib::signal_child_watch().connect(sigc::mem_fun(*this, &SpawnInfo::on_child_watch), get_pid());
  }

  ~SpawnInfo()
  {
#ifdef G_OS_WIN32
    // TODO: Is it allowed to close the process handle while the process is still running?
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

    if(!stdout_file.empty())
    {
      CloseHandle(startup_info.hStdOutput);
      DeleteFile(stdout_file.c_str());
    }

    if(!stderr_file.empty())
    {
      CloseHandle(startup_info.hStdError);
      DeleteFile(stderr_file.c_str());
    }
#else
    // TODO: What happens with the process if the child exits, but the mainloop
    // is not run anymore (and therefore our callback is not called) before
    // Glom exits itself? Is the child properly closed, or does it remain as
    // a zombie?
    if(running)
      Glib::signal_child_watch().connect(sigc::hide<1>(sigc::ptr_fun(Glib::spawn_close_pid)), pid);
#endif
  }

  Glib::Pid get_pid()
  {
#ifdef G_OS_WIN32
    return process_info.hProcess;
#else
    return pid;
#endif
  }

  bool is_running() const { return running; }
  int get_return_status() const { g_assert(!running); return return_status; }

  void get_stdout(std::string& out) const
  {
#ifdef G_OS_WIN32
    out = Glib::file_get_contents(stdout_file);
#else
    out = stdout_text;
#endif
  }

  void get_stderr(std::string& err) const
  {
#ifdef G_OS_WIN32
    err = Glib::file_get_contents(stderr_file);
#else
    err = stderr_text;
#endif
  }

  SignalFinished signal_finished() const { return m_signal_finished; }

private:
  void on_child_watch(Glib::Pid /* pid */, int returned)
  {
    running = false;
    return_status = returned;

    m_signal_finished.emit();
  }

  bool running;
  int return_status;
  SignalFinished m_signal_finished;

#ifdef G_OS_WIN32
  PROCESS_INFORMATION process_info;
  STARTUPINFO startup_info;
  std::string stderr_file;
  std::string stdout_file;
#else
  Glib::Pid pid;
  std::string stdout_text;
  std::string stderr_text;
#endif
};

std::auto_ptr<const SpawnInfo> spawn_async(const Glib::ustring& command_line, int redirect)
{
  return std::auto_ptr<const SpawnInfo>(new SpawnInfo(command_line, redirect));
}

bool spawn_async_end(std::auto_ptr<const SpawnInfo> info, std::string* stdout_text = NULL, std::string* stderr_text = NULL, int* return_status = NULL)
{
  if(stdout_text) info->get_stdout(*stdout_text);
  if(stderr_text) info->get_stderr(*stderr_text);
  if(return_status) *return_status = info->get_return_status();
  return !info->is_running();
}

int spawn_sync(const Glib::ustring& command_line, std::string* stdout_text, std::string* stderr_text)
{
  int redirect_flags = 0;
  if(stdout_text) redirect_flags |= REDIRECT_STDOUT;
  if(stderr_text) redirect_flags |= REDIRECT_STDERR;

  std::auto_ptr<const SpawnInfo> info = spawn_async(command_line, redirect_flags);
  info->signal_finished().connect(sigc::ptr_fun(&Gtk::Main::quit));

  // Wait for termination
  Gtk::Main::run();

  int return_status = 0;
  bool returned = spawn_async_end(info, stdout_text, stderr_text, &return_status);
  g_assert(returned);
  return return_status;
}

} // namespace Impl

static Dialog_ProgressCreating* get_and_show_pulse_dialog(const Glib::ustring& message, Gtk::Window* parent_window)
{
  if(!parent_window)
    std::cerr << "debug: Glom: get_and_show_pulse_dialog(): parent_window is NULL" << std::endl;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  Glib::RefPtr<Gtk::Builder> refXml = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom.glade"), "window_progress");
#else
  std::auto_ptr<Glib::Error> error;
  Glib::RefPtr<Gtk::Builder> refXml = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom.glade"), "window_progress", "", error);
  if(error.get())
    return 0;
#endif

  if(refXml)
  {
    Dialog_ProgressCreating* dialog_progress = 0;
    refXml->get_widget_derived("window_progress", dialog_progress);
    if(dialog_progress)
    {
      dialog_progress->set_message(_("Processing"), message);
      dialog_progress->set_modal();

      if(parent_window)
        dialog_progress->set_transient_for(*parent_window);

      dialog_progress->show();

      return dialog_progress;
    }
  }

  return NULL;
}



bool execute_command_line_and_wait(const std::string& command, const Glib::ustring& message, Gtk::Window* parent_window)
{
  if(!parent_window)
    std::cerr << "debug: Glom: execute_command_line_and_wait(): parent_window is NULL" << std::endl;

  //Show a dialog with a pulsing progress bar and a human-readable message, while we wait for the command to finish:
  //
  //Put the dialog in an auto_ptr so that it will be deleted (and hidden) when the current function returns.
  Dialog_ProgressCreating* dialog_temp = get_and_show_pulse_dialog(message, parent_window);
  std::auto_ptr<Dialog_ProgressCreating> dialog_progress;
  dialog_progress.reset(dialog_temp);

  std::auto_ptr<const Impl::SpawnInfo> info = Impl::spawn_async(command, 0);
  info->signal_finished().connect(
    sigc::bind(sigc::mem_fun(*dialog_progress, &Dialog_ProgressCreating::response), Gtk::RESPONSE_ACCEPT));

  // Pulse two times a second
  Glib::signal_timeout().connect(
    sigc::bind_return(sigc::mem_fun(*dialog_progress, &Dialog_ProgressCreating::pulse), true),
    500);

  dialog_progress->run();

  int return_status;
  bool returned = Impl::spawn_async_end(info, NULL, NULL, &return_status);
  if(!returned) return false; // User closed the dialog prematurely?

  return (return_status == 0);
}

// Callback handlers for execute_command_line_and_wait_until_second_command_returns_success
namespace {
  bool on_timeout(const std::string& second_command, const std::string& success_text, Dialog_ProgressCreating* dialog_progress)
  {
    Glib::ustring stored_env_lang;
    Glib::ustring stored_env_language;
    if(!success_text.empty())
    {
      // If we are going to check the text output of the second command, 
      // then we should make sure that we get a fairly canonical version of that text,
      // so we set the LANG for this command.
      // We have to set LANGUAGE (a GNU extension) as well as LANG, because it 
      // is probably defined on the system already and that definition would override our LANG:  
      // (Note that we can not just do "LANG=C;the_command", as on the command line, because g_spawn() does not support that.)

      std::cout << std::endl << "debug: temporarily setting LANG and LANGUAGE environment variables to \"C\"" << std::endl;
      stored_env_lang = Glib::getenv("LANG");
      stored_env_language = Glib::getenv("LANGUAGE");
      Glib::setenv("LANG", "C", true /* overwrite */);
      Glib::setenv("LANGUAGE", "C", true /* overwrite */);
    }

    std::cout << std::endl << "debug: command_line (second): " << second_command << std::endl << std::endl;

    int return_status = 0;
    std::string stdout_output;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    try
    {
      return_status = Impl::spawn_sync(second_command, &stdout_output, NULL);
    }
    catch(const Impl::SpawnError& ex)
    {
      std::cerr << "Glom::execute_command_line_and_wait_until_second_command_returns_success() Exception while calling Glib::spawn_command_line_sync(): " << ex.what() << std::endl;
      // TODO: We should cancel the whole call if this fails three times in 
      // a row or so.
    }
#else
    Impl::spawn_sync(second_commands, &stdout_output, NULL, &return_status);
#endif

    if(!success_text.empty())
    {
      // Restore the previous environment variable values:
      std::cout << std::endl << "debug: restoring the LANG and LANGUAGE environment variables." << std::endl;
      Glib::setenv("LANG", stored_env_lang, true /* overwrite */);
      Glib::setenv("LANGUAGE", stored_env_language, true /* overwrite */);
    }

    if(return_status == 0)
    {
      bool success = true; //Just check the return code.
      if(!success_text.empty()) //Check the output too.
      {
        std::cout << " debug: output=" << stdout_output << ", waiting for=" << success_text << std::endl;
        if(stdout_output.find(success_text) == std::string::npos)
          success = false;
      }

      if(success)
      {
        std::cout << "Success, do response" << std::endl;
        // Exit from run() in execute_command_line_and_wait_until_second_command_returns_success().
        dialog_progress->response(Gtk::RESPONSE_OK);
        // Cancel timeout. Actually, we also could return true here since
        // the signal is disconnect explicetely after run() anyway.
        return false;
      }
    }
    else
    {
       std::cout << " debug: second command failed. output=" << stdout_output << std::endl;
    }

    dialog_progress->pulse();
    return true;
  }
}

bool execute_command_line_and_wait_until_second_command_returns_success(const std::string& command, const std::string& second_command, const Glib::ustring& message, Gtk::Window* parent_window, const std::string& success_text)
{
  if(!parent_window)
    std::cerr << "debug: Glom: execute_command_line_and_wait_until_second_command_returns_success(): parent_window is NULL" << std::endl;

  Dialog_ProgressCreating* dialog_temp = get_and_show_pulse_dialog(message, parent_window);
  std::auto_ptr<Dialog_ProgressCreating> dialog_progress;
  dialog_progress.reset(dialog_temp);

  std::cout << "Command: " << command << std::endl;

  std::auto_ptr<const Impl::SpawnInfo> info = Impl::spawn_async(command, Impl::REDIRECT_STDERR);

  // While we wait for the second command to finish we
  // a) check whether the first command finished. If it did, and has a
  // negative error code, we assume it failed and return directly.
  // b) Get stderr data, to display an error message in case the command
  // fails:

  // Hide dialog when the first command finished for some reason
  sigc::connection watch_conn = info->signal_finished().connect(sigc::bind(sigc::mem_fun(*dialog_temp, &Dialog_ProgressCreating::response), Gtk::RESPONSE_REJECT));

  // Call the second command once every second
  sigc::connection timeout_conn = Glib::signal_timeout().connect(sigc::bind(sigc::ptr_fun(&on_timeout), sigc::ref(second_command), sigc::ref(success_text), dialog_temp), 1000);

  // Enter the main loop
  int response = dialog_temp->run();

  timeout_conn.disconnect();
  watch_conn.disconnect();

  std::string stderr_text;

  Impl::spawn_async_end(info, NULL, &stderr_text, NULL);

  if(response == Gtk::RESPONSE_OK)
  {
    //Sleep for a bit more, because I think that pg_ctl sometimes reports success too early.
    Glib::signal_timeout().connect(sigc::bind_return(sigc::ptr_fun(&Gtk::Main::quit), false), 3000);
    Gtk::Main::run();

    return true;
  }
  else
  {
    // The user either cancelled, or the first command failed, or exited prematurely
    if(response == Gtk::RESPONSE_REJECT)
    {
      // Command failed
      std::auto_ptr<Gtk::MessageDialog> error_dialog;
      if(parent_window)
        error_dialog.reset(new Gtk::MessageDialog(*parent_window, "Child command failed", false, Gtk::MESSAGE_ERROR));
      else
        error_dialog.reset(new Gtk::MessageDialog("Child command failed", false, Gtk::MESSAGE_ERROR));

      // TODO: i18n
      error_dialog->set_secondary_text("The command was:\n\n" + Glib::Markup::escape_text(command) + (stderr_text.empty() ? Glib::ustring("") : ("\n\n<small>" + Glib::Markup::escape_text(stderr_text) + "</small>")), true);
      error_dialog->run();
    }
    else
    {
      // User cancelled or closed pulse window
      // TODO: Terminate/Kill first command? We probably don't need
      // it any longer.
    }

    return false;
  }
}

#if 0
bool execute_command_line_and_wait_fixed_seconds(const std::string& command, unsigned int seconds, const Glib::ustring& message, Gtk::Window* parent_window)
{
  Gtk::MessageDialog dialog(Utils::bold_message(message), true, Gtk::MESSAGE_INFO, Gtk::BUTTONS_NONE, true /* modal */); 
  if(parent_window)
    dialog.set_transient_for(*parent_window);

  dialog.show();

  //Allow GTK+ to perform all updates for us
  //Without this, the dialog will seem empty.
  while(Gtk::Main::instance()->events_pending())
    Gtk::Main::instance()->iteration();

  std::cout << std::endl << "debug: command_line: " << command << std::endl << std::endl;

  try
  {
    Glib::spawn_command_line_async(command);
  }
  catch(const Glib::SpawnError& ex)
  {
    std::cerr << "Glom::Spawn::execute_command_line_and_wait_fixed_seconds() Exception while calling lib::spawn_command_line_async(): " << ex.what() << std::endl;
  }

  sleep(seconds); //Give the command enough time to make something ready for us to continue.
  return true;
}
#endif

} //Spawn

} //Glom

