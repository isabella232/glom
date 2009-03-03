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

#ifndef GLOM_BACKEND_POSTGRES_H
#define GLOM_BACKEND_POSTGRES_H

#include <libgdamm.h>
#include <libglom/connectionpool_backends/backend.h>

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#ifndef GLOM_ENABLE_POSTGRESQL
# error The Glom <postgres.h> backend has been included even though PostgreSQL support is disabled
#endif

namespace Glom
{

namespace ConnectionPoolBackends
{

class Postgres : public Backend
{
public:
  Postgres();

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

private:
  virtual Field::sql_format get_sql_format() const { return Field::SQL_FORMAT_POSTGRES; }
  virtual bool supports_remote_access() const { return true; }
  virtual Glib::ustring get_string_find_operator() const { return "ILIKE"; } // ILIKE is a postgres extension for locale-dependent case-insensitive matches.

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual bool change_columns(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const type_vecConstFields& old_fields, const type_vecConstFields& new_fields, std::auto_ptr<Glib::Error>& error);

protected:
  bool attempt_create_database(const Glib::ustring& database_name, const Glib::ustring& host, const Glib::ustring& port, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<Glib::Error>& error);
#endif

protected:
  Glib::RefPtr<Gnome::Gda::Connection> attempt_connect(const Glib::ustring& host, const Glib::ustring& port, const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<ExceptionConnection>& error);

private:
  float m_postgres_server_version;
};

} //namespace ConnectionPoolBackends

} //namespace Glom

#endif //GLOM_BACKEND_POSTGRES_H

