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
: m_has_one_or_more_records(false),
  m_first_col(0)
{
  m_layout_name = "list";
  
  m_strHint = gettext("When you change the data in a field the database is updated immediately.\n Click [Add] or enter data into the last row to add a new record.\n Leave automatic ID fields empty - they will be filled for you.\nOnly the first 100 records are shown.");

  pack_start(m_AddDel);
  m_AddDel.set_auto_add(false); //We want to add the row ourselves when the user clicks the Add button, because the default behaviour there is not suitable.
  m_AddDel.set_allow_column_chooser();
  m_AddDel.set_rules_hint(); //Use alternating row colors when the theme does that.
  
  //Connect signals:
  m_AddDel.signal_user_requested_add().connect(sigc::mem_fun(*this, &Box_Data_List::on_AddDel_user_requested_add)); //Only emitted when m_AddDel.set_auto_add(false) is used.
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Data_List::on_AddDel_user_requested_edit));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_Data_List::on_AddDel_user_requested_delete));
  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_Data_List::on_AddDel_user_added));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_Data_List::on_AddDel_user_changed));
  m_AddDel.signal_user_reordered_columns().connect(sigc::mem_fun(*this, &Box_Data_List::on_AddDel_user_reordered_columns));


  //Groups are not very helpful for a list view:
  m_pDialogLayout->set_show_groups(false);
 
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

        guint primary_key_field_index = 0;
        bool primary_key_found = get_field_primary_key(primary_key_field_index);
        if(primary_key_found)
        {
          for(guint result_row = 0; result_row < rows_count; result_row++)
          {
            Gnome::Gda::Value value = result->get_value_at(primary_key_field_index, result_row);
            Glib::ustring key = value.to_string(); //It is actually an integer, but that should not matter.
            if(key.empty())
              g_warning("Box_Data_List::fill_from_database(): primary key value is empty");
            else
            {
              Gtk::TreeModel::iterator tree_iter = m_AddDel.add_item(key);

              //each field:
              guint cols_count = result->get_n_columns();
              for(guint uiCol = 0; uiCol < cols_count; uiCol++)
              {
                Gnome::Gda::Value value = result->get_value_at(uiCol, result_row);
                m_AddDel.set_value(tree_iter, m_first_col + uiCol, value);
              }
            }
          }
        }
        else
          g_warning("Box_Data_List::fill_from_database(): primary key not found in table %s", m_strTableName.c_str());
      }
    }
  }
  catch(std::exception& ex)
  {
    handle_error(ex);
  }
   
  //Select first record:
  Glib::RefPtr<Gtk::TreeModel> refModel = m_AddDel.get_model();
  if(refModel)
    m_AddDel.select_item(refModel->children().begin(), m_first_col);
 
  fill_end();
}

void Box_Data_List::on_AddDel_user_requested_add()
{
  Gtk::TreeModel::iterator iter = m_AddDel.get_item_placeholder();

  //Start editing in the primary key or the first cell if the primary key is auto-incremented (because there is no point in editing an auto-generated value)..
  guint index_primary_key = 0;
  bool bPresent = get_field_primary_key(index_primary_key); //If there is no primary key then the default of 0 is OK.
  if(bPresent)
  {
    Field fieldPrimaryKey = m_Fields[index_primary_key];
    if(fieldPrimaryKey.get_field_info().get_auto_increment())
    {
      //Start editing in the first cell that is not the primary key:
      if(index_primary_key == 0)
      {
        index_primary_key += 1; //TODO: Check that there is > 1 column.
      }
    }
  }

  guint treemodel_column = m_first_col + index_primary_key;
  m_AddDel.select_item(iter, treemodel_column, true /* start_editing */);
}

void Box_Data_List::on_AddDel_user_requested_edit(const Gtk::TreeModel::iterator& row)
{
  Glib::ustring strPrimaryKeyValue = m_AddDel.get_value_key(row); //The primary key is in the key.

  signal_user_requested_details().emit(strPrimaryKeyValue);
}

void Box_Data_List::on_AddDel_user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator&  /* rowEnd TODO */)
{
  Glib::ustring strPrimaryKeyValue = m_AddDel.get_value_key(rowStart); //The primary key is in the key.
  record_delete(strPrimaryKeyValue);

  //Remove the row:
  m_AddDel.remove_item(rowStart);
}

void Box_Data_List::on_AddDel_user_added(const Gtk::TreeModel::iterator& row)
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
      guint primary_key_field_index = 0;
      bool test = get_field_primary_key(primary_key_field_index);
      if(test)
      {
        guint primary_key_model_col_index = m_first_col + primary_key_field_index;

        Gnome::Gda::FieldAttributes fieldInfo = field.get_field_info();
        //If it's an auto-increment, then get the value and show it:
        if(fieldInfo.get_auto_increment())
        {
          unsigned long ulAutoID = generated_id; //get_last_auto_increment_value(data_model, fieldinfo.get_name());
          if(ulAutoID)
          {
            m_AddDel.set_value_key(row, util_string_from_decimal(ulAutoID)); //The AddDel key is always a string.
            m_AddDel.set_value(row, primary_key_model_col_index, ulAutoID);
          }
          else
            handle_error();
        }

        //Allow derived class to respond to record addition.
        strPrimaryKeyValue = m_AddDel.get_value_key(row);
        on_record_added(strPrimaryKeyValue);
      }
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

void Box_Data_List::on_AddDel_user_changed(const Gtk::TreeModel::iterator& row, guint col)
{
  const Glib::ustring& strPrimaryKeyValue = get_primary_key_value(row);
  if(!strPrimaryKeyValue.empty()) //If the record's primary key is filled in:
  {
    //Just update the record:
    try
    {
      guint primary_key_field_index = 0;
      bool bPresent = get_field_primary_key(primary_key_field_index);
      if(bPresent)
      {
        Field field_primary_key = m_Fields[primary_key_field_index];

        if(col >= m_first_col)
        {
          const guint changed_field_col_index = col - m_first_col;
          
          const Glib::ustring strPrimaryKey_Name = field_primary_key.get_name();
          const Field field = m_Fields[changed_field_col_index];
          const Glib::ustring strFieldName = field.get_name();

          const Gnome::Gda::Value strFieldValue = m_AddDel.get_value_as_value(row, col);

          Glib::ustring strQuery = "UPDATE " + m_strTableName;
          strQuery += " SET " + strFieldName + " = " + field.sql(strFieldValue);

          strQuery += " WHERE " + strPrimaryKey_Name + " = " + field_primary_key.sql(strPrimaryKeyValue);
          /* bool bTest = */ Query_execute(strQuery);  //TODO: Handle errors

          //Get values for lookup fields, if this field triggers those relationships:
          type_vecRelationships triggered_relationships = get_relationships_triggered_by(strFieldName);
          for(type_vecRelationships::const_iterator iter = triggered_relationships.begin(); iter != triggered_relationships.end(); ++iter)
          {
            type_vecFields lookup_fields = get_lookup_fields(iter->get_name());
            for(type_vecFields::const_iterator iterFields = lookup_fields.begin(); iterFields != lookup_fields.end(); ++iterFields)
            {
              g_warning("Triggered field: %s", iterFields->get_name().c_str());
            }
            
          }

          
          //if(!bTest)
          //  fill_from_database(); //Replace with correct values. //TODO: Just replace this one row
        }
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
  m_AddDel.select_item(m_AddDel.get_model()->children().begin(), m_first_col);

  signal_user_requested_details().emit(m_AddDel.get_value_key_selected());
}

void Box_Data_List::on_details_nav_previous()
{
  Gtk::TreeModel::iterator iter = m_AddDel.get_item_selected();
  if(iter)
  {
    //Don't try to select a negative record number.
    if(!m_AddDel.get_is_first_row(iter))
    {
      iter--;

      m_AddDel.select_item(iter, m_first_col);
      signal_user_requested_details().emit(m_AddDel.get_value_key_selected());
    }
  }
}

void Box_Data_List::on_details_nav_next()
{
  Gtk::TreeModel::iterator iter = m_AddDel.get_item_selected();
  if(iter)
  {
    //Don't go past the last record:
    if( !m_AddDel.get_is_last_row(iter) )
    {
      iter++;    
      m_AddDel.select_item(iter, m_first_col);

      signal_user_requested_details().emit(m_AddDel.get_value_key_selected());
    }
  }
}

void Box_Data_List::on_details_nav_last()
{
  Gtk::TreeModel::iterator iter = m_AddDel.get_model()->children().end();
  iter--;
  if(iter)
  {
    m_AddDel.select_item(iter, m_first_col);
    signal_user_requested_details().emit(m_AddDel.get_value_key_selected());
  }
}

void Box_Data_List::on_Details_record_deleted(Glib::ustring strPrimaryKey)
{
  //Find out which row is affected:
  Gtk::TreeModel::iterator iter = m_AddDel.get_row(strPrimaryKey);
  if(iter)
  {
    //Remove the row:
    Gtk::TreeModel::iterator iterNext = iter;
    iterNext++;
    
    m_AddDel.remove_item(iter);

    //Show Details for the next one:
    if(iterNext != m_AddDel.get_model()->children().end())
    {
      //Next record moves up one:
      on_AddDel_user_requested_edit(iterNext);
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

Glib::ustring Box_Data_List::get_primary_key_value(const Gtk::TreeModel::iterator& row)
{
  //The primary key is stored in the hidden key column (as well as maybe in another visible column);
  return m_AddDel.get_value_key(row);
}

Glib::ustring Box_Data_List::get_primary_key_value_selected()
{
  return m_AddDel.get_value_key_selected();
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

    //Add a column for each table field:
    bool first_col_added = false;
    for(type_vecFields::const_iterator iter =  listFieldsToShow.begin(); iter != listFieldsToShow.end(); ++iter)
    {
      guint col = m_AddDel.add_column(*iter);
      if(!first_col_added)
      {
        m_first_col = col; //Remember for later.
        first_col_added = true;
      }
    }
  }
}

void Box_Data_List::on_record_added(const Glib::ustring& /* strPrimaryKey */)
{
g_warning("on_record_added");
  //Overridden by Box_Data_List_Related.
  //m_AddDel.add_item(strPrimaryKey); //Add blank row.
}

Box_Data_List::type_signal_user_requested_details Box_Data_List::signal_user_requested_details()
{
  return m_signal_user_requested_details;
}
