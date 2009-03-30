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

#include <config.h> // For GLOM_ENABLE_MAEMO

#include <glom/utils_ui.h>
#include <libglom/connectionpool.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/glomconversions.h>

#include <glibmm/i18n.h>
#include <gtkmm/messagedialog.h>

#include <giomm.h>

#ifdef GLOM_ENABLE_MAEMO
#include <hildonmm/note.h>
#endif

#include <string.h> // for strchr
#include <sstream> //For stringstream

#include <iostream>
#include <fstream>

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl
#include <iomanip>

#include <stack>

namespace
{

// Basically copied from libgnome (gnome-help.c, Copyright (C) 2001 Sid Vicious
// Copyright (C) 2001 Jonathan Blandford <jrb@alum.mit.edu>), but C++ified
std::string locate_help_file(const std::string& path, const std::string& doc_name)
{
  // g_get_language_names seems not to be wrapped by glibmm
  const char* const* lang_list = g_get_language_names ();

  for(unsigned int j = 0; lang_list[j] != NULL; ++j)
  {
    const char* lang = lang_list[j];

    /* This has to be a valid language AND a language with
     * no encoding postfix.  The language will come up without
     * encoding next. */
    if(lang == NULL || strchr(lang, '.') != NULL)
      continue;

    const char* exts[] = { "", ".xml", ".docbook", ".sgml", ".html", NULL };
    for(unsigned i = 0; exts[i] != NULL; ++i)
    {
      std::string name = doc_name + exts[i];
      std::string full = Glib::build_filename(path, Glib::build_filename(lang, name));

      if(Glib::file_test(full, Glib::FILE_TEST_EXISTS))
        return full;
    }
  }

  return std::string();
}

} //anonymous namespace

namespace Glom
{

/* Run dialog and response on Help if appropriate */

int Utils::dialog_run_with_help(Gtk::Dialog* dialog, const Glib::ustring& id)
{
  int result = dialog->run();
  while (result == Gtk::RESPONSE_HELP)
  {
    show_help(id);
    result = dialog->run();
  }

  dialog->hide();
  return result;
}

/*
 * Help::show_help(const std::string& id)
 *
 * Launch a help browser with the glom help and load the given id if given
 * If the help cannot be found an error dialog will be shown
 */

void Utils::show_help(const Glib::ustring& id)
{
  // TODO_maemo: Show help on maemo by some other means
#ifndef GLOM_ENABLE_MAEMO
  GError* err = 0;
  const gchar* pId;
  if(id.length())
  {
    pId = id.c_str();
  }
  else
  {
    pId = 0;
  }

  try
  {
    const char* path = DATADIR "/gnome/help/glom";
    std::string help_file = locate_help_file(path, "glom.xml");
    if(help_file.empty())
    {
      throw std::runtime_error(_("No help file available"));
    }
    else
    {
      std::string uri = "ghelp:" + help_file;
      if(pId) { uri += "?"; uri += pId; }

      // g_app_info_launch_default_for_uri seems not to be wrapped by giomm
      if(!g_app_info_launch_default_for_uri(uri.c_str(), NULL, &err))
      {
        std::string message(err->message);
        g_error_free(err);
        throw std::runtime_error(message);
      }
    }
  }
  catch(const std::exception& ex)
  {
    const Glib::ustring message = _("Could not display help: ") + Glib::ustring(ex.what());
    Gtk::MessageDialog dialog(message, false, Gtk::MESSAGE_ERROR);
    dialog.run();
  }
#endif
}

void Utils::show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message, Gtk::Window* parent, Gtk::MessageType message_type)
{
#ifdef GLOM_ENABLE_MAEMO
  // TODO_maemo: Map message_type to a senseful stock_id?
  Hildon::Note dialog(Hildon::NOTE_TYPE_INFORMATION, parent, message);
#else
  Gtk::MessageDialog dialog("<b>" + title + "</b>", true /* markup */, message_type, Gtk::BUTTONS_OK);
  dialog.set_secondary_text(message);
  if(parent)
    dialog.set_transient_for(*parent);
#endif

  dialog.run();
}

void Utils::show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message, Gtk::Window& parent, Gtk::MessageType message_type)
{
  show_ok_dialog(title, message, &parent, message_type);
}

namespace
{

static void on_window_hide(Glib::RefPtr<Glib::MainLoop> main_loop, sigc::connection handler_connection)
{
  handler_connection.disconnect(); //This should release a main_loop reference.
  main_loop->quit();

  //main_loop should be destroyed soon, because nothing else is using it.
}

} //anonymous namespace.

void Utils::show_window_until_hide(Gtk::Window* window)
{
  if(!window)
    return;

  Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create(false /* not running */);

  //Stop the main_loop when the window is hidden:
  sigc::connection handler_connection; //TODO: There seems to be a crash if this is on the same line.
  handler_connection = window->signal_hide().connect( 
    sigc::bind(
      sigc::ptr_fun(&on_window_hide),
      main_loop, handler_connection
    ) );
  
  window->show();
  main_loop->run(); //Run and block until it is stopped by the hide signal handler.
}

Glib::ustring Utils::bold_message(const Glib::ustring& message)
{
  return "<b>" + message + "</b>";
}


} //namespace Glom
