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

#include "config.h" // For GLOM_ENABLE_MAEMO
 
#include <glom/libglom/connectionpool.h>
#include <glom/libglom/document/document_glom.h>
#include <bakery/bakery.h>
#include <libgnomevfsmm.h>
#include <glib/gstdio.h> //For g_remove().
#include <glom/libglom/spawn_with_feedback.h>
#include <glom/libglom/utils.h>
#include <libgdamm/connectionevent.h>

#ifndef G_OS_WIN32
#include <libepc/shell.h> //For epc_shell_set_progress_hooks().
#endif

#include <glibmm/i18n.h>

#ifndef G_OS_WIN32
#include <libepc/publisher.h>
#endif


#ifdef GLOM_ENABLE_MAEMO
#include <hildonmm/note.h>
#endif

#ifndef G_OS_WIN32
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/socket.h>
#include <netinet/in.h> //For sockaddr_in
#else
#include <winsock2.h>
#endif

#include <signal.h> //To catch segfaults

#include "gst-package.h"

/* TODO: Should this be used in clientonly mode? */

#ifndef G_OS_WIN32
static EpcProtocol publish_protocol = EPC_PROTOCOL_HTTPS;
#endif

namespace
{
  std::string get_path_to_postgres_executable(const std::string& program)
  {
#ifdef G_OS_WIN32
    // Use postgres on Windows, since the postgresql installer does not
    // install the (deprecated) postmaster binary.
    if(program == "postmaster")
      return Glib::find_program_in_path("postgres.exe");

    return Glib::find_program_in_path(program + EXEEXT);
#else
    return Glib::build_filename(POSTGRES_UTILS_PATH, program + EXEEXT);
#endif
  }
}


namespace Glom
{

#define DEFAULT_CONFIG_PG_HBA "local   all         postgres                          ident sameuser\n\
\n\
# TYPE  DATABASE    USER        CIDR-ADDRESS          METHOD\n\
\n\
# local is for Unix domain socket connections only\n\
local   all         all                               ident sameuser\n\
# IPv4 local connections:\n\
host    all         all         0.0.0.0/0          md5\n\
# IPv6 local connections:\n\
host    all         all         ::1/128               md5\n"

#define PORT_POSTGRESQL_SELF_HOSTED_START 5433
#define PORT_POSTGRESQL_SELF_HOSTED_END 5500

#define DEFAULT_CONFIG_PG_IDENT ""


ExceptionConnection::ExceptionConnection(failure_type failure)
: m_failure_type(failure)
{
}

ExceptionConnection::~ExceptionConnection() throw()
{
}

const char* ExceptionConnection::what() const throw()
{
  return "Glom database connection failed.";
}

ExceptionConnection::failure_type ExceptionConnection::get_failure_type() const
{
  return m_failure_type;
}

SharedConnection::SharedConnection()
{
}

SharedConnection::SharedConnection(const Glib::RefPtr<Gnome::Gda::Connection>& gda_connection)
: m_gda_connection(gda_connection)
{
}

SharedConnection::~SharedConnection()
{
  if(m_gda_connection)
    m_signal_finished.emit();
}

Glib::RefPtr<Gnome::Gda::Connection> SharedConnection::get_gda_connection()
{
  return m_gda_connection;
}

Glib::RefPtr<const Gnome::Gda::Connection> SharedConnection::get_gda_connection() const
{
  return m_gda_connection;
}

SharedConnection::type_signal_finished SharedConnection::signal_finished()
{
  return m_signal_finished;
}

void SharedConnection::close()
{
  if(m_gda_connection)
    m_gda_connection.clear();


  //Tell the connection pool that we have finished with this connection.
  //It might want to close it, or keep it open if somebody else is using it.
  //It might even give it to someone else while it is waiting for that other person to finish with it.
  m_signal_finished.emit();
}


//init_db_details static data:
ConnectionPool* ConnectionPool::m_instance = 0;

ConnectionPool::ConnectionPool()
:
#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_self_hosting_active(false),
  m_epc_publisher(0),
  m_dialog_epc_progress(0),
#endif // !GLOM_ENABLE_CLIENT_ONLY
  m_sharedconnection_refcount(0),
  m_ready_to_connect(false),
  m_port(0), 
  m_try_other_ports(true), 
  m_pFieldTypes(0),
  m_postgres_server_version(0)
{
  m_list_ports.push_back("5432"); //Ubuntu Breezy seems to default to this for Postgres 7.4, and this is probably the default for most postgres installations, including Fedora.

  m_list_ports.push_back("5433"); //Ubuntu Dapper seems to default to this for Postgres 8.1, probably to avoid a clash with Postgres 7.4

  m_list_ports.push_back("5434"); //Earlier versions of Ubuntu Feistry defaulted to this for Postgres 8.2.
  m_list_ports.push_back("5435"); //In case Ubuntu increases the port number again in future.
  m_list_ports.push_back("5436"); //In case Ubuntu increases the port number again in future.
}

ConnectionPool::~ConnectionPool()
{
  if(m_pFieldTypes)
  {
    delete m_pFieldTypes;
    m_pFieldTypes = 0;
  }
}

//static
ConnectionPool* ConnectionPool::get_instance()
{
  //TODO: Synchronize this for threads?
  if(m_instance)
    return m_instance;
  else
  {
    m_instance = new ConnectionPool(); //TODO: Does it matter that this is never deleted?
    return m_instance;
  }
}

bool ConnectionPool::get_ready_to_connect() const
{
  return m_ready_to_connect;
}

void ConnectionPool::set_ready_to_connect(bool val)
{
  m_ready_to_connect = val;
}

//static:
#ifdef GLIBMM_EXCEPTIONS_ENABLED
sharedptr<SharedConnection> ConnectionPool::get_and_connect()
#else
sharedptr<SharedConnection> ConnectionPool::get_and_connect(std::auto_ptr<ExceptionConnection>& error)
#endif // GLIBMM_EXCEPTIONS_ENABLED
{
  sharedptr<SharedConnection> result(0);

  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    result = connection_pool->connect();
#else
    result = connection_pool->connect(error);
#endif // GLIBMM_EXCEPTIONS_ENABLED
  }

  return result;
}

static Glib::ustring port_as_string(int port_num)
{
  Glib::ustring result;
  char* cresult = g_strdup_printf("%d", port_num);
  if(cresult)
    result = cresult;
  g_free(cresult);

  return result;
}

#ifdef GLIBMM_EXCEPTIONS_ENABLED
sharedptr<SharedConnection> ConnectionPool::connect()
#else
sharedptr<SharedConnection> ConnectionPool::connect(std::auto_ptr<ExceptionConnection>& error)
#endif
{
  if(get_ready_to_connect())
  {
    //If the connection is already open (because it is being used by somebody):
    if(m_refGdaConnection)
    {
      sharedptr<SharedConnection> sharedConnection( new SharedConnection(m_refGdaConnection) );

      //Ask for notification when the SharedConnection has been finished with:
      sharedConnection->signal_finished().connect( sigc::mem_fun(*this, &ConnectionPool::on_sharedconnection_finished) );

      //Remember that somebody is using it:
      m_sharedconnection_refcount++;

      return sharedConnection;
    }
    else
    {
      //Create a new connection:

      m_GdaClient = Gnome::Gda::Client::create();


      //We must specify _some_ database even when we just want to create a database.
      //This _might_ be different on some systems. I hope not. murrayc
      const Glib::ustring default_database = "template1"; 
      if(m_GdaClient)
      {
        //m_GdaDataSourceInfo = Gnome::Gda::DataSourceInfo(); //init_db_details it.
        //m_GdaDataSourceInfo->

        bool connection_to_default_database_possible = false;

        //Try each possible network port:
        type_list_ports::const_iterator iter_port = m_list_ports.begin();

        //Start with the remembered-as-working port:
        Glib::ustring port = port_as_string(m_port);

        //If no port is known to work, start with the first possible port:
        bool trying_remembered_port = true;
        if(port.empty())
        {
          port = *iter_port;
          trying_remembered_port = false;
        }

        bool try_another_port = true;
        while(try_another_port)
        { 
//          const Glib::ustring cnc_string_main = "HOST=" + get_host() + ";USER=" + m_user + ";PASSWORD=" + m_password + ";PORT=" + port;
	  const Glib::ustring cnc_string_main = "HOST=" + get_host() + ";PORT=" + port;

          Glib::ustring cnc_string = cnc_string_main;

          if(!m_database.empty())
            cnc_string += (";DB_NAME=" + m_database);
          else
            cnc_string += (";DB_NAME=" + default_database);

          //std::cout << "debug: connecting: cnc string: " << cnc_string << std::endl;
          std::cout << std::endl << "Glom: trying to connect on port=" << port << std::endl;

          //*m_refGdaConnection = m_GdaClient->open_connection(m_GdaDataSourceInfo.get_name(), m_GdaDataSourceInfo.get_username(), m_GdaDataSourceInfo.get_password() );
          //m_refGdaConnection.clear(); //Make sure any previous connection is really forgotten.

#ifdef GLIBMM_EXCEPTIONS_ENABLED
          try
	  {
#else
          std::auto_ptr<Glib::Error> glib_error;
#endif
          // Use do/while without condition so we can easily jump out
          do
          {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
            m_refGdaConnection = m_GdaClient->open_connection_from_string("PostgreSQL", cnc_string, m_user, m_password);
#else
            m_refGdaConnection = m_GdaClient->open_connection_from_string("PostgreSQL", cnc_string, m_user, m_password, Gnome::Gda::ConnectionOptions(0), glib_error);
            if(glib_error.get())
              break;
#endif
            //g_warning("ConnectionPool: connection opened");

            //Remember what port is working:
            m_port = atoi(port.c_str());

            //Create the fieldtypes member if it has not already been done:
            if(!m_pFieldTypes)
              m_pFieldTypes = new FieldTypes(m_refGdaConnection);  

            //Enforce ISO formats in the communication:
#ifdef GLIBMM_EXCEPTIONS_ENABLED
            m_refGdaConnection->execute_non_select_command("SET DATESTYLE = 'ISO'");
#else
            m_refGdaConnection->execute_non_select_command("SET DATESTYLE = 'ISO'", glib_error);
            if(glib_error.get())
              break;
#endif

            //Open the database, if one has been specified:
            /* This does not seem to work in libgda's postgres provider, so we specify it in the cnc_string instead:
            std::cout << "  database = " << m_database << std::endl;
            if(!m_database.empty())
              m_refGdaConnection->change_database(m_database);
            */

#ifdef GLIBMM_EXCEPTIONS_ENABLED
            //Get postgres version:
            Glib::RefPtr<Gnome::Gda::DataModel> data_model = m_refGdaConnection->execute_select_command("SELECT version()");
#else
            Glib::RefPtr<Gnome::Gda::DataModel> data_model = m_refGdaConnection->execute_select_command("SELECT version()", glib_error);
            if(glib_error.get() != NULL) break;
#endif
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
                  char* end = 0;
                  m_postgres_server_version = strtof(versionPart.c_str(), &end);
                }
              }
            }

            std::cout << "  Postgres Server version: " << get_postgres_server_version() << std::endl << std::endl;

#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifndef G_OS_WIN32
           //Let other clients discover this server via avahi:
           //TODO: Only advertize if we are the first to open the document,
           //to avoid duplicates.
           avahi_start_publishing(); //Stopped in the signal_finished handler.
#endif // !G_OS_WIN32
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifdef GLIBMM_EXCEPTIONS_ENABLED
            return connect(); //Call this method recursively. This time m_refGdaConnection exists.
#else
            return connect(error); //Call this method recursively. This time m_refGdaConnection exists.
#endif // GLIBMM_EXCEPTIONS_ENABLED
          }
          while(false);

#ifndef GLIBMM_EXCEPTIONS_ENABLED
          if(glib_error.get())
          {
            const Glib::Error& ex = *glib_error.get();
#else
          }
          catch(const Gnome::Gda::ConnectionError& ex)
          {
#endif
            std::cout << "ConnectionPool::connect() Attempt to connect to database failed on port=" << port << ", database=" << m_database << ": " << ex.what() << std::endl;

            bool bJustDatabaseMissing = false;
            if(!m_database.empty())
            {
              std::cout << "  ConnectionPool::connect() Attempting to connect without specifying the database." << std::endl;

              //If the connection failed while looking for a database,
              //then try connecting without the database:
              Glib::ustring cnc_string = cnc_string_main;
              cnc_string += (";DB_NAME=" + default_database);

              //std::cout << "debug2: connecting: cnc string: " << cnc_string << std::endl;
              std::cout << "Glom: connecting." << std::endl;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
              try
#endif
              {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
                //If we could connect without specifying the database.
                Glib::RefPtr<Gnome::Gda::Connection> gda_connection = m_GdaClient->open_connection_from_string("PostgreSQL", cnc_string, m_user, m_password);
#else
                Glib::RefPtr<Gnome::Gda::Connection> gda_connection = m_GdaClient->open_connection_from_string("PostgreSQL", cnc_string, m_user, m_password, Gnome::Gda::ConnectionOptions(0), glib_error);
                if(glib_error.get() == NULL)
                {
#endif
                bJustDatabaseMissing = true;
                connection_to_default_database_possible = true;
                m_port = atoi(port.c_str());
#ifndef GLIBMM_EXCEPTIONS_ENABLED
                }
              }
              if(glib_error.get())
              {
                const Glib::Error& ex = *glib_error.get();
#else
              }
              catch(const Gnome::Gda::ConnectionError& ex)
              {
#endif
                std::cerr << "    ConnectionPool::connect() connection also failed when not specifying database: " << ex.what() << std::endl;
              }
            }

            if(bJustDatabaseMissing)
              std::cout << "  (Connection succeeds, but not to the specific database on port=" << port << ", database=" << m_database << std::endl;
            else
              std::cerr << "  (Could not connect even to the default database on port=" << port << ", database=" << m_database  << std::endl;


            //handle_error(true /* cerr only */);
          }


          //If we got this far then the connection failed, so we should try another port:
          if(!m_try_other_ports)
            try_another_port = false;
          else
          {
            if(trying_remembered_port)
              iter_port = m_list_ports.begin(); //Start looking at the possible ports.
            else
              ++iter_port;

            if(iter_port == m_list_ports.end())
              try_another_port = false;
            else
            {
              if(port == *iter_port) //Don't bother trying the same port again.
                ++iter_port;

              if(iter_port == m_list_ports.end()) //Check again, in case we iterated again.
                try_another_port = false;
              else            
                port = *iter_port;
            }
          }

          trying_remembered_port = false;
        }

        g_warning("ConnectionPool::connect() throwing exception.");
        if(connection_to_default_database_possible)
          std::cout << "  (Connection succeeds, but not to the specific database on port=" << m_port << ", database=" << m_database << std::endl;
        else
          std::cerr << "  (Could not connect even to the default database on any port. database=" << m_database  << std::endl;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
        throw ExceptionConnection(connection_to_default_database_possible ? ExceptionConnection::FAILURE_NO_DATABASE : ExceptionConnection::FAILURE_NO_SERVER);
#else
        error.reset(new ExceptionConnection(connection_to_default_database_possible ? ExceptionConnection::FAILURE_NO_DATABASE : ExceptionConnection::FAILURE_NO_SERVER));
#endif
      }
    }
  }
  else
  {
      //g_warning("ConnectionPool::connect(): not ready to connect.");
  }

  return sharedptr<SharedConnection>(0);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void ConnectionPool::set_self_hosted(const std::string& data_uri)
{
  m_self_hosting_data_uri = data_uri;
}

#ifdef GLIBMM_EXCEPTIONS_ENABLED
void ConnectionPool::create_database(const Glib::ustring& database_name)
#else
void ConnectionPool::create_database(const Glib::ustring& database_name, std::auto_ptr<Glib::Error>& error)
#endif
{
  if(m_GdaClient)
  {
    Glib::RefPtr<Gnome::Gda::ServerOperation> op = m_GdaClient->prepare_create_database(database_name, "PostgreSQL");
    g_assert(op);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    op->set_value_at("/SERVER_CNX_P/HOST", get_host());
    op->set_value_at("/SERVER_CNX_P/PORT", port_as_string(m_port));
    op->set_value_at("/SERVER_CNX_P/ADM_LOGIN", get_user());
    op->set_value_at("/SERVER_CNX_P/ADM_PASSWORD", get_password());
    m_GdaClient->perform_create_database(op);
#else
    op->set_value_at("/SERVER_CNX_P/HOST", get_host(), error);
    op->set_value_at("/SERVER_CNX_P/PORT", port_as_string(m_port), error);
    op->set_value_at("/SERVER_CNX_P/ADM_LOGIN", get_user(), error);
    op->set_value_at("/SERVER_CNX_P/ADM_PASSWORD", get_password(), error);

    if(error.get() != NULL)
      m_GdaClient->perform_create_database(op);
#endif
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void ConnectionPool::set_host(const Glib::ustring& value)
{
  if(value != m_host)
  {
    m_host = value;  
    m_port = 0; // Force us to try all ports again when connecting for the first time, then remember the working port again.
  }
}

void ConnectionPool::set_user(const Glib::ustring& value)
{
  if(value.empty())
  {
    std::cout << "debug: ConnectionPool::set_user(): user is empty." << std::endl;
  }

  m_user = value;
}

void ConnectionPool::set_port(int port)
{
  m_port = port;
}

void ConnectionPool::set_try_other_ports(bool val)
{
  m_try_other_ports = val;
}

void ConnectionPool::set_password(const Glib::ustring& value)
{
  m_password = value;
}

void ConnectionPool::set_database(const Glib::ustring& value)
{
  m_database = value;
}

Glib::ustring ConnectionPool::get_host() const
{
  return m_host;
}

int ConnectionPool::get_port() const
{
  return m_port;
}

bool ConnectionPool::get_try_other_ports() const
{
  return m_try_other_ports;
}

Glib::ustring ConnectionPool::get_user() const
{
  return m_user;
}

Glib::ustring ConnectionPool::get_password() const
{
  return m_password;
}

Glib::ustring ConnectionPool::get_database() const
{
  return m_database;
}

const FieldTypes* ConnectionPool::get_field_types() const
{
  return m_pFieldTypes;
}

void ConnectionPool::on_sharedconnection_finished()
{
  //g_warning("ConnectionPool::on_sharedconnection_finished().");

  //One SharedConnection is no longer being used:
  m_sharedconnection_refcount--;

  //If this was the last user of SharedConnection then we can close the connection.
  if(m_sharedconnection_refcount == 0)
  {
    //There should be no copies of the m_refConnection, so the Gnome::Gda::Connection destructor should
    //run when we clear this last RefPtr of it, but we will explicitly close it just in case.
    //g_warning("ConnectionPool::on_sharedconnection_finished(): closing GdaConnection");
    m_refGdaConnection->close();

    m_refGdaConnection.clear();

#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifndef G_OS_WIN32
    //TODO: this should only even be started if we are the first to open the .glom file:
    avahi_stop_publishing();
#endif
#endif

    //g_warning("ConnectionPool: connection closed");
  }

}

//static
bool ConnectionPool::handle_error(bool cerr_only)
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  sharedptr<SharedConnection> sharedconnection = get_and_connect();
#else
  std::auto_ptr<ExceptionConnection> conn_error;
  sharedptr<SharedConnection> sharedconnection = get_and_connect(conn_error);
  // Ignore error, sharedconnection presence is checked below
#endif
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

    typedef std::list< Glib::RefPtr<Gnome::Gda::ConnectionEvent> > type_list_errors;
    type_list_errors list_errors = gda_connection->get_events();

    if(!list_errors.empty())
    {
      Glib::ustring error_details;
      for(type_list_errors::iterator iter = list_errors.begin(); iter != list_errors.end(); ++iter)
      {
        Glib::RefPtr<Gnome::Gda::ConnectionEvent> event = *iter;
        if (event && (event->get_event_type() == Gnome::Gda::CONNECTION_EVENT_ERROR))
        {
          if(!error_details.empty())
            error_details += "\n"; //Add newline after each error.

          error_details += (*iter)->get_description();
          std::cerr << "Internal error (Database): " << error_details << std::endl;
        }
      }

      //For debugging only:
      //Gtk::Dialog* dialog = 0;
      //dialog->run(); //Force a crash.

      if(!cerr_only)
      {
#ifdef GLOM_ENABLE_MAEMO
        Hildon::Note dialog(Hildon::NOTE_TYPE_INFORMATION, error_details);
#else
        Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Internal error")), true, Gtk::MESSAGE_WARNING );
        dialog.set_secondary_text(error_details);
#endif
        //TODO: dialog.set_transient_for(*get_application());
        dialog.run(); //TODO: This segfaults in gtk_window_set_modal() when this method is run a second time, for instance if there are two database errors.
        std::cout << "debug: after Internal Error dialog run()." << std::endl;
      }

      return true; //There really was an error.
    }
  }

  //There was no error. libgda just did not return any data, and has no concept of an empty datamodel.
  return false;
}

float ConnectionPool::get_postgres_server_version()
{
  return m_postgres_server_version;
}

/*
bool ConnectionPool::directory_exists_filepath(const std::string& filepath)
{
  const std::string uri = Glib::filename_to_uri(filepath);

  return directory_exists_uri(uri);
}
*/

bool ConnectionPool::directory_exists_uri(const std::string& uri)
{
  Glib::RefPtr<Gnome::Vfs::Uri> vfsuri = Gnome::Vfs::Uri::create(uri);
  return vfsuri->uri_exists();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifdef G_OS_WIN32
// TODO: This is probably mingw specific
static __p_sig_fn_t previous_sig_handler = SIG_DFL;
#else
static sighandler_t previous_sig_handler = SIG_DFL; /* Arbitrary default */
#endif

/* This is a Linux/Unix signal handler, 
 * so we can respond to a crash.
 */
static void on_linux_signal(int signum)
{
  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(!connection_pool)
    return;

  if(signum == SIGSEGV)
  {
    connection_pool->stop_self_hosting();
	
    //TODO: How can we let GNOME's crash handler still handle this?
    exit(1);
  }
}

bool ConnectionPool::start_self_hosting()
{
  if(m_self_hosting_active)
    return true; //Just do it once.

  const std::string dbdir_uri = m_self_hosting_data_uri;

  if(!(directory_exists_uri(dbdir_uri)))
  {
    //TODO: Use a return enum or exception so we can tell the user about this:
    std::cerr << "ConnectionPool::create_self_hosting(): The data directory could not be found: " << dbdir_uri << std::endl;
    return false;
  }

  const std::string dbdir = Glib::filename_from_uri(dbdir_uri);
  g_assert(!dbdir.empty());

  const std::string dbdir_data = Glib::build_filename(dbdir, "data");
  const std::string dbdir_data_uri = Glib::filename_to_uri(dbdir_data);
  if(!(directory_exists_uri(dbdir_data_uri)))
  {
    //TODO: Use a return enum or exception so we can tell the user about this:
    std::cerr << "ConnectionPool::create_self_hosting(): The data sub-directory could not be found." << dbdir_data_uri << std::endl;
    return false;
  }


  const int available_port = discover_first_free_port(PORT_POSTGRESQL_SELF_HOSTED_START, PORT_POSTGRESQL_SELF_HOSTED_END);
  if(available_port == 0)
  {
    //TODO: Use a return enum or exception so we can tell the user about this:
    std::cerr << "ConnectionPool::create_self_hosting(): No port was available between " << PORT_POSTGRESQL_SELF_HOSTED_START << " and " << PORT_POSTGRESQL_SELF_HOSTED_END << std::endl;
    return false;
  }

  const Glib::ustring port_as_text = Utils::string_from_decimal(available_port);


  // -D specifies the data directory.
  // -c config_file= specifies the configuration file
  // -k specifies a directory to use for the socket. This must be writable by us.
  // POSTGRES_POSTMASTER_PATH is defined in config.h, based on the configure.
  const std::string command_postgres_start = Glib::shell_quote(get_path_to_postgres_executable("postmaster")) + " -D \"" + dbdir_data + "\" "
                                  + " -p " + port_as_text
                                  + " -h \"*\" " //Equivalent to listen_addresses in postgresql.conf. Listen to all IP addresses, so any client can connect (with a username+password)
                                  + " -c hba_file=\"" + dbdir + "/config/pg_hba.conf\""
                                  + " -c ident_file=\"" + dbdir + "/config/pg_ident.conf\""
                                  + " -k \"" + dbdir + "\""
                                  + " --external_pid_file=\"" + dbdir + "/pid\"";

  const std::string command_check_postgres_has_started = Glib::shell_quote(get_path_to_postgres_executable("pg_ctl")) + " status -D \"" + dbdir_data + "\"";

  //For postgres 8.1, this is "postmaster is running".
  //For postgres 8.2, this is "server is running".
  //This is a big hack that we should avoid. murrayc.
  //
  //pg_ctl actually seems to return a 0 result code for "is running" and a 1 for not running, at least with Postgres 8.2,
  //so maybe we can avoid this in future.  
  //Please do test it with your postgres version, using "echo $?" to see the result code of the last command.
  const std::string second_command_success_text = "is running"; //TODO: This is not a stable API. Also, watch out for localisation.

  //The first command does not return, but the second command can check whether it succeeded:
  const bool result = Glom::Spawn::execute_command_line_and_wait_until_second_command_returns_success(command_postgres_start, command_check_postgres_has_started, _("Starting Database Server"), 0 /* window*/, second_command_success_text);
  if(!result)
  {
    std::cerr << "Error while attempting to self-host a database." << std::endl;
    return false;
  }

  m_port = available_port; //Remember it for later.
  m_self_hosting_active = true;
  set_try_other_ports(false); //Only try to connect to this known instance, instead of finding others.

#ifndef G_OS_WIN32
  //Let clients discover this server via avahi:
  avahi_start_publishing();
#endif // !G_OS_WIN32

  //If we crash while self-hosting (unlikely, hopefully) 
  //then try to stop the postgres instance instead of leaving it running as an orphan.
  previous_sig_handler = signal(SIGSEGV, &on_linux_signal);

  return true;
}

void ConnectionPool::stop_self_hosting()
{
  if(!m_self_hosting_active)
    return; //Don't try to stop it if we have not started it.

#ifndef G_OS_WIN32
  /* Stop advertising the self-hosting database server via avahi: */
  avahi_stop_publishing();
#endif // !G_OS_WIN32

  const std::string dbdir_uri = m_self_hosting_data_uri;
  const std::string dbdir = Glib::filename_from_uri(dbdir_uri);
  g_assert(!dbdir.empty());

  const std::string dbdir_data = Glib::build_filename(dbdir, "data");


  // TODO: Detect other instances on the same computer, and use a different port number, 
  // or refuse to continue, showing an error dialog.

  // -D specifies the data directory.
  // -c config_file= specifies the configuration file
  // -k specifies a directory to use for the socket. This must be writable by us.
  // POSTGRES_POSTMASTER_PATH is defined in config.h, based on the configure.
  // We use "-m fast" instead of the default "-m smart" because that waits for clients to disconnect (and sometimes never succeeds).
  // TODO: Warn about connected clients on other computers? Warn those other users?
  const std::string command_postgres_stop = Glib::shell_quote(get_path_to_postgres_executable("pg_ctl")) + " -D \"" + dbdir_data + "\" stop -m fast";
  const bool result = Glom::Spawn::execute_command_line_and_wait(command_postgres_stop, _("Stopping Database Server"));
  if(!result)
  {
    std::cerr << "Error while attempting to stop self-hosting of the database. Trying again."  << std::endl;

    //I've seen it fail when running under valgrind, and there are reports of failures in bug #420962.
    //Maybe it will help to try again:
    const bool result = Glom::Spawn::execute_command_line_and_wait(command_postgres_stop, _("Stopping Database Server (retrying)"));
    if(!result)
    {
      std::cerr << "Error while attempting (for a second time) to stop self-hosting of the database."  << std::endl;
    }
  }

  //We don't need the segfault handler anymore:
  signal(SIGSEGV, previous_sig_handler);
  previous_sig_handler = SIG_DFL; /* Arbitrary default */

  m_self_hosting_active = false;
}

bool ConnectionPool::create_self_hosting(Gtk::Window* parent_window)
{
  if(m_self_hosting_data_uri.empty())
  {
    std::cerr << "ConnectionPool::create_self_hosting(): m_self_hosting_data_uri is empty." << std::endl;
    return false;
  }

  //Get the filepath of the directory that we should create:
  const std::string dbdir_uri = m_self_hosting_data_uri;
  //std::cout << "debug: dbdir_uri=" << dbdir_uri << std::endl;

  if(directory_exists_uri(dbdir_uri))
  { 
    if(parent_window)
    {
      Utils::show_ok_dialog(_("Directory Already Exists"), _("There is an existing directory with the same name as the directory that should be created for the new database files. You should specify a different filename to use a new directory instead."), *parent_window, Gtk::MESSAGE_ERROR);
    }
    
    return false;
  }

  const std::string dbdir = Glib::filename_from_uri(dbdir_uri);
  //std::cout << "debug: dbdir=" << dbdir << std::endl;
  g_assert(!dbdir.empty());

  //0770 means "this user and his group can read and write and "execute" (meaning add sub-files) this non-executable file".
  //The 0 prefix means that this is octal.
  int mkdir_succeeded = g_mkdir_with_parents(dbdir.c_str(), 0770);
  if(mkdir_succeeded == -1)
  {
    std::cerr << "Error from g_mkdir_with_parents() while trying to create directory: " << dbdir << std::endl;
    perror("Error from g_mkdir_with_parents");

    if(parent_window)
    {
       Utils::show_ok_dialog(_("Could Not Create Directory"), _("There was an error when attempting to create the directory for the new database files."), *parent_window, Gtk::MESSAGE_ERROR);
    }

    return false;
  }

  //Create the config directory:
  const std::string dbdir_config = dbdir + "/config";
  mkdir_succeeded = g_mkdir_with_parents(dbdir_config.c_str(), 0770);
  if(mkdir_succeeded == -1)
  {
    if(parent_window)
    {
       Utils::show_ok_dialog(_("Could Not Create Configuration Directory"), _("There was an error when attempting to create the configuration directory for the new database files."), *parent_window, Gtk::MESSAGE_ERROR);
    }

    std::cerr << "Error from g_mkdir_with_parents() while trying to create directory: " << dbdir_config << std::endl;
    perror("Error from g_mkdir_with_parents");
    return false;
  }

  //Create these files: environment  pg_hba.conf  pg_ident.conf  start.conf

  const std::string dbdir_uri_config = dbdir_uri + "/config";
  create_text_file(dbdir_uri_config + "/pg_hba.conf", DEFAULT_CONFIG_PG_HBA);

  create_text_file(dbdir_uri_config + "/pg_ident.conf", DEFAULT_CONFIG_PG_IDENT);

  //Check that there is not an existing data directory:
  const std::string dbdir_data = dbdir + "/data";
  mkdir_succeeded = g_mkdir_with_parents(dbdir_data.c_str(), 0770);
  g_assert(mkdir_succeeded != -1);

  
  // initdb creates a new postgres database cluster:
  const std::string username = get_user();
  if(username.empty())
  {
    std::cerr << "ConnectionPool::create_self_hosting(). Username was empty while attempting to create self-hosting  database" << std::endl;
    return false;
  }

  const std::string temp_pwfile = Glib::build_filename(Glib::get_tmp_dir(), "glom_initdb_pwfile");
  create_text_file(temp_pwfile, get_password());

  // We need to specify the path where initdb should look for shared files. If
  // we don't do this, it defaults to /usr/local/pgsql/share. The installer
  // installs the necessary files to $glomdir/share/postgresql which is what
  // we pass here.
#ifdef G_OS_WIN32
  gchar* share_path_ = g_win32_get_package_installation_subdirectory(NULL, NULL, "share/postgresql");
  const std::string share_path = std::string(" -L \"") + share_path_ + "\"";
  g_free(share_path_);
#else
  const std::string share_path;
#endif

  const std::string command_initdb = Glib::shell_quote(get_path_to_postgres_executable("initdb")) + " -D \"" + dbdir_data + "\"" +
                                        " -U " + username + " --pwfile=\"" + temp_pwfile + "\"" + share_path;

  //Note that --pwfile takes the password from the first line of a file. It's an alternative to supplying it when prompted on stdin.
  const bool result = Glom::Spawn::execute_command_line_and_wait(command_initdb, _("Creating Database Data"));
  if(!result)
  {
    std::cerr << "Error while attempting to create self-hosting database." << std::endl;
  }

  const int temp_pwfile_removed = g_remove(temp_pwfile.c_str()); //Of course, we don't want this to stay around. It would be a security risk.
  g_assert(temp_pwfile_removed == 0);
 
  return result;
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

bool ConnectionPool::create_text_file(const std::string& file_uri, const std::string& contents)
{
  Gnome::Vfs::Handle write_handle;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
#else
  std::auto_ptr<Gnome::Vfs::exception> error;
#endif
  {
    //0660 means "this user and his group can read and write this non-executable file".
    //The 0 prefix means that this is octal.
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    write_handle.create(file_uri, Gnome::Vfs::OPEN_WRITE, false, 0660 /* leading zero means octal */);
#else
    write_handle.create(file_uri, Gnome::Vfs::OPEN_WRITE, false, 0660 /* leading zero means octal */, error);
#endif
  }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  catch(const Gnome::Vfs::exception& ex)
  {
#else
  if(error.get() != NULL)
  {
    const Gnome::Vfs::exception& ex = *error.get();
#endif
    std::cerr << "ConnectionPool::create_text_file(): exception caught during file create: " << ex.what() << std::endl;

    // If the operation was not successful, print the error and abort
    return false; // print_error(ex, output_uri_string);
  }

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
#endif
  {
    //Write the data to the output uri
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    GnomeVFSFileSize bytes_written = write_handle.write(contents.data(), contents.size());
#else
    GnomeVFSFileSize bytes_written = write_handle.write(contents.data(), contents.size(), error);
    if(error.get() != NULL)
#endif
      if(bytes_written != contents.size())
        return false;
  }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  catch(const Gnome::Vfs::exception& ex)
  {
#else
  if(error.get() != NULL)
  {
    Gnome::Vfs::exception& ex = *error.get();
#endif
    std::cerr << "ConnectionPool::create_text_file(): exception caught during write: " << ex.what() << std::endl;

    // If the operation was not successful, print the error and abort
    return false; //print_error(ex, output_uri_string);
  }

  return true; //Success. (At doing nothing, because nothing needed to be done.)
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
int ConnectionPool::discover_first_free_port(int start_port, int end_port)
{
  //Open a socket so we can try to bind it to a port:
  const int fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd == -1)
  {
    perror("Create socket");
    return 0;
  }

  //This code was originally suggested by Lennart Poettering.

  struct ::sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;

  int port_to_try = start_port;
  while (port_to_try <= end_port)
  {
    sa.sin_port = htons(port_to_try);

    const int result = bind(fd, (sockaddr*)&sa, sizeof(sa));
    if((result == 0) || ((result < 0)
#ifdef G_OS_WIN32
    && (WSAGetLastError() != WSAEADDRINUSE)
#else // G_OS_WIN32
#ifdef EADDRINUSE //Some BSDs don't have this.
    && (errno != EADDRINUSE)
#endif 
#ifdef EPORTINUSE //Linux doesn't have this.
    && (errno != EPORTINUSE)
#endif
#endif // !G_OS_WIN32
    ))
    {
#ifdef G_OS_WIN32
      closesocket(fd);
#else
      close(fd);
#endif

      //std::cout << "debug: ConnectionPool::discover_first_free_port(): Found: returning " << port_to_try << std::endl;
      return port_to_try;
    }
    else
    {
      //std::cout << "debug: ConnectionPool::discover_first_free_port(): port in use: " << port_to_try << std::endl;
    }

    ++port_to_try;
  }

#ifdef G_OS_WIN32
  closesocket(fd);
#else
  close(fd);
#endif

  std::cerr << "debug: ConnectionPool::discover_first_free_port(): No port was available." << std::endl;
  return 0;
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

// Message to packagers:
// If your Glom package does not depend on PostgreSQL, for some reason, 
// then your distro-specific patch should uncomment this #define.
// and implement ConnectionPool::install_posgres().
// But please, just make your Glom package depend on PostgreSQL instead, 
// because this is silly.
//
//#define DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED 1

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool ConnectionPool::check_postgres_is_available_with_warning()
{
  //EXEEXT is defined in the Makefile.am
  const std::string binpath = get_path_to_postgres_executable("postmaster");
  // TODO: At least on Windows we should probably also check for initdb and
  // pg_ctl. Perhaps it would also be a good idea to access these files as
  // long as glom runs so they cannot be (re)moved.
  if(!binpath.empty())
  {
    const Glib::ustring uri_binpath = Glib::filename_to_uri(binpath);
    if(Bakery::App_WithDoc::file_exists(uri_binpath))
      return true;
  }
  else
  {
    #ifdef DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED

    //Show message to the user about the broken installation:
    //This is a packaging bug, but it would probably annoy packagers to mention that in the dialog:
    Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Incomplete Glom Installation")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_NONE, true /* modal */);
    dialog.set_secondary_text(_("Your installation of Glom is not complete, because PostgreSQL is not available on your system. PostgreSQL is needed for self-hosting of Glom databases.\n\nYou may now install PostgreSQL to complete the Glom installation."));
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("Install PostgreSQL"), Gtk::RESPONSE_OK);
    const int response = dialog.run();
    if(response != Gtk::RESPONSE_OK)
      return false; //Failure. Glom should now quit.
    else
      return install_postgres(&dialog);

    #else  //DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED

    //Show message to the user about the broken installation:
    Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Incomplete Glom Installation")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true /* modal */);
    dialog.set_secondary_text(_("Your installation of Glom is not complete, because PostgreSQL is not available on your system. PostgreSQL is needed for self-hosting of Glom databases.\n\nPlease report this bug to your vendor, or your system administrator so it can be corrected."));
    dialog.run();
    return false;

    #endif //DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED
  }
}

/** Try to install postgres on the distro, though this will require a distro-specific 
 * patch to the implementation.
 */
bool ConnectionPool::install_postgres(Gtk::Window* parent_window)
{
#if 0
  // This  is example code for Ubuntu, and possibly Debian,
  // using code from the gnome-system-tools Debian/Ubuntu patches.
  // (But please, just fix the dependencies instead. PostgreSQL is not optional.)
  //
  // You will also need to remove the "ifdef 0"s around the code in gst-package.[h|c],
  // and define DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED above.

  //Careful. Maybe you want a different version.
  //Also, Glom will start its own instance of PostgreSQL, on its own port, when it needs to,
  //so there is no need to start a Glom service after installation at system startup, 
  //though it will not hurt Glom if you do that.
  const gchar *packages[] = { "postgresql-8.1", NULL };
  const bool result = gst_packages_install(parent_window->gobj() /* parent window */, packages);
  if(result)
  {
    std::cout << "Glom: gst_packages_install() reports success." << std::endl;
    //Double-check, because gst_packages_install() incorrectly returns TRUE if it fails because 
    //a) synaptic is already running, or
    //b) synaptic did not know about the package (no warning is shown in this case.)
    //Maybe gst_packages_install() never returns FALSE.
    return check_postgres_is_available_with_warning(); //This is recursive, but clicking Cancel will stop everything.
  }
  else
  {
    std::cout << "Glom: gst_packages_install() reports failure." << std::endl;
    return false; //Failed to install postgres.
  }
#else
  return false; //Failed to install postgres because no installation technique was implemented.
#endif // #if 0
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

bool ConnectionPool::check_postgres_gda_client_is_available_with_warning()
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

bool ConnectionPool::check_user_is_not_root()
{
#ifdef G_OS_WIN32
  // TODO: Should we check for administator privileges here? Note that it is
  // not too easy/convenient to change user on Windows (at least XP, I am not
  // sure about Vista).
  return true;
#else
  //std::cout << "ConnectionPool::check_user_is_not_root(): geteuid()=" << geteuid() << ", getgid()=" << getgid() << std::endl;

  //This is very linux-specific. We should ifdef this out for other platforms.
  if(geteuid() == 0)
  {
    //Warn the user:
    const Glib::ustring message = _("You seem to be running Glom as root. Glom may not be run as root.\nPlease login to your system as a normal user.");
#ifndef GLOM_ENABLE_MAEMO
    Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Running As Root")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true /* modal */);
    dialog.set_secondary_text(message);
    dialog.run();
#else
    Hildon::Note note(Hildon::NOTE_TYPE_INFORMATION, message);
    note.run();
#endif

    return false; /* Is root. Bad. */
  }

  return true; /* Not root. It's OK. */
#endif
}


#ifndef GLOM_ENABLE_CLIENT_ONLY
Document_Glom* ConnectionPool::get_document()
{
  if(!m_slot_get_document)
  {
    std::cerr << "Glom ConnectionPool::get_document(): m_slot_get_document is null." << std::endl;
    return 0;
  }

  return m_slot_get_document();
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifndef G_OS_WIN32
//static
EpcContents* ConnectionPool::on_publisher_document_requested(EpcPublisher* publisher, const gchar* key, gpointer user_data)
{
  Glom::ConnectionPool* connection_pool = static_cast<Glom::ConnectionPool*>(user_data);
  if(!connection_pool)
    return 0;

  const Document_Glom* document = connection_pool->get_document();
  if(!document)
    return 0;

  const Glib::ustring contents = document->get_contents();
  //std::cout << "DEBUG: ConnectionPool::on_publisher_document_requested(): returning: " << std::endl << "  " << contents << std::endl;
  return epc_contents_new_dup ("text/plain", (void*)contents.c_str(), -1);
}




//static
gboolean ConnectionPool::on_publisher_document_authentication(EpcAuthContext* context, const gchar* user_name, gpointer user_data)
{
  g_return_val_if_fail(context, FALSE);

  ConnectionPool* connection_pool = (ConnectionPool*)(user_data);
  g_return_val_if_fail(connection_pool, FALSE);

  // Check if the username/password are correct:
  const gchar* password = epc_auth_context_get_password(context);
  g_return_val_if_fail(password, FALSE); //TODO: This seems to happen once before this callback is called again properly.

  //std::cout << "ConnectionPool::on_publisher_document_authentication(): username=" << user_name << ", password=" << password << std::endl;

 
  //Attempt a connection with this username/password:
  ConnectionPool temp_pool;
  temp_pool.set_host( connection_pool->get_host() );
  temp_pool.set_database( connection_pool->get_database() );
  temp_pool.set_user(user_name);
  temp_pool.set_password(password);
  temp_pool.set_port( connection_pool->get_port() );
  temp_pool.set_ready_to_connect();

  try
  {
    sharedptr<SharedConnection> connection = temp_pool.connect();
    if(connection)
    {
      //std::cout << "ConnectionPool::on_publisher_document_authentication(): succeeded." << std::endl;
      return true; //Succeeded.
    }
    else
    {
      //std::cout << "ConnectionPool::on_publisher_document_authentication(): failed." << std::endl;
      return false; //Failed.
    }
  }
  catch(...)
  {
     //std::cout << "ConnectionPool::on_publisher_document_authentication(): failed with exception" << std::endl;
     return false; //Failed.
  }
}


void ConnectionPool::on_epc_progress_begin(const gchar* /* title */, gpointer user_data)
{
  //We ignore the title parameter because there is no way that libepc could know what Glom wants to say.
 
  ConnectionPool* connection_pool = (ConnectionPool*)user_data;

  //Create the dialog:
  if(connection_pool->m_dialog_epc_progress)
  {
    delete connection_pool->m_dialog_epc_progress;
    connection_pool->m_dialog_epc_progress = 0;
  }

  Gtk::MessageDialog* message_dialog = new Gtk::MessageDialog(Bakery::App_Gtk::util_bold_message(_("Glom: Generating Encryption Certificates")), true, Gtk::MESSAGE_INFO);
  message_dialog->set_secondary_text(_("Please wait while Glom prepares your system for publishing over the network."));
  message_dialog->show();

  connection_pool->m_dialog_epc_progress = message_dialog; 
}

void ConnectionPool::on_epc_progress_update(gdouble /* progress */, const gchar* /* message */, gpointer user_data)
{
  //We ignore the title parameter because there is no way that libepc could know what Glom wants to say.
  //TODO: Show the progress in a ProgressBar.

  //ConnectionPool* connection_pool = (ConnectionPool*)user_data;

  //Allow GTK+ to process events, so that the UI is responsive:
  while(Gtk::Main::events_pending())
   Gtk::Main::iteration();
}

void ConnectionPool::on_epc_progress_end(gpointer user_data)
{
  ConnectionPool* connection_pool = (ConnectionPool*)user_data;

  //Delete the dialog:
  if(connection_pool->m_dialog_epc_progress)
  {
    delete connection_pool->m_dialog_epc_progress;
    connection_pool->m_dialog_epc_progress = 0;
  }
}


/** Advertise self-hosting via avahi:
 */
void ConnectionPool::avahi_start_publishing()
{
  if(m_epc_publisher)
    return;

  std::cout << "debug: ConnectionPool::avahi_start_publishing" << std::endl;

  //Publish the document contents over HTTPS (discoverable via avahi):
  const Document_Glom* document = get_document();
  if(!document)
    return;

  m_epc_publisher = epc_publisher_new(document->get_database_title().c_str(), "glom", NULL);
  epc_publisher_set_protocol(m_epc_publisher, publish_protocol);
  
  epc_publisher_add_handler(m_epc_publisher, "document", on_publisher_document_requested, this /* user_data */, NULL);

  //Password-protect the document,
  //because the document's structure could be considered as a business secret:
  epc_publisher_set_auth_flags(m_epc_publisher, EPC_AUTH_PASSWORD_TEXT_NEEDED);
  epc_publisher_set_auth_handler(m_epc_publisher, "document", on_publisher_document_authentication, this /* user_data */, NULL);

  //Install progress callback, so we can keep the UI responsive while libepc is generating certificates for the first time:
  EpcShellProgressHooks callbacks;
  callbacks.begin = &ConnectionPool::on_epc_progress_begin;
  callbacks.update = &ConnectionPool::on_epc_progress_update;
  callbacks.end = &ConnectionPool::on_epc_progress_end;
  epc_shell_set_progress_hooks(&callbacks, this, NULL);
      
  //Prevent the consumer from seeing duplicates,
  //if multiple client computers advertize the same document:
  //
  //Defer announcement until a duplicate disappears:
  epc_publisher_set_collision_handling(m_epc_publisher, EPC_COLLISIONS_UNIQUE_SERVICE);
  //
  // How to uniquely-identify the service. TODO: Use some additional criteria, such as the real host name.
  if(!m_database.empty())
    epc_publisher_set_service_cookie(m_epc_publisher, m_database.c_str());

  //Start the publisher, serving HTTPS:
  GError* error = 0;
  epc_publisher_run_async(m_epc_publisher, &error);
  if(error)
  {
    std::cout << "Glom: ConnectionPool::avahi_start_publishing(): Error while running epc_publisher_run_async: " << error->message << std::endl;
    g_clear_error(&error);
  }
}

void ConnectionPool::avahi_stop_publishing()
{
  if(!m_epc_publisher)
    return;

  std::cout << "debug: ConnectionPool::avahi_stop_publishing" << std::endl;

#ifndef G_OS_WIN32
  epc_publisher_quit(m_epc_publisher);
#endif // !G_OS_WIN32
  g_object_unref(m_epc_publisher);
  m_epc_publisher = NULL;
}
#endif // !G_OS_WIN32

void ConnectionPool::set_get_document_func(const SlotGetDocument& slot)
{
  m_slot_get_document = slot;
}

#endif // !GLOM_ENABLE_CLIENT_ONLY


} //namespace Glom
