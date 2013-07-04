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

Backend::InitErrors Backend::initialize(const SlotProgress& /* slot_progress */, const Glib::ustring& /* initial_username */, const Glib::ustring& /* password */, bool /* network_shared */)
{
  return INITERROR_NONE;
}

Backend::StartupErrors Backend::startup(const SlotProgress& /* slot_progress */, bool /* network_shared */)
{
  return STARTUPERROR_NONE;
}

bool Backend::cleanup(const SlotProgress& /* slot_progress */)
{
  return true;
}

bool Backend::set_network_shared(const SlotProgress& /* slot_progress */, bool /* network_shared */)
{
  return true; //Success at doing nothing.
}

bool Backend::add_column(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const std::shared_ptr<const Field>& field)
{
  Glib::RefPtr<Gnome::Gda::ServerProvider> provider = connection->get_provider();
  Glib::RefPtr<Gnome::Gda::ServerOperation> operation = provider->create_operation(connection, Gnome::Gda::SERVER_OPERATION_ADD_COLUMN);

  //TODO: Quote table_name and field_name?
  operation->set_value_at("/COLUMN_DEF_P/TABLE_NAME", table_name);
  operation->set_value_at("/COLUMN_DEF_P/COLUMN_NAME", field->get_name());
  operation->set_value_at("/COLUMN_DEF_P/COLUMN_TYPE", field->get_sql_type());
  operation->set_value_at("/COLUMN_DEF_P/COLUMN_PKEY", field->get_primary_key());
  operation->set_value_at("/COLUMN_DEF_P/COLUMN_UNIQUE", field->get_unique_key());

  return provider->perform_operation(connection, operation);
}

bool Backend::drop_column(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  Glib::RefPtr<Gnome::Gda::ServerProvider> provider = connection->get_provider();
  Glib::RefPtr<Gnome::Gda::ServerOperation> operation = provider->create_operation(connection, Gnome::Gda::SERVER_OPERATION_DROP_COLUMN);

  //TODO: Quote table name and column name?
  operation->set_value_at("/COLUMN_DESC_P/TABLE_NAME", table_name);
  operation->set_value_at("/COLUMN_DESC_P/COLUMN_NAME", field_name);
  return provider->perform_operation(connection, operation);
}

void Backend::set_database_directory_uri(const std::string& directory_uri)
{
  m_database_directory_uri = directory_uri;
}

std::string Backend::get_database_directory_uri() const
{
  return m_database_directory_uri;
}


} // namespace ConnectionPoolBackends

} // namespace Glom
