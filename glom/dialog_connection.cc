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
  //std::cout << "debug: Dialog_Connection::connect_to_server_with_connection_settings()" << std::endl;

  //TODO: Bakery::BusyCursor busy_cursor(get_app_window());

  sharedptr<SharedConnection> result(0);

  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
  {
    //Set the connection details in the ConnectionPool singleton.
    //The ConnectionPool will now use these every time it tries to connect.

    //const Document_Glom* document = get_document();
    //if(document)
    //{
      //std::cout << "debug: Dialog_Connection::connect_to_server_with_connection_settings(): m_database_name=" << m_database_name << std::endl;
      connection_pool->set_database(m_database_name);

      connection_pool->set_host(m_entry_host->get_text());
      connection_pool->set_user(m_entry_user->get_text());
      connection_pool->set_password(m_entry_password->get_text());
      //if(document)
      //{
      //  connection_pool->set_database(document->get_connection_database());
      //}
    //}

    connection_pool->set_ready_to_connect(); //Box_DB::connect_to_server() will now attempt the connection-> Shared instances of m_Connection will also be usable.

    result = Box_DB::connect_to_server();

    /*
    if(document)
    {
      document->set_connection_server(m_entry_host->get_text());
      document->set_connection_user(m_entry_user->get_text());
    }
    */
  }
  else
     std::cerr << "Dialog_Connection::connect_to_server_with_connection_settings(): ConnectionPool::get_instance() failed." << std::endl;

  return result;
}

void Dialog_Connection::load_from_document()
{
  Document_Glom* document = get_document();
  if(document)
  {
    //Load server and user:
    m_entry_host->set_text(document->get_connection_server());

    Glib::ustring user = document->get_connection_user(); //TODO: Offer a drop-down list of users.

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
    set_database_name(document->get_connection_database());
  }
  else
    g_warning("Dialog_Connection::load_from_document(): no document");

}

void Dialog_Connection::set_database_name(const Glib::ustring& name)
{
  m_database_name = name;
  if(m_database_name.empty())
    m_label_database->set_text(_("Not yet created."));
  else
    m_label_database->set_text(name);
}

