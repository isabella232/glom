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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_GLADE_UTILS_H
#define GLOM_GLADE_UTILS_H

#include <iostream> // For std::cerr
#include <gtkmm/builder.h>
#include <gtkmm/window.h>
#include <giomm/file.h>
#include <glibmm/miscutils.h>
#include <glibmm/markup.h>
#include <glibmm/fileutils.h>

namespace Glom
{

namespace Utils
{

inline std::string get_glade_resource_path(const std::string& filename)
{
  // This is the same prefix that is in the *gresource.xml.in file.
  return "/org/gnome/glom/data/ui/" + filename;
}

/** This assumes that there are no other top-level windows in the glade file.
 * This allows us to get all object, including any necessary secondary objects,
 * such as a GtkSpinButton's GtkAdjustment.
 */
template<class T_Widget>
void helper_get_glade_widget_derived_with_warning(const std::string& filename, const Glib::ustring& id, T_Widget*& widget, bool load_all)
{
  Glib::RefPtr<Gtk::Builder> refXml;

  try
  {
    const std::string filepath = Utils::get_glade_resource_path(filename);
    if(load_all)
      refXml = Gtk::Builder::create_from_resource(filepath);
     else
      refXml = Gtk::Builder::create_from_resource(filepath, id);
  }
  catch(const Gtk::BuilderError& ex)
  {
    std::cerr << G_STRFUNC << ": BuilderError Exception: " << ex.what() << std::endl;
  }
  catch(const Glib::MarkupError& ex)
  {
    std::cerr << G_STRFUNC << ": MarkupError exception:" << ex.what() << std::endl;
  }
  catch(const Glib::FileError& ex)
  {
    std::cerr << G_STRFUNC << ": FileError: exception" << ex.what() << std::endl;
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception of unexpected type: " << ex.what() << std::endl;
  }

  if(refXml)
  {
    refXml->get_widget_derived(id, widget);
  }
}

/** This should be used with classes that have a static glade_id member.
 * This loads only the widget's own object, as specified by its ID.
 */
template<class T_Widget>
void get_glade_child_widget_derived_with_warning(T_Widget*& widget)
{
  widget = 0;

  // Check the path to the installed .glade file:
  // The id is the same as the filename, in a developer/operator sub-directory:
  // TODO: Should we use build_filename()?
  const std::string filename = Glib::build_filename(
      (T_Widget::glade_developer ? "developer" : "operator"),
      std::string(T_Widget::glade_id) + ".glade"); 
  
  helper_get_glade_widget_derived_with_warning(filename, T_Widget::glade_id, widget, false /* load just this */);
}

/** This should be used with classes that have a static glade_id member.
 * This assumes that there are no other top-level windows in the glade file.
 * This allows us to get all object, including any necessary secondary objects,
 * such as a GtkSpinButton's GtkAdjustment.
 */
template<class T_Widget>
void get_glade_widget_derived_with_warning(T_Widget*& widget)
{
  widget = 0;

  // Check the path to the installed .glade file:
  // The id is the same as the filename, in a developer/operator sub-directory:
  const std::string filename = Glib::build_filename(
      (T_Widget::glade_developer ? "developer" : "operator"),
      std::string(T_Widget::glade_id) + ".glade"); 
  
  helper_get_glade_widget_derived_with_warning(filename, T_Widget::glade_id, widget, true /* load_all */);
}

template<class T_Widget>
void get_glade_widget_with_warning(const std::string& filename, const Glib::ustring& id, T_Widget*& widget)
{
  Glib::RefPtr<Gtk::Builder> refXml;

  try
  {
    refXml = Gtk::Builder::create_from_resource(Utils::get_glade_resource_path(filename), id);
  }
  catch(const Gtk::BuilderError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what() << std::endl;
  }
  catch(const Glib::MarkupError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what() << std::endl;
  }
  catch(const Glib::FileError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what() << std::endl;
  }
  catch(const Gio::ResourceError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what() << std::endl;
  }

  if(refXml)
  {
    refXml->get_widget(id, widget);
  }

  // Make sure that all windows have the Glom icon.
  // TODO: Though shouldn't all transient windows have this by default,
  // or should they even be visible in the task list? murrayc
  Gtk::Window* window = dynamic_cast<Gtk::Window*>(widget);
  if(window)
    window->set_icon_name("glom");
}

} //namespace Utils

} //namespace Glom

#endif //GLOM_GLADE_UTILS_H
