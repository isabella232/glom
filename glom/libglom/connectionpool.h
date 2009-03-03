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

#ifndef GLOM_CONNECTIONPOOL_H
#define GLOM_CONNECTIONPOOL_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <libgdamm.h>
#include <libglom/sharedptr.h>
#include <libglom/data_structure/fieldtypes.h>
#include <libglom/data_structure/field.h>
#include <libglom/connectionpool_backends/backend.h>

#include <memory> // For std::auto_ptr

//Avoid including the header here:
extern "C"
{
typedef struct _EpcPublisher EpcPublisher;
typedef struct _EpcContents EpcContents;
typedef struct _EpcAuthContext EpcAuthContext;
}

namespace Gtk
{
  class Window;
  class Dialog;
}

namespace Glom
{

class AvahiPublisher;

/** When the SharedConnection is destroyed, it will inform the connection pool,
 * so that the connection pool can keep track of who is using the connection,
 * so that it can close it when nobody is using it.
 */
class SharedConnection : public sigc::trackable
{
public:
  SharedConnection();
  SharedConnection(const Glib::RefPtr<Gnome::Gda::Connection>& gda_connection);
  virtual ~SharedConnection();

  Glib::RefPtr<Gnome::Gda::Connection> get_gda_connection();
  Glib::RefPtr<const Gnome::Gda::Connection> get_gda_connection() const;

  /** Be careful not to use the gda_connection, or any copies of the SharedConnection after calling this.
   */
  void close();

  typedef sigc::signal<void> type_signal_finished;
  type_signal_finished signal_finished();

private:
  Glib::RefPtr<Gnome::Gda::Connection> m_gda_connection;

  type_signal_finished m_signal_finished;
};

class Document_Glom;

/** This is a singleton.
 * Use get_instance().
 */
class ConnectionPool : public sigc::trackable
{
private:
  ConnectionPool();
  //ConnectionPool(const ConnectionPool& src);
  virtual ~ConnectionPool();
  //ConnectionPool& operator=(const ConnectionPool& src);

public:
  typedef ConnectionPoolBackends::Backend Backend;
  typedef Backend::type_vecConstFields type_vecConstFields;

  /** Get the singleton instance.
   * Use delete_instance() when the program quits.
   */
  static ConnectionPool* get_instance();

  /// Delete the singleton so it doesn't show up as leaked memory in, for instance, valgrind.
  static void delete_instance();

  bool get_ready_to_connect() const;
  void set_ready_to_connect(bool val = true);

  void set_backend(std::auto_ptr<Backend> backend);

  Backend* get_backend();
  const Backend* get_backend() const;

  /** This method will return a SharedConnection, either by opening a new connection or returning an already-open connection.
   * When that SharedConnection is destroyed, or when SharedConnection::close() is called, then the ConnectionPool will be informed.
   * The connection will only be closed when all SharedConnections have finished with their connections.
   *
   * @result a sharedptr to a SharedConnection. This sharedptr will be null if the connection failed.
   *
   * @throws an ExceptionConnection when the connection fails.
   */
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  sharedptr<SharedConnection> connect();
#else
  sharedptr<SharedConnection> connect(std::auto_ptr<ExceptionConnection>& error);
#endif

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  static sharedptr<SharedConnection> get_and_connect();
#else
  static sharedptr<SharedConnection> get_and_connect(std::auto_ptr<ExceptionConnection>& error);
#endif

  /** Creates a new database.
   */
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  void create_database(const Glib::ustring& database_name);
#else
  void create_database(const Glib::ustring& database_name, std::auto_ptr<Glib::Error>& error);
#endif

  void set_user(const Glib::ustring& value);
  void set_password(const Glib::ustring& value);
  void set_database(const Glib::ustring& value);

  Glib::ustring get_user() const;
  Glib::ustring get_password() const;
  Glib::ustring get_database() const;

  Field::sql_format get_sql_format() const;
  const FieldTypes* get_field_types() const;
  Glib::ustring get_string_find_operator() const;

  /** Do one-time initialization, such as  creating required database
   * files on disk for later use by their own  database server instance.
   * @param parent_window A parent window to use as the transient window when displaying errors.
   */
  bool initialize(Gtk::Window* parent_window);

  /** Start a database server instance for the exisiting database files.
   * @param parent_window The parent window (transient for) of any dialogs shown during this operation.
   * @result Whether the operation was successful.
   */
  bool startup(Gtk::Window* parent_window);

  /** Stop the database server instance for the database files.
   * @param parent_window The parent window (transient for) of any dialogs shown during this operation.
   */
  void cleanup(Gtk::Window* parent_window);

#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  bool add_column(const Glib::ustring& table_name, const sharedptr<const Field>& field);
#else
  bool add_column(const Glib::ustring& field_name, const sharedptr<const Field>& field, std::auto_ptr<Glib::Error>& error);
#endif

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  bool drop_column(const Glib::ustring& table_name, const Glib::ustring& field_name);
#else
  bool drop_column(const Glib::ustring& table_name, const Glib::ustring& field_name, std::auto_ptr<Glib::Error>& error);
#endif

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  bool change_column(const Glib::ustring& table_name, const sharedptr<const Field>& field_old, const sharedptr<const Field>& field);
#else
  bool change_column(const Glib::ustring& table_name, const sharedptr<const Field>& field_old, const sharedptr<const Field>& field, std::auto_ptr<Glib::Error>& error);
#endif

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  bool change_columns(const Glib::ustring& table_name, const type_vecConstFields& old_fields, const type_vecConstFields& fields);
#else
  bool change_columns(const Glib::ustring& table_name, const type_vecConstFields& old_fields, const type_vecConstFields& fields, std::auto_ptr<Glib::Error>& error);
#endif
#endif // !GLOM_ENABLE_CLIENT_ONLY

  /** Specify a callback that the ConnectionPool can call to get a pointer to the document.
   * This callback avoids Connection having to link to App_Glom,
   * and avoids us worrying about whether a previously-set document (via a set_document() method) is still valid.
   */ 
  typedef sigc::slot<Document_Glom*> SlotGetDocument; 
  void set_get_document_func(const SlotGetDocument& slot);

#ifndef G_OS_WIN32
  static EpcContents* on_publisher_document_requested (EpcPublisher* publisher, const gchar* key, gpointer user_data);
  static gboolean on_publisher_document_authentication(EpcAuthContext* context, const gchar* user_name, gpointer user_data);

  static void on_epc_progress_begin(const gchar *title, gpointer user_data);
  static void on_epc_progress_update(gdouble progress, const gchar* message, gpointer user_data);
  static void on_epc_progress_end(gpointer user_data);
#endif // !G_OS_WIN32

  //Show the gda error in a dialog.
  static bool handle_error(bool cerr_only = false);

  /** Postgres can't be started as root. initdb complains.
   * So just prevent this in general. It is safer anyway.
   */
  static bool check_user_is_not_root();

private:
  void on_sharedconnection_finished();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  /** Examine ports one by one, starting at @a starting_port, in increasing order,
   * and return the first one that is available.
   */
  //static int discover_first_free_port(int start_port, int end_port);

  Document_Glom* get_document();

#ifndef G_OS_WIN32
  /** Advertize self-hosting via avahi:
   */
  void avahi_start_publishing();
  void avahi_stop_publishing();
#endif // !G_OS_WIN32
#endif // !GLOM_ENABLE_CLIENT_ONLY

private:

#ifndef GLOM_ENABLE_CLIENT_ONLY
  EpcPublisher* m_epc_publisher;
  Gtk::Dialog* m_dialog_epc_progress; //For progress while generating certificates.
#endif // !GLOM_ENABLE_CLIENT_ONLY

  std::auto_ptr<Backend> m_backend;
  Glib::RefPtr<Gnome::Gda::Connection> m_refGdaConnection;
  guint m_sharedconnection_refcount;
  bool m_ready_to_connect;
  Glib::ustring m_host, m_user, m_password, m_database;

  FieldTypes* m_pFieldTypes;

private:

  static ConnectionPool* m_instance;
#ifndef GLOM_ENABLE_CLIENT_ONLY
  SlotGetDocument m_slot_get_document;
#endif
};

} //namespace Glom

#endif //GLOM_CONNECTIONPOOL_H

