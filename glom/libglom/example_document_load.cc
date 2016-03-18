/* Glom
 *
 * Copyright (C) 2009 Openismus GmbH
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

// Compile this like so:
// g++ test_document.cc `pkg-config glom-1.0 --libs --cflags`

#include <libglom/document/document.h>
#include <libglom/init.h>
#include <glibmm/convert.h>

#include <iostream>

void print_layout_group(const std::shared_ptr<Glom::LayoutGroup>& layout_group, const Glib::ustring& indent)
{
  if(!layout_group)
    return;

  //Look at each child item:
  for(const auto& layout_item : layout_group->get_items())
  {
    if(!layout_item)
      continue;

    std::cout << indent << "Layout Item: title=" << layout_item->get_title_or_name(Glib::ustring() /* locale */)
      << ", item type=" << layout_item->get_part_type_name();

    const auto display_name = layout_item->get_layout_display_name();
    if(!display_name.empty())
      std::cout << " (" << layout_item->get_layout_display_name() << ")";

    std::cout << std::endl;

    //Recurse into child groups:
    auto group = std::dynamic_pointer_cast<Glom::LayoutGroup>(layout_item);
    if(group)
    {
      print_layout_group(group, indent + "  ");
    }
  }
}

void print_layout(const Glom::Document::type_list_layout_groups& layout_groups)
{
  for(const auto& layout_group : layout_groups)
  {
    if(!layout_group)
      continue;

    std::cout << "    Layout Group: title=" << layout_group->get_title_or_name(Glib::ustring() /* locale */) << std::endl;
    print_layout_group(layout_group, "      ");
  }
}

int main()
{
  Glom::libglom_init();


  // Get a URI for a test file:
  Glib::ustring uri;

  try
  {
    uri = Glib::filename_to_uri("/usr/share/glom/doc/examples/example_music_collection.glom");
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what();
    return 1;
  }

  std::cout << "URI=" << uri << std::endl;


  // Load the document:
  auto document = std::make_shared<Glom::Document>();
  document->set_file_uri(uri);
  int failure_code = 0;
  const auto test = document->load(failure_code);
  std::cout << "Document load result=" << test << std::endl;

  if(!test)
    return 1;

  std::cout << "Database Title: " << document->get_database_title_original() << std::endl;
  std::cout << "Default Table: " << document->get_default_table() << std::endl;


  // Look at each table:
  for(const auto& table_name : document->get_table_names())
  {
    std::cout << "Table: " << table_name << std::endl;

    // List the fields for this table:
    for(const auto& field : document->get_table_fields(table_name))
    {
       if(!field)
         continue;

       const auto field_type = field->get_glom_type();

       std::cout << "  Field: name=" << field->get_name()
         << ", title=" << field->get_title_or_name(Glib::ustring() /* locale */)
         << ", type=" << Glom::Field::get_type_name_ui(field_type) << std::endl;

    }

    // List the relationships for this table:
    for(const auto& relationship : document->get_relationships(table_name))
    {
       if(!relationship)
         continue;

       std::cout << "  Relationship: from field=" << relationship->get_from_field()
         << ", to table=" << relationship->get_to_table()
         << ", to field=" << relationship->get_to_field()  << std::endl;
    }

    //Show the layouts for this table:
    const auto layout_list =
      document->get_data_layout_groups("list", table_name);
    std::cout << "  Layout: List:\n";
    print_layout(layout_list);

    const auto layout_details =
      document->get_data_layout_groups("details", table_name);
    std::cout << "  Layout: Details:\n";
    print_layout(layout_details);
  }

  Glom::libglom_deinit();


  return 0;
}
