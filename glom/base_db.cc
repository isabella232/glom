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

#include "base_db.h"
#include "application.h" //App_Glom.
#include "appstate.h"
//#include <libgnomeui/gnome-app-helper.h>

#include <sstream> //For stringstream

Base_DB::Base_DB()
{
  m_pDocument = 0;
}

Base_DB::~Base_DB()
{
}

void Base_DB::init_db_details(const Glib::ustring& strDatabaseName)
{
  m_strDatabaseName = strDatabaseName;

  fill_from_database();
}

void Base_DB::fill_from_database()
{
  //m_AddDel.remove_all();
}

void Base_DB::fill_end()
{
  //Call this from the end of fill_from_database() overrides.
}

//static:
sharedptr<SharedConnection> Base_DB::connect_to_server()
{
  Bakery::BusyCursor(*get_app_window());

  sharedptr<SharedConnection> result(0);

  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
  {
    result = connection_pool->connect();
  }

  return result;
}


Glib::ustring Base_DB::get_database_name()
{
  return m_strDatabaseName;
}

void Base_DB::handle_error(const std::exception& ex)
{
  Gtk::MessageDialog dialog(Glib::ustring("Internal error:\n") + ex.what(), Gtk::MESSAGE_WARNING );
  dialog.run();
}

void Base_DB::handle_error()
{
  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

    typedef std::list< Glib::RefPtr<Gnome::Gda::Error> > type_list_errors;
    type_list_errors list_errors = gda_connection->get_errors();

    Glib::ustring error_details;
    for(type_list_errors::iterator iter = list_errors.begin(); iter != list_errors.end(); ++iter)
    {
      if(iter != list_errors.begin())
        error_details += "\n"; //Add newline after each error.

      error_details += (*iter)->get_description();
      std::cerr << "Internal error: " << error_details << std::endl;
    }

    Gtk::MessageDialog dialog(Glib::ustring("Internal error:\n") + error_details, Gtk::MESSAGE_WARNING );
    dialog.run();
  }
}


Glib::RefPtr<Gnome::Gda::DataModel> Base_DB::Query_execute(const Glib::ustring& strQuery)
{
  Glib::RefPtr<Gnome::Gda::DataModel> result;

  Bakery::BusyCursor(*get_app_window());

  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

    std::cout << "Query_execute() before: " << strQuery << std::endl;
    result = gda_connection->execute_single_command(strQuery);
    std::cout << "Query_execute() after: " << strQuery << std::endl;
    if(!result)
    {
      handle_error();
    }
  }

  return result;
}

void Base_DB::load_from_document()
{
  if(m_pDocument)
  {
    fill_from_database(); //virtual.

    //Call base class:
    View_Composite_Glom::load_from_document();
  }
}


Base_DB::type_vecStrings Base_DB::get_table_names()
{
  type_vecStrings result;

  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();
    Glib::RefPtr<Gnome::Gda::DataModel> data_model_tables = gda_connection->get_schema(Gnome::Gda::CONNECTION_SCHEMA_TABLES);
    if(data_model_tables && (data_model_tables->get_n_columns() == 0))
    {
      std::cerr << "Base_DB_Table::get_table_names(): libgda reported 0 tables for the database." << std::endl;
    }
    else if(data_model_tables)
    {
      int rows = data_model_tables->get_n_rows();
      for(int i = 0; i < rows; ++i)
      {
        Gnome::Gda::Value value = data_model_tables->get_value_at(0, i);

        //Get the table name:
        Glib::ustring table_name;
        if(value.get_value_type() ==  Gnome::Gda::VALUE_TYPE_STRING)
          result.push_back( value.get_string() );
      }
    }
  }

  return result;
}


AppState::userlevels Base_DB::get_userlevel() const
{
  const Document_Glom* document = dynamic_cast<const Document_Glom*>(get_document());
  if(document)
  {
    return document->get_userlevel();
  }
  else
  {
    g_warning("Base_DB::get_userlevel(): document not found.");
    return AppState::USERLEVEL_OPERATOR;
  }
}

void Base_DB::set_userlevel(AppState::userlevels value)
{
  Document_Glom* document = get_document();
  if(document)
  {
    document->set_userlevel(value);
  }
}

void Base_DB::on_userlevel_changed(AppState::userlevels /* userlevel */)
{
  //Override this in derived classes.
}

Glib::ustring Base_DB::util_string_from_decimal(guint decimal)
{
    std::stringstream stream;
    stream << decimal;

    Glib::ustring result;
    stream >> result;

    return result;
}

guint Base_DB::util_decimal_from_string(const Glib::ustring& str)
{
    //Convert it to a numeric type:
    std::stringstream stream;
    stream << str;
    guint id_numeric = 0;
    stream >> id_numeric;

    return id_numeric;
}

void Base_DB::set_document(Document_Glom* pDocument)
{
  View_Composite_Glom::set_document(pDocument);

  //Connect to a signal that is only on the derived document class:
  if(m_pDocument)
  {
    m_pDocument->signal_userlevel_changed().connect( sigc::mem_fun(*this, &Base_DB::on_userlevel_changed) );

    //Show the appropriate UI for the user level that is specified by this new document:
    on_userlevel_changed(m_pDocument->get_userlevel());
  }
}
