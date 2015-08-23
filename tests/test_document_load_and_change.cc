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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "tests/test_utils.h"
#include <libglom/document/document.h>
#include <libglom/init.h>
#include <giomm/file.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>

#include <iostream>

static bool field_is_on_a_layout(Glom::Document& document, const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  //Check that the field name is no longer used on a layout:
  for(const auto& layout_table_name : document.get_table_names())
  {
    for(const auto& group : document.get_data_layout_groups("details", layout_table_name))
    {
      if(group->has_field(layout_table_name, table_name, field_name))
      {
        //std::cerr << G_STRFUNC << ": Failure: The field is still used on a layout for table: " << layout_table_name << std::endl;
        return true;
      }
    }
  }
  
  return false;
}

static bool groups_contain_named(const Glom::Document::type_list_groups& container, const Glib::ustring& name)
{
  Glom::Document::type_list_groups::const_iterator iter =
    std::find_if(container.begin(), container.end(),
      [&name] (const Glom::GroupInfo& info)
      {
        return info.get_name() == name;
      }
    );
  return iter != container.end();
}

int main()
{
  Glom::libglom_init();

  // Get a URI for a test file:
  Glib::ustring uri;

  try
  {
    const std::string path =
      Glib::build_filename(GLOM_DOCDIR_EXAMPLES_NOTINSTALLED,
        "example_smallbusiness.glom");
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
  const auto test = document.load(failure_code);
  //std::cout << "Document load result=" << test << std::endl;

  if(!test)
  {
    std::cerr << G_STRFUNC << ": Document::load() failed with failure_code=" << failure_code << std::endl;
    return EXIT_FAILURE;
  }

  //Prevent these test changes from being saved back to the example file:
  document.set_allow_autosave(false);

  //Change a field name throughout the document:
  const Glib::ustring table_name = "products";
  const Glib::ustring field_name_original = "product_id";
  const Glib::ustring field_name_new = "newfieldname";
  document.change_field_name(table_name, field_name_original, field_name_new);

  //Check that the original field name is not known to the document:
  if(document.get_field(table_name, field_name_original))
  {
    std::cerr << G_STRFUNC << ": Failure: The document should have forgotten about the original field name." << std::endl;
    return false;
  }

  //Check that the new field name is known to the document:
  if(!(document.get_field(table_name, field_name_new)))
  {
    std::cerr << G_STRFUNC << ": Failure: The document does not know about the new field name." << std::endl;
    return false;
  }

  //Check that the original field name is no longer used in the relationship:
  std::shared_ptr<const Glom::Relationship> relationship = document.get_relationship("invoice_lines", "products");
  if(!relationship)
  {
    std::cerr << G_STRFUNC << ": Failure: The relationship could not be found in the document." << std::endl;
    return false;
  }

  if(relationship->get_to_field() == field_name_original)
  {
    std::cerr << G_STRFUNC << ": Failure: The relationship still uses the original field name." << std::endl;
    return false;
  }

  //Check that the original field name is no longer used on a layout:
  if(field_is_on_a_layout(document, table_name, field_name_original))
  {
    std::cerr << G_STRFUNC << ": Failure: The original field name is still used on a layout." << std::endl;
    return false;
  }

  {
    //Change a relationship name:
    const Glib::ustring table_name_invoices = "invoices";
    const Glib::ustring relationship_name_original = "contacts";
    const Glib::ustring relationship_name_new = "newrelationshipname";
    document.change_relationship_name(table_name_invoices, 
      relationship_name_original, relationship_name_new);
    if(document.get_relationship(table_name, relationship_name_original))
    {
      std::cerr << G_STRFUNC << ": Failure: The original relationship name still exists." << std::endl;
      return false;
    }

    if(!document.get_relationship(table_name, relationship_name_new))
    {
      std::cerr << G_STRFUNC << ": Failure: The new relationship name does not exist." << std::endl;
      return false;
    }

    //Check that the old relationship name is not used.
    std::shared_ptr<const Glom::LayoutItem_Field> field_on_layout = 
      get_field_on_layout(document, table_name, "contacts", "name_full");
    g_assert(field_on_layout);
    if(field_on_layout->get_relationship_name() != relationship_name_new)
    {
      std::cerr << G_STRFUNC << ": Failure: A layout item does not use the new relationship name as expected." << std::endl;
      return false;
    }
  }

  //Remove a field from the whole document:
  document.remove_field("publisher", "publisher_id");
  if(field_is_on_a_layout(document, "publisher", "publisher_id"))
  {
    std::cerr << G_STRFUNC << ": Failure: The removed field name is still used on a layout." << std::endl;
    return false;
  }
  
  //Remove a relationship:
  document.remove_relationship(relationship);
  relationship = document.get_relationship("invoice_lines", "products");
  if(relationship)
  {
    std::cerr << G_STRFUNC << ": Failure: The removed relationship still exists." << std::endl;
    return false;
  }
  
  //Change a table name:
  const Glib::ustring table_renamed = "invoiceslinesrenamed";
  document.change_table_name("invoice_lines", table_renamed);
  if(document.get_table("invoice_lines"))
  {
    std::cerr << G_STRFUNC << ": Failure: The renamed table still exists." << std::endl;
    return false;
  }
  
  relationship = document.get_relationship("invoices", "invoice_lines");
  if(!relationship)
  {
    std::cerr << G_STRFUNC << ": Failure: The expected relationship does not exist." << std::endl;
    return false;
  }

  if(relationship->get_to_table() != table_renamed)
  {
    std::cerr << G_STRFUNC << ": Failure: The relationship's to_table does have been renamed." << std::endl;
    return false;
  }
  
  document.remove_table("products");
  if(document.get_table("products"))
  {
    std::cerr << G_STRFUNC << ": Failure: The removed table still exists." << std::endl;
    return false;
  }

 
  //Remove a print layout:
  std::shared_ptr<const Glom::PrintLayout> print_layout = 
    document.get_print_layout("contacts", "contact_details");
  if(!print_layout)
  {
    std::cerr << G_STRFUNC << ": Failure: Could not get an expected print layout." << std::endl;
    return false;
  }
  
  document.remove_print_layout("contacts", "contact_details");
  print_layout = 
    document.get_print_layout("contacts", "contact_details");
  if(print_layout)
  {
    std::cerr << G_STRFUNC << ": Failure: The removed print layotu still exists." << std::endl;
    return false;
  }
  
  //Test user groups:
  Glom::Document::type_list_groups groups = document.get_groups();
  g_assert(groups_contain_named(groups, "glom_developer"));
  
  const Glib::ustring group_name = "accounts";
  g_assert(groups_contain_named(groups, group_name));
  document.remove_group(group_name);
  groups = document.get_groups();
  g_assert(!groups_contain_named(groups, group_name));
  
  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
