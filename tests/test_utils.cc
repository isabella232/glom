/* Glom
 *
 * Copyright (C) 2011 Murray Cumming
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

#include "tests/test_utils.h"
#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <iostream>

std::shared_ptr<const Glom::LayoutItem_Field> get_field_on_layout(const std::shared_ptr<Glom::Document>& document, const Glib::ustring& layout_table_name, const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  for(const auto& group : document->get_data_layout_groups("details", layout_table_name))
  {
    if(!group)
      continue;
    
    for(const auto& layout_item : group->get_items_recursive())
    {
      const auto layout_item_field =
        std::dynamic_pointer_cast<const Glom::LayoutItem_Field>(layout_item);
      if(!layout_item_field)
        continue;

      if( (layout_item_field->get_table_used(layout_table_name) == table_name) &&
        (layout_item_field->get_name() == field_name) )
      {
        return layout_item_field;
      }
    }
  }
  
  return std::shared_ptr<const Glom::LayoutItem_Field>();
}


Gnome::Gda::Value get_value_for_image_from_file(const std::string& filename)
{
  std::string data;
  try
  {
    data = Glib::file_get_contents(filename);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Failed: file_get_contents() failed: " << ex.what() << std::endl;
    return Gnome::Gda::Value(); //Something went wrong. It does not exist.
  }

  if(data.empty())
  {
    std::cerr << G_STRFUNC << ": Failed: The data read from the file was empty. filepath=" << filename << std::endl;
    return Gnome::Gda::Value();
  }

  //Set the value:
  return Gnome::Gda::Value((const guchar*)data.c_str(), data.size());
}

Gnome::Gda::Value get_value_for_image()
{
  //Fill a value from a file:
  const std::string filename =
    Glib::build_filename(GLOM_TESTS_IMAGE_DATA_NOTINSTALLED, "test_image.jpg");
  return get_value_for_image_from_file(filename);
}
