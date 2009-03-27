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

#ifndef GLOM_BACKEND_BACKEND_H
#define GLOM_BACKEND_BACKEND_H

#include <libglom/libglom_config.h> // For GLOM_ENABLE_CLIENT_ONLY

#include <libglom/sharedptr.h>
#include <libglom/data_structure/field.h>

#include <gtkmm/window.h>

#include <memory>

namespace Glom
{

class ConnectionPool;

class ExceptionConnection : public std::exception
{
public:
  enum failure_type
  {
    FAILURE_NO_SERVER, //Either there was no attempt to connect to a specific database, or the connection failed both with and without specifying the database.
    FAILURE_NO_DATABASE //Connection without specifying the database was possible.
  };

  ExceptionConnection(failure_type failure);
  virtual ~ExceptionConnection() throw();

  virtual const char* what() const throw();

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
  typedef std::vector<sharedptr<const Field> > type_vecConstFields;

protected:
  /** Helper functions for backend implementations to use, so that these don't
   * need to worry whether glibmm was compiled with exceptions or not.
   */
  bool query_execute(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& sql_query, std::auto_ptr<Glib::Error>& error);
  bool set_server_operation_value(const Glib::RefPtr<Gnome::Gda::ServerOperation>& operation, const Glib::ustring& path, const Glib::ustring& value, std::auto_ptr<Glib::Error>& error);
  Glib::RefPtr<Gnome::Gda::ServerOperation> create_server_operation(const Glib::RefPtr<Gnome::Gda::ServerProvider>& provider, const Glib::RefPtr<Gnome::Gda::Connection>& connection, Gnome::Gda::ServerOperationType type, std::auto_ptr<Glib::Error>& error);
  bool perform_server_operation(const Glib::RefPtr<Gnome::Gda::ServerProvider>& provider, const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::RefPtr<Gnome::Gda::ServerOperation>& operation, std::auto_ptr<Glib::Error>& error);
  bool begin_transaction(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& name, Gnome::Gda::TransactionIsolation level, std::auto_ptr<Glib::Error>& error);
  bool commit_transaction(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& name, std::auto_ptr<Glib::Error>& error);
  bool rollback_transaction(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& name, std::auto_ptr<Glib::Error>& error);

  /* TODO: Merge create_database() and initialize() into a single function?
   */

  /** This method specifies the format of values in SQL expressions.
   */
  virtual Field::sql_format get_sql_format() const = 0;

  /** Whether the database can be accessed from remote machines, once startup()
   * was called.
   */
  virtual bool supports_remote_access() const = 0;

  /** The operator to use to compare strings in a case-independant way. This
   * is backend-depandent. For example, postgres uses ILIKE but SQLite uses
   * LIKE.
   * TODO: Maybe we can use libgda to construct the expression, so we don't
   * need this function.
   */
  virtual Glib::ustring get_string_find_operator() const = 0;

  /** This callback should show UI to indicate that work is still happening.
   * For instance, a pulsing ProgressBar.
   */
  typedef sigc::slot<void> SlotProgress;

  /** This method is called for one-time initialization of the database
   * storage. No need to implement this function if the data is centrally
   * hosted, not managed by Glom.
   *
   * @slot_progress A callback to call while the work is still happening.
   */
  virtual bool initialize(const SlotProgress& slot_progress, const Glib::ustring& initial_username, const Glib::ustring& password);

  /** This method is called before the backend is used otherwise. This can
   * be used to start a self-hosted database server. There is no need to implement
   * this function if there is no need for extra startup code.
   *
   * @slot_progress A callback to call while the work is still happening.
   */
  virtual bool startup(const SlotProgress& slot_progress);

  /** This method is called when the backend is no longer used. This can be
   * used to shut down a self-hosted database server. There is no need to
   * implement this function if there is no need for extra cleanup code.
   *
   * @slot_progress A callback to call while the work is still happening.
   */
  virtual void cleanup(const SlotProgress& slot_progress);

  /** This method is called to create a connection to the database server.
   * There exists only the variant with an error variable as last parameter
   * so we don't need #ifdefs all over the code. This part of the API is only
   * used by the ConnectionPool which will translate the error back into
   * an exception in case exceptions are enabled.
    */
  virtual Glib::RefPtr<Gnome::Gda::Connection> connect(const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<ExceptionConnection>& error) = 0;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual bool add_column(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const sharedptr<const Field>& field, std::auto_ptr<Glib::Error>& error);

  virtual bool drop_column(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const Glib::ustring& field_name, std::auto_ptr<Glib::Error>& error);

  virtual bool change_columns(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const type_vecConstFields& old_fields, const type_vecConstFields& new_fields, std::auto_ptr<Glib::Error>& error);

  /** This method is called to create a new database on the
   * database server. */
  virtual bool create_database(const Glib::ustring& database_name, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<Glib::Error>& error) = 0;
#endif
};

} // namespace ConnectionPoolBackends

} //namespace Glom

#endif // GLOM_BACKEND_BACKEND_H

