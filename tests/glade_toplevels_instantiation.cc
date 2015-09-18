/* main.cc
 *
 * Copyright (C) 2010 The Glom development team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <gtkmm/builder.h>
#include <gtkmm/dialog.h>
#include <gtkmm/application.h>
#include <gtksourceviewmm/init.h>
#include <libxml++/libxml++.h>

#include <iostream>

static bool attempt_instantiation(const std::string& filepath, const xmlpp::Element* child)
{
  const auto id = child->get_attribute_value("id");
  const auto gclassname = child->get_attribute_value("class");

  // Try to instantiate the object:
  Glib::RefPtr<Gtk::Builder> builder;
  try
  {
    builder = Gtk::Builder::create_from_file(filepath, id);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception from Gtk::Builder::create_from_file() with id=" << id << " from file " << filepath << std::endl;
    std::cerr << G_STRFUNC << ":   Error: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  // Try to get the widget, checking that it has the correct type:
  Gtk::Widget* widget = nullptr;
  if(gclassname == "GtkWindow")
  {
    Gtk::Window* window = nullptr;
    builder->get_widget(id, window);
    widget = window;
  }
  else if(gclassname == "GtkDialog")
  {
    Gtk::Dialog* dialog = nullptr;
    builder->get_widget(id, dialog);
    widget = dialog;
  }
  else
  {
    //We try to avoid using non-window top-level widgets in Glom.
    std::cerr << G_STRFUNC << ": Non-window top-level object in Glade file (unexpected by Glom): id=" << id << " from file " << filepath << std::endl;

    //But let's try this anyway:
    Glib::RefPtr<Glib::Object> object = builder->get_object(id);

    return false;
  }

  if(!widget)
  {
    std::cerr << G_STRFUNC << ": Failed to instantiate object with id=" << id << " from file " << filepath << std::endl;
    return false;
  }

  //Check that it is not visible by default,
  //because applications generally want to separate instantiation from showing.
  if(widget->get_visible())
  {
     std::cerr << G_STRFUNC << ": Top-level window is visible by default (unwanted by Glom): id=" << id << " from file " << filepath << std::endl;
     return false;
  }

  //std::cout << "Successful instantiation of object with id=" << id << " from file " << filepath << std::endl;

  delete widget;
  return true;
}

int main(int argc, char* argv[])
{
  Glib::RefPtr<Gtk::Application> app = 
    Gtk::Application::create(argc, argv, "org.glom.test_glade_toplevels_instantiation");
  Gsv::init(); //Our .glade files contain gtksourceview widgets too.

  std::string filepath;
  if(argc > 1 )
    filepath = argv[1]; //Allow the user to specify a different XML file to parse.
  else
  {
    std::cerr << G_STRFUNC << ": Usage: glade_toplevels_instantiation filepath" << std::endl;
    return EXIT_FAILURE;
  }

  #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  try
  {
  #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
    xmlpp::DomParser parser;
    //parser.set_validate();
    parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
    parser.parse_file(filepath);
    if(!parser)
      return EXIT_FAILURE;

    const auto root = parser.get_document()->get_root_node(); //deleted by DomParser.
    if(!root)
      return EXIT_FAILURE;

    for(const auto& item : root->get_children("object"))
    {
       const auto child = dynamic_cast<const xmlpp::Element*>(item);

       //Try to instante the object with Gtk::Builder:
       if(child && !attempt_instantiation(filepath, child))
         return EXIT_FAILURE;
    }

  #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
  }
  catch(const std::exception& ex)
  {
    std::cout << "Exception caught: " << ex.what() << std::endl;
  }
  #endif //LIBXMLCPP_EXCEPTIONS_ENABLED

  return EXIT_SUCCESS;
}
