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

#include <libglom/data_structure/field.h>
#include <libglom/data_structure/numeric_format.h>
#include <libglom/document/document.h>

#include <libglom/data_structure/layout/layoutitem_field.h>
#include <libgdamm/sqlexpr.h>

namespace Glom
{

///field, ascending
typedef std::pair< sharedptr<const LayoutItem_Field>, bool> type_pair_sort_field;
typedef std::list<type_pair_sort_field> type_sort_clause;

namespace Utils
{

Glib::ustring trim_whitespace(const Glib::ustring& text);

Glib::ustring string_replace(const Glib::ustring& src, const Glib::ustring search_for, const Glib::ustring& replace_with);

//typedef Base_DB::type_vecLayoutFields type_vecLayoutFields;
typedef std::vector< sharedptr<LayoutItem_Field> > type_vecLayoutFields;
typedef std::vector< sharedptr<const LayoutItem_Field> > type_vecConstLayoutFields;

//TODO: Move these to their own file:

// Create a Gnome::Gda::SqlExpr.
Gnome::Gda::SqlExpr build_simple_where_expression(const Glib::ustring& table_name, const sharedptr<const Field>& key_field, const Gnome::Gda::Value& key_value);

// Create a where clause that is two other conditions combined together.
Gnome::Gda::SqlExpr build_combined_where_expression(const Gnome::Gda::SqlExpr& a, const Gnome::Gda::SqlExpr& b, Gnome::Gda::SqlOperatorType op);

/** Generate a SQL statement to SELECT field values,
 * even if the fields are in related (or doubly related) records.
 */
void build_sql_select_add_fields_to_get(
  const Glib::RefPtr<Gnome::Gda::SqlBuilder>& builder,
  const Glib::ustring& table_name,
  const type_vecConstLayoutFields& fieldsToGet,
  const type_sort_clause& sort_clause);

/** Generate a SQL statement to SELECT field values,
 * even if the fields are in related (or doubly related) records,
 * narrowing the records down with a WHERE clause.
 */
Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_with_where_clause(
  const Glib::ustring& table_name,
  const type_vecLayoutFields& fieldsToGet,
  const Gnome::Gda::SqlExpr& where_clause = Gnome::Gda::SqlExpr(),
  const Glib::ustring& extra_join = Glib::ustring(),
  const type_sort_clause& sort_clause = type_sort_clause(),
  const Glib::ustring& extra_group_by = Glib::ustring(),
  guint limit = 0);

/** Just a version of build_sql_select_with_where_clause() that takes a list of const fields.
 */
Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_with_where_clause(
  const Glib::ustring& table_name,
  const type_vecConstLayoutFields& fieldsToGet,
  const Gnome::Gda::SqlExpr& where_clause = Gnome::Gda::SqlExpr(),
  const Glib::ustring& extra_join = Glib::ustring(),
  const type_sort_clause& sort_clause = type_sort_clause(),
  const Glib::ustring& extra_group_by = Glib::ustring(),
  guint limit = 0);

Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_with_key(
  const Glib::ustring& table_name,
  const type_vecLayoutFields& fieldsToGet,
  const sharedptr<const Field>& key_field,
  const Gnome::Gda::Value& key_value,
  guint limit = 0);

/** Just a version of build_sql_select_with_key() that takes a list of const fields.
 */
Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_with_key(
  const Glib::ustring& table_name,
  const type_vecConstLayoutFields& fieldsToGet,
  const sharedptr<const Field>& key_field,
  const Gnome::Gda::Value& key_value,
  guint limit = 0);

Gnome::Gda::SqlExpr get_find_where_clause_quick(Document* document, const Glib::ustring& table_name, const Gnome::Gda::Value& quick_search);


typedef std::list< std::pair<Gnome::Gda::Value, Gnome::Gda::Value> > type_list_values_with_second;
type_list_values_with_second get_choice_values(const sharedptr<const LayoutItem_Field>& field);

/// Get the full query string suitable for use with std::cout.
std::string sqlbuilder_get_full_query(
  const Glib::RefPtr<Gnome::Gda::Connection>& connection,
  const Glib::ustring& query,
  const Glib::RefPtr<const Gnome::Gda::Set>& params);

/// Get the full query string suitable for use with std::cout.
std::string sqlbuilder_get_full_query(
  const Glib::RefPtr<const Gnome::Gda::SqlBuilder>& builder);

/** Guess an appropriate identifier name based on a human-readable title
 */
Glib::ustring create_name_from_title(const Glib::ustring& title);

Glib::ustring string_escape_underscores(const Glib::ustring& text);

/** Get just the first part of a locale, such as de_DE,
 * ignoring, for instance, .UTF-8 or \@euro at the end.
 */
Glib::ustring locale_simplify(const Glib::ustring& locale_id);

/** Get just the language ID part of a locale, such as de from "de_DE",
 */
Glib::ustring locale_language_id(const Glib::ustring& locale_id);

Glib::ustring create_local_image_uri(const Gnome::Gda::Value& value);

/** Get a decimal text representation of the number,
 * in the current locale.
 */
Glib::ustring string_from_decimal(guint decimal);

/** Create an appropriate title for an ID string.
 * For instance, date_of_birth would become Date Of Birth.
 */
Glib::ustring title_from_string(const Glib::ustring& text);

typedef std::vector<Glib::ustring> type_vec_strings;
type_vec_strings string_separate(const Glib::ustring& str, const Glib::ustring& separator, bool ignore_quoted_separator = false);

Glib::ustring string_trim(const Glib::ustring& str, const Glib::ustring& to_remove);

Glib::ustring string_remove_suffix(const Glib::ustring& str, const Glib::ustring& suffix, bool case_sensitive = true);

bool file_exists(const Glib::ustring& uri);

} //namespace Utils

} //namespace Glom

#endif //GLOM_UTILS_H
