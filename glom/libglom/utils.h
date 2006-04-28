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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLOM_UTILS_H
#define GLOM_UTILS_H

#include <glom/libglom/data_structure/field.h>
#include <glom/libglom/data_structure/numeric_format.h>

#include <glom/libglom/data_structure/layout/layoutitem_field.h>

///field, ascending
typedef std::pair< sharedptr<const LayoutItem_Field>, bool> type_pair_sort_field;
typedef std::list<type_pair_sort_field> type_sort_clause;

namespace GlomUtils
{

Glib::ustring trim_whitespace(const Glib::ustring& text);

Glib::ustring string_replace(const Glib::ustring& src, const Glib::ustring search_for, const Glib::ustring& replace_with);

//typedef Base_DB::type_vecLayoutFields type_vecLayoutFields;
typedef std::vector< sharedptr<LayoutItem_Field> > type_vecLayoutFields;

  //TODO: Move this to its own file:
Glib::ustring build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const Glib::ustring& where_clause = Glib::ustring(), const type_sort_clause& sort_clause = type_sort_clause());

Glib::ustring build_sql_select_with_key(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const sharedptr<const Field>& key_field, const Gnome::Gda::Value& key_value);

typedef std::list< std::pair<Gnome::Gda::Value, Gnome::Gda::Value> > type_list_values_with_second;
type_list_values_with_second get_choice_values(const sharedptr<const LayoutItem_Field>& field);

/** Guess an appropriate identifier name based on a human-readable title
 */
Glib::ustring create_name_from_title(const Glib::ustring& title);

Glib::ustring string_escape_underscores(const Glib::ustring& text);

/** Get just the first part of a locale, such as de_DE, 
 * ignoring, for instance, .UTF-8 or @euro at the end.
 */
Glib::ustring locale_simplify(const Glib::ustring& locale_id);

/** Get just the language ID part of a locale, such as de from "de_DE", 
 */
Glib::ustring locale_language_id(const Glib::ustring& locale_id);

Glib::ustring create_local_image_uri(const Gnome::Gda::Value& value);

} //namespace GlomUtils

#endif //GLOM_UTILS_H

