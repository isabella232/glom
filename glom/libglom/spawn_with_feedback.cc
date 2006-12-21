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

  try
  {
    Glib::spawn_command_line_sync(data->m_command, NULL, NULL, &return_status);
  }
  catch(const Glib::SpawnError& ex)
  {
    std::cerr << "Glom:: execute_command_line_on_thread_create() Exception while calling lib::spawn_command_line_sync(): " << ex.what() << std::endl;
  }
  
  *(data->m_result) = (return_status == 0);
  delete data; //Note that this doesn't delete the data pointed to by data->m_result.

  std::cout << "  debug: in thread: signalling condition" << std::endl; 

  data->m_mutex->lock(); //The documentation for g_cond_broadcast() says "It is good practice to lock the same mutex as the waiting threads, while calling this function, though not required."
  data->m_cond->broadcast(); //Allows the caller to continue.
  data->m_mutex->unlock();
}

/** Perform the command in a separate thread, releasing the mutex when it is finished.
 * This function returns immediately.
 * You should repeatedly wait for the condition, to wait for the thread to finish, 
 * doing whatever else you need to do (such as updating the UI).
 */
static void execute_command_line_and_wait_do_work(const std::string& command, Glib::Cond& condition, Glib::Mutex& condition_mutex, bool& result)
{
  CommandLineThreadData* data = new CommandLineThreadData(); //This will be deleted by the slot when it has finished.
  data->m_command = command;
  data->m_cond = &condition;
  data->m_mutex = &condition_mutex;
  data->m_result = &result;

  // Create a thread, which will start by calling our slot
  // We use sigc::bind to pass extra information to that slot.
  //
  // The slot locks the mutex and unlocks it when it has finished,
  // so the caller can (try to) lock the mutex to block/loop until the thread has finished. 
  try
  {
    Glib::Thread::create( sigc::bind(sigc::ptr_fun(&execute_command_line_on_thread_create), data), false /* joinable */);
  }
  catch(const Glib::ThreadError& ex)
  {
    std::cerr << "Glom::Spawn::execute_command_line_and_wait_do_work(): Glib::Thread::create() failed." << std::endl;
  }
}

/** Execute a command-line command, and wait for it to return.
 * @param command The command-line command.
 * @param message A human-readable message to be shown, for instance in a dialog, while waiting. 
 */
bool execute_command_line_and_wait(const std::string& command, const Glib::ustring& message, Gtk::Window* parent_window)
{
  std::auto_ptr<Dialog_ProgressCreating> dialog_progress;
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_progress");
  if(refXml)
  {
    Dialog_ProgressCreating* dialog_progress_temp = 0;
    refXml->get_widget_derived("window_progress", dialog_progress_temp);
    if(dialog_progress_temp)
    {
      dialog_progress_temp->set_message(_("Processing"), message);
      dialog_progress_temp->set_modal();

      dialog_progress.reset(dialog_progress_temp); //Put the dialog in an auto_ptr so that it will be deleted (and hidden) when the current function returns.

      if(parent_window)
        dialog_progress->set_transient_for(*parent_window);

      dialog_progress->show();

      //Ensure that the dialog is shown, instead of waiting for the application to be idle:
      while(Gtk::Main::instance()->events_pending())
        Gtk::Main::instance()->iteration();
    }
  }

  std::cout << std::endl << "debug: command_line: " << command << std::endl << std::endl;

 
  // Spawn the command in a thread, waiting for a condition to be signalled by that thread when it has finished.
  // This allows us to update the UI in this thread while we wait:
  Glib::Cond cond;
  Glib::Mutex cond_mutex;

  bool result = false;
  execute_command_line_and_wait_do_work(command, cond, cond_mutex, result);

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
      dialog_progress->pulse();

      while(Gtk::Main::instance()->events_pending())
        Gtk::Main::instance()->iteration();
    }
  }
  cond_mutex.unlock();

  return result;
}



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

} //Spawn

} //Glom

