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

#include "config.h"

#include <glom/libglom/connectionpool_backends/postgres_central.h>
#include <glibmm/i18n.h>
#include <bakery/bakery.h>

#ifdef GLOM_ENABLE_MAEMO
# include <hildonmm/note.h>
#else
# include <gtkmm/messagedialog.h>
#endif


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
  m_try_other_ports(true),
  m_postgres_server_version(0.0f)
{
  m_list_ports.push_back("5432"); //Ubuntu Breezy seems to default to this for Postgres 7.4, and this is probably the default for most postgres installations, including Fedora.

  m_list_ports.push_back("5433"); //Ubuntu Dapper seems to default to this for Postgres 8.1, probably to avoid a clash with Postgres 7.4

  m_list_ports.push_back("5434"); //Earlier versions of Ubuntu Feistry defaulted to this for Postgres 8.2.
  m_list_ports.push_back("5435"); //In case Ubuntu increases the port number again in future.
  m_list_ports.push_back("5436"); //In case Ubuntu increases the port number again in future.
}

void PostgresCentralHosted::set_host(const Glib::ustring& value)
{
  if(value != m_host)
  {
    m_host = value;
    m_port = 0; // Force us to try all ports again when connecting for the first time, then remember the working port again.
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

float PostgresCentralHosted::get_postgres_server_version() const
{
  return m_postgres_server_version;
}

Glib::RefPtr<Gnome::Gda::Connection> PostgresCentralHosted::attempt_connect(const Glib::RefPtr<Gnome::Gda::Client>& client, const Glib::ustring& host, const Glib::ustring& port, const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, float& postgres_server_version)
{
  //We must specify _some_ database even when we just want to create a database.
  //This _might_ be different on some systems. I hope not. murrayc
  const Glib::ustring default_database = "template1";
  const Glib::ustring& actual_database = (!database.empty()) ? database : default_database;;
  const Glib::ustring cnc_string = "HOST=" + host + ";PORT=" + port + ";DB_NAME=" + actual_database;

  //std::cout << "debug: connecting: cnc string: " << cnc_string << std::endl;
#ifdef GLOM_CONNECTION_DEBUG          
  std::cout << std::endl << "Glom: trying to connect on port=" << port << std::endl;
#endif

  Glib::RefPtr<Gnome::Gda::Connection> connection;
  Glib::RefPtr<Gnome::Gda::DataModel> data_model;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    connection = client->open_connection_from_string("PostgreSQL", cnc_string, username, password);
    connection->execute_non_select_command("SET DATESTYLE = 'ISO'");
    data_model = connection->execute_select_command("SELECT version()");
  }
  catch(const Glib::Error& ex)
  {
#else
  std::auto_ptr<Glib::Error> error;
  connection = m_GdaClient->open_connection_from_string("PostgreSQL", cnc_string, m_user, m_password, error);
  if(!error)
    connection->execute_non_select_command("SET DATESTYLE = 'ISO'", error);
  if(!error)
    data_model = connection->execute_select_command("SELECT version()", error);

  if(error.get())
  {
    const Glib::Error& ex = *glib_error;
#endif

#ifdef GLOM_CONNECTION_DEBUG
    std::cout << "ConnectionPoolBackends::PostgresCentralHosted::attempt_connect(): Attempt to connect to database failed on port=" << port << ", database=" << actual_database << ": " << ex.what() << std::endl;
#endif
    return Glib::RefPtr<Gnome::Gda::Connection>();
  }

  if(data_model && data_model->get_n_rows() && data_model->get_n_columns())
  {
    Gnome::Gda::Value value = data_model->get_value_at(0, 0);
    if(value.get_value_type() == G_TYPE_STRING)
    {
      const Glib::ustring version_text = value.get_string();
      //This seems to have the format "PostgreSQL 7.4.11 on i486-pc-linux"
      const Glib::ustring namePart = "PostgreSQL ";
      const Glib::ustring::size_type posName = version_text.find(namePart);
      if(posName != Glib::ustring::npos)
      {
        const Glib::ustring versionPart = version_text.substr(namePart.size());
        postgres_server_version = strtof(versionPart.c_str(), NULL);

#ifdef GLOM_CONNECTION_DEBUG
        std::cout << "  Postgres Server version: " << postgres_server_version << std::endl;
#endif
      }
    }
  }

  return connection;
}

Glib::RefPtr<Gnome::Gda::Connection> PostgresCentralHosted::connect(const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password)
{
  if(!m_refGdaClient)
  {
    m_refGdaClient = Gnome::Gda::Client::create();
    if(!m_refGdaClient)
      return Glib::RefPtr<Gnome::Gda::Connection>();
  }

  Glib::RefPtr<Gnome::Gda::Connection> connection;

  //Try each possible network port:
  type_list_ports::const_iterator iter_port = m_list_ports.begin();

  //Start with the remembered-as-working port:
  Glib::ustring port = port_as_string(m_port);
  if(port == 0)
    port = *iter_port ++;

  connection = attempt_connect(m_refGdaClient, m_host, port, database, username, password, m_postgres_server_version);

  // Try more ports if so desired, and we don't have a connection yet
  if(m_try_other_ports && !connection)
  {
    while(!connection && iter_port != m_list_ports.end())
    {
      port = *iter_port;
      connection = attempt_connect(m_refGdaClient, m_host, port, database, username, password, m_postgres_server_version);

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

  return connection;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifdef GLIBMM_EXCEPTIONS_ENABLED
bool PostgresCentralHosted::create_database(const Glib::ustring& database_name, const Glib::ustring& username, const Glib::ustring& password)
#else
bool PostgresCentralHosted::create_database(const Glib::ustring& database_name, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<Glib::Error>& error)
#endif
{
  if(!m_refGdaClient)
  {
    m_refGdaClient = Gnome::Gda::Client::create();
    if(!m_refGdaClient)
      return false;
  }

  Glib::RefPtr<Gnome::Gda::ServerOperation> op = m_refGdaClient->prepare_create_database(database_name, "PostgreSQL");
  g_assert(op);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  op->set_value_at("/SERVER_CNX_P/HOST", get_host());
  op->set_value_at("/SERVER_CNX_P/PORT", port_as_string(m_port));
  op->set_value_at("/SERVER_CNX_P/ADM_LOGIN", username);
  op->set_value_at("/SERVER_CNX_P/ADM_PASSWORD", password);
  m_refGdaClient->perform_create_database(op);
#else
  op->set_value_at("/SERVER_CNX_P/HOST", get_host(), error);
  op->set_value_at("/SERVER_CNX_P/PORT", port_as_string(m_port), error);
  op->set_value_at("/SERVER_CNX_P/ADM_LOGIN", username, error);
  op->set_value_at("/SERVER_CNX_P/ADM_PASSWORD", password, error);

  if(error.get() != NULL)
    m_refGdaClient->perform_create_database(op);
  else
    return false;
#endif

  return true;
}
#endif

bool PostgresCentralHosted::check_postgres_gda_client_is_available_with_warning()
{
  Glib::RefPtr<Gnome::Gda::Client> gda_client = Gnome::Gda::Client::create();
  if(gda_client)
  {
    //Get the list of providers:
    typedef std::list<Gnome::Gda::ProviderInfo> type_list_of_provider_info;
    type_list_of_provider_info providers = Gnome::Gda::Config::get_providers();

    //Examine the information about each Provider:
    for(type_list_of_provider_info::const_iterator iter = providers.begin(); iter != providers.end(); ++iter)
    { 
      const Gnome::Gda::ProviderInfo& info = *iter;
      if (info.get_id() == "PostgreSQL")
        return true;
    }
  }

  const Glib::ustring message = _("Your installation of Glom is not complete, because the PostgreSQL libgda provider is not available on your system. This provider is needed to access Postgres database servers.\n\nPlease report this bug to your vendor, or your system administrator so it can be corrected.");
#ifndef GLOM_ENABLE_MAEMO
  /* The Postgres provider was not found, so warn the user: */
  Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Incomplete Glom Installation")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true /* modal */);
  dialog.set_secondary_text(message);
  dialog.run();
#else
  Hildon::Note note(Hildon::NOTE_TYPE_INFORMATION, message);
  note.run();
#endif

  return false;
}

}

}
