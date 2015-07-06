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
#include <libglom/db_utils.h>
#include <giomm/file.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>

#include <iostream>

template<typename T_Container, typename T_Value>
bool contains(const T_Container& container, const T_Value& name)
{
  typename T_Container::const_iterator iter =
    std::find(container.begin(), container.end(), name);
  return iter != container.end();
}

template<typename T_Container>
bool contains_named(const T_Container& container, const Glib::ustring& name)
{
  typedef typename T_Container::value_type::element_type type_item;
  typename T_Container::const_iterator iter =
    std::find_if(container.begin(), container.end(),
      Glom::predicate_FieldHasName<type_item>(name));
  return iter != container.end();
}

template<typename T_Container>
bool contains_value(const T_Container& container, const Glib::ustring& name)
{
  for(const auto& item : container)
  {
    if(item->get_value() == Gnome::Gda::Value(name))
      return true;
  }

  return false;
}


static bool get_group_named(const Glom::Document::type_list_groups& container, const Glib::ustring& name, Glom::GroupInfo& group_info)
{
  Glom::Document::type_list_groups::const_iterator iter =
    std::find_if(container.begin(), container.end(),
      Glom::predicate_FieldHasName<Glom::GroupInfo>(name));
  if(iter != container.end())
  {
    group_info = *iter;
    return true;
  }
  
  group_info = Glom::GroupInfo();
  return false;
}

static bool needs_navigation(Glom::Document& document, const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  std::shared_ptr<Glom::LayoutItem_Field> layout_item = std::make_shared<Glom::LayoutItem_Field>();
  layout_item->set_name(field_name);
  layout_item->set_full_field_details(
    document.get_field(table_name, field_name));

  std::shared_ptr<Glom::Relationship> field_used_in_relationship_to_one;
  return Glom::DbUtils::layout_field_should_have_navigation(table_name, 
    layout_item, &document, field_used_in_relationship_to_one);
}

static std::shared_ptr<const Glom::LayoutItem_Portal> get_portal_from_details_layout(const Glom::Document& document, const Glib::ustring& table_name, const Glib::ustring& relationship_name)
{
  const Glom::Document::type_list_layout_groups groups = 
    document.get_data_layout_groups("details", table_name);
  if(groups.empty())
  {
    std::cerr << G_STRFUNC << ": groups is empty." << std::endl;
  }
  
  for(const auto& group : groups)
  {
    const Glom::LayoutGroup::type_list_const_items items = 
      group->get_items_recursive_with_groups();
    for(const auto& layout_item : items)
    { 
      const std::shared_ptr<const Glom::LayoutGroup> child_group =
        std::dynamic_pointer_cast<const Glom::LayoutGroup>(layout_item);
      if(!child_group)
        continue;

      const std::shared_ptr<const Glom::LayoutItem_Portal> portal =
        std::dynamic_pointer_cast<const Glom::LayoutItem_Portal>(layout_item);
      if(!portal)
        continue;

      if(portal->get_relationship_name() == relationship_name)
        return portal;
    }
  }
      
  return std::shared_ptr<Glom::LayoutItem_Portal>();
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
         "example_film_manager.glom");
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

  //Test some known details:
  g_assert(document.get_is_example_file());
  g_assert(document.get_database_title_original() == "Openismus Film Manager");

  const auto table_names = document.get_table_names();
  g_assert(contains(table_names, "accommodation"));
  g_assert(contains(table_names, "cars"));
  g_assert(contains(table_names, "characters"));
  g_assert(contains(table_names, "companies"));
  g_assert(contains(table_names, "contacts"));
  g_assert(contains(table_names, "locations"));
  g_assert(contains(table_names, "scenes"));
  g_assert(!contains(table_names, "Scenes")); //The title, not the name.

  std::shared_ptr<Glom::TableInfo> table = document.get_table("scenes");
  g_assert(table);
  g_assert( table->get_title_original() == "Scenes" );
  g_assert( table->get_title_singular_original() == "Scene" );

  //Test known fields of one table:
  const auto fields = document.get_table_fields("scenes");
  g_assert(contains_named(fields, "scene_id"));
  g_assert(contains_named(fields, "comments"));
  g_assert(contains_named(fields, "description"));
  g_assert(contains_named(fields, "date"));
  g_assert(!contains_named(fields, "nosuchfield"));

  const auto relationships = document.get_relationships("scenes");
  g_assert(contains_named(relationships, "location"));
  g_assert(contains_named(relationships, "scene_crew"));
  g_assert(contains_named(relationships, "scene_cast"));

  //Check some fields:
  std::shared_ptr<const Glom::Field> field = document.get_field("contacts", "contact_id");
  g_assert(field);
  g_assert( field->get_title_original() == "Contact ID" );
  g_assert(field->get_glom_type() == Glom::Field::TYPE_NUMERIC);
  g_assert(field->get_auto_increment());
  field = document.get_field("locations", "rent");
  g_assert(field);
  g_assert( field->get_title_original() == "Rent" );
  g_assert(field->get_glom_type() == Glom::Field::TYPE_NUMERIC);
  g_assert(!field->get_auto_increment());
  g_assert(!field->get_unique_key());

  //Check a relationship:
  const std::shared_ptr<const Glom::Relationship> relationship = document.get_relationship("characters", "contacts_actor");
  g_assert(relationship);
  g_assert(relationship->get_from_field() == "contact_id");
  g_assert(relationship->get_to_table() == "contacts");
  g_assert(relationship->get_to_field() == "contact_id");


  //Check a layout:
  const Glom::Document::type_list_layout_groups groups = 
    document.get_data_layout_groups("details", "scenes");
  g_assert(groups.size() == 3);
  const std::shared_ptr<const Glom::LayoutGroup> group =
    groups[1];
  const Glom::LayoutGroup::type_list_const_items items = 
    group->get_items_recursive();
  //std::cout << "size: " << items.size() << std::endl;
  g_assert(items.size() == 13);
  const Glom::LayoutGroup::type_list_const_items items_with_groups = 
    group->get_items_recursive_with_groups();
  //std::cout << "size: " << items_with_groups.size() << std::endl;
  g_assert(items_with_groups.size() == 15);

  //Check that expected fields can be found on a layout.
  std::shared_ptr<const Glom::LayoutItem_Field> field_on_layout = 
    get_field_on_layout(document, "scenes", "locations", "address_town");
  g_assert(field_on_layout);
  g_assert(field_on_layout->get_table_used("scenes") == "locations");
  g_assert(field_on_layout->get_name() == "address_town");

  
  //Check Field Formatting:
  field = document.get_field("contacts", "name_title");  
  g_assert(field);
  g_assert(field->get_glom_type() == Glom::Field::TYPE_TEXT);
  const Glom::Formatting& formatting = field->m_default_formatting;
  g_assert(formatting.get_horizontal_alignment() == Glom::Formatting::HORIZONTAL_ALIGNMENT_AUTO);
  
  g_assert(formatting.get_has_choices());
  g_assert(formatting.get_has_custom_choices());
  g_assert(!formatting.get_has_related_choices());
  Glom::Formatting::type_list_values choices = formatting.get_choices_custom();
  g_assert(!choices.empty());
  g_assert(contains_value(choices, "Mr"));
  g_assert(contains_value(choices, "Mrs"));
  
  //Check that the default formatting is used on the layout:
  field_on_layout = 
    get_field_on_layout(document, "contacts", "contacts", "name_title");
  g_assert(field_on_layout);
  g_assert(field_on_layout->get_table_used("contacts") == "contacts");
  g_assert(field_on_layout->get_name() == "name_title");
  g_assert(field_on_layout->get_formatting_use_default());
  g_assert(field_on_layout->get_formatting_used() == formatting);

  //Test this utility method:
  g_assert( document.get_data_layout_groups_have_any_fields("list", "cars") );


  //Test library modules:
  const auto module_names = document.get_library_module_names();
  if(!module_names.empty()) //TODO: Test a document that actually has some?
  {
    std::cerr << G_STRFUNC << ": Failure: Unexpected library module names." << std::endl;
    return false;
  }


  //Test print layouts:  
  const std::vector<Glib::ustring> print_layout_names = 
    document.get_print_layout_names("contacts");
  if(print_layout_names.size() != 1)
  {
    std::cerr << G_STRFUNC << ": Failure: Unexpected number of print layouts." << std::endl;
    return false;
  }

  if(!contains(print_layout_names, "contact_details"))
  {
    std::cerr << G_STRFUNC << ": Failure: Could not find the expected print layout name." << std::endl;
    return false;
  }
  
  const std::shared_ptr<const Glom::PrintLayout> print_layout = document.get_print_layout("contacts", "contact_details");
  if(!print_layout)
  {
    std::cerr << G_STRFUNC << ": Failure: Could not get an expected print layout." << std::endl;
    return false;
  }
  
  if(print_layout->get_title_original() != "Contact Details")
  {
    std::cerr << G_STRFUNC << ": Failure: Unexpected print layout title." << std::endl;
    return false;
  }
  
  if(!print_layout->get_layout_group())
  {
    std::cerr << G_STRFUNC << ": Failure: The print layout has no layout group." << std::endl;
    return false;
  }


  const std::vector<Glib::ustring> report_names = 
    document.get_report_names("contacts");
  if(report_names.size() != 2)
  {
    std::cerr << G_STRFUNC << ": Failure: Unexpected number of reports." << std::endl;
    return false;
  }

  if(!contains(report_names, "by_country"))
  {
    std::cerr << G_STRFUNC << ": Failure: Could not find the expected report name." << std::endl;
    return false;
  }

  const std::shared_ptr<const Glom::Report> report = document.get_report("contacts", "by_country_by_town");
  if(!report)
  {
    std::cerr << G_STRFUNC << ": Failure: Could not get an expected report." << std::endl;
    return false;
  }
  
  if(report->get_title_original() != "By Country, By Town")
  {
    std::cerr << G_STRFUNC << ": Failure: Unexpected report title." << std::endl;
    return false;
  }
  
  if(!report->get_layout_group())
  {
    std::cerr << G_STRFUNC << ": Failure: The report has no layout group." << std::endl;
    return false;
  }

  
  //Test user groups:
  Glom::Document::type_list_groups user_groups = document.get_groups();
  Glom::GroupInfo group_info_ignored;
  g_assert(get_group_named(user_groups, "glom_developer", group_info_ignored));

  Glom::GroupInfo group_info_accounts;
  g_assert(get_group_named(user_groups, "props_department", group_info_accounts));
  Glom::GroupInfo::type_map_table_privileges::const_iterator iterFind =
    group_info_accounts.m_map_privileges.find("scenes");
  const bool privileges_found = (iterFind != group_info_accounts.m_map_privileges.end());
  g_assert(privileges_found);
  const Glom::Privileges privs = iterFind->second;
  g_assert(privs.m_view == true);
  g_assert(privs.m_edit == true);
  g_assert(privs.m_create == false);
  g_assert(privs.m_delete == false);

  //Test navigation:
  if(!needs_navigation(document, "scenes", "location_id"))
  {
    std::cerr << G_STRFUNC << ": Failure: DbUtils::layout_field_should_have_navigation() did not return the expected result." << std::endl;
    return false;
  }

  if(needs_navigation(document, "scenes", "description"))
  {
    std::cerr << G_STRFUNC << ": Failure: DbUtils::layout_field_should_have_navigation() did not return the expected result." << std::endl;
    return false;
  }


  //Test portal navigation.
  //Note that related records portals don't have names.
  //This example portal shows the scenes_cast table, but should navigate though that to the cast table.
  const Glib::ustring portal_relationship_name = "scene_cast";
  std::shared_ptr<const Glom::LayoutItem_Portal> portal =
    get_portal_from_details_layout(document, "scenes", portal_relationship_name);
  if(!portal)
  {
    std::cerr << G_STRFUNC << ": Failure: Could not get the portal from the layout." << std::endl;
    return false;
  }

  Glib::ustring navigation_table_name;
  std::shared_ptr<const Glom::UsesRelationship> navigation_relationship;
  portal->get_suitable_table_to_view_details(navigation_table_name, navigation_relationship, &document);

  if(navigation_table_name != "characters")
  {
    std::cerr << G_STRFUNC << ": Failure: get_suitable_table_to_view_details() returned an unexpected table name: " << navigation_table_name << std::endl;
    return false;
  }

  if(!navigation_relationship)
  {
    std::cerr << G_STRFUNC << ": Failure: get_suitable_table_to_view_details() returned an empty navigation_relationship." << std::endl;
    return false;
  }

  if(navigation_relationship->get_relationship_name() != "cast")
  {
    std::cerr << G_STRFUNC << ": Failure: get_suitable_table_to_view_details() returned an unexpected navigation_relationship name: " << navigation_relationship->get_relationship_name() << std::endl;
    return false;
  }

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
