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
 
#include <glom/libglom/connectionpool.h>
#include <bakery/bakery.h>
#include <libgnomevfsmm.h>
#include <glib/gstdio.h> //For g_remove().
#include <glom/libglom/spawn_with_feedback.h>
#include <glom/libglom/utils.h>
#include <glibmm/i18n.h>

#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/socket.h>
#include <netinet/in.h> //For sockaddr_in

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>
#include <avahi-common/alternative.h>
#include <avahi-glib/glib-watch.h>
#include <avahi-glib/glib-malloc.h>

#include "gst-package.h"

#include "config.h"

namespace Glom
{

#define DEFAULT_CONFIG_PG_HBA "local   all         postgres                          ident sameuser\n\
\n\
# TYPE  DATABASE    USER        CIDR-ADDRESS          METHOD\n\
\n\
# local is for Unix domain socket connections only\n\
local   all         all                               ident sameuser\n\
# IPv4 local connections:\n\
host    all         all         127.0.0.1/32          md5\n\
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
: m_self_hosting_active(false),
  m_sharedconnection_refcount(0),
  m_ready_to_connect(false),
  m_pFieldTypes(0),
  m_avahi_group(0),
  m_avahi_client(0),
  m_avahi_mainloop(0)
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
sharedptr<SharedConnection> ConnectionPool::get_and_connect()
{
  sharedptr<SharedConnection> result(0);

  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
  {
    result = connection_pool->connect();
  }

  return result;
}

sharedptr<SharedConnection> ConnectionPool::connect()
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
        Glib::ustring port = m_port;

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
          const Glib::ustring cnc_string_main = "HOST=" + get_host() + ";USER=" + m_user + ";PASSWORD=" + m_password + ";PORT=" + port;

          Glib::ustring cnc_string = cnc_string_main;

          if(!m_database.empty())
            cnc_string += (";DATABASE=" + m_database);
          else
            cnc_string += (";DATABASE=" + default_database);

          //std::cout << "debug: connecting: cnc string: " << cnc_string << std::endl;
          std::cout << std::endl << "Glom: trying to connect on port=" << port << std::endl;

          //*m_refGdaConnection = m_GdaClient->open_connection(m_GdaDataSourceInfo.get_name(), m_GdaDataSourceInfo.get_username(), m_GdaDataSourceInfo.get_password() );
          //m_refGdaConnection.clear(); //Make sure any previous connection is really forgotten.
          m_refGdaConnection = m_GdaClient->open_connection_from_string("PostgreSQL", cnc_string);
          if(m_refGdaConnection)
          {
            //g_warning("ConnectionPool: connection opened");

            //Remember what port is working:
            m_port = port;

            //Create the fieldtypes member if it has not already been done:
            if(!m_pFieldTypes)
              m_pFieldTypes = new FieldTypes(m_refGdaConnection);  

            //Enforce ISO formats in the communication:
            m_refGdaConnection->execute_single_command("SET DATESTYLE = 'ISO'");  

            //Open the database, if one has been specified:
            /* This does not seem to work in libgda's postgres provider, so we specify it in the cnc_string instead:
            std::cout << "  database = " << m_database << std::endl;
            if(!m_database.empty())
              m_refGdaConnection->change_database(m_database);
            */

            //Get postgres version:
            Glib::RefPtr<Gnome::Gda::DataModel> data_model = m_refGdaConnection->execute_single_command("SELECT version()");
            if(data_model && data_model->get_n_rows() && data_model->get_n_columns())
            {
              Gnome::Gda::Value value = data_model->get_value_at(0, 0);
              if(value.get_value_type() == Gnome::Gda::VALUE_TYPE_STRING)
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

            return connect(); //Call this method recursively. This time m_refGdaConnection exists.
          }
          else
          {
            std::cout << "ConnectionPool::connect() Attempt to connect to database failed on port=" << port << ", database=" << m_database << std::endl;

            bool bJustDatabaseMissing = false;
            if(!m_database.empty())
            {
              std::cout << "  ConnectionPool::connect() Attempting to connect without specifying the database." << std::endl;

              //If the connection failed while looking for a database,
              //then try connecting without the database:
              Glib::ustring cnc_string = cnc_string_main;
              cnc_string += (";DATABASE=" + default_database);

              //std::cout << "debug2: connecting: cnc string: " << cnc_string << std::endl;
              std::cout << "Glom: connecting." << std::endl;

              Glib::RefPtr<Gnome::Gda::Connection> gda_connection =  m_GdaClient->open_connection_from_string("PostgreSQL", cnc_string);
              if(gda_connection) //If we could connect without specifying the database.
              {
                bJustDatabaseMissing = true;
                connection_to_default_database_possible = true;
                m_port = port;
              }
              else
              {
                std::cerr << "    ConnectionPool::connect() connection also failed when not specifying database." << std::endl;
              }
            }

            if(bJustDatabaseMissing)
              std::cout << "  (Connection succeeds, but not to the specific database on port=" << port << ", database=" << m_database << std::endl;
            else
              std::cerr << "  (Could not connect even to the default database on port=" << port << ", database=" << m_database  << std::endl;


            //handle_error(true /* cerr only */);
          }

          //If we got this far then the connection failed, so we should try another port:
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

          trying_remembered_port = false;
        }

        g_warning("ConnectionPool::connect() throwing exception.");
        if(connection_to_default_database_possible)
          std::cout << "  (Connection succeeds, but not to the specific database on port=" << m_port << ", database=" << m_database << std::endl;
        else
              std::cerr << "  (Could not connect even to the default database on any port. database=" << m_database  << std::endl;
        throw ExceptionConnection(connection_to_default_database_possible ? ExceptionConnection::FAILURE_NO_DATABASE : ExceptionConnection::FAILURE_NO_SERVER);
      }
    }
  }
  else
  {
      //g_warning("ConnectionPool::connect(): not ready to connect.");
  }

  return sharedptr<SharedConnection>(0);
}

void ConnectionPool::set_self_hosted(const std::string& data_uri)
{
  m_self_hosting_data_uri = data_uri;
}

void ConnectionPool::set_host(const Glib::ustring& value)
{
  if(value != m_host)
  {
    m_host = value;  
    m_port = Glib::ustring(); /* Force us to try all ports again when connecting for the first time, then remember the working port again. */
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

    //g_warning("ConnectionPool: connection closed");
  }

}

//static
bool ConnectionPool::handle_error(bool cerr_only)
{
  sharedptr<SharedConnection> sharedconnection = get_and_connect();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

    typedef std::list< Glib::RefPtr<Gnome::Gda::Error> > type_list_errors;
    type_list_errors list_errors = gda_connection->get_errors();

    if(!list_errors.empty())
    {
      Glib::ustring error_details;
      for(type_list_errors::iterator iter = list_errors.begin(); iter != list_errors.end(); ++iter)
      {
        if(iter != list_errors.begin())
          error_details += "\n"; //Add newline after each error.

        error_details += (*iter)->get_description();
        std::cerr << "Internal error (Database): " << error_details << std::endl;
      }

      //For debugging only:
      //Gtk::Dialog* dialog = 0;
      //dialog->run(); //Force a crash.

      if(!cerr_only)
      {
        Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Internal error")), true, Gtk::MESSAGE_WARNING );
        dialog.set_secondary_text(error_details);
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

bool ConnectionPool::directory_exists(const std::string& uri)
{
  Glib::RefPtr<Gnome::Vfs::Uri> vfsuri = Gnome::Vfs::Uri::create(uri);
  return vfsuri->uri_exists();
}

bool ConnectionPool::start_self_hosting()
{
  if(m_self_hosting_active)
    return true; //Just do it once.

  const std::string dbdir_uri = m_self_hosting_data_uri;

  if(!(directory_exists(dbdir_uri)))
  {
    std::cerr << "ConnectionPool::create_self_hosting(): The directory could not be found: " << dbdir_uri << std::endl;
    return false;
  }

  const std::string dbdir = Glib::filename_from_uri(dbdir_uri);
  g_assert(!dbdir.empty());

  const std::string dbdir_data = Glib::build_filename(dbdir, "data");
  const std::string dbdir_data_uri = Glib::filename_to_uri(dbdir_data);
  if(!(directory_exists(dbdir_data_uri)))
  {
    std::cerr << "ConnectionPool::create_self_hosting(): The data sub-directory could not be found." << dbdir_data_uri << std::endl;
    return false;
  }


  const int available_port = discover_first_free_port(PORT_POSTGRESQL_SELF_HOSTED_START, PORT_POSTGRESQL_SELF_HOSTED_END);
  if(available_port == 0)
  {
    std::cerr << "ConnectionPool::create_self_hosting(): No port was available between " << PORT_POSTGRESQL_SELF_HOSTED_START << " and " << PORT_POSTGRESQL_SELF_HOSTED_END << std::endl;
    return false;
  }

  const Glib::ustring port_as_text = Utils::string_from_decimal(available_port);


  // -D specifies the data directory.
  // -c config_file= specifies the configuration file
  // -k specifies a directory to use for the socket. This must be writable by us.
  // POSTGRES_POSTMASTER_PATH is defined in config.h, based on the configure.
  const std::string command_postgres_start = POSTGRES_UTILS_PATH "/postmaster -D \"" + dbdir_data + "\" "
                                  + " -p " + port_as_text 
                                  + " -h \"*\" " //Equivalent to listen_addresses in postgresql.conf. Listen to all IP addresses, so any client can connect (with a username+password)
                                  + " -c hba_file=\"" + dbdir + "/config/pg_hba.conf\""
                                  + " -c ident_file=\"" + dbdir + "/config/pg_ident.conf\""
                                  + " -k \"" + dbdir + "\""
                                  + " --external_pid_file=\"" + dbdir + "/pid\"";

  const std::string command_check_postgres_has_started = POSTGRES_UTILS_PATH "/pg_ctl status -D \"" + dbdir_data + "\"";

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
  }

  m_port = port_as_text;
  m_self_hosting_active = true;

  //Let clients discover this server via avahi:
  avahi_start_publishing();

  return true;
}

void ConnectionPool::stop_self_hosting()
{
  if(!m_self_hosting_active)
    return; //Don't try to stop it if we have not started it.

  /* Stop advertising the self-hosting database server via avahi: */
  avahi_stop_publishing();

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
  const std::string command_postgres_stop = POSTGRES_UTILS_PATH "/pg_ctl -D \"" + dbdir_data + "\" stop";
  const bool result = Glom::Spawn::execute_command_line_and_wait(command_postgres_stop, _("Stopping Database Server"));
  if(!result)
  {
    std::cerr << "Error while attempting to stop self-hosting of the database."  << std::endl;
  }

  m_self_hosting_active = false;
}


bool ConnectionPool::create_self_hosting()
{
  if(m_self_hosting_data_uri.empty())
  {
    std::cerr << "ConnectionPool::create_self_hosting(): m_self_hosting_data_uri is empty." << std::endl;
    return false;
  }

  //Get the filepath of the directory that we should create:
  const std::string dbdir_uri = m_self_hosting_data_uri;
  //std::cout << "debug: dbdir_uri=" << dbdir_uri << std::endl;
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
    return false;
  }

  //Create the config directory:
  const std::string dbdir_config = dbdir + "/config";
  mkdir_succeeded = g_mkdir_with_parents(dbdir_config.c_str(), 0770);
  if(mkdir_succeeded == -1)
  {
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

  const std::string temp_pwfile = "/tmp/glom_initdb_pwfile";
  create_text_file(temp_pwfile, get_password());

  const std::string command_initdb = POSTGRES_UTILS_PATH "/initdb -D \"" + dbdir_data + "\"" +
                                        " -U " + username + " --pwfile=\"" + temp_pwfile + "\""; 
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

bool ConnectionPool::create_text_file(const std::string& file_uri, const std::string& contents)
{
  Gnome::Vfs::Handle write_handle;

  try
  {
    //0660 means "this user and his group can read and write this non-executable file".
    //The 0 prefix means that this is octal.
    write_handle.create(file_uri, Gnome::Vfs::OPEN_WRITE, false, 0660 /* leading zero means octal */);
  }
  catch(const Gnome::Vfs::exception& ex)
  {
    std::cerr << "ConnectionPool::create_text_file(): exception caught during file create: " << ex.what() << std::endl;

    // If the operation was not successful, print the error and abort
    return false; // print_error(ex, output_uri_string);
  }

  try
  {
    //Write the data to the output uri
    GnomeVFSFileSize bytes_written = write_handle.write(contents.data(), contents.size());
    if(bytes_written != contents.size())
      return false;
  }
  catch(const Gnome::Vfs::exception& ex)
  {
    std::cerr << "ConnectionPool::create_text_file(): exception caught during write: " << ex.what() << std::endl;

    // If the operation was not successful, print the error and abort
    return false; //print_error(ex, output_uri_string);
  }

  return true; //Success. (At doing nothing, because nothing needed to be done.)
}

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
#ifdef EADDRINUSE //Some BSDs don't have this.
    && (errno != EADDRINUSE)
#endif 
#ifdef EPORTINUSE //Linux doesn't have this.
    && (errno != EPORTINUSE)
#endif
    ))
    {
      close(fd);

      std::cout << "debug: ConnectionPool::discover_first_free_port(): Found: returning " << port_to_try << std::endl;
      return port_to_try;
    }
    else
    {
      std::cout << "debug: ConnectionPool::discover_first_free_port(): port in use: " << port_to_try << std::endl;
    }

    ++port_to_try;
  }

  close(fd);

  std::cout << "debug: ConnectionPool::discover_first_free_port(): No port was available." << std::endl;
  return 0;
}

// Message to packagers:
// If your Glom package does not depend on PostgreSQL, for some reason, 
// then your distro-specific patch should uncomment this #define.
// and implement ConnectionPool::install_posgres().
// But please, just make your Glom package depend on PostgreSQL instead, 
// because this is silly.
//
//#define DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED 1

bool ConnectionPool::check_postgres_is_available_with_warning()
{
  const std::string binpath = Glib::build_filename(POSTGRES_UTILS_PATH, "postmaster");
  const Glib::ustring uri_binpath = Glib::filename_to_uri(binpath);
  if(Bakery::App_WithDoc::file_exists(uri_binpath))
    return true;
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
#endif
}



static void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, void *userdata)
{
    ConnectionPool* self = (ConnectionPool*)userdata;

    assert(g == self->m_avahi_group || self->m_avahi_group == NULL);

    /* Called whenever the entry group state changes */

    switch (state) {
        case AVAHI_ENTRY_GROUP_ESTABLISHED :
            /* The entry group has been established successfully */
            fprintf(stderr, "Service '%s' successfully established.\n", self->m_avahi_service_name.c_str());
            break;

        case AVAHI_ENTRY_GROUP_COLLISION : {
   
            /* A service name collision happened. Let's pick a new name */
            char* n = avahi_alternative_service_name(self->m_avahi_service_name.c_str());
            self->m_avahi_service_name = n;
            avahi_free(n);
            n = 0;
            
            fprintf(stderr, "Service name collision, renaming service to '%s'\n", self->m_avahi_service_name.c_str());
            
            /* And recreate the services */
            self->avahi_create_services(avahi_entry_group_get_client(g));
            break;
        }

        case AVAHI_ENTRY_GROUP_FAILURE :

            fprintf(stderr, "Entry group failure: %s\n", avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(g))));

            /* Some kind of failure happened while we were registering our services */
            g_main_loop_quit (self->m_avahi_mainloop);
            break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
            ;
    }
}

void ConnectionPool::avahi_create_services(AvahiClient *c)
{
    int ret;
    assert(c);

    bool failed = false;

    /* If this is the first time we're called, let's create a new entry group */
    if (!m_avahi_group)
        if (!(m_avahi_group = avahi_entry_group_new(c, entry_group_callback, this /* user_data */))) {
            fprintf(stderr, "avahi_entry_group_new() failed: %s\n", avahi_strerror(avahi_client_errno(c)));
            failed = true;
        }
    
    if(!failed)
    {
      fprintf(stderr, "Adding service '%s'\n", m_avahi_service_name.c_str());
    }

    const int port = atoi(m_port.c_str());

    /* Add the service for Glom: */
    if (!failed && (ret = avahi_entry_group_add_service(m_avahi_group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0, m_avahi_service_name.c_str(), "_glom._tcp", NULL, NULL, port, NULL)) < 0) {
        fprintf(stderr, "Failed to add _ipp._tcp service: %s\n", avahi_strerror(ret));
        failed = true;
    }
    
    /* Tell the server to register the service */
    if (!failed && (ret = avahi_entry_group_commit(m_avahi_group)) < 0) {
        fprintf(stderr, "Failed to commit entry_group: %s\n", avahi_strerror(ret));
        failed = true;
    }

    if(failed)
    {
      g_main_loop_quit (m_avahi_mainloop);
    }
}


/* Callback for Avahi API Timeout Event */
static void
avahi_timeout_event (AVAHI_GCC_UNUSED AvahiTimeout *timeout, AVAHI_GCC_UNUSED void *userdata)
{
    g_message ("Avahi API Timeout reached!");
}

/* Callback for GLIB API Timeout Event */
static gboolean
avahi_timeout_event_glib (void *userdata)
{
    GMainLoop *loop = (GMainLoop*)userdata;

    g_message ("GLIB API Timeout reached, quitting main loop!");
    
    /* Quit the application */
    g_main_loop_quit (loop);

    return FALSE; /* Don't re-schedule timeout event */
}

/* Callback for state changes on the Client */
static void
avahi_client_callback (AvahiClient *client, AvahiClientState state, void *userdata)
{
  ConnectionPool* self = (ConnectionPool*)userdata;

  g_message ("Avahi Client State Change: %d", state);

   /* Called whenever the client or server state changes */
   switch (state) {
        case AVAHI_CLIENT_S_RUNNING:
        
            /* The server has startup successfully and registered its host
             * name on the network, so it's time to create our services */
            if (!self->m_avahi_group)
                self->avahi_create_services(client);
            break;

        case AVAHI_CLIENT_FAILURE:
            
            fprintf(stderr, "Client failure: %s\n", avahi_strerror(avahi_client_errno(client)));
            g_main_loop_quit (self->m_avahi_mainloop);
            
            break;

        case AVAHI_CLIENT_S_COLLISION:
        
            /* Let's drop our registered services. When the server is back
             * in AVAHI_SERVER_RUNNING state we will register them
             * again with the new host name. */
            
        case AVAHI_CLIENT_S_REGISTERING:

            /* The server records are now being established. This
             * might be caused by a host name change. We need to wait
             * for our own records to register until the host name is
             * properly esatblished. */
            
            if (self->m_avahi_group)
                avahi_entry_group_reset(self->m_avahi_group);
            
            break;

        case AVAHI_CLIENT_CONNECTING:
            ;
    }
}

/** Advertise self-hosting via avahi:
 */
void ConnectionPool::avahi_start_publishing()
{
  m_avahi_service_name = "glom_selfhosted_" + get_database();

  /* Optional: Tell avahi to use g_malloc and g_free */
  avahi_set_allocator (avahi_glib_allocator ());

  /* Create the GLIB main loop */
  m_avahi_mainloop = g_main_loop_new (NULL, FALSE);

  /* Create the GLIB Adaptor */
  AvahiGLibPoll* glib_poll = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT);
  const AvahiPoll* poll_api = avahi_glib_poll_get (glib_poll);

  /* Example, schedule a timeout event with the Avahi API */
  struct timeval tv;
  avahi_elapse_time (&tv,                         /* timeval structure */
            1000,                                   /* 1 second */
            0);                                     /* "jitter" - Random additional delay from 0 to this value */

  poll_api->timeout_new (poll_api,                /* The AvahiPoll object */
                      &tv,                          /* struct timeval indicating when to go activate */
                      avahi_timeout_event,          /* Pointer to function to call */
                      NULL);                        /* User data to pass to function */

  /* Schedule a timeout event with the glib api */
  g_timeout_add (5000,                            /* 5 seconds */
            avahi_timeout_event_glib,               /* Pointer to function callback */
            m_avahi_mainloop);                                  /* User data to pass to function */

  /* Create a new AvahiClient instance */
  int error = 0;
  m_avahi_client = avahi_client_new (poll_api,            /* AvahiPoll object from above */
                               (AvahiClientFlags)0,
            avahi_client_callback,                  /* Callback function for Client state changes */
            this,                                   /* User data */
            &error);                                /* Error return */

  bool failed = false;
  const char* version = 0;

  /* Check the error return code */
  if (m_avahi_client == NULL)
  {
    /* Print out the error string */
    g_warning ("Error initializing Avahi: %s", avahi_strerror (error));

    failed = true;
  }
  else
  {
    /* Make a call to get the version string from the daemon */
    const char* version = avahi_client_get_version_string (m_avahi_client);

    /* Check if the call suceeded */
    if (version == NULL)
    {
      g_warning ("Error getting version string: %s", avahi_strerror (avahi_client_errno (m_avahi_client)));

      failed = true;
    }
  }
    
  if(!failed)
  {
    if(version)
      g_message ("Avahi Server Version: %s", version);

    /* Start the GLIB Main Loop */
    g_main_loop_run (m_avahi_mainloop);
  }
  else
  {
    /* Clean up */
    if(m_avahi_mainloop)
    {
      g_main_loop_unref (m_avahi_mainloop);
      m_avahi_mainloop = 0;
    }

    if(m_avahi_client)
    {
      avahi_client_free (m_avahi_client);
      m_avahi_client = 0;
    }

    if(glib_poll)
      avahi_glib_poll_free (glib_poll);
  }
}

void ConnectionPool::avahi_stop_publishing()
{
    if(m_avahi_client)
    {
      avahi_client_free (m_avahi_client);
      m_avahi_client = 0;
    }

    /* Clean up */
    if(m_avahi_mainloop)
    {
      g_main_loop_unref (m_avahi_mainloop);
      m_avahi_mainloop = 0;
    }
}



} //namespace Glom
