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

#include "config.h"

#include <libglom/document/document.h>
#include <libglom/init.h>
#include <libglom/db_utils.h>
#include <giomm/file.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>
#include <glibmm/i18n.h>

#include <iostream>

template<typename T_Container, typename T_Value>
bool contains(const T_Container& container, const T_Value& name)
{
  typename T_Container::const_iterator iter =
    std::find(container.begin(), container.end(), name);
  return iter != container.end();
}

template<typename T_Container>
typename T_Container::value_type get_titled(const T_Container& container, const Glib::ustring& title)
{
  typedef typename T_Container::value_type type_sharedptr;
  type_sharedptr result;

  typename T_Container::const_iterator iter =
    std::find_if(container.begin(), container.end(),
      [&title] (const typename T_Container::value_type& element)
      {
        return (element && element->get_title_original() == title);
      }
    );
  if(iter != container.end())
    result = *iter;

  return result;
}

template<typename T_TypeToFind>
bool contains_item_type(const Glom::Document::type_list_translatables& container)
{
  Glom::Document::type_list_translatables::const_iterator iter =
    std::find_if(container.begin(), container.end(),
      [] (const Glom::Document::type_list_translatables::value_type& element)
      {
        std::shared_ptr<Glom::TranslatableItem> item = element.first;
        std::shared_ptr<T_TypeToFind> derived = std::dynamic_pointer_cast<T_TypeToFind>(item);
        if(derived)
          return true;
        else
          return false;
      }
    );
  if(iter != container.end())
    return true;

  return false;
}

static std::shared_ptr<const Glom::LayoutItem_Field> get_field_on_layout(const Glom::Document& document, const Glib::ustring& layout_table_name, const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  for(const auto& group : document.get_data_layout_groups("details", layout_table_name))
  {
    if(!group)
      continue;
    
    for(const auto& layout_item : group->get_items_recursive())
    {
      const std::shared_ptr<const Glom::LayoutItem_Field> layout_item_field =
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

static const char* locale_de = "de_DE.UTF-8";

template<typename T_Item>
void check_title(const T_Item& item, const char* title_en, const char* title_de)
{
  g_assert(item);

  //The get_title_original() and get_title_translation() should not be called 
  //on items that delegate to a child item:
  bool has_own_title = true;
  std::shared_ptr<const Glom::LayoutItem_Field> field = 
    std::dynamic_pointer_cast<const Glom::LayoutItem_Field>(item);
  if(field)
     has_own_title = false;

  if(has_own_title)
    g_assert( item->get_title_original() == title_en );

  g_assert( item->get_title(Glib::ustring()) == title_en );
  g_assert( item->get_title("en_US") == title_en );

  if(has_own_title)
    g_assert( item->get_title_translation(locale_de) == title_de );
    
  g_assert( item->get_title(locale_de) == title_de );

  g_assert( item->get_title_or_name(Glib::ustring()) == title_en );
  g_assert( item->get_title_or_name("en_US") == title_en );
  g_assert( item->get_title_or_name(locale_de) == title_de );

  if(has_own_title)
  {
    //Check fallbacks:
    g_assert( item->get_title_translation(Glib::ustring()) == title_en );
    g_assert( item->get_title_translation("en_US") == title_en );
    g_assert( item->get_title_translation("en_GB") == title_en );
    g_assert( item->get_title_translation("de_AU") == title_de );

    //Check that fallbacks do not happen when we don't want them:
    g_assert( item->get_title_translation(Glib::ustring(), false) == Glib::ustring() );
    g_assert( item->get_title_translation("en_US", false) == Glib::ustring() );
    g_assert( item->get_title_translation("de_AU", false) == Glib::ustring() );
  }
}

int main()
{
  //Make this application use the current locale for _() translation:
  bindtextdomain(GETTEXT_PACKAGE, GLOM_LOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

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

  const auto locales = document.get_translation_available_locales();
  g_assert(locales.size() == 16);
  g_assert(contains(locales, "de"));

  const auto table_names = document.get_table_names();
  g_assert(contains(table_names, "scenes"));

  g_assert( document.get_table_title_original("scenes") == "Scenes" );
  g_assert( document.get_table_title_singular_original("scenes") == "Scene" );
  
  g_assert( document.get_table_title("scenes", locale_de) == "Szenen" );
  g_assert( document.get_table_title_singular("scenes", locale_de) == "Szene" );

  //Check a field:
  std::shared_ptr<const Glom::Field> field = document.get_field("contacts", "contact_id");
  g_assert(field);
  check_title(field, "Contact ID", "Kontaktkennung");

  //Check a field and its custom choices:
  field = document.get_field("scenes", "day_or_night");
  g_assert(field);
  check_title(field, "Day/Night", "Tag/Nacht");

  Glom::Formatting formatting = field->m_default_formatting;
  g_assert(formatting.get_has_custom_choices());
  Glom::Formatting::type_list_values values = formatting.get_choices_custom();
  //g_assert(contains(values, "Day"));
  std::shared_ptr<Glom::ChoiceValue> value = get_titled(values, "Day");
  g_assert(value);
  check_title(value, "Day", "Tag");
  g_assert(value->get_value() == Gnome::Gda::Value("Day"));

  g_assert( value->get_title_original() == "Day" );
  g_assert( formatting.get_custom_choice_original_for_translated_text("Nacht", locale_de) == "Night" );
  g_assert( formatting.get_custom_choice_original_for_translated_text("aaaa", locale_de) == "" );
  g_assert( formatting.get_custom_choice_translated("Night", locale_de) == "Nacht" );
  g_assert( formatting.get_custom_choice_translated("aaaa", locale_de) == "" );
  g_assert( value->get_title_original() == "Day" );

  //Check a relationship:
  const std::shared_ptr<const Glom::Relationship> relationship = document.get_relationship("characters", "contacts_actor");
  g_assert(relationship);
  check_title(relationship, "Actor", "Schauspieler");

  //Check a LayoutItemField's CustomTitle:
  std::shared_ptr<const Glom::LayoutItem_Field> field_on_layout = 
    get_field_on_layout(document, "characters", "contacts", "name_full");
  g_assert(field_on_layout);
  g_assert(field_on_layout->get_has_relationship_name());
  g_assert(field_on_layout->get_relationship_name() == "contacts_actor");
  check_title(field_on_layout, "Actor's Name", "Name des Schauspielers");

  //Check a LayoutItemField's Field title:
  field_on_layout = 
    get_field_on_layout(document, "scenes", "locations", "name");
  g_assert(field_on_layout);
  check_title(field_on_layout, "Name", "Name" );

  field_on_layout = 
    get_field_on_layout(document, "scenes", "scenes", "day_or_night");
  g_assert(field_on_layout);
  check_title(field_on_layout,  "Day/Night", "Tag/Nacht");
  g_assert(field_on_layout->get_formatting_used_has_translatable_choices());

  //Check a print layout:
  const std::shared_ptr<const Glom::PrintLayout> print_layout = document.get_print_layout("contacts", "contact_details");
  g_assert(print_layout);
  check_title(print_layout, "Contact Details", "Kontakt Details" );

  //Check the whole list of translatable items:
  Glom::Document::type_list_translatables list_layout_items = document.get_translatable_items();
  g_assert(!list_layout_items.empty());
  const bool contains_databasetitle =
    contains_item_type<Glom::DatabaseTitle>(list_layout_items);
  g_assert( contains_databasetitle );
  const bool contains_tableinfo =
    contains_item_type<Glom::TableInfo>(list_layout_items);
  g_assert( contains_tableinfo );
  const bool contains_layoutitem =
    contains_item_type<Glom::LayoutItem>(list_layout_items);
  g_assert( contains_layoutitem );
  /*
  const bool contains_layoutitemfield =
    contains_item_type<Glom::LayoutItem_Field>(list_layout_items);
  g_assert( contains_layoutitemfield );
  */
  const bool contains_relationship =
    contains_item_type<Glom::Relationship>(list_layout_items);
  g_assert( contains_relationship );
  const bool contains_field =
    contains_item_type<Glom::Field>(list_layout_items);
  g_assert( contains_field );
  const bool contains_choicevalue =
    contains_item_type<Glom::ChoiceValue>(list_layout_items);
  g_assert( contains_choicevalue );
  const bool contains_customtitle =
    contains_item_type<Glom::CustomTitle>(list_layout_items);
  g_assert( contains_customtitle );
  const bool contains_static_text =
    contains_item_type<Glom::LayoutItem_Text>(list_layout_items);
  g_assert( contains_static_text );
  const bool contains_report =
    contains_item_type<Glom::Report>(list_layout_items);
  g_assert( contains_report );
  const bool contains_print_layout =
    contains_item_type<Glom::PrintLayout>(list_layout_items);
  g_assert( contains_print_layout );

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
