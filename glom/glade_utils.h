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

#ifndef GLOM_GLADE_UTILS_H
#define GLOM_GLADE_UTILS_H

#include <iostream> // For std::cerr
#include <gtkmm/builder.h>
#include <glom/dialog_progress_creating.h>

namespace Glom
{

namespace Utils
{

inline std::string get_glade_file_path(const std::string& filename)
{
#ifdef G_OS_WIN32
  gchar* directory = g_win32_get_package_installation_directory_of_module(NULL);
  const std::string result = Glib::build_filename(directory, Glib::build_filename("share/glom/glade", filename));
  g_free(directory);
  return result;
#else
  return Glib::build_filename(GLOM_PKGDATADIR G_DIR_SEPARATOR_S "glade", filename);
#endif
}

template<class T_Widget>
void get_glade_widget_derived_with_warning(const Glib::ustring& id, T_Widget*& widget)
{
  Glib::RefPtr<Gtk::Builder> refXml;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    refXml = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom.glade"), id);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
#else
  std::auto_ptr<Glib::Error> error;
  refXml = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom.glade"), id, error);
  if (error.get())
  {
    std::cerr << error->what() << std::endl;
  }
#endif

  if(refXml)
  {
    refXml->get_widget_derived(id, widget);
  }
}

template<class T_Widget>
void get_glade_developer_widget_derived_with_warning(const Glib::ustring& id, T_Widget*& widget)
{
  Glib::RefPtr<Gtk::Builder> refXml;

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    refXml = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom_developer.glade"), id);
  }
  catch(const Gtk::BuilderError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
#else
  std::auto_ptr<Glib::Error> error;
  refXml = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom_developer.glade"), id, error);
  if (error.get())
  {
    std::cerr << error->what() << std::endl;
  }
#endif

  if(refXml)
  {
    refXml->get_widget_derived(id, widget);
  }
}

template<class T_Widget>
void get_glade_widget_with_warning(const Glib::ustring& id, T_Widget*& widget)
{
  Glib::RefPtr<Gtk::Builder> refXml;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    refXml = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom.glade"), id);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
#else
  std::auto_ptr<Glib::Error> error;
  refXml = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom.glade"), id, error);
  if (error.get())
  {
    std::cerr << error->what() << std::endl;
  }
#endif

  if(refXml)
  {
    refXml->get_widget(id, widget);
  }
}

Dialog_ProgressCreating* get_and_show_pulse_dialog(const Glib::ustring& message, Gtk::Window* parent_window);

} //namespace Utils

} //namespace Glom

#endif //GLOM_GLADE_UTILS_H

