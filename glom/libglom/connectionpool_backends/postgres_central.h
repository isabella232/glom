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

#ifndef GLOM_BACKEND_POSTGRES_CENTRAL_H
#define GLOM_BACKEND_POSTGRES_CENTRAL_H

#include <libgdamm.h>
#include <glom/libglom/connectionpool.h>

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

namespace Glom
{

namespace ConnectionPoolBackends
{

class PostgresCentralHosted : public ConnectionPoolBackend
{
public:
  PostgresCentralHosted();

  /** 0 means any port
   * Other ports will be tried if the specified port fails.
   */
  void set_host(const Glib::ustring& value);
  void set_port(int port);
  void set_try_other_ports(bool val);

  Glib::ustring get_host() const;
  int get_port() const;
  bool get_try_other_ports() const;

  /** Return the version number of the connected postgres server.
   * This can be used to adapt to different server features.
   *
   * @result The version, or 0 if no connection has been made.
   */
  float get_postgres_server_version() const;

  /** Check whether the libgda postgres provider is really available, 
   * so we can connect to postgres servers,
   * in case the distro package has incorrect dependencies.
   *
   * @results True if everything is OK.
   */
  static bool check_postgres_gda_client_is_available_with_warning();

  static Glib::RefPtr<Gnome::Gda::Connection> attempt_connect(const Glib::RefPtr<Gnome::Gda::Client>& client, const Glib::ustring& host, const Glib::ustring& port, const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, float& postgres_server_version, std::auto_ptr<ExceptionConnection>& error);

protected:
  virtual Field::sql_format get_sql_format() const { return Field::SQL_FORMAT_POSTGRES; }
  virtual bool supports_remote_access() const { return true; }

  virtual Glib::RefPtr<Gnome::Gda::Connection> connect(const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<ExceptionConnection>& error);

  /** Creates a new database.
   */
#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual bool create_database(const Glib::ustring& database_name, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<Glib::Error>& error);
#endif

private:
  typedef std::list<Glib::ustring> type_list_ports;
  type_list_ports m_list_ports;

  Glib::RefPtr<Gnome::Gda::Client> m_refGdaClient;
  Glib::ustring m_host;
  int m_port;
  bool m_try_other_ports;

  float m_postgres_server_version;
};

} //namespace ConnectionPoolBackends

} //namespace Glom

#endif //GLOM_BACKEND_POSTGRES_CENTRAL_H

