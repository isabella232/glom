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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "config.h" //For GLOM_ENABLE_POSTGRESQL
#include "dialog_connection.h"
#include <glibmm/i18n.h>

#include <libglom/connectionpool.h>

#ifdef GLOM_ENABLE_POSTGRESQL
#include <libglom/connectionpool_backends/postgres_central.h>
#include <libglom/connectionpool_backends/postgres_self.h>
#endif //#ifdef GLOM_ENABLE_POSTGRESQL

#include <iostream>

namespace Glom
{

const char Dialog_Connection::glade_id[] = "dialog_connection";
const bool Dialog_Connection::glade_developer(false);

Dialog_Connection::Dialog_Connection(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  Base_DB(),
  m_entry_host(nullptr),
  m_entry_user(nullptr),
  m_entry_password(nullptr),
  m_label_database(nullptr),
  m_label_note(nullptr)
{
  builder->get_widget("entry_host", m_entry_host);
  builder->get_widget("entry_user", m_entry_user);
  builder->get_widget("entry_password", m_entry_password);
  builder->get_widget("label_database", m_label_database);
  builder->get_widget("connection_note", m_label_note);
}

std::shared_ptr<SharedConnection> Dialog_Connection::connect_to_server_with_connection_settings() const
{
  //std::cout << "debug: Dialog_Connection::connect_to_server_with_connection_settings()" << std::endl;

  //TODO: BusyCursor busy_cursor(get_app_window());

  std::shared_ptr<SharedConnection> result;

  auto connection_pool = ConnectionPool::get_instance();
  g_assert(connection_pool);
  
  //Set the connection details in the ConnectionPool singleton.
  //The ConnectionPool will now use these every time it tries to connect.

  const auto document = get_document();
  if(!document)
    return result;

  //std::cout << "debug: " << G_STRFUNC << ": m_database_name=" << m_database_name << std::endl;
  connection_pool->set_database(m_database_name);

#ifdef GLOM_ENABLE_POSTGRESQL
  if(document->get_hosting_mode() == Document::HostingMode::POSTGRES_CENTRAL)
  {
    auto backend = connection_pool->get_backend();
    auto central = std::dynamic_pointer_cast<ConnectionPoolBackends::PostgresCentralHosted>(backend);
    g_assert(central);

    central->set_host(m_entry_host->get_text());
  }
#endif //GLOM_ENABLE_POSTGRESQL

  connection_pool->set_user(m_entry_user->get_text());
  connection_pool->set_password(m_entry_password->get_text());


  result = Base_DB::connect_to_server(const_cast<Dialog_Connection*>(this));

#ifdef GLOM_ENABLE_POSTGRESQL
  //Remember the port, 
  //to make opening faster next time,
  //and so we can tell connecting clients (using browse network) what port to use:
  auto unconst = std::const_pointer_cast<Document>(document);

  if(document->get_hosting_mode() == Document::HostingMode::POSTGRES_CENTRAL)
  {
    auto backend = connection_pool->get_backend();
    auto central = std::dynamic_pointer_cast<ConnectionPoolBackends::PostgresCentralHosted>(backend);
    g_assert(central);

    unconst->set_connection_port(central->get_port() );
    // As we know the port of the database already, we don't need to try
    // other ports anymore:
    unconst->set_connection_try_other_ports(false);
  }

#ifndef GLOM_ENABLE_CLIENT_ONLY
  else if(document->get_hosting_mode() == Document::HostingMode::POSTGRES_SELF)
  {
    auto backend = connection_pool->get_backend();
    auto self = std::dynamic_pointer_cast<ConnectionPoolBackends::PostgresSelfHosted>(backend);
    g_assert(self);

    unconst->set_connection_port(self->get_port() );
    unconst->set_connection_try_other_ports(false);
  }
#endif //GLOM_ENABLE_CLIENT_ONLY
 
#endif //GLOM_ENABLE_POSTGRESQL
 
  return result;
}

void Dialog_Connection::load_from_document()
{
  auto document = get_document();
  if(document)
  {
#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifdef GLOM_ENABLE_POSTGRESQL
    //Load server and user:
    if(document->get_hosting_mode() != Document::HostingMode::POSTGRES_CENTRAL)
    {
       m_entry_host->set_text("(self hosted)");
       m_entry_host->set_sensitive(false);
    }
    else
#endif // LOM_ENABLE_POSTGRESQL
#endif // !GLOM_ENABLE_CLIENT_ONLY
    {
      Glib::ustring host = document->get_connection_server();
      if(host.empty())
        host = "localhost";
     
      m_entry_host->set_text(host);
      m_entry_host->set_sensitive(true);
    }

    Glib::ustring user = document->get_connection_user(); //TODO: Offer a drop-down list of users.

    if(user.empty())
    {
      //Default to the UNIX user name, which is often the same as the Postgres user name:
      const auto pchUser = getenv("USER"); 
      if(pchUser)
        user = pchUser;
    }

    m_entry_user->set_text(user);

    //Show the database to be opened, or created.
    //TODO: In future, we can hide this completely.
    set_database_name(document->get_connection_database());
  }
  else
    std::cerr << G_STRFUNC << ": no document" << std::endl;

}

///Disable irrelevant fields:
void Dialog_Connection::set_connect_to_browsed()
{
  m_entry_host->set_sensitive(false);
}

void Dialog_Connection::set_username(const Glib::ustring& user)
{
  m_entry_user->set_text(user);
}

void Dialog_Connection::set_password(const Glib::ustring& password)
{
  m_entry_password->set_text(password);
}

void Dialog_Connection::set_database_name(const Glib::ustring& name)
{
  m_database_name = name;
  if(m_database_name.empty())
    m_label_database->set_text(_("Not yet created."));
  else
    m_label_database->set_text(name);
}

void Dialog_Connection::get_username_and_password(Glib::ustring& username, Glib::ustring& password) const
{
  username = m_entry_user->get_text();
  password = m_entry_password->get_text();
}

void Dialog_Connection::set_confirm_existing_user_note()
{
  m_label_note->set_text(Glib::ustring());
}

} //namespace Glom
