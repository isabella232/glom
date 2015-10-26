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

#ifndef GLOM_BACKEND_SQLITE_H
#define GLOM_BACKEND_SQLITE_H

#include <libgdamm/connection.h>
#include <libglom/connectionpool_backends/backend.h>

#include <libglom/libglom_config.h>

namespace Glom
{

namespace ConnectionPoolBackends
{

class Sqlite : public Backend
{
public:
  Sqlite();

private:
  bool supports_remote_access() const override;
  Gnome::Gda::SqlOperatorType get_string_find_operator() const override;
  const char* get_public_schema_name() const override;

  bool add_column_to_server_operation(const Glib::RefPtr<Gnome::Gda::ServerOperation>& operation, GdaMetaTableColumn* column, unsigned int i);
  bool add_column_to_server_operation(const Glib::RefPtr<Gnome::Gda::ServerOperation>& operation, const std::shared_ptr<const Field>& column, unsigned int i);

  typedef std::vector<Glib::ustring> type_vec_strings;
  //typedef std::vector<std::shared_ptr<const Field> > type_vec_fields;
  typedef std::map<Glib::ustring, std::shared_ptr<const Field> > type_mapFieldChanges;

  bool recreate_table(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const type_vec_strings& fields_removed, const type_vec_const_fields& fields_added, const type_mapFieldChanges& fields_changed) noexcept;

  bool add_column(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const std::shared_ptr<const Field>& field) override;
  bool drop_column(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const Glib::ustring& field_name) override;
  bool change_columns(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const type_vec_const_fields& old_fields, const type_vec_const_fields& new_fields) noexcept override;

  Glib::RefPtr<Gnome::Gda::Connection> connect(const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, bool fake_connection = false) override;

  /** Creates a new database.
   */
  bool create_database(const SlotProgress& slot_progress, const Glib::ustring& database_name, const Glib::ustring& username, const Glib::ustring& password) override;

  bool save_backup(const SlotProgress& slot_progress, const Glib::ustring& username, const Glib::ustring& password, const Glib::ustring& database_name) override;
  bool convert_backup(const SlotProgress& slot_progress, const std::string& backup_data_file_path, const Glib::ustring& username, const Glib::ustring& password, const Glib::ustring& database_name) override;
};

} //namespace ConnectionPoolBackends

} //namespace Glom

#endif //GLOM_BACKEND_SQLITE_H
