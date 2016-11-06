/* Glom
 *
 * Copyright (C) 2001-2010 Murray Cumming
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

#ifndef GLOM_DB_UTILS_H
#define GLOM_DB_UTILS_H

#include <libglom/document/document.h>
#include <libglom/data_structure/system_prefs.h>
#include <memory> //For shared_ptr<>.

namespace Glom
{

namespace DbUtils
{

/**
 * This also saves the connection port in the document if self-hosting.
 */
bool create_database(const std::shared_ptr<Document>& document, const Glib::ustring& database_name, const Glib::ustring& title, const std::function<void()>& progress);

//TODO: Use this in Glom::AppWindow?
/** Create the database on an already-connected server.
 * This also saves some details in the document.
 */
bool recreate_database_from_document(const std::shared_ptr<Document>& document, const std::function<void()>& progress);

/** This creates the standard tables if necessary,
 * filling them with some information from the document.
 */
SystemPrefs get_database_preferences(const std::shared_ptr<const Document>& document);

/**
 * This also saves the preferences in the document.
 */
void set_database_preferences(const std::shared_ptr<Document>& document, const SystemPrefs& prefs);

bool add_standard_tables(const std::shared_ptr<const Document>& document);

/**
 * This also saves the groups in the document.
 */
bool add_standard_groups(const std::shared_ptr<Document>& document);

bool add_groups_from_document(const std::shared_ptr<const Document>& document);
bool set_table_privileges_groups_from_document(const std::shared_ptr<const Document>& document);

typedef std::vector< std::shared_ptr<Field> > type_vec_fields;
type_vec_fields get_fields_for_table_from_database(const Glib::ustring& table_name, bool including_system_fields = false);
bool get_field_exists_in_database(const Glib::ustring& table_name, const Glib::ustring& field_name);

/** Get all the fields for a table, including any from the database that are not yet known in the document.
 *
 * @param table_name The name of the table whose fields should be listed.
 * @param including_system_fields Whether extra non-user-visible fields should be included in the list.
 * @result A list of fields.
 */
type_vec_fields get_fields_for_table(const std::shared_ptr<const Document>& document, const Glib::ustring& table_name, bool including_system_fields = false);

/** Get a single field definition for a table, even if the field is in the datasbase but not yet known in the document.
 *
 * @param table_name The name of the table whose fields should be listed.
 * @param field_name The name of the field for which to get the definition.
 * @result The field definition.
 */
std::shared_ptr<Field> get_fields_for_table_one_field(const std::shared_ptr<const Document>& document, const Glib::ustring& table_name, const Glib::ustring& field_name);

typedef std::vector<Glib::ustring> type_vec_strings;

/** Get the table names from the database server.
 * This could theoretically be different than the ones listed in the document.
 */
type_vec_strings get_table_names_from_database(bool ignore_system_tables = false);

bool get_table_exists_in_database(const Glib::ustring& table_name);

bool create_table(Document::HostingMode hosting_mode, const std::shared_ptr<const TableInfo>& table_info, const Document::type_vec_fields& fields);

/// Also saves the table information in the document:
bool create_table_with_default_fields(const std::shared_ptr<Document>& document, const Glib::ustring& table_name);

bool create_table_add_missing_fields(const std::shared_ptr<const TableInfo>& table_info, const Document::type_vec_fields& fields);

// TODO: Should these functions update the document, so callers don't need
// to do it?
bool add_column(const Glib::ustring& table_name, const std::shared_ptr<const Field>& field, Gtk::Window* parent_window);

bool drop_column(const Glib::ustring& table_name, const Glib::ustring& field_name);


/** Insert example data, from the document, into the table on the database server.
 */
bool insert_example_data(const std::shared_ptr<const Document>& document, const Glib::ustring& table_name);

/** Execute a SQL Select command, returning the result.
 * @param builder The finished SqlBuilder object.
 * @param use_cursor Whether the data model should be cursor-based (not allowing random access).
 */
Glib::RefPtr<Gnome::Gda::DataModel> query_execute_select(
  const Glib::RefPtr<const Gnome::Gda::SqlBuilder>& builder,
  bool use_cursor = false);


/** Execute a SQL non-select command, returning true if it succeeded.
  * See also query_execute(), which takes a SqlBuilder.
  * This should only be used for SQL commands that are not supported by SqlBuilder,
  * such as ADD GROUP.
  */
bool query_execute_string(const Glib::ustring& strQuery,
  const Glib::RefPtr<Gnome::Gda::Set>& params = Glib::RefPtr<Gnome::Gda::Set>(0));

/** Execute a SQL non-select command, returning true if it succeeded.
  */
bool query_execute(const Glib::RefPtr<const Gnome::Gda::SqlBuilder>& builder);

/** Insert the auto-increment row in the database preferences table, if necessary,
 * returning the next value.
 */
Gnome::Gda::Value auto_increment_insert_first_if_necessary(const Glib::ustring& table_name, const Glib::ustring& field_name);

/** Get the next auto-increment value for this primary key, from the glom system table.
  * Add a row for this field in the system table if it does not exist already.
  * This increments the next value after obtaining the current next value.
  */
Gnome::Gda::Value get_next_auto_increment_value(const Glib::ustring& table_name, const Glib::ustring& field_name);

/** Use this, for instance, when deleting a table.
 */
void remove_auto_increment(const Glib::ustring& table_name, const Glib::ustring& field_name);

void layout_item_fill_field_details(const std::shared_ptr<const Document>& document, const Glib::ustring& parent_table_name, std::shared_ptr<LayoutItem_Field>& layout_item);


//TODO: It would be nice to use std::shared_ptr<const Relationship>& instead of std::shared_ptr<Relationship>&,
//but it does not seem possible to pass a std::shared_ptr<const Relationship> for a std::shared_ptr<const Relationship>&.
/** Decides whether a field should have an Open button next to it,
 * allowing the user to navigate to a related record.
 *
 * @param layout_item A field on a layout. This must have full field details.
 * @param field_used_in_relationship_to_one A relationship, if the field identifies a single record, so a Find button would also make sense, to choose the ID, in editing mode.
 */
bool layout_field_should_have_navigation(const Glib::ustring& table_name, const std::shared_ptr<const LayoutItem_Field>& layout_item, const std::shared_ptr<const Document>& document, std::shared_ptr<Relationship>& field_used_in_relationship_to_one);

/** Discover a database name that is not yet used.
 * This assumes that all other connection details are correctly set.
 *
 * @param base_name The wished-for name, to be modified until an unused name is found.
 * @result A database name that does not yet exist on the server.
 */
Glib::ustring get_unused_database_name(const Glib::ustring& base_name);

/** Discover how many rows a SQL query would return if it was run.
 *
 * This uses a COUNT * on a the @a sql_query as a sub-statement.
 * Be careful not to include ORDER BY clauses in the supplied SQL query, because that would make it unnecessarily slow.
 *
 * @param sql_query A SQL query.
 * @result The number of rows. Or -1 if something went wrong.
 */
int count_rows_returned_by(const Glib::RefPtr<const Gnome::Gda::SqlBuilder>& sql_query);

/** Rename a table in the database.
 */
bool rename_table(const Glib::ustring& table_name, const Glib::ustring& new_table_name);

/* Remove a table from the database.
 */
bool drop_table(const Glib::ustring& table_name);

/** Escape, and quote, SQL identifiers such as table names.
 * This requires a current connection.
 */
Glib::ustring escape_sql_id(const Glib::ustring& id);

/** Just a wrapper around gda_rfc1738_encode(),
 * for use when building libgda connection strings or authentication strings.
 */
Glib::ustring gda_cnc_string_encode(const Glib::ustring& str);

Glib::ustring build_query_create_group(const Glib::ustring& group, bool superuser = false);

Glib::ustring build_query_add_user_to_group(const Glib::ustring& group, const Glib::ustring& user);

/** Add a @a user to the database, with the specified @a password, in the specified @a group.
 * @result true if the addition succeeded.
 */
bool add_user(const std::shared_ptr<const Document>& document, const Glib::ustring& user, const Glib::ustring& password, const Glib::ustring& group);

/** Remove the @a user from the database.
 * @result true if the removal succeeded.
 */
bool remove_user(const Glib::ustring& user);

/** Add a @a group to the database.
 * @result true if the addition succeeded.
 */
bool add_group(const std::shared_ptr<const Document>& document, const Glib::ustring& group, bool superuser = false);

bool remove_user_from_group(const Glib::ustring& user, const Glib::ustring& group);

/** Get the value of the @a source_field from the @a relationship, using the @a key_value.
 */
Gnome::Gda::Value get_lookup_value(const std::shared_ptr<const Document>& document, const Glib::ustring& table_name, const std::shared_ptr<const Relationship>& relationship, const std::shared_ptr<const Field>& source_field, const Gnome::Gda::Value & key_value);

typedef std::unordered_map<Glib::ustring, Gnome::Gda::Value, std::hash<std::string>> type_map_fields;

//TODO: Performance: This is massively inefficient:
type_map_fields get_record_field_values(const std::shared_ptr<const Document>& document, const Glib::ustring& table_name, const std::shared_ptr<const Field>& primary_key, const Gnome::Gda::Value& primary_key_value);

/** Allow a fake connection, so sqlbuilder_get_full_query() can work.
 */
void set_fake_connection();

typedef std::vector<Gnome::Gda::Value> type_list_values;
typedef std::vector< std::pair<Gnome::Gda::Value, type_list_values> > type_list_values_with_second; //TODO: Rename this now that we have more than just 1 extra field.
type_list_values_with_second get_choice_values_all(const std::shared_ptr<const Document>& document, const LayoutItem_Field& field);

type_list_values_with_second get_choice_values(const std::shared_ptr<const Document>& document, const LayoutItem_Field& field, const Gnome::Gda::Value& foreign_key_value);

} //namespace DbUtils

} //namespace Glom

#endif //GLOM_DB_UTILS_H
