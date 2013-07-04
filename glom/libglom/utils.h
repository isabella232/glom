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

#include <libgdamm/sqlexpr.h>
#include <giomm/file.h>

namespace Glom
{

///field, ascending
typedef std::pair< std::shared_ptr<const LayoutItem_Field>, bool> type_pair_sort_field;
typedef std::vector<type_pair_sort_field> type_sort_clause;

namespace Utils
{

Glib::ustring trim_whitespace(const Glib::ustring& text);

Glib::ustring string_replace(const Glib::ustring& src, const Glib::ustring& search_for, const Glib::ustring& replace_with);

/** Remove any characters that may not be in XML even when escaped.
 */
Glib::ustring string_clean_for_xml(const Glib::ustring& src);

//typedef Base_DB::type_vecLayoutFields type_vecLayoutFields;
typedef std::vector< std::shared_ptr<LayoutItem_Field> > type_vecLayoutFields;
typedef std::vector< std::shared_ptr<const LayoutItem_Field> > type_vecConstLayoutFields;

//TODO: Move these to their own file:

// Create a Gnome::Gda::SqlExpr.
Gnome::Gda::SqlExpr build_simple_where_expression(const Glib::ustring& table_name, const std::shared_ptr<const Field>& key_field, const Gnome::Gda::Value& key_value);

// Create a where clause that is two other conditions combined together.
Gnome::Gda::SqlExpr build_combined_where_expression(const Gnome::Gda::SqlExpr& a, const Gnome::Gda::SqlExpr& b, Gnome::Gda::SqlOperatorType op);

/** Generate a SQL statement to SELECT field values,
 * even if the fields are in related (or doubly related) records.
 */
void build_sql_select_add_fields_to_get(
  const Glib::RefPtr<Gnome::Gda::SqlBuilder>& builder,
  const Glib::ustring& table_name,
  const type_vecConstLayoutFields& fieldsToGet,
  const type_sort_clause& sort_clause,
  bool extra_join);

/** Generate a SQL statement to SELECT field values,
 * even if the fields are in related (or doubly related) records,
 * narrowing the records down with a WHERE clause.
 */
Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_with_where_clause(
  const Glib::ustring& table_name,
  const type_vecLayoutFields& fieldsToGet,
  const Gnome::Gda::SqlExpr& where_clause = Gnome::Gda::SqlExpr(),
  const std::shared_ptr<const Relationship>& extra_join = std::shared_ptr<const Relationship>(),
  const type_sort_clause& sort_clause = type_sort_clause(),
  guint limit = 0);

/** Just a version of build_sql_select_with_where_clause() that takes a list of const fields.
 */
Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_with_where_clause(
  const Glib::ustring& table_name,
  const type_vecConstLayoutFields& fieldsToGet,
  const Gnome::Gda::SqlExpr& where_clause = Gnome::Gda::SqlExpr(),
  const std::shared_ptr<const Relationship>& extra_join = std::shared_ptr<const Relationship>(),
  const type_sort_clause& sort_clause = type_sort_clause(),
  guint limit = 0);

/**
 * @param key_value If this is empty then all records in the tables will be retrieved.
 */
Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_with_key(
  const Glib::ustring& table_name,
  const type_vecLayoutFields& fieldsToGet,
  const std::shared_ptr<const Field>& key_field,
  const Gnome::Gda::Value& key_value,
  const type_sort_clause& sort_clause = type_sort_clause(),
  guint limit = 0);

/** Just a version of build_sql_select_with_key() that takes a list of const fields.
 */
Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_with_key(
  const Glib::ustring& table_name,
  const type_vecConstLayoutFields& fieldsToGet,
  const std::shared_ptr<const Field>& key_field,
  const Gnome::Gda::Value& key_value,
  const type_sort_clause& sort_clause = type_sort_clause(),
  guint limit = 0);

//Note: This is not used by glom itself, but it is used by java-libglom.
/** Build a SQL query to discover how many rows a SQL query would return if it was run.
 *
 * This uses a COUNT * on a the @a sql_query as a sub-statement.
 * Be careful not to include ORDER BY clauses in the supplied SQL query, because that would make it unnecessarily slow.
 *
 * @sql_query A SQL query.
 * @result The number of rows.
 */
Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_count_rows(const Glib::RefPtr<const Gnome::Gda::SqlBuilder>& sql_query);

Gnome::Gda::SqlExpr get_find_where_clause_quick(const Document* document, const Glib::ustring& table_name, const Gnome::Gda::Value& quick_search);

/** Generate a SQL statement to UPDATE field values,
 */
Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_update_with_where_clause(
  const Glib::ustring& table_name,
  const std::shared_ptr<const Field>& field, const Gnome::Gda::Value& value,
  const Gnome::Gda::SqlExpr& where_clause);

typedef std::vector<Gnome::Gda::Value> type_list_values;
typedef std::vector< std::pair<Gnome::Gda::Value, type_list_values> > type_list_values_with_second; //TODO: Rename this now that we have more than just 1 extra field.
type_list_values_with_second get_choice_values_all(const Document* document, const std::shared_ptr<const LayoutItem_Field>& field);

type_list_values_with_second get_choice_values(const Document* document, const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& foreign_key_value);

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
bool file_exists(const Glib::RefPtr<Gio::File>& file);

/** Delete a directory, if it exists, and its contents.
 * Unlike g_file_delete(), this does not fail if the directory is not empty.
 */
bool delete_directory(const Glib::RefPtr<Gio::File>& directory);

/** Delete a directory, if it exists, and its contents.
 * Unlike g_file_delete(), this does not fail if the directory is not empty.
 * See also delete_file().
 */
bool delete_directory(const std::string& uri);

/** Delete a file, if it exists.
 * See also delete_directory().
 */
bool delete_file(const std::string& uri);

/** For instance, to find the first file in the directory with a .glom extension.
 */
Glib::ustring get_directory_child_with_suffix(const Glib::ustring& uri_directory, const std::string& suffix, bool recursive);

/** Get a URI with the extension (any extension, not just .glom) removed.
 */
Glib::ustring get_file_uri_without_extension(const Glib::ustring& uri);

/** Get a filepath with the extension (any extension, not just .glom) removed.
 */
std::string get_file_path_without_extension(const std::string& filepath);

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
LayoutGroup::type_list_const_items get_layout_items_plus_primary_key(const LayoutGroup::type_list_const_items& items, const Document* document, const Glib::ustring& table_name);

//TODO: Avoid the overload just for constness.
/** This returns the provided list of layout items,
 * plus the primary key, if the primary key is not already present in the list
 */
LayoutGroup::type_list_items get_layout_items_plus_primary_key(const LayoutGroup::type_list_items& items, const Document* document, const Glib::ustring& table_name);

std::string get_temp_file_path(const std::string& prefix = std::string(), const std::string& extension = std::string());
Glib::ustring get_temp_file_uri(const std::string& prefix = std::string(), const std::string& extension = std::string());

/** This actually creates the directory.
 */
std::string get_temp_directory_path(const std::string& prefix = std::string());

/** This actually creates the directory.
 */
Glib::ustring get_temp_directory_uri(const std::string& prefix = std::string());

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

} //namespace Utils

} //namespace Glom

#endif //GLOM_UTILS_H
