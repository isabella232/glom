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
#include <gtkmm/messagedialog.h>
#include <bakery/bakery.h>
#include <glibmm/i18n.h>

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
: m_sharedconnection_refcount(0),
  m_ready_to_connect(false),
  m_pFieldTypes(0)
{
  m_list_ports.push_back("5433"); //Ubuntu Dapper seems to default to this for Postgres 8.1, probably to avoid a clash with Postgres 7.4
  m_list_ports.push_back("5432"); 
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
        for(type_list_ports::const_iterator iter_port = m_list_ports.begin(); iter_port != m_list_ports.end(); ++iter_port)
        { 
          const Glib::ustring cnc_string_main = "HOST=" + get_host() + ";USER=" + m_user + ";PASSWORD=" + m_password + ";PORT=" + *iter_port;

          Glib::ustring cnc_string = cnc_string_main;

          if(!m_database.empty())
            cnc_string += (";DATABASE=" + m_database);
          else
            cnc_string += (";DATABASE=" + default_database);

          //std::cout << "connecting: cnc string: " << cnc_string << std::endl;
          std::cout << std::endl << "Glom: trying to connect on port=" << *iter_port << std::endl;

          //*m_refGdaConnection = m_GdaClient->open_connection(m_GdaDataSourceInfo.get_name(), m_GdaDataSourceInfo.get_username(), m_GdaDataSourceInfo.get_password() );
          //m_refGdaConnection.clear(); //Make sure any previous connection is really forgotten.
          m_refGdaConnection = m_GdaClient->open_connection_from_string("PostgreSQL", cnc_string);
          if(m_refGdaConnection)
          {
            //g_warning("ConnectionPool: connection opened");

            //Remember what port is working:
            m_port = *iter_port;

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
            std::cout << "ConnectionPool::connect() Attempt to connect to database failed on port=" << *iter_port << ", database=" << m_database << std::endl;

            bool bJustDatabaseMissing = false;
            if(!m_database.empty())
            {
              std::cout << "  ConnectionPool::connect() Attempting to connect without specifying the database." << std::endl;

              //If the connection failed while looking for a database,
              //then try connecting without the database:
              Glib::ustring cnc_string = cnc_string_main;
              cnc_string += (";DATABASE=" + default_database);

              //std::cout << "connecting: cnc string: " << cnc_string << std::endl;
              std::cout << "Glom: connecting." << std::endl;

              Glib::RefPtr<Gnome::Gda::Connection> gda_connection =  m_GdaClient->open_connection_from_string("PostgreSQL", cnc_string);
              if(gda_connection) //If we could connect without specifying the database.
              {
                bJustDatabaseMissing = true;
                connection_to_default_database_possible = true;
                m_port = *iter_port;
              }
              else
              {
                std::cerr << "    ConnectionPool::connect() connection also failed when not specifying database." << std::endl;
              }
            }

            if(bJustDatabaseMissing)
              std::cout << "  (Connection succeeds, but not to the specific database on port=" << *iter_port << ", database=" << m_database << std::endl;
            else
              std::cerr << "  (Could not connect even to the default database on port=" << *iter_port << ", database=" << m_database  << std::endl;


            //handle_error(true /* cerr only */);
          }
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


void ConnectionPool::set_host(const Glib::ustring& value)
{
  m_host = value;  
}

void ConnectionPool::set_user(const Glib::ustring& value)
{
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
        dialog.run();
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
