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

#include "box_tables.h"

Box_Tables::Box_Tables(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Box_DB(cobject),
  m_pLabelFrameTitle(0),
  m_pCheckButtonShowHidden(0),
  m_colHidden(0) ,
  m_colTitle(0)
{
  m_strHint = gettext("Select a table and click [Edit]. You may create a new table by entering the name in the last row.");

  //Get the Glade-instantiated widgets, and connect signal handlers:
  Gtk::Button* pButtonCancel = 0;
  refGlade->get_widget("button_cancel", pButtonCancel);
  set_button_cancel(*pButtonCancel);

  Gtk::Alignment* pAligmentPlaceholder = 0;
  refGlade->get_widget("alignment_placeholder_adddel", pAligmentPlaceholder);
  pAligmentPlaceholder->add(m_AddDel);

  refGlade->get_widget("label_frame_title", m_pLabelFrameTitle);
  
  refGlade->get_widget("checkbutton_show_hidden", m_pCheckButtonShowHidden);
  m_pCheckButtonShowHidden->signal_toggled().connect(sigc::mem_fun(*this, &Box_Tables::on_show_hidden_toggled));

  if( get_userlevel() == AppState::USERLEVEL_DEVELOPER)
    m_pCheckButtonShowHidden->set_active(true); //Set a suitable default;
  else
    m_pCheckButtonShowHidden->set_sensitive(false); //Operators have no choice - they can't see hidden tables ever.
   
  m_AddDel.set_show_row_titles(false);
  m_AddDel.add_column(gettext("Tables"));

  if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  {
    //Be carefule not to use these columns unless we are in developer mode.
    m_colHidden = m_AddDel.add_column(gettext("Hidden"), AddDelColumnInfo::STYLE_Boolean);
    m_colTitle =  m_AddDel.add_column(gettext("Title"));
  }
  
  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_Tables::on_AddDel_Add));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_Tables::on_AddDel_Delete));
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Tables::on_AddDel_Edit));

  show_all_children();
}

Box_Tables::~Box_Tables()
{
}

void Box_Tables::fill_from_database()
{
  Bakery::BusyCursor(*get_app_window());
    
  Box_DB::fill_from_database();

  //gettext("Server: ") +  m_strServerName + ", " + 
  Glib::ustring strTitle = Glib::ustring("<b>") + gettext("Tables from Database: ") + get_databaseName() + "</b>";
  m_pLabelFrameTitle->set_markup(strTitle);

  //Get the list of hidden tables:

  Document_Glom::type_listTableInfo listTables;
  if(m_pDocument)
  {
    listTables = m_pDocument->get_tables();
  }
  else
    g_warning("debug: m_pDocument is null");
    
  const bool developer_mode = ( get_userlevel() == AppState::USERLEVEL_DEVELOPER);
  
  
  //Get the list of tables in the database, from the server:
  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    m_AddDel.remove_all();
    Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

    type_vecStrings vecTables = get_table_names();

    for(type_vecStrings::iterator iter = vecTables.begin(); iter != vecTables.end(); iter++)
    {
      const Glib::ustring& strName = *iter;

      //Check whether it should be hidden:
      Document_Glom::type_listTableInfo::iterator iterFind = std::find_if(listTables.begin(), listTables.end(), predicate_FieldHasName<TableInfo>(strName));
      bool hidden = false;
      Glib::ustring title;
      if(iterFind != listTables.end())
      {
        hidden = iterFind->m_hidden;
        title = iterFind->m_title;
      }

      bool bAddIt = true;
      if(hidden && !developer_mode)  //Don't add hidden tables unless we are in developer mode:
        bAddIt = false;

      if(hidden && !m_pCheckButtonShowHidden->get_active()) //Don't add hidden tables if that checkbox is unset.
        bAddIt = false;

      if(bAddIt)
      {
        if(developer_mode)
        {
          guint uiRow = m_AddDel.add_item(strName);

          if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
          {
            m_AddDel.set_value(uiRow, m_colHidden, hidden);
            m_AddDel.set_value(uiRow, m_colTitle, title);
          }
        }
      }
    }
  }

  fill_end();
}

void Box_Tables::on_AddDel_Add(guint row)
{
  Glib::ustring table_name = m_AddDel.get_value(row);

  Glib::ustring primary_key_name =   table_name + "_id";
  
  //Create a table with 1 "ID" field:
  #if 0 //MSYQL:
  Query_execute( "CREATE TABLE " + table_name + " (" + primary_key_name + " INT NOT NULL AUTO_INCREMENT PRIMARY KEY)" );
  Query_execute( "INSERT INTO " + table_name + " VALUES (0)" );
  #else
  //PostgresSQL:
  //Query_execute( "CREATE TABLE " + table_name + " (" + primary_key_name + " serial NOT NULL  PRIMARY KEY)" );
  Query_execute( "CREATE TABLE " + table_name + " (" + primary_key_name + " numeric NOT NULL  PRIMARY KEY)" );

  //Save the field information directly into the database, because we can not get all the correct information from the database.
  //Otherwise some information would be forgotten:
  Field field_primary_key;
  Gnome::Gda::FieldAttributes field_info = field_primary_key.get_field_info();
  field_info.set_name(primary_key_name);
  field_info.set_primary_key();
  field_info.set_auto_increment();
  field_info.set_allow_null(false);
  field_primary_key.set_field_info(field_info);
  field_primary_key.set_field_type(FieldType(FieldType::TYPE_NUMERIC));
  
  type_vecFields fields;
  fields.push_back(field_primary_key);

   if(m_pDocument)
      m_pDocument->set_table_fields(table_name, fields);
  
  //Query_execute( "INSERT INTO " + strName + " VALUES (0)" );
  #endif

  fill_from_database();
}

void Box_Tables::on_AddDel_Delete(guint rowStart, guint rowEnd)
{
  for(guint row = rowStart; row <= rowEnd; row++)
  {
    Glib::ustring strName = m_AddDel.get_value(row);
    if(strName.size())
    {
      //Ask the user to confirm:
      Glib::ustring strMsg = gettext("Are you sure that you want to delete this table?\nTable name: ")
                          + strName;
      Gtk::MessageDialog dialog(strMsg);
      int iButtonClicked = dialog.run();

      //Delete the table:
      if(iButtonClicked == Gtk::RESPONSE_OK)
      {
        Query_execute( "DROP TABLE " + strName);
      }
    }
  }

  fill_from_database();
}

void Box_Tables::on_AddDel_Edit(guint row)
{
  Glib::ustring table_name = m_AddDel.get_value(row);
  
  if(m_pDocument)
  {
    //Don't open a table that the document does not know about, because we need information from the document:
    if(!m_pDocument->get_table_is_known(table_name))
    {
       //TODO: Do not show tables that are not in the document.
       Gtk::MessageDialog dialog(gettext("You can not open this table, because there is no information about this table in the document."));
       dialog.set_transient_for(*get_app_window());
       dialog.run();
    }
    else
    {
       //Go ahead:
       
       if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
       {
         //Save the hidden tables. TODO_usermode: Only if we are in developer mode.
         Document_Glom::type_listTableInfo listTables;

         guint rows_count = m_AddDel.get_count();
         for(guint row_index = 0; row_index < rows_count; ++row_index)
         {
           TableInfo table_info;
           table_info.m_name = m_AddDel.get_value(row_index, 0);
           table_info.m_hidden  = m_AddDel.get_value_as_bool(row_index, m_colHidden);
           table_info.m_title  = m_AddDel.get_value(row_index, m_colTitle);
           
           listTables.push_back(table_info);
         }

         if(m_pDocument)
           m_pDocument->set_tables( listTables); //TODO: Don't save all new tables - just the ones already in the document.
       }


       //Emit the signal:
       signal_selected.emit(table_name);
    }
  }
}

void Box_Tables::on_show_hidden_toggled()
{
  fill_from_database();
}
