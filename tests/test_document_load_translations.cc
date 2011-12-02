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

static const char* locale_de = "de_DE.UTF-8";

template<typename T_Item>
void check_title(const T_Item& item, const char* title_en, const char* title_de)
{
  g_assert(item);

  g_assert( item->get_title() == title_en );

  const Glib::ustring locale_original = Glom::TranslatableItem::get_current_locale();
  Glom::TranslatableItem::set_current_locale(locale_de);
  g_assert( item->get_title() == title_de );
  Glom::TranslatableItem::set_current_locale(locale_original);
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

  const std::vector<Glib::ustring> table_names = document.get_table_names();
  g_assert(contains(table_names, "scenes"));

  g_assert( document.get_table_title("scenes") == "Scenes" );
  
  const Glib::ustring locale_original = Glom::TranslatableItem::get_current_locale();
  Glom::TranslatableItem::set_current_locale(locale_de);
  g_assert( document.get_table_title("scenes") == "Szenen" );
  Glom::TranslatableItem::set_current_locale(locale_original);

  Glom::sharedptr<const Glom::Field> field = document.get_field("contacts", "contact_id");
  g_assert(field);
  check_title(field, "Contact ID", "Kontakt ID");

  //Check a relationship:
  const Glom::sharedptr<const Glom::Relationship> relationship = document.get_relationship("characters", "contacts_actor");
  g_assert(relationship);
  check_title(relationship, "Actor", "Schauspieler");

  Glom::sharedptr<const Glom::LayoutItem_Field> field_on_layout = 
    get_field_on_layout(document, "characters", "contacts", "name_full");
  g_assert(field_on_layout);
  check_title(field_on_layout, "Actor's Name", "Schauspieler Name" );

  const Glom::sharedptr<const Glom::PrintLayout> print_layout = document.get_print_layout("contacts", "contact_details");
  g_assert(print_layout);
  check_title(print_layout, "Contact Details", "Kontakt Details" );


  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
