/* Glom
 *
 * Copyright (C) 2010 Openismus GmbH
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
71 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "tests/test_utils.h"
#include <libglom/document/document.h>
#include <libglom/init.h>
#include <libglom/db_utils.h>
#include <giomm/file.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>
#include <gdkmm/wrap_init.h> //TODO: Add a Gdk::init() to gtkmm.
#include <gdkmm/pixbufloader.h>

#include <iostream>
 
int main()
{
  Glom::libglom_init();

  // Get a URI for a test file:
  Glib::ustring uri;

  try
  {
    const std::string path =
       Glib::build_filename(GLOM_DOCDIR_EXAMPLES_NOTINSTALLED,
   "example_project_manager.glom");
    uri = Glib::filename_to_uri(path);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what();
    return EXIT_FAILURE;
  }

  //std::cout << "URI=" << uri << std::endl;


  // Load the document:
  Glom::Document document;
  document.set_file_uri(uri);
  int failure_code = 0;
  const bool test = document.load(failure_code);
  //std::cout << "Document load result=" << test << std::endl;

  if(!test)
  {
    std::cerr << "Document::load() failed with failure_code=" << failure_code << std::endl;
    return EXIT_FAILURE;
  }

  //Test some known details:
  g_assert(document.get_is_example_file());
  g_assert(document.get_database_title_original() == "Project Manager Example");

  //Check a layout:
  const Glom::Document::type_list_layout_groups groups = 
    document.get_data_layout_groups("details", "projects");
  g_assert(groups.size() == 3);
  const Glom::sharedptr<const Glom::LayoutGroup> group =
    groups[0];
  g_assert(group);
  g_assert(group->get_name() == "overview");

  const Glom::LayoutGroup::type_list_const_items items = 
    group->get_items();
  //std::cout << "size: " << items.size() << std::endl;
  g_assert(items.size() == 3);
  Glom::sharedptr<const Glom::LayoutItem> item = items[2];
  g_assert(item);
  Glom::sharedptr<const Glom::LayoutItem_Image> image_item =
    Glom::sharedptr<const Glom::LayoutItem_Image>::cast_dynamic(item);
  g_assert(image_item);

  const Gnome::Gda::Value value = image_item->get_image();
  g_assert(!value.is_null());
  g_assert(value.get_value_type() == GDA_TYPE_BINARY);
  long data_length = 0;
  const guchar* data = value.get_binary(data_length);
  g_assert(data);
  g_assert(data_length);

  //Check that it can be interpreted as an image:
  //Luckily, the use of GdkPixbufLoader here does not seem to require an X display.
  Gdk::wrap_init();
  Glib::RefPtr<Gdk::PixbufLoader> refPixbufLoader;      
  try
  {
    refPixbufLoader = Gdk::PixbufLoader::create();
  }
  catch(const Gdk::PixbufError& ex)
  {
    std::cerr << "PixbufLoader::create failed: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  Glib::RefPtr<Gdk::Pixbuf> pixbuf;
  guint8* puiData = (guint8*)data;
  try
  {
    refPixbufLoader->write(puiData, static_cast<gsize>(data_length));
    pixbuf = refPixbufLoader->get_pixbuf();
    refPixbufLoader->close(); //This throws if write() threw, so it must be inside the try block.
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << "PixbufLoader::write() failed: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
