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
71 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <libglom/document/document.h>
#include <libglom/init.h>
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
  typedef typename T_Container::value_type::object_type type_item;
  typename T_Container::const_iterator iter =
    std::find_if(container.begin(), container.end(),
      Glom::predicate_FieldHasName<type_item>(name));
  return iter != container.end();
}

static Glom::sharedptr<const Glom::LayoutItem_Field> get_field_on_layout(const Glom::Document& document, const Glib::ustring& layout_table_name, const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  const Glom::Document::type_list_layout_groups groups = 
    document.get_data_layout_groups("details", layout_table_name);

  for(Glom::Document::type_list_layout_groups::const_iterator iter = groups.begin(); iter != groups.end(); ++iter)
  {
    const Glom::sharedptr<const Glom::LayoutGroup> group = *iter;
    if(!group)
      continue;
    
    const Glom::LayoutGroup::type_list_const_items items = group->get_items_recursive();
    for(Glom::LayoutGroup::type_list_const_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
    {
      const Glom::sharedptr<const Glom::LayoutItem> layout_item = *iter;
      const Glom::sharedptr<const Glom::LayoutItem_Field> layout_item_field =
        Glom::sharedptr<const Glom::LayoutItem_Field>::cast_dynamic(layout_item);
      if(!layout_item_field)
        continue;

      if( (layout_item_field->get_table_used(layout_table_name) == table_name) &&
        (layout_item_field->get_name() == field_name) )
      {
        return layout_item_field;
      }
    }
  }
  
  return Glom::sharedptr<const Glom::LayoutItem_Field>();
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
  const bool test = document.load(failure_code);
  //std::cout << "Document load result=" << test << std::endl;

  if(!test)
  {
    std::cerr << "Document::load() failed with failure_code=" << failure_code << std::endl;
    return EXIT_FAILURE;
  }

  //Test some known details:
  g_assert(document.get_is_example_file());
  g_assert(document.get_database_title() == "Openismus Film Manager");

  const std::vector<Glib::ustring> table_names = document.get_table_names();
  g_assert(contains(table_names, "accommodation"));
  g_assert(contains(table_names, "cars"));
  g_assert(contains(table_names, "characters"));
  g_assert(contains(table_names, "companies"));
  g_assert(contains(table_names, "contacts"));
  g_assert(contains(table_names, "locations"));
  g_assert(contains(table_names, "scenes"));
  g_assert(!contains(table_names, "Scenes")); //The title, not the name.

  //Test known details of one table:
  const Glom::Document::type_vec_fields fields = document.get_table_fields("scenes");
  g_assert(contains_named(fields, "scene_id"));
  g_assert(contains_named(fields, "comments"));
  g_assert(contains_named(fields, "description"));
  g_assert(contains_named(fields, "date"));
  g_assert(!contains_named(fields, "nosuchfield"));

  const Glom::Document::type_vec_relationships relationships = document.get_relationships("scenes");
  g_assert(contains_named(relationships, "location"));
  g_assert(contains_named(relationships, "scene_crew"));
  g_assert(contains_named(relationships, "scene_cast"));

  //Check some fields:
  Glom::sharedptr<const Glom::Field> field = document.get_field("contacts", "contact_id");
  g_assert(field);
  g_assert(field->get_glom_type() == Glom::Field::TYPE_NUMERIC);
  g_assert(field->get_auto_increment());
  field = document.get_field("locations", "rent");
  g_assert(field);
  g_assert(field->get_glom_type() == Glom::Field::TYPE_NUMERIC);
  g_assert(!field->get_auto_increment());
  g_assert(!field->get_unique_key());

  //Check a relationship:
  const Glom::sharedptr<const Glom::Relationship> relationship = document.get_relationship("characters", "contacts_actor");
  g_assert(relationship);
  g_assert(relationship->get_from_field() == "contact_id");
  g_assert(relationship->get_to_table() == "contacts");
  g_assert(relationship->get_to_field() == "contact_id");


  //Check that expected fields can be found on a layout.
  Glom::sharedptr<const Glom::LayoutItem_Field> field_on_layout = 
    get_field_on_layout(document, "scenes", "locations", "address_town");
  g_assert(field_on_layout);
  g_assert(field_on_layout->get_table_used("scenes") == "locations");
  g_assert(field_on_layout->get_name() == "address_town");

  
  //Check Field Formatting:
  field = document.get_field("contacts", "name_title");  
  g_assert(field);
  g_assert(field->get_glom_type() == Glom::Field::TYPE_TEXT);
  const Glom::FieldFormatting& formatting = field->m_default_formatting;
  g_assert(formatting.get_horizontal_alignment() == Glom::FieldFormatting::HORIZONTAL_ALIGNMENT_AUTO);
  
  g_assert(formatting.get_has_choices());
  g_assert(formatting.get_has_custom_choices());
  g_assert(!formatting.get_has_related_choices());
  Glom::FieldFormatting::type_list_values choices = formatting.get_choices_custom();
  g_assert(!choices.empty());
  g_assert(contains(choices, Gnome::Gda::Value("Mr")));
  g_assert(contains(choices, Gnome::Gda::Value("Mrs")));
  
  //Check that the default formatting is used on the layout:
  field_on_layout = 
    get_field_on_layout(document, "contacts", "contacts", "name_title");
  g_assert(field_on_layout);
  g_assert(field_on_layout->get_table_used("contacts") == "contacts");
  g_assert(field_on_layout->get_name() == "name_title");
  g_assert(field_on_layout->get_formatting_use_default());
  g_assert(field_on_layout->get_formatting_used() == formatting);


  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
