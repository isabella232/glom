/* Glom
 *
 * Copyright (C) 2001-2016 Murray Cumming
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

#ifndef GLOM_SQL_UTILS_H
#define GLOM_SQL_UTILS_H

#include <libglom/document/document.h>
#include <libglom/data_structure/layout/layoutitem_field.h>
#include <libgdamm/sqlbuilder.h>
#include <libgdamm/sqlexpr.h>


namespace Glom
{

///field, ascending
typedef std::pair< std::shared_ptr<const LayoutItem_Field>, bool> type_pair_sort_field;
typedef std::vector<type_pair_sort_field> type_sort_clause;

namespace Utils
{

typedef std::vector< std::shared_ptr<const LayoutItem_Field> > type_vecConstLayoutFields;

//typedef Base_DB::type_vecLayoutFields type_vecLayoutFields;
typedef std::vector< std::shared_ptr<LayoutItem_Field> > type_vecLayoutFields;

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
 * @param sql_query A SQL query.
 * @result The number of rows.
 */
Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_count_rows(const Glib::RefPtr<const Gnome::Gda::SqlBuilder>& sql_query);

Gnome::Gda::SqlExpr get_find_where_clause_quick(const std::shared_ptr<const Document>& document, const Glib::ustring& table_name, const Gnome::Gda::Value& quick_search);

/** Generate a SQL statement to UPDATE field values,
 */
Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_update_with_where_clause(
        const Glib::ustring& table_name,
        const std::shared_ptr<const Field>& field, const Gnome::Gda::Value& value,
        const Gnome::Gda::SqlExpr& where_clause);

/// Get the full query string suitable for use with std::cout.
std::string sqlbuilder_get_full_query(
        const Glib::RefPtr<const Gnome::Gda::SqlBuilder>& builder);

} //namespace Utils

} //namespace Glom

#endif //GLOM_SQL_UTILS_H
