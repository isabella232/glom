/* Glom
 *
 * Copyright (C) 2012 Murray Cumming
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

#include "application.h"
#include "appwindow.h"
#include <glom/glade_utils.h>
#include <iostream>

namespace Glom
{

// We use Gio::APPLICATION_NON_UNIQUE because we have some singletons and other static data,
// to simplify our code.
// We also want to prevent all instances from crashing when one instance crashes.
Application::Application()
: Gtk::Application("org.glom.application", Gio::APPLICATION_HANDLES_OPEN | Gio::APPLICATION_NON_UNIQUE)
{
}

Glib::RefPtr<Application> Application::create()
{
  return Glib::RefPtr<Application>( new Application() );
}

void Application::create_window(const Glib::RefPtr<Gio::File>& file)
{
  AppWindow* window = 0;
  Glom::Utils::get_glade_widget_derived_with_warning(window);
  g_assert(window);

  //Make sure that the application runs for as long this window is still open:
  add_window(*window);

  //Delete the window when it is hidden:
  window->signal_hide().connect(sigc::bind<Gtk::Window*>(sigc::mem_fun(*this,
    &Application::on_window_hide), window));
      
  Glib::ustring input_uri;
  if(file) //If it's empty then this is a new empty file, as a result of an activation rather than an open.
  {
    input_uri = file->get_uri();
  }

  const bool test = window->init(input_uri, false /* TODO: group.m_arg_restore */); //Sets it up and shows it.
  if(!test) //The user could cancel the offer of a new or existing database.
  {
    window->hide(); //This will cause it to be deleted by on_window_hide.
  }
}

void Application::on_window_hide(Gtk::Window* window)
{
  delete window;
}

void Application::on_activate()
{
  //std::cout << "debug1: " << G_STRFUNC << std::endl;
  // The application has been started, so let's show a window:
  create_window();
}

void Application::on_open(const Gio::Application::type_vec_files& files,
  const Glib::ustring& hint)
{
  // The application has been asked to open some files,
  // so let's open a new window for each one.
  //std::cout << "debug: files.size()=" << files.size() << std::endl;
  for(guint i = 0; i < files.size(); i++)
  {
    Glib::RefPtr<Gio::File> file = files[i];
    if(!file)
    {
      std::cerr << G_STRFUNC << ": file is null." << std::endl;
    }
    else
      create_window(file);
  }

  Application::on_open(files, hint);
}

} //namespace Glom
