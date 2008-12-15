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

#include <glom/libglom/connectionpool_backends/sqlite.h>

#include <giomm/file.h>

namespace Glom
{

namespace ConnectionPoolBackends
{

Sqlite::Sqlite()
{
}

void Sqlite::set_database_directory_uri(const std::string& directory_uri)
{
  m_database_directory_uri = directory_uri;
}

const std::string& Sqlite::get_database_directory_uri() const
{
  return m_database_directory_uri;
}

Glib::RefPtr<Gnome::Gda::Connection> Sqlite::connect(const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<ExceptionConnection>& error)
{
  // Check if the database file exists. If it does not, then we don't try to
  // connect. libgda would create the database file if necessary, but we need
  // to ensure slightly different semantics.
  Glib::RefPtr<Gio::File> db_dir = Gio::File::create_for_uri(m_database_directory_uri);
  Glib::RefPtr<Gio::File> db_file = db_dir->get_child(database + ".db");

  Glib::RefPtr<Gnome::Gda::Connection> connection;
  if(db_file->query_file_type() == Gio::FILE_TYPE_REGULAR)
  {
    // Convert URI to path, for GDA connection string
    std::string database_directory = db_dir->get_path();

    const Glib::ustring cnc_string = "DB_DIR=" + database_directory + ";DB_NAME=" + database;
    const Glib::ustring auth_string = Glib::ustring::compose("USERNAME=%1;PASSWORD=%2", username, password);
    
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    try
    {
      connection = Gnome::Gda::Connection::open_from_string("SQLite", cnc_string, auth_string);
    }
    catch(const Glib::Error& ex)
    {
#else
    std::auto_ptr<Glib::Error> error;
    connection = Gnome::Gda::Connection::open_from_string("SQLite", cnc_string, username, auth_string, Gnome::Gda::CONNECTION_OPTIONS_NONE, error);
    if(error.get())
    {
      const Glib::Error& ex = *error.get();
#endif
    }
  }

  if(!connection)
  {
    // If the database directory is valid, then only the database (file) is
    // missing, otherwise we pretend the "server" is not running.
    if(db_dir->query_file_type() == Gio::FILE_TYPE_DIRECTORY)
      error.reset(new ExceptionConnection(ExceptionConnection::FAILURE_NO_DATABASE));
    else
      error.reset(new ExceptionConnection(ExceptionConnection::FAILURE_NO_SERVER));
  }

  return connection;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool Sqlite::create_database(const Glib::ustring& database_name, const Glib::ustring& /* username */, const Glib::ustring& /* password */, std::auto_ptr<Glib::Error>& error)
{
  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(m_database_directory_uri);
  std::string database_directory = file->get_path(); 
  const Glib::ustring cnc_string = Glib::ustring::compose("DB_DIR=%1;DB_NAME=%2", database_directory, database_name);

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    Glib::RefPtr<Gnome::Gda::Connection> cnc = 
      Gnome::Gda::Connection::open_from_string("SQLite", cnc_string, "");
  }
  catch(const Glib::Error& ex)
  {
    error.reset(new Glib::Error(ex));
    return false;
  }
#else
  Glib::RefPtr<Gnome::Gda::Connection> cnc = 
    Gnome::Gda::Connection::open_from_string("SQLite", cnc_string, "", Gnome::Gda::CONNECTION_OPTIONS_NONE, error);
  if(error.get() != 0)
    return false
#endif
    
  return true;
}
#endif

} // namespace ConnectionPoolBackends

} // namespace Glom
