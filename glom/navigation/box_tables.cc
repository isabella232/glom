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
#include <libintl.h>

Box_Tables::Box_Tables(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Box_DB(cobject),
  m_pLabelFrameTitle(0),
  m_pCheckButtonShowHidden(0),
  m_colTableName(0),
  m_colHidden(0),
  m_colTitle(0),
  m_colDefault(0),
  m_modified(false)
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
  
  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_Tables::on_AddDel_Add));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_Tables::on_AddDel_Delete));
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Tables::on_AddDel_Edit));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_Tables::on_AddDel_changed));

  show_all_children();
}

Box_Tables::~Box_Tables()
{
}

void Box_Tables::fill_table_row(const Gtk::TreeModel::iterator& iter, const TableInfo& table_info)
{
  if(iter)
  {
    m_AddDel.set_value_key(iter, table_info.m_name);
    m_AddDel.set_value(iter, m_colTableName, table_info.m_name);
    m_AddDel.set_value(iter, m_colHidden, table_info.m_name);
    m_AddDel.set_value(iter, m_colTitle, table_info.m_title);
    m_AddDel.set_value(iter, m_colDefault, table_info.m_default);
  }
}
  
void Box_Tables::fill_from_database()
{
  Bakery::BusyCursor(*get_app_window());

  Box_DB::fill_from_database();
    
  //Enable/Disable extra widgets:
  bool developer_mode = (get_userlevel() == AppState::USERLEVEL_DEVELOPER);
  m_pCheckButtonShowHidden->set_sensitive(developer_mode); //Operators have no choice - they can't see hidden tables ever.
  if( get_userlevel() != AppState::USERLEVEL_DEVELOPER)
    m_pCheckButtonShowHidden->set_active(false); //Operators have no choice - they can't see hidden tables ever.

  m_AddDel.remove_all();

  //Add the columns:
  m_AddDel.remove_all_columns();

  bool editable = developer_mode;
  m_colTableName = m_AddDel.add_column(gettext("Tables"), AddDelColumnInfo::STYLE_Text, editable);

  bool visible_extras = developer_mode;
  m_colHidden = m_AddDel.add_column(gettext("Hidden"), AddDelColumnInfo::STYLE_Boolean, true, visible_extras);
  m_colTitle =  m_AddDel.add_column(gettext("Title"), AddDelColumnInfo::STYLE_Text, true, visible_extras);
  m_colDefault =  m_AddDel.add_column(gettext("Default"), AddDelColumnInfo::STYLE_Boolean,  true, visible_extras);
  

  //gettext("Server: ") +  m_strServerName + ", " + 
  Glib::ustring strTitle = Glib::ustring("<b>") + gettext("Tables from Database: ") + get_database_name() + "</b>";
  m_pLabelFrameTitle->set_markup(strTitle);

  //Get the list of hidden tables:

  Document_Glom::type_listTableInfo listTablesDocument;
  if(m_pDocument)
  {
    listTablesDocument = m_pDocument->get_tables();
  }
  else
    g_warning("debug: m_pDocument is null");
  
  //Get the list of tables in the database, from the server:
  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    m_AddDel.remove_all();
    Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

    type_vecStrings vecTables = get_table_names();

    for(type_vecStrings::iterator iter = vecTables.begin(); iter != vecTables.end(); iter++)
    {
      const Glib::ustring strName = *iter;

      //Check whether it should be hidden:
      Document_Glom::type_listTableInfo::iterator iterFind = std::find_if(listTablesDocument.begin(), listTablesDocument.end(), predicate_FieldHasName<TableInfo>(strName));
      if(iterFind != listTablesDocument.end())
      {
        bool hidden = iterFind->m_hidden;

        bool bAddIt = true;
        if(hidden && !developer_mode)  //Don't add hidden tables unless we are in developer mode:
          bAddIt = false;

        if(hidden && !m_pCheckButtonShowHidden->get_active()) //Don't add hidden tables if that checkbox is unset.
          bAddIt = false;

        if(bAddIt)
        {
          Gtk::TreeModel::iterator iter = m_AddDel.add_item(strName);
          fill_table_row(iter, *iterFind);
        }
      }
      else
      {
        //This table is in the database, but not in the document.
        //Show it as hidden:
        if(developer_mode)
        {
          TableInfo table_info;
          table_info.m_name = strName;
          table_info.m_hidden = true;
          
          Gtk::TreeModel::iterator iter = m_AddDel.add_item(strName);
          fill_table_row(iter, table_info);
        }
      }
    }
  }

  fill_end();

  m_AddDel.set_allow_add(developer_mode);
  m_AddDel.set_allow_delete(developer_mode);
      
  m_modified = false;
}

void Box_Tables::on_AddDel_Add(const Gtk::TreeModel::iterator& row)
{
  Glib::ustring table_name = m_AddDel.get_value(row, m_colTableName);
  if(!table_name.empty())
  {
    Glib::ustring primary_key_name =   table_name + "_id";

    //Create a table with 1 "ID" field:
   //MSYQL:
    //Query_execute( "CREATE TABLE " + table_name + " (" + primary_key_name + " INT NOT NULL AUTO_INCREMENT PRIMARY KEY)" );
    //Query_execute( "INSERT INTO " + table_name + " VALUES (0)" );

    //PostgresSQL:
    //Query_execute( "CREATE TABLE " + table_name + " (" + primary_key_name + " serial NOT NULL  PRIMARY KEY)" );
    Query_execute( "CREATE TABLE " + table_name + " (" + primary_key_name + " numeric NOT NULL  PRIMARY KEY)" );

    //Show the new information for this whole row:
    TableInfo table_info;
    table_info.m_name = table_name;
    fill_table_row(row, table_info);
    
    //Save the field information directly into the database, because we can not get all the correct information from the database.
    //Otherwise some information would be forgotten:
    Field field_primary_key;
    Gnome::Gda::FieldAttributes field_info = field_primary_key.get_field_info();
    field_info.set_name(primary_key_name);
    field_info.set_primary_key();
    field_info.set_auto_increment();
    field_info.set_allow_null(false);
    field_primary_key.set_field_info(field_info);
    field_primary_key.set_glom_type(Field::TYPE_NUMERIC);

    type_vecFields fields;
    fields.push_back(field_primary_key);

     if(m_pDocument)
        m_pDocument->set_table_fields(table_name, fields);

    save_to_document();
    //fill_from_database(); //We should not modify the model structure in a cellrenderer signal handler.

    m_modified = true;
  }
}

void Box_Tables::on_AddDel_Delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd)
{
  Gtk::TreeModel::iterator iterAfter = rowEnd;
  ++iterAfter;

  bool something_changed = false;
  for(Gtk::TreeModel::iterator iter = rowStart; iter != iterAfter; ++iter)
  {
    Glib::ustring strName = m_AddDel.get_value_key(iter);

    if(!strName.empty())
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
        something_changed = true;
      }
    }
  }

  if(something_changed)
  {
    save_to_document();

    fill_from_database();

    m_modified = true;
  }
}

void Box_Tables::on_AddDel_Edit(const Gtk::TreeModel::iterator& row)
{
  Glib::ustring table_name = m_AddDel.get_value_key(row);
  
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
       
       save_to_document();

       //Emit the signal:
       signal_selected.emit(table_name);
    }
  }
}

void Box_Tables::save_to_document()
{
  if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  {
    if(true); //m_modified
    {
      //Save the hidden tables. TODO_usermode: Only if we are in developer mode.
      Document_Glom::type_listTableInfo listTables;

      for(Gtk::TreeModel::iterator iter = m_AddDel.get_model()->children().begin(); iter != m_AddDel.get_model()->children().end(); ++iter)
      {
        TableInfo table_info;
        table_info.m_name = m_AddDel.get_value(iter, m_colTableName);
        table_info.m_hidden  = m_AddDel.get_value_as_bool(iter, m_colHidden);
        table_info.m_title  = m_AddDel.get_value(iter, m_colTitle);
        table_info.m_default  = m_AddDel.get_value_as_bool(iter, m_colDefault);

        listTables.push_back(table_info);
      }

      if(m_pDocument)
        m_pDocument->set_tables( listTables); //TODO: Don't save all new tables - just the ones already in the document.
      }
   }
}

void Box_Tables::on_show_hidden_toggled()
{
  fill_from_database();
}

void Box_Tables::on_AddDel_changed(const Gtk::TreeModel::iterator& row, guint column)
{
  if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  {
    if( (column == m_colHidden) )
    {
      save_to_document();
      //TODO: This causes a crash. fill_from_database(); //Hide/show the table.
      m_modified = true;
    }
    else if( (column == m_colTitle) || (column == m_colDefault) )
    {
      save_to_document();
      m_modified = true;
    } 
    else if(column == m_colTableName)
    {
      Glib::ustring table_name = m_AddDel.get_value_key(row);
      Glib::ustring table_name_new = m_AddDel.get_value(row, m_colTableName);
      if(!table_name.empty() && !table_name_new.empty())
      {
        Glib::ustring strMsg = gettext("Are you sure that you want to rename this table?");  //TODO: Show old and new names?
        Gtk::MessageDialog dialog(strMsg);
        int iButtonClicked = dialog.run();

        //Rename the table:
        if(iButtonClicked == Gtk::RESPONSE_OK)
        {
          Query_execute( "ALTER TABLE " + table_name + " RENAME TO " + table_name_new);
          set_modified();
          //fill_from_database(); //We should not modify the model structure in a cellrenderer signal handler.
        }
      }
    }
  }
}

void  Box_Tables::on_userlevel_changed(AppState::userlevels /* userlevel */)
{
  fill_from_database();
}

