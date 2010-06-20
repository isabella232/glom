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
#include <libglom/connectionpool_backends/backend.h>

#include <libglom/libglom_config.h>

namespace Glom
{

namespace ConnectionPoolBackends
{

class Sqlite : public Backend
{
public:
  Sqlite();

  void set_database_directory_uri(const std::string& directory_uri);
  const std::string& get_database_directory_uri() const;

private:
  virtual Field::sql_format get_sql_format() const { return Field::SQL_FORMAT_SQLITE; }
  virtual bool supports_remote_access() const { return false; }
  virtual Glib::ustring get_string_find_operator() const { return "LIKE"; }
  virtual const char* get_public_schema_name() const { return "main"; }

  bool add_column_to_server_operation(const Glib::RefPtr<Gnome::Gda::ServerOperation>& operation, GdaMetaTableColumn* column, unsigned int i);
  bool add_column_to_server_operation(const Glib::RefPtr<Gnome::Gda::ServerOperation>& operation, const sharedptr<const Field>& column, unsigned int i);

  typedef std::vector<Glib::ustring> type_vec_strings;
  //typedef std::vector<sharedptr<const Field> > type_vec_fields;
  typedef std::map<Glib::ustring, sharedptr<const Field> > type_mapFieldChanges;

  bool recreate_table(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const type_vec_strings& fields_removed, const type_vec_const_fields& fields_added, const type_mapFieldChanges& fields_changed) throw();

  virtual void add_column(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const sharedptr<const Field>& field);
  virtual void drop_column(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const Glib::ustring& field_name);
  virtual bool change_columns(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const type_vec_const_fields& old_fields, const type_vec_const_fields& new_fields) throw();

  virtual Glib::RefPtr<Gnome::Gda::Connection> connect(const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password);

  /** Creates a new database.
   */
  virtual bool create_database(const Glib::ustring& database_name, const Glib::ustring& username, const Glib::ustring& password);

private:
  std::string m_database_directory_uri;
};

} //namespace ConnectionPoolBackends

} //namespace Glom

#endif //GLOM_BACKEND_SQLITE_H

