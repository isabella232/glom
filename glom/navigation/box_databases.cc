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
 
#include "box_databases.h"
#include "config.h" //To get MYSQL_INSTALL_PREFIX
#include <libintl.h>

Box_DataBases::Box_DataBases(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Box_DB(cobject),
  m_button_connect(0),
  m_col_name(0)
{
  m_AddDel.set_show_column_titles(false);
  
  m_strHint = gettext("Select a database and click [Edit]. You may create a new database by entering the name in the last row.");

  //Get the Glade-instantiated widgets, and connect signal handlers:
  refGlade->get_widget("entry_connection_host", m_Entry_Host);
  refGlade->get_widget("entry_connection_user", m_Entry_User);
  refGlade->get_widget("entry_connection_password", m_Entry_Password);

  refGlade->get_widget("button_connect", m_button_connect);
  refGlade->get_widget("expander_tables", m_Expander_Tables);

  Gtk::Button* pButtonCancel = 0;
  refGlade->get_widget("button_cancel", pButtonCancel);
  set_button_cancel(*pButtonCancel);
     
  Gtk::Alignment* pAligmentPlaceholder = 0;
  refGlade->get_widget("alignment_placeholder_adddel", pAligmentPlaceholder);
      
  m_bUseList = false; //set to true later by call to set_use_list().

  m_button_connect->signal_clicked().connect(sigc::mem_fun(*this, &Box_DataBases::on_button_connect));

  pAligmentPlaceholder->add(m_AddDel);

  m_col_name = m_AddDel.add_column(gettext("Databases"));

  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_DataBases::on_adddel_Add));
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_DataBases::on_adddel_Edit));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_DataBases::on_adddel_Delete));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_DataBases::on_adddel_Changed));
  
  set_use_list(); 

  show_all_children();
}

Box_DataBases::~Box_DataBases()
{
}


void Box_DataBases::on_button_connect()
{
  sharedptr<SharedConnection> sharedconnection = connect_to_server_with_connection_settings();
  if(sharedconnection)
  {
    Bakery::BusyCursor(*get_app_window());

    Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

    if(!m_bUseList)
    {
      //Just use the current database:
      //e.g. when reloading a saved document.
      //TODO: Handle error if database is not present.
      m_strDatabaseName = m_pDocument->get_connection_database();

      signal_selected.emit(m_strDatabaseName);
    }
    else
    {
      //Offer the user a list of databases at the connected server:

      m_AddDel.remove_all();

      typedef std::vector<Glib::ustring> type_vecStrings;
      type_vecStrings vecDatabases = get_database_names();

      for(type_vecStrings::iterator iter = vecDatabases.begin(); iter != vecDatabases.end(); iter++)
      {
        const Glib::ustring& strName = *iter;
        Gtk::TreeModel::iterator iter = m_AddDel.add_item(strName);
        m_AddDel.set_value(iter, m_col_name, strName);
      }

      //Select the current database:
      m_AddDel.select_item(m_pDocument->get_connection_database(), m_col_name);
    }
  }
  else
  {
    Glib::ustring strMsg = ("The connection failed.");
    strMsg += gettext("\nThe user or password might be wrong,");
    strMsg += gettext("\nor you might need to start your database backend.");

    //strMsg += gettext("\ne.g. ");
    //strMsg += "<MYSQL_INSTALL_PREFIX>"; //TODO
    //strMsg += "/mysql/bin/safe_mysqld&";

    Gtk::MessageDialog dialog(strMsg, false, Gtk::MESSAGE_WARNING);
    dialog.run();
  }
}

void Box_DataBases::on_adddel_Add(const Gtk::TreeModel::iterator& row)
{
  Glib::ustring strName = m_AddDel.get_value(row, m_col_name);
  if(!strName.empty())
  {
    sharedptr<SharedConnection> sharedconnection = connect_to_server_with_connection_settings();
    if(sharedconnection)
    {
      Bakery::BusyCursor(*get_app_window());
          
      Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();
      bool result = connection->create_database(strName);
      if(!result)
      {
        g_warning("Box_DataBases::on_adddel_Add(): create_database() failed: database_name=%s", strName.c_str());
        handle_error();
      }
    }

    on_button_connect(); //Refresh the list of databases.
  }
}

sharedptr<SharedConnection> Box_DataBases::connect_to_server_with_connection_settings() const
{
  Bakery::BusyCursor(*get_app_window());

  sharedptr<SharedConnection> result(0);

  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
  {
    //Set the connection details in the ConnectionPool singleton.
    //The ConnectionPool will now use these every time it tries to connect.
    connection_pool->set_host(m_Entry_Host->get_text());
    connection_pool->set_user(m_Entry_User->get_text());
    connection_pool->set_password(m_Entry_Password->get_text());
    connection_pool->set_ready_to_connect(); //Box_DB::connect_to_server() will now attempt the connection-> Shared instances of m_Connection will also be usable.

    result = Box_DB::connect_to_server();

    if(m_pDocument)
    {
      m_pDocument->set_connection_server(m_Entry_Host->get_text());
      m_pDocument->set_connection_user(m_Entry_User->get_text());
    }
  }
  
  return result;
}

void Box_DataBases::on_adddel_Edit(const Gtk::TreeModel::iterator& row)
{
  m_strDatabaseName = m_AddDel.get_value_key(row);
 
  signal_selected.emit(m_strDatabaseName);
}

void Box_DataBases::on_adddel_Delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& /* rowEnd TODO */)
{
  Glib::ustring strValue = m_AddDel.get_value_key(rowStart);
  if(!strValue.empty())
  {
    //Ask the user to confirm:
    Gtk::MessageDialog dialog(gettext("Are you sure that you want to delete this database?"), Gtk::MESSAGE_QUESTION);
    int iButtonClicked = dialog.run();

    //Delete the databsase:
    if(iButtonClicked == Gtk::RESPONSE_OK)
    {
      sharedptr<SharedConnection> sharedconnection = connect_to_server_with_connection_settings();
      if(sharedconnection)
      {
        Bakery::BusyCursor(*get_app_window());
            
        Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();
        connection->drop_database(strValue);
      }

      on_button_connect(); //Refresh the list of databases.
    }
  }
}

void Box_DataBases::load_from_document()
{
  if(m_pDocument)
  {
    //Load server and user:
    m_Entry_Host->set_text(m_pDocument->get_connection_server());
    m_Entry_User->set_text(m_pDocument->get_connection_user());
  }

}

void Box_DataBases::set_use_list(bool bVal /* = true */)
{
  //Hide AddDel if necessary:
  
  m_Expander_Tables->set_expanded(bVal);

  if(!bVal)
  {
    //The host and user name should already have been filled in from the document:
    m_Entry_Password->grab_focus();
  }
  else
    m_Entry_User->grab_focus();
    
  m_bUseList = bVal;  
}

Box_DataBases::type_vecStrings Box_DataBases::get_database_names()
{
  type_vecStrings result;

  sharedptr<SharedConnection> sharedconnection = connect_to_server_with_connection_settings();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

    Glib::RefPtr<Gnome::Gda::DataModel> data_model_databases = gda_connection->get_schema(Gnome::Gda::CONNECTION_SCHEMA_DATABASES);
    if(data_model_databases && (data_model_databases->get_n_columns() == 0))
    {
      std::cerr << "Box_DB_Table::get_table_names(): libgda reported 0 tables for the database." << std::endl;
    }
    else if(data_model_databases)
    {
      int rows = data_model_databases->get_n_rows();
      for(int i = 0; i < rows; ++i)
      {
        Gnome::Gda::Value value = data_model_databases->get_value_at(0, i);

        //Get the table name:
        Glib::ustring table_name;
        if(value.get_value_type() ==  Gnome::Gda::VALUE_TYPE_STRING)
          result.push_back( value.get_string() );
      }
    }
  }

  return result;
}

void Box_DataBases::on_adddel_Changed(const Gtk::TreeModel::iterator& /* row */, guint /* number */)
{
  //TODO: Get the old value and change the database name to the new value.
}

Gtk::Widget* Box_DataBases::get_default_button()
{
  return m_button_connect;
}




