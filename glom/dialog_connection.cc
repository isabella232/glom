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
 
#include "dialog_connection.h"
#include "box_db.h" //For Box_DB::connect_to_server().
#include <glibmm/i18n.h>

Dialog_Connection::Dialog_Connection(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  Base_DB(),
  m_entry_host(0),
  m_entry_user(0),
  m_entry_password(0),
  m_label_database(0)
{
  refGlade->get_widget("entry_host", m_entry_host);
  refGlade->get_widget("entry_user", m_entry_user);
  refGlade->get_widget("entry_password", m_entry_password);
  refGlade->get_widget("label_database", m_label_database);
}

Dialog_Connection::~Dialog_Connection()
{
}

sharedptr<SharedConnection> Dialog_Connection::connect_to_server_with_connection_settings() const
{
  //TODO: Bakery::BusyCursor(*get_app_window());

  sharedptr<SharedConnection> result(0);

  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
  {
    //Set the connection details in the ConnectionPool singleton.
    //The ConnectionPool will now use these every time it tries to connect.
    connection_pool->set_database(m_pDocument->get_connection_database());
    
    connection_pool->set_host(m_entry_host->get_text());
    connection_pool->set_user(m_entry_user->get_text());
    connection_pool->set_password(m_entry_password->get_text());
    if(m_pDocument)
    {
      connection_pool->set_database(m_pDocument->get_connection_database());
    }
    
    connection_pool->set_ready_to_connect(); //Box_DB::connect_to_server() will now attempt the connection-> Shared instances of m_Connection will also be usable.

    result = Box_DB::connect_to_server();

    /*
    if(m_pDocument)
    {
      m_pDocument->set_connection_server(m_entry_host->get_text());
      m_pDocument->set_connection_user(m_entry_user->get_text());
    }
    */
  }

  return result;
}

void Dialog_Connection::load_from_document()
{
  if(m_pDocument)
  {
    //Load server and user:
    m_entry_host->set_text(m_pDocument->get_connection_server());

    Glib::ustring user = m_pDocument->get_connection_user(); //TODO: Offer a drop-down list of users.

    if(user.empty())
    {
      //Default to the UNIX user name, which is often the same as the Postgres user name:
      const char* pchUser = getenv("USER"); 
      if(pchUser)
        user = pchUser;
    }

    m_entry_user->set_text(user);

    //Show the database to be opened, or created.
    //TODO: In future, we can hide this completely.
    Glib::ustring database = m_pDocument->get_connection_database();
    if(database.empty())
      database = _("Not yet created.");

    m_label_database->set_text(database);
  }
  else
    g_warning("ialog_Connection::load_from_document(): no document");

}

