/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
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

#ifndef GLOM_UTILS_H
#define GLOM_UTILS_H

#include <libglom/data_structure/field.h>
#include <libglom/data_structure/numeric_format.h>
#include <libglom/document/document.h>
#include <libglom/data_structure/layout/layoutitem_field.h>
#include <libglom/algorithms_utils.h>

#include <giomm/file.h>

namespace Glom
{

namespace Utils
{

typedef std::vector< std::shared_ptr<const LayoutItem_Field> > type_vecConstLayoutFields;
typedef std::vector< std::shared_ptr<Field> > type_vec_fields;


/** Get just the first part of a locale, such as de_DE,
 * ignoring, for instance, .UTF-8 or \@euro at the end.
 */
Glib::ustring locale_simplify(const Glib::ustring& locale_id);

/** Get just the language ID part of a locale, such as de from "de_DE",
 */
Glib::ustring locale_language_id(const Glib::ustring& locale_id);


/** Get a string to display to the user, as a representation of a list of layout items.
 */
Glib::ustring get_list_of_layout_items_for_display(const LayoutGroup::type_list_items& list_layout_fields);

/** Get a string to display to the user, as a representation of a list of layout items.
 */
Glib::ustring get_list_of_layout_items_for_display(const std::shared_ptr<const LayoutGroup>& layout_group);

/** Get a string to display to the user, as a representation of a sort order
 */
Glib::ustring get_list_of_sort_fields_for_display(const Formatting::type_list_sort_fields& sort_fields);

/** This returns the provided list of layout items,
 * plus the primary key, if the primary key is not already present in the list
 */
LayoutGroup::type_list_const_items get_layout_items_plus_primary_key(const LayoutGroup::type_list_const_items& items, const std::shared_ptr<const Document>& document, const Glib::ustring& table_name);

//TODO: Avoid the overload just for constness.
/** This returns the provided list of layout items,
 * plus the primary key, if the primary key is not already present in the list
 */
LayoutGroup::type_list_items get_layout_items_plus_primary_key(const LayoutGroup::type_list_items& items, const std::shared_ptr<const Document>& document, const Glib::ustring& table_name);

type_vecConstLayoutFields get_table_fields_to_show_for_sequence(const std::shared_ptr<const Document>& document, const Glib::ustring& table_name, const Document::type_list_layout_groups& mapGroupSequence);


/** @returns true if the script is OK, or 
 * false if the script uses pygtk2, which would cause a crash,
 * because Glom itself uses GTK+ 3.
 */
bool script_check_for_pygtk2(const Glib::ustring& script);

/** 
 * This is simpler than catching the exception from Gio::Resource::get_info_global().
 *
 * @returns true if the GResource exists.
 */
bool get_resource_exists(const std::string& resource_path);

template<typename E>
constexpr typename std::underlying_type<E>::type
to_utype(E enumerator) noexcept
{
  return static_cast<typename std::underlying_type<E>::type>(enumerator);
}


/**
 * Find the element in the container which is a LayoutItem_Field which refers 
 * to the same field, without comparing irrelevant stuff such as formatting.
 * This assumes that the element is a shared_ptr<>.
 */
template
<typename T_Container>
bool find_if_layout_item_field_is_same_field_exists(T_Container& container, const std::shared_ptr<const LayoutItem_Field>& layout_item)
{
  return Utils::find_if_exists(container,
    [&layout_item](const typename T_Container::value_type& element)
    {
      //Assume that element is a shared_ptr<>.

      if(!layout_item && !element)
        return true;

      //Allow this to be used on a container of LayoutItems,
      //as well as just of LayoutItem_Fields.
      const auto element_field = std::dynamic_pointer_cast<const LayoutItem_Field>(element);
      if(!element_field)
        return false;

      return layout_item && layout_item->is_same_field(element_field);
    }
  );
}

} //namespace Utils

} //namespace Glom

#endif //GLOM_UTILS_H
