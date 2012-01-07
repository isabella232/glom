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



/** A predicate for use with std::find_if() to find a Field or LayoutItem which refers 
 * to the same field, looking at just the name.
 */
template<class T_Element>
class predicate_ItemHasTitle
{
public:
  predicate_ItemHasTitle(const Glib::ustring& title)
  {
    m_title = title;
  }

  virtual ~predicate_ItemHasTitle()
  {
  }

  bool operator() (const Glom::sharedptr<T_Element>& element)
  {
    return (element->get_title() == m_title);
  }

  bool operator() (const Glom::sharedptr<const T_Element>& element)
  {
    return (element->get_title() == m_title);
  }

private:
  Glib::ustring m_title;
};

/** A predicate for use with std::find_if() to find a LayoutItem of a particular type.
 */
template<class T_Element, class T_TypeToFind>
class predicate_ItemHasType
{
public:
  predicate_ItemHasType()
  {
  }

  virtual ~predicate_ItemHasType()
  {
  }

  bool operator() (const Glom::sharedptr<T_Element>& element)
  {
    Glom::sharedptr<T_TypeToFind> derived = Glom::sharedptr<T_TypeToFind>::cast_dynamic(element);
    if(derived)
      return true;
    else
      return false;
  }

  bool operator() (const Glom::sharedptr<const T_Element>& element)
  {
     Glom::sharedptr<const T_TypeToFind> derived = Glom::sharedptr<const T_TypeToFind>::cast_dynamic(element);
    if(derived)
      return true;
    else
      return false;
  }
};


template<typename T_Container>
typename T_Container::value_type get_titled(const T_Container& container, const Glib::ustring& title)
{
  typedef typename T_Container::value_type type_sharedptr;
  type_sharedptr result;

  typedef typename T_Container::value_type::object_type type_item;
  typename T_Container::const_iterator iter =
    std::find_if(container.begin(), container.end(),
      predicate_ItemHasTitle<type_item>(title));
  if(iter != container.end())
    result = *iter;

  return result;
}

template<typename T_Container, typename T_TypeToFind>
bool contains_item_type(const T_Container& container)
{
  typedef typename T_Container::value_type type_sharedptr;
  type_sharedptr result;

  typedef typename T_Container::value_type::object_type type_item;
  typename T_Container::const_iterator iter =
    std::find_if(container.begin(), container.end(),
      predicate_ItemHasType<type_item, T_TypeToFind>());
  if(iter != container.end())
    result = *iter;

  return result;
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

static const char* locale_de = "de_DE.UTF-8";

template<typename T_Item>
void check_title(const T_Item& item, const char* title_en, const char* title_de)
{
  g_assert(item);

  g_assert( item->get_title() == title_en );

  //Check when changing the current locale:
  const Glib::ustring locale_original = Glom::TranslatableItem::get_current_locale();
  Glom::TranslatableItem::set_current_locale(locale_de);
  g_assert( item->get_title() == title_de );
  Glom::TranslatableItem::set_current_locale(locale_original);


  //Don't do the following checks if get_title() would actually delegate to the 
  //child Field, instead of using the LayoutItem's own title translations:
  const Glom::sharedptr<const Glom::LayoutItem_Field> layout_field = 
     Glom::sharedptr<const Glom::LayoutItem_Field>::cast_dynamic(item);
  if(layout_field)
  {
    return;
  }

  //Check when getting the translations directly:
  g_assert( item->get_title_original() == title_en );
  g_assert( item->get_title_translation(locale_de) == title_de );

  //Check fallbacks:
  g_assert( item->get_title_translation(Glib::ustring()) == title_en );
  g_assert( item->get_title_translation(locale_original) == title_en );

  //Check that fallbacks do not happen when we don't want them:
  g_assert( item->get_title_translation(Glib::ustring(), false) == Glib::ustring() );
  g_assert( item->get_title_translation(locale_original, false) == Glib::ustring() );
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
  const bool test = document.load(failure_code);
  //std::cout << "Document load result=" << test << std::endl;

  if(!test)
  {
    std::cerr << "Document::load() failed with failure_code=" << failure_code << std::endl;
    return EXIT_FAILURE;
  }

  const std::vector<Glib::ustring> locales = document.get_translation_available_locales();
  g_assert(locales.size() == 3);
  g_assert(contains(locales, "de_DE"));

  const std::vector<Glib::ustring> table_names = document.get_table_names();
  g_assert(contains(table_names, "scenes"));

  g_assert( document.get_table_title("scenes") == "Scenes" );
  
  const Glib::ustring locale_original = Glom::TranslatableItem::get_current_locale();
  Glom::TranslatableItem::set_current_locale(locale_de);
  g_assert( document.get_table_title("scenes") == "Szenen" );
  Glom::TranslatableItem::set_current_locale(locale_original);

  //Check a field:
  Glom::sharedptr<const Glom::Field> field = document.get_field("contacts", "contact_id");
  g_assert(field);
  check_title(field, "Contact ID", "Kontakt ID");

  //Check a field and its custom choices:
  field = document.get_field("scenes", "day_or_night");
  g_assert(field);
  check_title(field, "Day/Night", "Tag/Nacht");

  Glom::FieldFormatting formatting = field->m_default_formatting;
  g_assert(formatting.get_has_custom_choices());
  Glom::FieldFormatting::type_list_values values = formatting.get_choices_custom();
  //g_assert(contains(values, "Day"));
  Glom::sharedptr<Glom::ChoiceValue> value = get_titled(values, "Day");
  g_assert(value);
  check_title(value, "Day", "Tag");
  g_assert(value->get_value() == Gnome::Gda::Value("Day"));

  Glom::TranslatableItem::set_current_locale(locale_de);
  g_assert( value->get_title_original() == "Day" );
  g_assert( formatting.get_custom_choice_original_for_translated_text("Nacht") == "Night" );
  g_assert( formatting.get_custom_choice_original_for_translated_text("aaaa") == "" );
  g_assert( formatting.get_custom_choice_translated("Night") == "Nacht" );
  g_assert( formatting.get_custom_choice_translated("aaaa") == "" );
  Glom::TranslatableItem::set_current_locale(locale_original);
  g_assert( value->get_title_original() == "Day" );

  //Check a relationship:
  const Glom::sharedptr<const Glom::Relationship> relationship = document.get_relationship("characters", "contacts_actor");
  g_assert(relationship);
  check_title(relationship, "Actor", "Schauspieler");

  Glom::sharedptr<const Glom::LayoutItem_Field> field_on_layout = 
    get_field_on_layout(document, "characters", "contacts", "name_full");
  g_assert(field_on_layout);
  check_title(field_on_layout, "Actor's Name", "Schauspieler Name" );

  field_on_layout = 
    get_field_on_layout(document, "scenes", "scenes", "day_or_night");
  g_assert(field_on_layout);
  check_title(field_on_layout,  "Day/Night", "Tag/Nacht");
  g_assert(field_on_layout->get_formatting_used_has_translatable_choices());

  //Check a print layout:
  const Glom::sharedptr<const Glom::PrintLayout> print_layout = document.get_print_layout("contacts", "contact_details");
  g_assert(print_layout);
  check_title(print_layout, "Contact Details", "Kontakt Details" );

  //Check the whole list of translatable items:
  Glom::Document::type_list_translatables list_layout_items = document.get_translatable_items();
  g_assert(!list_layout_items.empty());
  const bool contains_tableinfo =
    contains_item_type<Glom::Document::type_list_translatables, Glom::TableInfo>(list_layout_items);
  g_assert( contains_tableinfo );
  const bool contains_layoutitem =
    contains_item_type<Glom::Document::type_list_translatables, Glom::LayoutItem>(list_layout_items);
  g_assert( contains_layoutitem );
  /*
  const bool contains_layoutitemfield =
    contains_item_type<Glom::Document::type_list_translatables, Glom::LayoutItem_Field>(list_layout_items);
  g_assert( contains_layoutitemfield );
  */
  const bool contains_relationship =
    contains_item_type<Glom::Document::type_list_translatables, Glom::Relationship>(list_layout_items);
  g_assert( contains_relationship );
  const bool contains_field =
    contains_item_type<Glom::Document::type_list_translatables, Glom::Field>(list_layout_items);
  g_assert( contains_field );
  const bool contains_choicevalue =
    contains_item_type<Glom::Document::type_list_translatables, Glom::ChoiceValue>(list_layout_items);
  g_assert( contains_choicevalue );
  const bool contains_customtitle =
    contains_item_type<Glom::Document::type_list_translatables, Glom::CustomTitle>(list_layout_items);
  g_assert( contains_customtitle );
  const bool contains_report =
    contains_item_type<Glom::Document::type_list_translatables, Glom::Report>(list_layout_items);
  g_assert( contains_report );

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
