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

#include <glom/libglom/spawn_with_feedback.h>
#include <glom/libglom/dialog_progress_creating.h>
#include <bakery/bakery.h>
#include <glibmm/i18n.h>
#include <iostream>


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


static void execute_command_line_on_thread_create(CommandLineThreadData* data)
{
  std::cout << "  debug: thread start" << std::endl; 

  int return_status = 0;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    Glib::spawn_command_line_sync(data->m_command, NULL, NULL, &return_status);
  }
  catch(const Glib::SpawnError& ex)
  {
    std::cerr << "Glom:: execute_command_line_on_thread_create() Exception while calling lib::spawn_command_line_sync(): " << ex.what() << std::endl;
  }
#else
  // TODO: I guess we can't find out whether this failed.
  // This might be a glibmm bug.
  Glib::spawn_command_line_sync(data->m_command, NULL, NULL, &return_status);
#endif // !GLIBMM_EXCEPTIONS_ENABLED
  
  std::cout << "  debug: in thread: signalling condition" << std::endl; 

  // TODO: Use WIFEXITED() and WEXITSTATUS()? 
  *(data->m_result) = (return_status == 0);

  data->m_mutex->lock(); //The documentation for g_cond_broadcast() says "It is good practice to lock the same mutex as the waiting threads, while calling this function, though not required."
  data->m_cond->broadcast(); //Allows the caller to continue.
  data->m_mutex->unlock();

  delete data; //Note that this doesn't delete the data pointed to by data->m_result.
}

static bool pulse_until_thread_finished(Dialog_ProgressCreating& dialog_progress, const std::string& command, const sigc::slot<void, CommandLineThreadData*>& thread_slot)
{
  // Spawn the command in a thread, waiting for a condition to be signalled by that thread when it has finished.
  // This allows us to update the UI in this thread while we wait:
  Glib::Cond cond;
  Glib::Mutex cond_mutex;

  bool result = false;
 
  CommandLineThreadData* data = new CommandLineThreadData(); //This will be deleted by the slot when it has finished.
  data->m_command = command;
  data->m_cond = &cond;
  data->m_mutex = &cond_mutex;
  data->m_result = &result;

  // Create a thread, which will start by calling our slot
  // We use sigc::bind to pass extra information to that slot.
  //
  // The slot signals the condition when it has finished.
  // so the caller can repeatedly wait for the signal until the thread has finished. 
  try
  {
    Glib::Thread::create( sigc::bind(thread_slot, data), false /* joinable */);
  }
  catch(const Glib::ThreadError& ex)
  {
    std::cerr << "Glom::Spawn::execute_command_line_and_wait_do_work(): Glib::Thread::create() failed." << std::endl;
  }


  // Loop, updating the UI, waiting for the condition to be signalled by the thread:
  cond_mutex.lock(); //The mutex used for timed_wait() must be locked.
  bool keep_waiting = true;
  while(keep_waiting)
  {
    Glib::TimeVal abs_time;
    abs_time.assign_current_time();
    abs_time.add_milliseconds(500); /* Check 2 times per second */
    if(cond.timed_wait(cond_mutex, abs_time))
    {
      keep_waiting = false;
    }
    else
    {
      dialog_progress.pulse();

      while(Gtk::Main::instance()->events_pending())
        Gtk::Main::instance()->iteration();
    }
  }
  cond_mutex.unlock();

  return result;
}


static Dialog_ProgressCreating* get_and_show_pulse_dialog(const Glib::ustring& message, Gtk::Window* parent_window)
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_progress");
#else
  std::auto_ptr<Gnome::Glade::XmlError> error;
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_progress", "", error);
  if(error.get()) return NULL;
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

      //Ensure that the dialog is shown, instead of waiting for the application to be idle:
      while(Gtk::Main::instance()->events_pending())
        Gtk::Main::instance()->iteration();

      return dialog_progress;
    }
  }

  return NULL;
}



bool execute_command_line_and_wait(const std::string& command, const Glib::ustring& message, Gtk::Window* parent_window)
{
  //Show a dialog with a pulsing progress bar and a human-readable message, while we wait for the command to finish:
  //
  //Put the dialog in an auto_ptr so that it will be deleted (and hidden) when the current function returns.
  Dialog_ProgressCreating* dialog_temp = get_and_show_pulse_dialog(message, parent_window);
  std::auto_ptr<Dialog_ProgressCreating> dialog_progress;
  dialog_progress.reset(dialog_temp);


  std::cout << std::endl << "debug: command_line: " << command << std::endl << std::endl;

  return pulse_until_thread_finished(*dialog_progress, command, sigc::ptr_fun(&execute_command_line_on_thread_create) );
}


bool execute_command_line_and_wait_until_second_command_returns_success(const std::string& command, const std::string& second_command, const Glib::ustring& message, Gtk::Window* parent_window, const std::string& success_text)
{
  Dialog_ProgressCreating* dialog_temp = get_and_show_pulse_dialog(message, parent_window);
  std::auto_ptr<Dialog_ProgressCreating> dialog_progress;
  dialog_progress.reset(dialog_temp);


  std::cout << std::endl << "debug: command_line: " << command << std::endl << std::endl;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  // Execute the first thread asynchronously (so we don't wait for it):
  try
  {
    Glib::spawn_command_line_async(command);
  }
  catch(const Glib::SpawnError& ex)
  {
    std::cerr << "Glom::Spawn::pulse_until_second_command_succeed() Exception while calling lib::spawn_command_line_async(): " << ex.what() << std::endl;
  }
#else
  // TODO: I guess we can't find out whether this failed.
  // This might be a glibmm bug.
  Glib::spawn_command_line_async(command);
#endif

  // Loop, updating the UI, repeatedly trying the second commmand, until the second command succeeds:
  while(true)
  {
    sleep(1); // To stop us calling the second command too often.

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
      Glib::spawn_command_line_sync(second_command, &stdout_output, NULL, &return_status);
    }
    catch(const Glib::SpawnError& ex)
    {
      std::cerr << "Glom::execute_command_line_and_wait_until_second_command_returns_success() Exception while calling lib::spawn_command_line_sync(): " << ex.what() << std::endl;
    }
#else
    // TODO: I guess we can't find out whether this failed.
    // This might be a glibmm bug.
    Glib::spawn_command_line_sync(second_command, &stdout_output, NULL, &return_status);
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
      if(success_text.empty()) //Just check the return code.
        return true;
      else //Check the output too.
      {
        std::cout << " debug: output=" << stdout_output << ", waiting for=" << success_text << std::endl;
        if(stdout_output.find(success_text) != std::string::npos)
        {
          sleep(3); //Sleep for a bit more, because I think that pg_ctl sometimes reports success too early.
          return true;
        }
      }
    }
    else
    {
       std::cout << " debug: second command failed. output=" << stdout_output << std::endl;
    }

    dialog_progress->pulse();

    while(Gtk::Main::instance()->events_pending())
      Gtk::Main::instance()->iteration();
  }

  return false;
}


#if 0
bool execute_command_line_and_wait_fixed_seconds(const std::string& command, unsigned int seconds, const Glib::ustring& message, Gtk::Window* parent_window)
{
  Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(message), true, Gtk::MESSAGE_INFO, Gtk::BUTTONS_NONE, true /* modal */); 
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

