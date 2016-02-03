/* Glom
 *
 * Copyright (C) 2011 Murray Cumming
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

#ifndef GLOM_TEST_SELFHOSTING_UTILS_H
#define GLOM_TEST_SELFHOSTING_UTILS_H

#include <libglom/document/document.h>
#include <libgdamm/datamodel.h>
#include <string>

/** Create a .glom file from an example, with database data, and start a PostgreSQL server if necessary.
 *
 * @param document A new empty document that will be filled with hosting details.
 * @param hosting_mode Either HostingMode::POSTGRES_SELF or HostingMode::SQLITE
 * @param subdirectory_path: An additional directory path to use under the temporary directory that will be used to save the file.
 */
bool test_create_and_selfhost_new_empty(const std::shared_ptr<Glom::Document>& document, Glom::Document::HostingMode hosting_mode, const std::string& subdirectory_path = std::string());

/** Create a .glom file from an example, with database data, and start a PostgreSQL server if necessary.
 *
 * @param document A new empty document that will be filled with hosting details.
 * @param hosting_mode Either HostingMode::POSTGRES_SELF or HostingMode::SQLITE
 * @param database_name The name of the database to created.
 * @param subdirectory_path: An additional directory path to use under the temporary directory that will be used to save the file.
 */
bool test_create_and_selfhost_new_database(const std::shared_ptr<Glom::Document>& document, Glom::Document::HostingMode hosting_mode, const Glib::ustring& database_name,  const std::string& subdirectory_path = std::string());

/** Create a .glom file from an example, with database data, and start a PostgreSQL server if necessary.
 *
 * @param example_filename The filename (not the full path) of the example .glom file.
 * @param hosting_mode Either HostingMode::POSTGRES_SELF or HostingMode::SQLITE
 * @param subdirectory_path: An additional directory path to use under the temporary directory that will be used to save the file.
 */
bool test_create_and_selfhost_from_example(const std::string& example_filename, const std::shared_ptr<Glom::Document>& document, Glom::Document::HostingMode hosting_mode, const std::string& subdirectory_path = std::string());

/** Create a .glom file from a test example, with database data, and start a PostgreSQL server if necessary.
 *
 * @param example_filename The filename (not the full path) of the example .glom file.
 * @param hosting_mode Either HostingMode::POSTGRES_SELF or HostingMode::SQLITE
 */
bool test_create_and_selfhost_from_test_example(const std::string& example_filename, const std::shared_ptr<Glom::Document>& document, Glom::Document::HostingMode hosting_mode);


/** Create a .glom file from an existing .glom example file with database data, and start a PostgreSQL server if necessary.
 *
 * @param file_uri The full URI of the example .glom file.
 * @param hosting_mode Either HostingMode::POSTGRES_SELF or HostingMode::SQLITE
 * @param subdirectory_path: An additional directory path to use under the temporary directory that will be used to save the file.
 */
bool test_create_and_selfhost_from_uri(const Glib::ustring& example_file_uri, const std::shared_ptr<Glom::Document>& document, Glom::Document::HostingMode hosting_mode, const std::string& subdirectory_path = std::string());

/** Create a .glom file from an existing .glom example file with database data, and start a PostgreSQL server if necessary.
 *
 * @param file_uri The full URI of the example .glom file.
 * @param hosting_mode Either HostingMode::POSTGRES_SELF or HostingMode::SQLITE
 * @param subdirectory_path: An additional directory path to use under the temporary directory that will be used to save the file.
 */
bool test_create_and_selfhost_from_data(const Glib::ustring& example_file_contents, const std::shared_ptr<Glom::Document>& document, Glom::Document::HostingMode hosting_mode, const std::string& subdirectory_path = std::string());

/** Start self-hosting of a .glom document.
 * @param document The document must already be saved to a file.
 */
bool test_selfhost(const std::shared_ptr<Glom::Document>& document, const Glib::ustring& user, const Glib::ustring& password);


bool test_model_expected_size(const Glib::RefPtr<const Gnome::Gda::DataModel>& data_model, guint columns_count, guint rows_count);
bool test_table_exists(const Glib::ustring& table_name, const std::shared_ptr<Glom::Document>& document);

/** Return the URI of the temporary .glom file created by the test_create_and_selfhost_*() methods.
 * This should only be used by some special tests.
 */
Glib::ustring test_get_temp_file_uri();

/** Stop the self-hosting server process,
 * and (optionally) delete the temporary .glom file and its data.
 */
void test_selfhosting_cleanup(bool delete_file = true);

bool test_example_musiccollection_data(const std::shared_ptr<const Glom::Document>& document);

typedef sigc::slot<bool, Glom::Document::HostingMode> SlotTest;

/** Call the test @a slot with various hosting modes.
 * @result A result code for main().
 */
int test_all_hosting_modes(const SlotTest& slot);

/** Return true if the Value does not have an expected numeric type.
 */
bool test_check_numeric_value_type(Glom::Document::HostingMode hosting_mode, const Gnome::Gda::Value& value);

#endif //GLOM_TEST_SELFHOSTING_UTILS_H

