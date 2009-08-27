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

#include <libglom/connectionpool_backends/postgres_central.h>
#include <libglom/libglom_config.h>
#include <glibmm/i18n.h>

// Uncomment to see debug messages
//#define GLOM_CONNECTION_DEBUG

namespace
{

static Glib::ustring port_as_string(int port_num)
{
  Glib::ustring result;
  char* cresult = g_strdup_printf("%d", port_num);
  if(cresult)
    result = cresult;
  g_free(cresult);

  return result;
}

}

namespace Glom
{

namespace ConnectionPoolBackends
{

PostgresCentralHosted::PostgresCentralHosted()
:
  m_port(0),
  m_try_other_ports(true)
{
  m_list_ports.push_back("5432"); //Ubuntu Breezy seems to default to this for Postgres 7.4, and this is probably the default for most postgres installations, including Fedora.

  m_list_ports.push_back("5433"); //Ubuntu Dapper seems to default to this for Postgres 8.1, probably to avoid a clash with Postgres 7.4

  m_list_ports.push_back("5434"); //Earlier versions of Ubuntu Feisty defaulted to this for Postgres 8.2.
  m_list_ports.push_back("5435"); //In case Ubuntu increases the port number again in future.
  m_list_ports.push_back("5436"); //In case Ubuntu increases the port number again in future.
}

void PostgresCentralHosted::set_host(const Glib::ustring& value)
{
  if(value != m_host)
  {
    m_host = value;

    // Force us to try all ports again when connecting for the first time, then remember the working port again. Except when a specific port was set to be used.
    if(m_try_other_ports)
      m_port = 0;
  }
}

void PostgresCentralHosted::set_port(int port)
{
  m_port = port;
}

void PostgresCentralHosted::set_try_other_ports(bool val)
{
  m_try_other_ports = val;
}

Glib::ustring PostgresCentralHosted::get_host() const
{
  return m_host;
}

int PostgresCentralHosted::get_port() const
{
  return m_port;
}

bool PostgresCentralHosted::get_try_other_ports() const
{
  return m_try_other_ports;
}

Glib::RefPtr<Gnome::Gda::Connection> PostgresCentralHosted::connect(const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<ExceptionConnection>& error)
{
  Glib::RefPtr<Gnome::Gda::Connection> connection;

  //Try each possible network port:
  type_list_ports::const_iterator iter_port = m_list_ports.begin();

  //Start with the remembered-as-working port:
  Glib::ustring port = port_as_string(m_port);
  if(m_port == 0)
    port = *iter_port ++;

  connection = attempt_connect(m_host, port, database, username, password, error);

  // Remember port if only the database was missing
  bool connection_possible = false;
  if(error.get() && error->get_failure_type() == ExceptionConnection::FAILURE_NO_DATABASE)
  {
    connection_possible = true;
    m_port = atoi(port.c_str());
  }

  // Try more ports if so desired, and we don't have a connection yet
  if(m_try_other_ports && !connection)
  {
    while(!connection && iter_port != m_list_ports.end())
    {
      port = *iter_port;
      connection = attempt_connect(m_host, port, database, username, password, error);

      // Remember port if only the database was missing
      if(error.get() && error->get_failure_type() == ExceptionConnection::FAILURE_NO_DATABASE)
      {
        connection_possible = true;
        m_port = atoi(port.c_str());
      }

      // Skip if we already tried this port
      if(iter_port != m_list_ports.end() && *iter_port == port)
        ++ iter_port;
    }
  }

  if(connection)
  {
    //Remember working port:
    m_port = atoi(port.c_str());
  }
  else
  {
    if(connection_possible)
      error.reset(new ExceptionConnection(ExceptionConnection::FAILURE_NO_DATABASE));
    else
      error.reset(new ExceptionConnection(ExceptionConnection::FAILURE_NO_SERVER));
  }

  return connection;
}

bool PostgresCentralHosted::create_database(const Glib::ustring& database_name, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<Glib::Error>& error)
{
  return attempt_create_database(database_name, get_host(), port_as_string(m_port), username, password, error);
}

}

}
