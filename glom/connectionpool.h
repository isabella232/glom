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
#include "sharedptr.h"
#include "data_structure/fieldtypes.h"

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
  
  virtual const char* what();

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

/** This is a singleton.
 * Use get_instance().
 */
class ConnectionPool
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
  sharedptr<SharedConnection> connect();


  void set_host(const Glib::ustring& value);
  void set_user(const Glib::ustring& value);
  void set_password(const Glib::ustring& value);
  void set_database(const Glib::ustring& value);
  
  Glib::ustring get_host() const;
  Glib::ustring get_user() const;
  Glib::ustring get_password() const;
  Glib::ustring get_database() const;

  const FieldTypes* get_field_types() const;

protected:
  void on_sharedconnection_finished();

  Glib::RefPtr<Gnome::Gda::Client> m_GdaClient;
  //Gnome::Gda::DataSourceInfo m_GdaDataSourceInfo;


  Glib::RefPtr<Gnome::Gda::Connection> m_refGdaConnection;
  guint m_sharedconnection_refcount;
  bool m_ready_to_connect;
  Glib::ustring m_host, m_user, m_password, m_database;
  FieldTypes* m_pFieldTypes;

  static ConnectionPool* m_instance;
};


#endif //GLOM_CONNECTIONPOOL_H

