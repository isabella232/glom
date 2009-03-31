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

#include <libglom/connectionpool_backends/backend.h>

namespace Glom
{

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

namespace ConnectionPoolBackends
{

Backend::InitErrors Backend::initialize(const SlotProgress& /* slot_progress */, const Glib::ustring& /* initial_username */, const Glib::ustring& /* password */)
{
  return INITERROR_NONE;
}

bool Backend::startup(const SlotProgress& /* slot_progress */)
{
  return true;
}

void Backend::cleanup(const SlotProgress& /* slot_progress */)
{
}

bool Backend::set_server_operation_value(const Glib::RefPtr<Gnome::Gda::ServerOperation>& operation, const Glib::ustring& path, const Glib::ustring& value, std::auto_ptr<Glib::Error>& error)
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    operation->set_value_at(path, value);
    return true;
  }
  catch(const Glib::Error& ex)
  {
    error.reset(new Glib::Error(ex));
    return false;
  }
#else
  operation->set_value_at(path, value, error);
  if(error.get()) return false;
  return true;
#endif
}

Glib::RefPtr<Gnome::Gda::ServerOperation> Backend::create_server_operation(const Glib::RefPtr<Gnome::Gda::ServerProvider>& provider, const Glib::RefPtr<Gnome::Gda::Connection>& connection, Gnome::Gda::ServerOperationType type, std::auto_ptr<Glib::Error>& error)
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    return provider->create_operation(connection, type);
  }
  catch(const Glib::Error& ex)
  {
    error.reset(new Glib::Error(ex));
    return Glib::RefPtr<Gnome::Gda::ServerOperation>();
  }
#else
  return provider->create_operation(connection, type, error);
#endif
}

bool Backend::perform_server_operation(const Glib::RefPtr<Gnome::Gda::ServerProvider>& provider, const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::RefPtr<Gnome::Gda::ServerOperation>& operation, std::auto_ptr<Glib::Error>& error)
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    provider->perform_operation(connection, operation);
    return true;
  }
  catch(const Glib::Error& ex)
  {
    error.reset(new Glib::Error(ex));
    return false;
  }
#else
  provider->perform_operation(connection, operation, error);
  if(error.get()) return false;
  return true;
#endif
}

bool Backend::begin_transaction(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& name, Gnome::Gda::TransactionIsolation level, std::auto_ptr<Glib::Error>& error)
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    return connection->begin_transaction(name, level);
  }
  catch(const Glib::Error& ex)
  {
    error.reset(new Glib::Error(ex));
    return false;
  }
#else
  return connection->begin_transaction(name, level, error);
#endif
}

bool Backend::commit_transaction(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& name, std::auto_ptr<Glib::Error>& error)
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    return connection->commit_transaction(name);
  }
  catch(const Glib::Error& ex)
  {
    error.reset(new Glib::Error(ex));
    return false;
  }
#else
  return connection->commit_transaction(name, error);
#endif
}

bool Backend::rollback_transaction(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& name, std::auto_ptr<Glib::Error>& error)
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    return connection->rollback_transaction(name);
  }
  catch(const Glib::Error& ex)
  {
    error.reset(new Glib::Error(ex));
    return false;
  }
#else
  return connection->rollback_transaction(name, error);
#endif
}

bool Backend::query_execute(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& sql_query, std::auto_ptr<Glib::Error>& error)
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    return connection->statement_execute_non_select(sql_query) != -1;
  }
  catch(const Glib::Error& ex)
  {
    error.reset(new Glib::Error(ex));
    return false;
  }
#else
  return connection->statement_execute_non_select(sql_query, error) != -1;
#endif
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool Backend::add_column(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const sharedptr<const Field>& field, std::auto_ptr<Glib::Error>& error)
{
  Glib::RefPtr<Gnome::Gda::ServerProvider> provider = connection->get_provider();
  Glib::RefPtr<Gnome::Gda::ServerOperation> operation = create_server_operation(provider, connection, Gnome::Gda::SERVER_OPERATION_ADD_COLUMN, error);
  if(!operation) return false;

  if(!set_server_operation_value(operation, "/COLUMN_DEF_P/TABLE_NAME", table_name, error))
    return false;

  if(!set_server_operation_value(operation, "/COLUMN_DEF_P/COLUMN_NAME", field->get_name(), error))
    return false;

  if(!set_server_operation_value(operation, "/COLUMN_DEF_P/COLUMN_TYPE", field->get_sql_type(), error))
    return false;

  if(!set_server_operation_value(operation, "/COLUMN_DEF_P/COLUMN_PKEY", field->get_primary_key() ? "TRUE" : "FALSE", error))
    return false;

  if(!set_server_operation_value(operation, "/COLUMN_DEF_P/COLUMN_UNIQUE", field->get_unique_key() ? "TRUE" : "FALSE", error))
    return false;

  if(!perform_server_operation(provider, connection, operation, error))
    return false;

  return true;
}

bool Backend::drop_column(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const Glib::ustring& field_name, std::auto_ptr<Glib::Error>& error)
{
  Glib::RefPtr<Gnome::Gda::ServerProvider> provider = connection->get_provider();
  Glib::RefPtr<Gnome::Gda::ServerOperation> operation = create_server_operation(provider, connection, Gnome::Gda::SERVER_OPERATION_DROP_COLUMN, error);
  if(!operation)
    return false;

  if(!set_server_operation_value(operation, "/COLUMN_DESC_P/TABLE_NAME", table_name, error))
    return false;

  if(!set_server_operation_value(operation, "/COLUMN_DESC_P/COLUMN_NAME", field_name, error))
    return false;
  
  if(!perform_server_operation(provider, connection, operation, error))
    return false;

  return true;
}

//TODO: Why/When do we need to change multiple columns instead of a single one? murrayc.
//When changing a table's primary key, we unset the primary key for the old
//column and set it for the new column. Using a single call to the
//ConnectionPoolBackend for this, the backend can do all the required
//operations at once, maybe optimizing them. For example, for SQLite we need
//to recreate the whole table when changing columns, so we only need to do
//this once instead of twice when changing the primary key. armin.
bool Backend::change_columns(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const type_vec_const_fields& old_fields, const type_vec_const_fields& new_fields, std::auto_ptr<Glib::Error>& error)
{
  static const char* TRANSACTION_NAME = "glom_change_columns_transaction";
  static const gchar* TEMP_COLUMN_NAME = "glom_temp_column"; // TODO: Find a unique name.

  if(!begin_transaction(connection, TRANSACTION_NAME, Gnome::Gda::TRANSACTION_ISOLATION_UNKNOWN, error)) return false; // TODO: What does the transaction isolation do?

  for(unsigned int i = 0; i < old_fields.size(); ++ i)
  {
    // TODO: Don't create an intermediate column if the name of the column
    // changes anyway.
    sharedptr<Field> temp_field = glom_sharedptr_clone(new_fields[i]);
    temp_field->set_name(TEMP_COLUMN_NAME);
    // The temporary column must not be a primary key, otherwise
    // we might end up with two primary key columns temporarily, which most
    // database systems do not allow.
    temp_field->set_primary_key(false);

    if(!add_column(connection, table_name, temp_field, error))
      break;

    const Glib::ustring temp_move_query = "UPDATE " + table_name + " SET " + TEMP_COLUMN_NAME + " = CAST(" + old_fields[i]->get_name() + " AS " + new_fields[i]->get_sql_type() + ")";
    if(!query_execute(connection, temp_move_query, error))
      break;
    // TODO: If this CAST was not successful, then just go on,
    // dropping the data in the column?

    if(!drop_column(connection, table_name, old_fields[i]->get_name(), error))
      return false;

    if(!add_column(connection, table_name, new_fields[i], error))
      break;

    const Glib::ustring final_move_query = "UPDATE " + table_name + " SET " + new_fields[i]->get_name() + " = " + TEMP_COLUMN_NAME; // TODO: Do we need a cast here, even though the type matches?
    if(!query_execute(connection, final_move_query, error))
      break;

    if(!drop_column(connection, table_name, TEMP_COLUMN_NAME, error))
      break;
  }

  if(error.get() || !commit_transaction(connection, TRANSACTION_NAME, error))
  {
    std::auto_ptr<Glib::Error> rollback_error;
    rollback_transaction(connection, TRANSACTION_NAME, rollback_error);
    return false;
  }

  return true;
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

} // namespace ConnectionPoolBackends

} // namespace Glom
