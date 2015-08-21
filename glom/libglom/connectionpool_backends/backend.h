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

#ifndef GLOM_BACKEND_BACKEND_H
#define GLOM_BACKEND_BACKEND_H

#include <libglom/libglom_config.h>

#include <libglom/sharedptr.h>
#include <libglom/data_structure/field.h>

#include <memory>
#include <functional>

namespace Glom
{

class ConnectionPool;

class ExceptionConnection : public std::exception
{
public:
  enum class failure_type
  {
    NO_SERVER, //Either there was no attempt to connect to a specific database, or the connection failed both with and without specifying the database.
    NO_DATABASE, //Connection without specifying the database was possible.
    NO_BACKEND //No backend instance available. Should never happen.
  };

  ExceptionConnection(failure_type failure);
  virtual ~ExceptionConnection() noexcept;

  virtual const char* what() const noexcept;

  virtual failure_type get_failure_type() const;

private:
  failure_type m_failure_type;
};

namespace ConnectionPoolBackends
{

/** This hides database specific functionality from the ConnectionPool, so
 * the ConnectionPool can be used without worrying about the actual database
 * backend in use. Use ConnectionPool::set_backend() to set the backend for
 * the connectionpool to use. */
class Backend
{
  friend class Glom::ConnectionPool;
public:
  virtual ~Backend() {}
  typedef std::vector<std::shared_ptr<const Field> > type_vec_const_fields;

  enum class InitErrors
  {
     NONE,
     DIRECTORY_ALREADY_EXISTS,
     COULD_NOT_CREATE_DIRECTORY,
     COULD_NOT_START_SERVER,
     OTHER
  };

  enum class StartupErrors
  {
    NONE, /*< The database is ready for use. */
    FAILED_NO_DATA, /*< There is no data for the database. */
    FAILED_NO_DATA_HAS_BACKUP_DATA, /*< There is no data for the database, but there is a backup file instead. */
    FAILED_NO_MAIN_DIRECTORY, /*< The main directory (containing data and config directories) could not be found. */
    FAILED_NO_PORT_AVAILABLE, /*< There was no network port available in the normal range of ports. */
    FAILED_UNKNOWN_REASON /*< Something else failed. */
  };

protected:

  /* TODO: Merge create_database() and initialize() into a single function?
   */

  /** Whether the database can be accessed from remote machines, once startup()
   * was called.
   */
  virtual bool supports_remote_access() const = 0;

  /** The operator to use to compare strings in a case-insensitive way. This
   * is backend-dependent. For example, postgres uses ILIKE but SQLite uses
   * LIKE.
   */
  virtual Gnome::Gda::SqlOperatorType get_string_find_operator() const = 0;

  /** This specifies the database schema which contains the non-internal
   * tables. This is used to speedup the libgda meta store update by only
   * updating the non-internal tables. libgda might later be able to do this
   * without us specifying it explicitely. See #575235.
   */
  virtual const char* get_public_schema_name() const = 0;

  /** This specifies that Glom should start its own database server instance (if it's PostgreSQL)
   * for this database, using the database files stored at the specified uri,
   * or just use that file (if it's sqlite).
   * Or it can be used temporarily when calling save_backup() to provide the top-level directory path.
   */
  void set_database_directory_uri(const std::string& directory_uri);
  std::string get_database_directory_uri() const;

  /** This callback should show UI to indicate that work is still happening.
   * For instance, a pulsing ProgressBar.
   */
  typedef std::function<void()> SlotProgress;

  /** This method is called for one-time initialization of the database
   * storage. There is no need to implement this function if the data is centrally
   * hosted rather than hosted by Glom.
   *
   * @param slot_progress A callback to call while the work is still happening.
   * @param network_shared Whether the database (and document) should be available to other users over the network,
   * if possible.
   */
  virtual InitErrors initialize(const SlotProgress& slot_progress, const Glib::ustring& initial_username, const Glib::ustring& password, bool network_shared = false);

  /** This method is called before the backend is used otherwise. This can
   * be used to start a self-hosted database server. There is no need to implement
   * this function if there is no need for extra startup code.
   *
   * @param slot_progress A callback to call while the work is still happening.
   * @param network_shared Whether the database (and document) should be available to other users over the network,
   * if possible.
   */
  virtual StartupErrors startup(const SlotProgress& slot_progress, bool network_shared = false);

  /** This method is called when the backend is no longer used. This can be
   * used to shut down a self-hosted database server. There is no need to
   * implement this function if there is no need for extra cleanup code.
   *
   * @param slot_progress A callback to call while the work is still happening.
   */
  virtual bool cleanup(const SlotProgress& slot_progress);

  /** Change the database server's configration to allow or prevent access from
   * other users on the network.
   *
   * For current backends, you may use this only before startup(),
   * or after cleanup().
   *
   * @param slot_progress A callback to call while the work is still happening.
   * @param network_shared Whether the database (and document) should be available to other users over the network,
   * if possible.
   */
  virtual bool set_network_shared(const SlotProgress& slot_progress, bool network_shared = true);

  /** This method is called to create a connection to the database server.
   *
   * @param fake_connection Whether the connection should not actually be opened.
   *
   * @throws An ExceptionConnection if the correction failed.
   */
  virtual Glib::RefPtr<Gnome::Gda::Connection> connect(const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, bool fake_connection = false) = 0;

  /** @throws Glib::Error (from libgdamm)
   */
  virtual bool add_column(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const std::shared_ptr<const Field>& field);

  /** @throws Glib::Error (from libgdamm)
   */
  virtual bool drop_column(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const Glib::ustring& field_name);

  virtual bool change_columns(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const type_vec_const_fields& old_fields, const type_vec_const_fields& new_fields) noexcept = 0;

  /** This method is called to create a new database on the
   * database server. */
  virtual bool create_database(const SlotProgress& slot_progress, const Glib::ustring& database_name, const Glib::ustring& username, const Glib::ustring& password) = 0;

  /** Save a backup of the database in a tarball.
   * This backup can later be used to recreate the database,
   * for instance with a later version of PostgreSQL.
   */
  virtual bool save_backup(const SlotProgress& slot_progress, const Glib::ustring& username, const Glib::ustring& password, const Glib::ustring& database_name) = 0;

  /** Use a backup of the database in a tarball to create the tables and data in an existing empty database.
   * The database (server) should already have the necessary groups and users.
   * See save_backup().
   */
  virtual bool convert_backup(const SlotProgress& slot_progress, const std::string& backup_data_file_path, const Glib::ustring& username, const Glib::ustring& password, const Glib::ustring& database_name) = 0;

protected:
  std::string m_database_directory_uri;
};

} // namespace ConnectionPoolBackends

} //namespace Glom

#endif // GLOM_BACKEND_BACKEND_H
