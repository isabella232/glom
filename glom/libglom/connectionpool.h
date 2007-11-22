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

#include <libgdamm.h>
#include <glom/libglom/sharedptr.h>
#include <glom/libglom/data_structure/fieldtypes.h>

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

//Avoid including the header here:
extern "C"
{
typedef struct _EpcPublisher EpcPublisher;
typedef struct _EpcContents EpcContents;
}

namespace Gtk
{
  class Window;
}

namespace Glom
{

class AvahiPublisher;

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

protected:
  failure_type m_failure_type;
};

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

protected:
  Glib::RefPtr<Gnome::Gda::Connection> m_gda_connection;

  type_signal_finished m_signal_finished;
};

class Document_Glom;

/** This is a singleton.
 * Use get_instance().
 */
class ConnectionPool : public sigc::trackable
{
protected:
  ConnectionPool();
  //ConnectionPool(const ConnectionPool& src);
  virtual ~ConnectionPool();
  //ConnectionPool& operator=(const ConnectionPool& src);

public:

  static ConnectionPool* get_instance();

  bool get_ready_to_connect() const;
  void set_ready_to_connect(bool val = true);

  /** This method will return a SharedConnection, either by opening a new connection or returning an already-open connection.
   * When that SharedConnection is destroyed, or when SharedConnection::close() is called, then the ConnectionPool will be informed.
   * The connection will only be closed when all SharedConnections have finished with their connections.
   *
   * @result a shareptr to a SharedConnection. This sharedptr will be null if the connection failed.
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

#ifndef GLOM_ENABLE_CLIENT_ONLY
  /** This specifies that Glom should start its own database server instance for this database,
   *  using the database files stored at the specified uri.
   */
  void set_self_hosted(const std::string& data_uri);
#endif //GLOM_ENABLE_CLIENT_ONLY

  /** Creates a new database.
   */
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  void create_database(const Glib::ustring& database_name);
#else
  void create_database(const Glib::ustring& database_name, std::auto_ptr<Glib::Error>& error);
#endif

  void set_host(const Glib::ustring& value);
  void set_user(const Glib::ustring& value);
  void set_password(const Glib::ustring& value);
  void set_database(const Glib::ustring& value);

  Glib::ustring get_host() const;
  //Glib::ustring get_port() const;
  Glib::ustring get_user() const;
  Glib::ustring get_password() const;
  Glib::ustring get_database() const;

  const FieldTypes* get_field_types() const;

  /** Return the version number of the connected postgres server.
   * This can be used to adapt to different server features.
   *
   * @result The version, or 0 if no connection has been made.
   */
  float get_postgres_server_version();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  /** Start a database server instance for the exisiting database files.
   */
  bool start_self_hosting();

  /** Stop the database server instance for the database files.
   */
  void stop_self_hosting();

  /** Create new database files, for later use by their own  database server instance.
   * @param parent_window A parent window to use as the transient window when displaying errors.
   */
  bool create_self_hosting(Gtk::Window* parent_window);

  /** Specify a callback that the ConnectionPool can call to get a pointer to the document.
   * This callback avoids Connection having to link to App_Glom,
   * and avoids us worrying about whether a previously-set document (via a set_document() method) is still valid.
   */ 
  typedef sigc::slot<Document_Glom*> SlotGetDocument; 
  void set_get_document_func(const SlotGetDocument& slot);

  static EpcContents* on_publisher_document_requested (EpcPublisher* publisher, const gchar* key, gpointer user_data);

  /** Check whether PostgreSQL is really available for self-hosting,
   * in case the distro package has incorrect dependencies.
   *
   * @results True if everything is OK.
   */
  static bool check_postgres_is_available_with_warning();

  /** Try to install postgres on the distro, though this will require a distro-specific 
   * patch to the implementation.
   */
  static bool install_postgres(Gtk::Window* parent_window);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  //Show the gda error in a dialog.
  static bool handle_error(bool cerr_only = false);

  /** Check whether the libgda postgres provider is really available, 
   * so we can connect to postgres servers,
   * in case the distro package has incorrect dependencies.
   *
   * @results True if everything is OK.
   */
  static bool check_postgres_gda_client_is_available_with_warning();

  /** Postgres can't be started as root. initdb complains.
   * So just prevent this in general. It is safer anyway.
   */
  static bool check_user_is_not_root();

protected:
  void on_sharedconnection_finished();

  static bool create_text_file(const std::string& file_uri, const std::string& contents);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  /** Examine ports one by one, starting at @a starting_port, in increasing order,
   * and return the first one that is available.
   */
  static int discover_first_free_port(int start_port, int end_port);

  /** Advertize self-hosting via avahi:
   */
  void avahi_start_publishing();
  void avahi_stop_publishing();
#endif // !GLOM_ENABLE_CLIENT_ONLY

protected:

  //bool directory_exists_filepath(const std::string& filepath);
  bool directory_exists_uri(const std::string& uri);

  typedef std::list<Glib::ustring> type_list_ports;
  type_list_ports m_list_ports; //Network ports on which to try connecting to postgres.

  Glib::RefPtr<Gnome::Gda::Client> m_GdaClient;
  //Gnome::Gda::DataSourceInfo m_GdaDataSourceInfo;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  bool m_self_hosting_active;
  std::string m_self_hosting_data_uri;

  EpcPublisher* m_epc_publisher;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  Glib::RefPtr<Gnome::Gda::Connection> m_refGdaConnection;
  guint m_sharedconnection_refcount;
  bool m_ready_to_connect;
  Glib::ustring m_host, m_user, m_password, m_database, m_port;
  FieldTypes* m_pFieldTypes;
  float m_postgres_server_version;

private:

  static ConnectionPool* m_instance;

  SlotGetDocument m_slot_get_document;
};

} //namespace Glom

#endif //GLOM_CONNECTIONPOOL_H

