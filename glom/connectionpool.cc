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
 
#include "connectionpool.h"


ExceptionConnection::ExceptionConnection(failure_type failure)
: m_failure_type(failure)
{
}

ExceptionConnection::~ExceptionConnection() throw()
{
}

const char* ExceptionConnection::what()
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

        Glib::ustring cnc_string = "HOST=" + get_host() + ";USER=" + m_user + ";PASSWORD=" + m_password;
      
        if(!m_database.empty())
          cnc_string += (";DATABASE=" + m_database);
	else
	  cnc_string += (";DATABASE=" + default_database);

        std::cout << "connecting: cnc string: " << cnc_string << std::endl;

        //*m_refGdaConnection = m_GdaClient->open_connection(m_GdaDataSourceInfo.get_name(), m_GdaDataSourceInfo.get_username(), m_GdaDataSourceInfo.get_password() );
        m_refGdaConnection = m_GdaClient->open_connection_from_string("PostgreSQL", cnc_string);
        if(m_refGdaConnection)
        {
          //g_warning("ConnectionPool: connection opened");
          
          //Create the fieldtypes member if it has not already been done:
          if(!m_pFieldTypes)
            m_pFieldTypes = new FieldTypes(m_refGdaConnection);
            
          //Open the database, if one has been specified:
          /* This does not seem to work in libgda's postgres provider, so we specify it in the cnc_string instead:
          std::cout << "  database = " << m_database << std::endl;
          if(!m_database.empty())
            m_refGdaConnection->change_database(m_database);
          */
          
          return connect(); //Call this method recursively. This time m_refGdaConnection exists.
        }
        else
        {
          std::cout << "ConnectionPool::connect() Attempt to connect to database failed: " << m_database << std::endl;

          bool bJustDatabaseMissing = false;
          if(!m_database.empty())
          {
             std::cout << "  ConnectionPool::connect() Attempting to connect without specifying the database." << std::endl;
             
             //If the connection failed while looking for a database,
             //then try connecting without the database:
             Glib::ustring cnc_string = "HOST=" + get_host() + ";USER=" + m_user + ";PASSWORD=" + m_password;
	     cnc_string += (";DATABASE=" + default_database);
             
             std::cout << "connecting: cnc string: " << cnc_string << std::endl;
              
             Glib::RefPtr<Gnome::Gda::Connection> gda_connection =  m_GdaClient->open_connection_from_string("PostgreSQL", cnc_string);
             if(gda_connection) //If we could connect without specifying the database.
               bJustDatabaseMissing = true;
             else
             {
                 std::cerr << "    ConnectionPool::connect() connection also failed when not specifying database." << std::endl;
             }
          }

          g_warning("ConnectionPool::connect() throwing exception.");
          if(bJustDatabaseMissing)
            g_warning("  (Connection succeeds, but not to the specific database).");
          else
            g_warning("  (Could not connect even to the default database.)");
            
          throw ExceptionConnection(bJustDatabaseMissing ? ExceptionConnection::FAILURE_NO_DATABASE : ExceptionConnection::FAILURE_NO_SERVER);
        }
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




  

