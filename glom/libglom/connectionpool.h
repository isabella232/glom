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

#ifndef GLOM_CONNECTIONPOOL_H
#define GLOM_CONNECTIONPOOL_H

#include <libglom/libglom_config.h>

#include <libgdamm/connection.h>
#include <libglom/sharedptr.h>
#include <libglom/data_structure/fieldtypes.h>
#include <libglom/data_structure/field.h>
#include <libglom/connectionpool_backends/backend.h>

#include <memory> // For std::shared_ptr

//Avoid including the header here:
extern "C"
{
typedef struct _EpcPublisher EpcPublisher;
typedef struct _EpcContents EpcContents;
typedef struct _EpcAuthContext EpcAuthContext;
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
  explicit SharedConnection(const Glib::RefPtr<Gnome::Gda::Connection>& gda_connection);
  virtual ~SharedConnection();

  Glib::RefPtr<Gnome::Gda::Connection> get_gda_connection();
  Glib::RefPtr<const Gnome::Gda::Connection> get_gda_connection() const;

  /** Be careful not to use the gda_connection, or any copies of the SharedConnection after calling this.
   */
  void close();

  //TODO: Document this:
  typedef sigc::signal<void()> type_signal_finished;
  type_signal_finished signal_finished();

private:
  Glib::RefPtr<Gnome::Gda::Connection> m_gda_connection;

  type_signal_finished m_signal_finished;
};

class Document;

/** This is a singleton.
 * Use get_instance().
 */
class ConnectionPool : public sigc::trackable
{
public:

  //This are public for use by std::make_shared<>().
  //TODO: Does this need to be virtual?
  ConnectionPool();
  virtual ~ConnectionPool();

  ConnectionPool(const ConnectionPool& src) = delete;
  ConnectionPool(ConnectionPool&& src) = delete;
  ConnectionPool& operator=(const ConnectionPool& src) = delete;
  ConnectionPool& operator=(ConnectionPool&& src) = delete;

  typedef ConnectionPoolBackends::Backend Backend;
  typedef Backend::type_vec_const_fields type_vec_const_fields;

  /** Get the singleton instance.
   * Use delete_instance() when the program quits.
   */
  static std::shared_ptr<ConnectionPool> get_instance();
  
  /** Whether the connection is ready to be used.
   */ 
  static bool get_instance_is_ready();

  /** Make the ConnectionPool use the correct backend, with the necessary details,
   * as required by the document.
   */
  void setup_from_document(const std::shared_ptr<const Document>& document);

  /// Delete the singleton so it doesn't show up as leaked memory in, for instance, valgrind.
  static void delete_instance();

  typedef std::function<void()> type_void_slot;

#ifndef G_OS_WIN32
  /** Set callbacks that will be called to show UI while starting to advertise
   * on the network via Avahi.
   *
   * @param slot_begin Show an explanatory message.
   * @param slot_progress Show a pulse progress and/or keep the UI updating.
   * @param slot_done Stop showing the message.
   */
  void set_avahi_publish_callbacks(const type_void_slot& slot_begin, const type_void_slot& slot_progress, const type_void_slot& slot_done);
#endif

  bool get_ready_to_connect() const;
  void set_ready_to_connect(bool val = true);

  /** Do not actually connect to the database,
   * but create a Gnome::Gda::Connection object
   * just so that utility functions such as SqlUtils::sqlbuilder_get_full_query()
   * and DbUtils::escape_sql_id() can work.
   */
  void set_fake_connection();

  void set_backend(const std::shared_ptr<Backend>& backend);

  std::shared_ptr<Backend> get_backend();
  std::shared_ptr<const Backend> get_backend() const;
  
  /** Discover whether the backend can create GdaDataModels that can be iterated,
   * by creating them with the GDA_STATEMENT_MODEL_CURSOR_FORWARD flag.
   * If not (with sqlite, for instance), the GdaDataAccessWrapper model can provide that API, without the performance.
   */
  bool get_backend_supports_cursor() const;

  /** This method will return a SharedConnection, either by opening a new connection or returning an already-open connection.
   * When that SharedConnection is destroyed, or when SharedConnection::close() is called, then the ConnectionPool will be informed.
   * The connection will only be closed when all SharedConnections have finished with their connections.
   *
   * @result a std::shared_ptr to a SharedConnection. This std::shared_ptr will be null if the connection failed.
   *
   * @throws an ExceptionConnection when the connection fails.
   */
  std::shared_ptr<SharedConnection> connect();

  static std::shared_ptr<SharedConnection> get_and_connect();

  /** This callback should show UI to indicate that work is still happening.
   * For instance, a pulsing ProgressBar.
   */
  typedef Backend::SlotProgress SlotProgress;

  /** Creates a new database.
   */
  void create_database(const SlotProgress& slot_progress, const Glib::ustring& database_name);

  /** Save a backup of the database in a tarball.
   * This backup can later be used to recreate the database,
   * for instance with a later version of PostgreSQL.
   * See @convert_backup().
   *
   * @param path_dir The top-level directory for the backup file, using the normal directory structure.
   *
   */
  bool save_backup(const SlotProgress& slot_progress, const std::string& path_dir);

  /** Use a backup of the database in a tarball to create tables and data in an existing empty database.
   * The database (server) should already have the necessary groups and users.
   *
   * @param path_dir The path to the database-server-specific backup file.
   * See save_backup().
   */
  bool convert_backup(const SlotProgress& slot_progress, const std::string& backup_data_file_path);

  void set_user(const Glib::ustring& value);
  void set_password(const Glib::ustring& value);
  void set_database(const Glib::ustring& value);

  Glib::ustring get_user() const;
  Glib::ustring get_password() const;
  Glib::ustring get_database() const;

  std::shared_ptr<const FieldTypes> get_field_types() const;
  Gnome::Gda::SqlOperatorType get_string_find_operator() const;

  typedef Backend::InitErrors InitErrors;

  /** Do one-time initialization, such as  creating required database
   * files on disk for later use by their own  database server instance.
   *
   * @param slot_progress A callback to call while the work is still happening.
   * @param network_shared Whether the database (and document) should be available to other users over the network,
   * if possible.
   * @param parent_window A parent window to use as the transient window when displaying errors.
   */
  InitErrors initialize(const SlotProgress& slot_progress, bool network_shared = false);

  typedef Backend::StartupErrors StartupErrors;

  /** Start a database server instance for the existing database files.
   *
   * @param slot_progress A callback to call while the work is still happening.
   * @param network_shared Whether the database (and document) should be available to other users over the network,
   * if possible.
   * @param parent_window The parent window (transient for) of any dialogs shown during this operation.
   * @result Whether the operation was successful.
   */
  StartupErrors startup(const SlotProgress& slot_progress, bool network_shared = false);

  /** Stop the database server instance for the database files.
   *
   * @param slot_progress A callback to call while the work is still happening.
   * @param parent_window The parent window (transient for) of any dialogs shown during this operation.
   */
  bool cleanup(const SlotProgress& slot_progress);

  ///Whether to automatically shutdown the database server when Glom crashes.
  void set_auto_server_shutdown(bool val = true);

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
  bool set_network_shared(const SlotProgress& slot_progress, bool network_shared = true);

  /** Add a field to the database.
   * The caller should then update the document's list of fields,
   * for instance by calling Document::set_table_fields(). 
   *
   * @param table_name The parent table of the fields to be changed.
   * @param field The field to be added.
   */
  bool add_column(const Glib::ustring& table_name, const std::shared_ptr<const Field>& field) noexcept;

  /** Remove a field from the database.
   * The caller should then update the document's list of fields,
   * for instance by calling Document::set_table_fields(). 
   *
   * @param table_name The parent table of the fields to be changed.
   * @param field_name The name of the field to be removed.
   */
  bool drop_column(const Glib::ustring& table_name, const Glib::ustring& field_name) noexcept;

  /** Change some detail about a field in the database.
   * The caller should then update the document's list of fields,
   * for instance by calling Document::set_table_fields(). 
   *
   * @param table_name The parent table of the field to be changed.
   * @param field_old The old field information.
   * @param field The new field information. 
   */
  bool change_column(const Glib::ustring& table_name, const std::shared_ptr<const Field>& field_old, const std::shared_ptr<const Field>& field) noexcept;

  /** Change some detail about some fields in the database.
   * The caller should then update the document's list of fields,
   * for instance by calling Document::set_table_fields(). 
   *
   * @param table_name The parent table of the fields to be changed.
   * @param old_fields The old field information.
   * @param fields The new field information. 
   */
  bool change_columns(const Glib::ustring& table_name, const type_vec_const_fields& old_fields, const type_vec_const_fields& fields) noexcept;

  bool update_meta_store_for_table_names();

  /** Specify a callback that the ConnectionPool can call to get a pointer to the document.
   * This callback avoids Connection having to link to AppWindow,
   * and avoids us worrying about whether a previously-set document (via a set_document() method) is still valid.
   */
  typedef std::function<std::shared_ptr<Document>()> SlotGetDocument;
  void set_get_document_func(const SlotGetDocument& slot);

#ifndef G_OS_WIN32
  static EpcContents* on_publisher_document_requested (EpcPublisher* publisher, const gchar* key, gpointer user_data);
  static gboolean on_publisher_document_authentication(EpcAuthContext* context, const gchar* user_name, gpointer user_data);

  static void on_epc_progress_begin(const gchar *title, gpointer user_data);
  static void on_epc_progress_update(gdouble progress, const gchar* message, gpointer user_data);
  static void on_epc_progress_end(gpointer user_data);
#endif // !G_OS_WIN32

  //TODO: Document
  void set_show_debug_output(bool val);
  bool get_show_debug_output() const;

  //Show the gda error in a dialog.
  static bool handle_error_cerr_only();

private:
  bool connect_nothrow();

  void on_sharedconnection_finished();

  /** We call this when we know that the current connection will no longer work,
   * for instance after we have stopped the server.
   */
  void invalidate_connection();

  /** Examine ports one by one, starting at @a starting_port, in increasing order,
   * and return the first one that is available.
   */
  //static int discover_first_free_port(int start_port, int end_port);

  std::shared_ptr<Document> get_document();

#ifndef G_OS_WIN32
  /** Advertize self-hosting via avahi:
   */
  void avahi_start_publishing();
  void avahi_stop_publishing();
#endif // !G_OS_WIN32

private:

  EpcPublisher* m_epc_publisher;

  std::shared_ptr<Backend> m_backend;
  Glib::RefPtr<Gnome::Gda::Connection> m_gda_connection;
  guint m_sharedconnection_refcount;
  bool m_ready_to_connect;
  Glib::ustring m_user, m_password, m_database;

  std::shared_ptr<FieldTypes> m_field_types;
  bool m_show_debug_output, m_auto_server_shutdown;

  bool m_fake_connection;

private:

  static std::shared_ptr<ConnectionPool> m_instance;
  SlotGetDocument m_slot_get_document;

  type_void_slot m_epc_slot_begin, m_epc_slot_progress, m_epc_slot_done;
};

} //namespace Glom

#endif //GLOM_CONNECTIONPOOL_H
