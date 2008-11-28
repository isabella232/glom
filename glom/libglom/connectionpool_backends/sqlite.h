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

#ifndef GLOM_BACKEND_SQLITE_H
#define GLOM_BACKEND_SQLITE_H

#include <libgdamm.h>
#include <glom/libglom/connectionpool.h>

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

namespace Glom
{

namespace ConnectionPoolBackends
{

class Sqlite : public ConnectionPoolBackend
{
public:
  Sqlite();

  void set_database_directory_uri(const std::string& directory_uri);
  const std::string& get_database_directory_uri() const;

protected:
  virtual Field::sql_format get_sql_format() const { return Field::SQL_FORMAT_SQLITE; }

  virtual Glib::RefPtr<Gnome::Gda::Connection> connect(const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<ExceptionConnection>& error);

  /** Creates a new database.
   */
#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual bool create_database(const Glib::ustring& database_name, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<Glib::Error>& error);
#endif

private:
  Glib::RefPtr<Gnome::Gda::Client> m_refGdaClient;
  std::string m_database_directory_uri;
};

} //namespace ConnectionPoolBackends

} //namespace Glom

#endif //GLOM_BACKEND_SQLITE_H

