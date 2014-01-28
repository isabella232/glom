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

#ifndef GLOM_BACKEND_POSTGRES_H
#define GLOM_BACKEND_POSTGRES_H

#include <libgdamm/connection.h>
#include <libglom/connectionpool_backends/backend.h>

#include <libglom/libglom_config.h>

namespace Glom
{

namespace ConnectionPoolBackends
{

class Postgres : public Backend
{
public:
  Postgres();

  /** Check whether the libgda postgres provider is really available,
   * so we can connect to postgres servers,
   * in case the distro package has incorrect dependencies.
   *
   * @results True if everything is OK.
   */
  static bool check_postgres_gda_client_is_available();

  /** Save a backup file, using the same directory layout as used by self-hosting.
   */
  virtual bool save_backup(const SlotProgress& slot_progress, const Glib::ustring& username, const Glib::ustring& password, const Glib::ustring& database_name);

  virtual bool convert_backup(const SlotProgress& slot_progress, const std::string& backup_data_file_path, const Glib::ustring& username, const Glib::ustring& password, const Glib::ustring& database_name);

  /** Return the quoted path to the specified PostgreSQL utility.
   */
  static std::string get_path_to_postgres_executable(const std::string& program, bool quoted = true);



private:
  virtual bool supports_remote_access() const;
  virtual Gnome::Gda::SqlOperatorType get_string_find_operator() const;
  virtual const char* get_public_schema_name() const;

  virtual bool change_columns(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const type_vec_const_fields& old_fields, const type_vec_const_fields& new_fields) throw();

protected:
  bool attempt_create_database(const SlotProgress& slot_progress, const Glib::ustring& database_name, const Glib::ustring& host, const Glib::ustring& port, const Glib::ustring& username, const Glib::ustring& password);

  /** Attempt to connect to the database with the specified criteria.
   * @throws An ExceptionConnection if the correction failed.
   */ 
  Glib::RefPtr<Gnome::Gda::Connection> attempt_connect(const Glib::ustring& port, const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, bool fake_connection);

 std::string get_self_hosting_path(bool create = false, const std::string& child_directory = std::string());

  /** Get the path to the config sub-directory, optionally creating it.
   */
  std::string get_self_hosting_config_path(bool create = false);

  /** Get the path to the data sub-directory, optionally creating it.
   */
  std::string get_self_hosting_data_path(bool create = false);

  //TODO: Remove this?
  /** Get the path to the backup file, regardless of whether it exists.
   * @param base_directory Where to find the backup file, under a normal Glom directory structure.
   * If @a base_directory is empty then it uses get_database_directory_uri().
   */
  std::string get_self_hosting_backup_path(const std::string& base_directory = std::string(), bool create_parent_dir = false);

  bool create_directory_filepath(const std::string& filepath);
  bool file_exists_filepath(const std::string& filepath);
  bool file_exists_uri(const std::string& uri) const;

  /**
   * @param current_user_only If true then only the current user will be able to read or write the file.
   */
  static bool create_text_file(const std::string& file_uri, const std::string& contents, bool current_user_only = false);

  /**
   * @param filepath_previous The path to which the previous .pgpass, if any was moved.
   * @param filepath_original The path to which filepath_previous should be moved back after the caller has finished.
   * @param result whether it succeeded.
   */
  bool save_password_to_pgpass(const Glib::ustring username, const Glib::ustring& password, std::string& filepath_previous, std::string& filepath_original);

protected:
  static Glib::ustring port_as_string(unsigned int port_num);

  Glib::ustring m_host;
  unsigned int m_port;

private:
  float m_postgres_server_version;
};

} //namespace ConnectionPoolBackends

} //namespace Glom

#endif //GLOM_BACKEND_POSTGRES_H
