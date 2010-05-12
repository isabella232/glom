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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLOM_DB_UTILS_H
#define GLOM_DB_UTILS_H

#include <libglom/document/document.h>
#include <libglom/data_structure/system_prefs.h>

namespace Glom
{

namespace DbUtils
{

bool create_database(Document* document, const Glib::ustring& database_name, const Glib::ustring& title, const sigc::slot<void>& progress);

bool recreate_database_from_document(Document* document, const sigc::slot<void>& progress);

/** This creates the standard tables if necessary,
 * filling them with some information from the document.
 */
SystemPrefs get_database_preferences(Document* document);

void set_database_preferences(Document* document, const SystemPrefs& prefs);

bool add_standard_tables(Document* document);

bool add_standard_groups(Document* document);

typedef std::vector< sharedptr<Field> > type_vec_fields;
type_vec_fields get_fields_for_table_from_database(const Glib::ustring& table_name, bool including_system_fields = false);
bool get_field_exists_in_database(const Glib::ustring& table_name, const Glib::ustring& field_name);

//TODO: Is this used directly?
typedef std::vector<Glib::ustring> type_vec_strings;
type_vec_strings get_table_names_from_database(bool ignore_system_tables = false);

bool get_table_exists_in_database(const Glib::ustring& table_name);

bool create_table(const sharedptr<const TableInfo>& table_info, const Document::type_vec_fields& fields);

/// Also saves the table information in the document:
bool create_table_with_default_fields(Document* document, const Glib::ustring& table_name);

bool create_table_add_missing_fields(const sharedptr<const TableInfo>& table_info, const Document::type_vec_fields& fields);

// TODO: Should these functions update the document, so callers don't need
// to do it?
bool add_column(const Glib::ustring& table_name, const sharedptr<const Field>& field, Gtk::Window* parent_window);

bool drop_column(const Glib::ustring& table_name, const Glib::ustring& field_name);


//TODO: Is this used directly?
bool insert_example_data(Document* document, const Glib::ustring& table_name);

/** Execute a SQL Select command, returning the result.
  */
Glib::RefPtr<Gnome::Gda::DataModel> query_execute_select(
  const Glib::RefPtr<const Gnome::Gda::SqlBuilder>& builder);


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

//TODO: Is this used directly?
Gnome::Gda::Value auto_increment_insert_first_if_necessary(const Glib::ustring& table_name, const Glib::ustring& field_name);

/** Get the next auto-increment value for this primary key, from the glom system table.
  * Add a row for this field in the system table if it does not exist already.
  */
Gnome::Gda::Value get_next_auto_increment_value(const Glib::ustring& table_name, const Glib::ustring& field_name);


} //namespace DbUtils

} //namespace Glom

#endif //GLOM_DB_UTILS_H
