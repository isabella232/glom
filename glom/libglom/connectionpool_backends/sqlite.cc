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
    const std::string database_directory = db_dir->get_path();

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
  const std::string database_directory = file->get_path(); 
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

#ifndef GLOM_ENABLE_CLIENT_ONLY

bool Sqlite::add_column_to_server_operation(const Glib::RefPtr<Gnome::Gda::ServerOperation>& operation, GdaMetaTableColumn* column, unsigned int i, std::auto_ptr<Glib::Error>& error)
{
  Glib::ustring name_path = Glib::ustring::compose("/FIELDS_A/@COLUMN_NAME/%1", i);
  Glib::ustring type_path = Glib::ustring::compose("/FIELDS_A/@COLUMN_TYPE/%1", i);
  Glib::ustring pkey_path = Glib::ustring::compose("/FIELDS_A/@COLUMN_PKEY/%1", i);
  Glib::ustring nnul_path = Glib::ustring::compose("/FIELDS_A/@COLUMN_NNUL/%1", i);
  // TODO: Find out whether the column is unique.
  Glib::ustring default_path = Glib::ustring::compose("/FIELDS_A/@COLUMN_DEFAULT/%1", i);

  if(!set_server_operation_value(operation, name_path, column->column_name, error)) return false;
  if(!set_server_operation_value(operation, type_path, column->column_type, error)) return false;
  if(!set_server_operation_value(operation, pkey_path, column->pkey ? "TRUE" : "FALSE", error)) return false;
  if(!set_server_operation_value(operation, nnul_path, !column->nullok ? "TRUE" : "FALSE", error)) return false;

  if(column->default_value)
    if(!set_server_operation_value(operation, default_path, column->default_value, error))
      return false;

  return true;
}

bool Sqlite::add_column_to_server_operation(const Glib::RefPtr<Gnome::Gda::ServerOperation>& operation, const sharedptr<const Field>& column, unsigned int i, std::auto_ptr<Glib::Error>& error)
{
  Glib::ustring name_path = Glib::ustring::compose("/FIELDS_A/@COLUMN_NAME/%1", i);
  Glib::ustring type_path = Glib::ustring::compose("/FIELDS_A/@COLUMN_TYPE/%1", i);
  Glib::ustring pkey_path = Glib::ustring::compose("/FIELDS_A/@COLUMN_PKEY/%1", i);
  Glib::ustring unique_path = Glib::ustring::compose("/FIELDS_A/@COLUMN_UNIQUE/%1", i);
  Glib::ustring default_path = Glib::ustring::compose("/FIELDS_A/@COLUMN_DEFAULT/%1", i);

  if(!set_server_operation_value(operation, name_path, column->get_name(), error)) return false;
  if(!set_server_operation_value(operation, type_path, column->get_sql_type(), error)) return false;
  if(!set_server_operation_value(operation, pkey_path, column->get_primary_key() ? "TRUE" : "FALSE", error)) return false;
  if(!set_server_operation_value(operation, unique_path, column->get_unique_key() ? "TRUE" : "FALSE", error)) return false;

  return true;
}

bool Sqlite::recreate_table(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const type_vecStrings& fields_removed, const type_vecConstFields& fields_added, const type_mapFieldChanges& fields_changed, std::auto_ptr<Glib::Error>& error)
{
  static const gchar* TEMPORARY_TABLE_NAME = "GLOM_TEMP_TABLE"; // TODO: Make sure this is unique.
  static const gchar* TRANSACTION_NAME = "GLOM_RECREATE_TABLE_TRANSACTION";

  Glib::RefPtr<Gnome::Gda::MetaStore> store = connection->get_meta_store();
  Glib::RefPtr<Gnome::Gda::MetaStruct> metastruct = Gnome::Gda::MetaStruct::create(store, Gnome::Gda::META_STRUCT_FEATURE_NONE);
  GdaMetaDbObject* object = metastruct->complement(Gnome::Gda::META_DB_TABLE, Gnome::Gda::Value(), Gnome::Gda::Value(), Gnome::Gda::Value(table_name));
  if(!object) return false;

  Glib::RefPtr<Gnome::Gda::ServerOperation> operation = create_server_operation(connection->get_provider(), connection, Gnome::Gda::SERVER_OPERATION_CREATE_TABLE, error);
  if(!operation) return false;

  if(!set_server_operation_value(operation, "/TABLE_DEF_P/TABLE_NAME", TEMPORARY_TABLE_NAME, error)) return false;

  GdaMetaTable* table = GDA_META_TABLE(object);
  unsigned int i = 0;

  Glib::ustring trans_fields;

  for(GSList* item = table->columns; item != NULL; item = item->next)
  {
    GdaMetaTableColumn* column = GDA_META_TABLE_COLUMN(item->data);

    // Don't add if field was removed
    if(std::find(fields_removed.begin(), fields_removed.end(), column->column_name) != fields_removed.end())
      continue;
#if 0
    {
      // If it was removed, and added again, then it has changed, so use the
      // definition from the added_fields vector.
      type_vecFields::const_iterator iter = std::find_if(fields_added.begin(), fields_added.end(), predicate_FieldHasName<Field>(column->column_name));
      if(iter == fields_added.end())
        continue;
      else
        added = *iter;
    }
#endif

    if(!trans_fields.empty())
      trans_fields += ",";

    const type_mapFieldChanges::const_iterator changed_iter = fields_changed.find(column->column_name);
    if(changed_iter != fields_changed.end())
    {
      // Convert values to date or time, accordingly.
      switch(changed_iter->second->get_glom_type())
      {
      case Field::TYPE_TEXT:
        if(column->gtype == G_TYPE_BOOLEAN)
	  trans_fields += Glib::ustring("(CASE WHEN ") + column->column_name + " = 1 THEN 'true' "
                                              "WHEN "  + column->column_name + " = 0 THEN 'false' "
                                              "WHEN "  + column->column_name + " IS NULL THEN 'false' END)";
	else if(column->gtype == GDA_TYPE_BLOB)
	  trans_fields += "''";
	else
          // Make sure we don't insert NULL strings, as we ignore that concept in Glom.
          trans_fields += Glib::ustring("(CASE WHEN ") + column->column_name + " IS NULL THEN '' "
                                              "WHEN "  + column->column_name + " IS NOT NULL THEN " + column->column_name + " END)";
	break;
      case Field::TYPE_NUMERIC:
        if(column->gtype == G_TYPE_BOOLEAN)
          trans_fields += Glib::ustring("(CASE WHEN ") + column->column_name + " = 0 THEN 0 "
                                              "WHEN "  + column->column_name + " != 0 THEN 1 "
                                              "WHEN "  + column->column_name + " IS NULL THEN 0 END)";
        else if(column->gtype == GDA_TYPE_BLOB || column->gtype == G_TYPE_DATE || column->gtype == GDA_TYPE_TIME)
          trans_fields += "0";
        else
          trans_fields += Glib::ustring("CAST(") + column->column_name + " AS real)";
        break;
      case Field::TYPE_BOOLEAN:
        if(column->gtype == G_TYPE_STRING)
          trans_fields += Glib::ustring("(CASE WHEN ") + column->column_name + " = 'true' THEN 1 "
                                              "WHEN "  + column->column_name + " = 'false' THEN 0 "
                                              "WHEN "  + column->column_name + " IS NULL THEN 0 END)";
        else if(column->gtype == G_TYPE_DOUBLE)
          trans_fields += Glib::ustring("(CASE WHEN ") + column->column_name + " = 0 THEN 0 "
                                              "WHEN "  + column->column_name + " != 0 THEN 1 "
                                              "WHEN "  + column->column_name + " IS NULL THEN 0 END)";
        else if(column->gtype == G_TYPE_BOOLEAN)
          trans_fields += column->column_name;
        else
          trans_fields += Glib::ustring(column->column_name) + " IS NOT NULL";
        break;
      case Field::TYPE_DATE:
        if(column->gtype == G_TYPE_BOOLEAN || column->gtype == GDA_TYPE_BLOB || column->gtype == G_TYPE_DOUBLE)
          trans_fields += "NULL";
        else
          trans_fields += Glib::ustring("date(") + column->column_name + ")";
        break;
      case Field::TYPE_TIME:
        if(column->gtype == G_TYPE_BOOLEAN || column->gtype == GDA_TYPE_BLOB || column->gtype == G_TYPE_DOUBLE)
          trans_fields += "NULL";
        else
          trans_fields += Glib::ustring("time(") + column->column_name + ")";
        break;
      case Field::TYPE_IMAGE:
        if(column->gtype == GDA_TYPE_BLOB)
          trans_fields += column->column_name;
        else
          trans_fields += "NULL";
        break;
      default:
        trans_fields += column->column_name;
        break;
      };

      if(!add_column_to_server_operation(operation, changed_iter->second, i++, error))
        return false;
    }
    else
    {
      trans_fields += column->column_name;
      if(!add_column_to_server_operation(operation, column, i++, error))
        return false;
    }
  }

  for(type_vecConstFields::const_iterator iter = fields_added.begin(); iter != fields_added.end(); ++ iter)
  {
    // Add new fields to the table. Fields that have changed have already
    // been handled above.
    const sharedptr<const Field>& field = *iter;
    type_vecStrings::const_iterator removed_iter = std::find(fields_removed.begin(), fields_removed.end(), field->get_name());
    if(removed_iter == fields_removed.end())
    {
      if(!add_column_to_server_operation(operation, field, i++, error))
        return false;
      if(!trans_fields.empty())
        trans_fields += ",";
      Gnome::Gda::Value default_value = field->get_default_value();
      if(default_value.get_value_type() != G_TYPE_NONE && !default_value.is_null())
        trans_fields += field->sql(default_value, connection);
      else
      {
        switch(field->get_glom_type())
        {
        case Field::TYPE_NUMERIC:
          trans_fields += "0";
          break;
        case Field::TYPE_BOOLEAN:
          trans_fields += "0";
          break;
        case Field::TYPE_TEXT:
          trans_fields += "''";
          break;
        default:
          trans_fields += "NULL";
          break;
        }
      }
    }
  }

  if(!begin_transaction(connection, TRANSACTION_NAME, Gnome::Gda::TRANSACTION_ISOLATION_UNKNOWN, error)) return false;

  if(perform_server_operation(connection->get_provider(), connection, operation, error))
  {
    if(trans_fields.empty() || query_execute(connection, Glib::ustring("INSERT INTO ") + TEMPORARY_TABLE_NAME + " SELECT " + trans_fields + " FROM " + table_name, error))
    {
      if(query_execute(connection, "DROP TABLE " + table_name, error))
      {
        if(query_execute(connection, Glib::ustring("ALTER TABLE ") + TEMPORARY_TABLE_NAME + " RENAME TO " + table_name, error))
        {
          if(commit_transaction(connection, TRANSACTION_NAME, error))
          {
            return true;
          }
        }
      }
    }
  }

  std::auto_ptr<Glib::Error> rollback_error;
  if(!rollback_transaction(connection, TRANSACTION_NAME, rollback_error))
  {
    std::cerr << "Sqlite::recreate_table: Failed to rollback failed transaction";
    if(rollback_error.get())
      std::cerr << ": " << rollback_error->what();
    std::cerr << std::endl;
  }

  return false;
}

bool Sqlite::add_column(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const sharedptr<const Field>& field, std::auto_ptr<Glib::Error>& error)
{
  // Sqlite does not support adding primary key columns. So recreate the table
  // in that case.
  if(!field->get_primary_key())
  {
    return ConnectionPoolBackend::add_column(connection, table_name, field, error);
  }
  else
  {
    return recreate_table(connection, table_name, type_vecStrings(), type_vecConstFields(1, field), type_mapFieldChanges(), error);
  }
}

bool Sqlite::drop_column(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const Glib::ustring& field_name, std::auto_ptr<Glib::Error>& error)
{
  return recreate_table(connection, table_name, type_vecStrings(1, field_name), type_vecConstFields(), type_mapFieldChanges(), error);
}

bool Sqlite::change_columns(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const type_vecConstFields& old_fields, const type_vecConstFields& new_fields, std::auto_ptr<Glib::Error>& error)
{
  type_mapFieldChanges fields_changed;

  for(type_vecConstFields::size_type i = 0; i < old_fields.size(); ++ i)
    fields_changed[old_fields[i]->get_name()] = new_fields[i];

  return recreate_table(connection, table_name, type_vecStrings(), type_vecConstFields(), fields_changed, error);
}

#endif // !GLOM_ENABLE_CLIENT_ONLY

} // namespace ConnectionPoolBackends

} // namespace Glom
