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

#include "box_data_list.h"
#include <sstream> //For stringstream
#include <libintl.h>

Box_Data_List::Box_Data_List()
: m_has_one_or_more_records(false)
{
  m_layout_name = "list";
  
  m_strHint = gettext("When you change the data in a field the database is updated immediately.\n Click [Add] or enter data into the last row to add a new record.\n Leave automatic ID fields empty - they will be filled for you.\nOnly the first 100 records are shown.");

  pack_start(m_AddDel);
  m_AddDel.set_auto_add(false); //We want to add the row ourselves when the user clicks the Add button, because the default behaviour there is not suitable.
  m_AddDel.set_allow_column_chooser();
  m_AddDel.set_show_row_titles(false);

  //Connect signals:
  m_AddDel.signal_user_requested_add().connect(sigc::mem_fun(*this, &Box_Data_List::on_AddDel_user_requested_add)); //Only emitted when m_AddDel.set_auto_add(false) is used.
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Data_List::on_AddDel_user_requested_edit));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_Data_List::on_AddDel_user_requested_delete));
  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_Data_List::on_AddDel_user_added));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_Data_List::on_AddDel_user_changed));
  m_AddDel.signal_user_reordered_columns().connect(sigc::mem_fun(*this, &Box_Data_List::on_AddDel_user_reordered_columns));


  //Groups are not very helpful for a list view:
  m_pDialogLayout->set_show_groups(false);
 
  m_bPrimaryKeyFound = false;
  m_iPrimaryKey = 0;
}

Box_Data_List::~Box_Data_List()
{
}

void Box_Data_List::fill_from_database()
{
  Bakery::BusyCursor(*get_app_window());
    
  Box_DB_Table::fill_from_database();

  m_AddDel.remove_all();
  
  //Field Names:
  fill_column_titles();

  try
  {
    sharedptr<SharedConnection> sharedconnection = connect_to_server();
    if(sharedconnection)
    {
      Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

      Glib::ustring strWhereClausePiece;
      if(m_strWhereClause.size())
        strWhereClausePiece = " WHERE " + m_strWhereClause;

      type_vecFields listFieldsToShow = get_fields_to_show();
      m_Fields = listFieldsToShow; //Store the fields for later:
      Glib::ustring sql_part_fields;
      for(type_vecFields::const_iterator iter =  listFieldsToShow.begin(); iter != listFieldsToShow.end(); ++iter)
      {
        if(iter != listFieldsToShow.begin())
          sql_part_fields += ",";
          
        sql_part_fields += iter->get_name();
      }
       
      //TODO: Remove hardcoded Limit 100.
      std::stringstream query;
      query << "SELECT " << sql_part_fields << " FROM " << m_strTableName << strWhereClausePiece; // << " LIMIT 100";
      Glib::RefPtr<Gnome::Gda::DataModel> result = Query_execute(query.str());
      if(!result)
        handle_error();
      else
      {
        //Field contents:
        m_AddDel.remove_all();

        //each row:
        guint rows_count = result->get_n_rows();
        if(rows_count)
          m_has_one_or_more_records = true; //Save it for later.
          
        if(rows_count > 100)
          rows_count = 100; //Don't get more than 100. TODO: Get other rows dynamically.
        for(guint result_row = 0; result_row < rows_count; result_row++)
        {
          guint uiRow = m_AddDel.add_item("");

          //each field:
          guint cols_count = result->get_n_columns();
          for(guint uiCol = 0; uiCol < cols_count; uiCol++)
          {
            Gnome::Gda::Value value = result->get_value_at(uiCol, result_row);
            const Glib::ustring& strData = m_Fields[uiCol].value_to_string(value); //TODO: Use a strongly-typed AddDel set_value() API.

            m_AddDel.set_value(uiRow, uiCol, strData);
          }
        }
      }
    }
  }
  catch(std::exception& ex)
  {
    handle_error(ex);
  }

  //Select first record:
  m_AddDel.select_item(0);

  fill_end();
}

void Box_Data_List::on_AddDel_user_requested_add()
{
  guint row = m_AddDel.add_item();

  //Start editing in the primary key or the first cell if the primary key is auto-incremented (because there is no point in editing an auto-generated value)..
  guint column = 0;
  guint uiColPrimaryKey = 0;
  bool bPresent = get_field_primary_key(uiColPrimaryKey); //If there is no primary key then the default of 0 is OK.
  if(bPresent)
  {
    Field fieldPrimaryKey = m_Fields[uiColPrimaryKey];
    if(fieldPrimaryKey.get_field_info().get_auto_increment())
    {
      //Start editing in the first cell that is not the primary key:
      if(uiColPrimaryKey == 0)
        column = 1; //TODO: Check that there is > 1 column.
      else
      {
        column = 0;
      }
    }
    else
    {
      column = uiColPrimaryKey; //Start editing in the primary key, because it needs the user to give it a value.
    }
  }

  m_AddDel.select_item(row, column, true /* start_editing */);
}

void Box_Data_List::on_AddDel_user_requested_edit(guint row)
{
  Glib::ustring strPrimaryKeyValue = get_primary_key_value(row);
  signal_user_requested_details().emit(strPrimaryKeyValue);
}

void Box_Data_List::on_AddDel_user_requested_delete(guint rowStart, guint /* rowEnd TODO */)
{
  guint iRow = m_AddDel.get_item_selected();

  Glib::ustring strPrimaryKeyValue = m_AddDel.get_value(rowStart);
  record_delete(strPrimaryKeyValue);

  //Remove the row:
  m_AddDel.remove_item(iRow);
}

void Box_Data_List::on_AddDel_user_added(guint row)
{
  Glib::ustring strPrimaryKeyValue;

  const Glib::ustring& strPrimaryKeyName = get_PrimaryKey_Name();
  Field field;
  bool found = get_field(strPrimaryKeyName, field);
  if(!found)
    g_warning("Box_Data_List::on_AddDel_user_added(): primary key %s not found.", strPrimaryKeyName.c_str());

  guint generated_id = 0;
  if(field.get_field_info().get_auto_increment())
  {
    //Auto-increment is awkward (we can't get the last-generated ID) with postgres, so we auto-generate it ourselves;
    generated_id = generate_next_auto_increment(m_strTableName, strPrimaryKeyName);
    strPrimaryKeyValue = util_string_from_decimal(generated_id);
    //strPrimaryKeyValue = "0"; //Auto-generates
  }
  else
  {
    strPrimaryKeyValue = get_primary_key_value(row);
  }

  sharedptr<SharedConnection> sharedconnection = connect_to_server(); //Keep it alive while we need the data_model.
  if(sharedconnection)
  {      
    Glib::RefPtr<Gnome::Gda::DataModel> data_model = record_new(strPrimaryKeyValue);
    if(data_model)
    {
      Gnome::Gda::FieldAttributes fieldInfo = field.get_field_info();
      //If it's an auto-increment, then get the value and show it:
      if(fieldInfo.get_auto_increment())
      { 
        unsigned long ulAutoID = generated_id; //get_last_auto_increment_value(data_model, fieldinfo.get_name());
        if(ulAutoID)
        {
          m_AddDel.set_value(row, 0, ulAutoID);
        }
        else
          handle_error();
      }

      //Allow derived class to respond to record addition.
      strPrimaryKeyValue = m_AddDel.get_value(row, 0);
      on_record_added(strPrimaryKeyValue);
    }
    else
      handle_error();
  }
  else
  {
    //Add Record failed.
    //Replace with correct values:
    fill_from_database();
  }
}

void Box_Data_List::on_AddDel_user_reordered_columns()
{
  Document_Glom* pDoc = dynamic_cast<Document_Glom*>(get_document());
  if(pDoc)
  {
    AddDel::type_vecStrings vec_field_names = m_AddDel.get_columns_order();

    Document_Glom::type_mapFieldSequence mapFieldSequence;
    guint index = 0;
    for(AddDel::type_vecStrings::iterator iter = vec_field_names.begin(); iter != vec_field_names.end(); ++iter)
    {
      LayoutItem layout_item;
      layout_item.m_field_name = *iter;
      layout_item.m_sequence = index;
      mapFieldSequence[index] = layout_item; 
      ++index;
    }
  
    pDoc->set_data_layout_list(m_strTableName, mapFieldSequence);
    
  }
}

void Box_Data_List::on_AddDel_user_changed(guint row, guint col)
{
  const Glib::ustring& strPrimaryKeyValue = get_primary_key_value(row);
  if(strPrimaryKeyValue.size()) //If the record's primary key is filled in:
  {
    //Just update the record:
    try
    {
      guint primary_key_col = 0;
      bool bPresent = get_field_primary_key(primary_key_col);
      if(bPresent)
      {
        Field field_primary_key = m_Fields[primary_key_col];

        const Glib::ustring& strPrimaryKey_Name = field_primary_key.get_name();
        const Glib::ustring& strFieldName = m_Fields[col].get_name();
        const Glib::ustring& strFieldValue = m_AddDel.get_value(row, col);

        Glib::ustring strQuery = "UPDATE " + m_strTableName;
        strQuery += " SET " + strFieldName + " = " + m_Fields[col].sql(strFieldValue);

        strQuery += " WHERE " + strPrimaryKey_Name + " = " + field_primary_key.sql(strPrimaryKeyValue);
        bool bTest = Query_execute(strQuery);

        if(!bTest)
          fill_from_database(); //Replace with correct values. //TODO: Just replace this one row
      }
    }
    catch(const std::exception& ex)
    {
      handle_error(ex);
    }
  }
  else
  {
    //This record probably doesn't exist yet.
    //Add new record, which will generate the primary key:

    Field field_primary_key;

    bool found = get_field(get_PrimaryKey_Name(), field_primary_key);
    if(found && field_primary_key.get_field_info().get_auto_increment())
    {
      on_AddDel_user_added(row);

       const Glib::ustring& strPrimaryKeyValue = get_primary_key_value(row);
       if(strPrimaryKeyValue.size()) //If the Add succeeeded:
         on_AddDel_user_changed(row, col); //Change this field in the new record.
    }
  }

}

void Box_Data_List::on_details_nav_first()
{
  m_AddDel.select_item(0);

  signal_user_requested_details().emit(m_AddDel.get_value_selected());
}

void Box_Data_List::on_details_nav_previous()
{
  guint rowToEdit = 0;
  guint rowCurrent = m_AddDel.get_item_selected();

  //Don't try to select a negative record number.
  if(rowCurrent > 0)
    rowToEdit = rowCurrent - 1;

  m_AddDel.select_item(rowToEdit);
  signal_user_requested_details().emit(m_AddDel.get_value_selected());
}

void Box_Data_List::on_details_nav_next()
{
  guint rowCurrent = m_AddDel.get_item_selected();

  //Don't go past the last record:
  guint rowToEdit = rowCurrent;
  if( (rowToEdit + 1) < m_AddDel.get_count())
    rowToEdit++;

  m_AddDel.select_item(rowToEdit);

  signal_user_requested_details().emit(m_AddDel.get_value_selected());
}

void Box_Data_List::on_details_nav_last()
{
  guint rowCount = m_AddDel.get_count();

  if(rowCount > 0) //Only select a record if there are any.
  {
    m_AddDel.select_item(rowCount - 1);
    signal_user_requested_details().emit(m_AddDel.get_value_selected());
  }
}

void Box_Data_List::on_Details_record_deleted(Glib::ustring strPrimaryKey)
{
  //Find out which row is affected:
  guint iRowSelected = 0;
  bool bTest = m_AddDel.get_row_number(strPrimaryKey, iRowSelected);
  if(bTest)
  {
    //Remove the row:
    m_AddDel.remove_item(iRowSelected);

    //Show Details for the next one:
    if(iRowSelected < m_AddDel.get_count())
    {
      //Next record moves up one:
      on_AddDel_user_requested_edit(iRowSelected);
    }
    else
    {
      //Just show the last one:
      on_details_nav_last();
    }
  }
  else
  {
    //Just update everything and go the first record.
    //This shouldn't happen.
    fill_from_database();
    on_details_nav_first();
  }
}

Glib::ustring Box_Data_List::get_primary_key_value(guint row)
{
  Glib::ustring strResult;

  guint uiCol = 0;
  bool bPresent = get_field_primary_key(uiCol);
  if(bPresent)
    strResult = m_AddDel.get_value(row, uiCol);

  return strResult;
}

Glib::ustring Box_Data_List::get_primary_key_value_selected()
{
  Glib::ustring strResult;

  guint uiCol = 0;
  bool bPresent = get_field_primary_key(uiCol);
  if(bPresent)
    strResult = m_AddDel.get_value_selected(uiCol);

  return strResult;
}

Field Box_Data_List::get_Entered_Field(guint index)
{
  //Get Gnome::Gda::FieldAttributes from base:
  Field FieldResult = Box_Data::get_Entered_Field(index);

  //Get text from widget:
  if(index < get_Entered_Field_count())
  {
    const Glib::ustring strData = m_AddDel.get_value_selected(index);
  }

  return FieldResult;
}

guint Box_Data_List::get_records_count() const
{
  return m_AddDel.get_count();
}

void Box_Data_List::fill_column_titles()
{
  const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
  if(pDoc)
  {    
    //Field Names:
    m_AddDel.remove_all_columns();
    //m_AddDel.set_columns_count(m_Fields.size());

    type_vecFields listFieldsToShow = get_fields_to_show();

    for(type_vecFields::const_iterator iter =  listFieldsToShow.begin(); iter != listFieldsToShow.end(); ++iter)
    {
      const Glib::ustring strName = iter->get_name();
      const Glib::ustring strFieldTitle = iter->get_title_or_name();

      m_AddDel.add_column(strFieldTitle, strName);
    }
  } //if(pDoc)
}

void Box_Data_List::on_record_added(const Glib::ustring& /* strPrimaryKey */)
{
  //Overridden by Box_Data_List_Related.
  m_AddDel.add_item(); //Add blank row.
}

Box_Data_List::type_signal_user_requested_details Box_Data_List::signal_user_requested_details()
{
  return m_signal_user_requested_details;
}
