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
#include "../data_structure/glomconversions.h"
#include "dialog_layout_list.h"
#include <sstream> //For stringstream
#include <libintl.h>

Box_Data_List::Box_Data_List()
: m_has_one_or_more_records(false)
{
  m_layout_name = "list";

  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_data_layout_list"); //TODO: Use a generic layout dialog?
  if(refXml)
  {
    Dialog_Layout_List* dialog = 0;
    refXml->get_widget_derived("window_data_layout_list", dialog);
    if(dialog)
    {
      m_pDialogLayout = dialog;
      m_pDialogLayout->signal_hide().connect( sigc::mem_fun(*this, &Box_Data::on_dialog_layout_hide) );
    }
  }
  
  m_strHint = gettext("When you change the data in a field the database is updated immediately.\n Click [Add] or enter data into the last row to add a new record.\n Leave automatic ID fields empty - they will be filled for you.\nOnly the first 100 records are shown.");

  pack_start(m_AddDel);
  m_AddDel.set_auto_add(false); //We want to add the row ourselves when the user clicks the Add button, because the default behaviour there is not suitable.
  m_AddDel.set_rules_hint(); //Use alternating row colors when the theme does that.
  
  //Connect signals:
  m_AddDel.signal_user_requested_add().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_requested_add)); //Only emitted when m_AddDel.set_auto_add(false) is used.
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_requested_edit));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_requested_delete));
  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_added));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_changed));
  m_AddDel.signal_user_reordered_columns().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_reordered_columns));
  
  m_AddDel.signal_user_requested_layout().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_requested_layout));


  //Groups are not very helpful for a list view:
  //m_pDialogLayout->set_show_groups(false);

  m_AddDel.show();
}

Box_Data_List::~Box_Data_List()
{
}

void Box_Data_List::fill_from_database()
{
  Bakery::BusyCursor(*get_app_window());
 
  try
  {
    sharedptr<SharedConnection> sharedconnection = connect_to_server();

    Box_DB_Table::fill_from_database();

    m_AddDel.remove_all();

    //Field Names:
    fill_column_titles();
   
    if(sharedconnection)
    {    
      Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

      Glib::ustring strWhereClausePiece;
      if(!m_strWhereClause.empty())
        strWhereClausePiece = " WHERE " + m_strWhereClause;

      m_Fields = get_fields_to_show();
      if(!m_Fields.empty())
      {
        Glib::ustring sql_part_fields;
        for(type_vecFields::const_iterator iter =  m_Fields.begin(); iter != m_Fields.end(); ++iter)
        {
          if(iter != m_Fields.begin())
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

              Gnome::Gda::Value key = result->get_value_at(primary_key_field_index, result_row);
              //It is usually an integer.
              if(GlomConversions::value_is_empty(key))
                g_warning("Box_Data_List::fill_from_database(): primary key value is empty");
              else
              {
                Gtk::TreeModel::iterator tree_iter = m_AddDel.add_item(key);

                type_vecFields::const_iterator iterFields = m_Fields.begin();

                //each field:
                guint cols_count = result->get_n_columns();
                for(guint uiCol = 0; uiCol < cols_count; uiCol++)
                {
                  Gnome::Gda::Value value = result->get_value_at(uiCol, result_row);
                  guint index = 0;

                  //g_warning("list fill: field_name=%s", iterFields->get_name().c_str());
                  //g_warning("  value_as_string=%s", value.to_string().c_str());

                  //TODO_Performance: This searches m_Fields again each time:
                  const Glib::ustring field_name = iterFields->get_name();
                  bool test = get_field_column_index(field_name, index);
                  ++iterFields;

                  if(test)
                  {
                    m_AddDel.set_value(tree_iter, index, value);
                    //g_warning("addedel size=%d", m_AddDel.get_count());
                  }
		  else
		    g_warning("  get_field_column_index failed: field name=%s", field_name.c_str());
                }
              }
            }
          }
          else
            g_warning("Box_Data_List::fill_from_database(): primary key not found in visible fields for table %s", m_strTableName.c_str());
        }
      } //If !m_Fields.empty()
    }
   
    //Select first record:
    Glib::RefPtr<Gtk::TreeModel> refModel = m_AddDel.get_model();
    if(refModel)
      m_AddDel.select_item(refModel->children().begin());
 
    fill_end();

  }
  catch(std::exception& ex)
  {
    handle_error(ex);
  }
}

void Box_Data_List::on_adddel_user_requested_add()
{
  Gtk::TreeModel::iterator iter = m_AddDel.get_item_placeholder();
  if(iter)
  {
    //Start editing in the primary key or the first cell if the primary key is auto-incremented (because there is no point in editing an auto-generated value)..
    guint index_primary_key = 0;
    bool bPresent = get_field_primary_key(index_primary_key); //If there is no primary key then the default of 0 is OK.
    guint index_field_to_edit = 0;
    if(bPresent)
    {
      index_field_to_edit = index_primary_key;

      Field fieldPrimaryKey = m_Fields[index_primary_key];
      if(fieldPrimaryKey.get_field_info().get_auto_increment())
      {
        //Start editing in the first cell that is not the primary key:
        if(index_primary_key == 0)
        {
          index_field_to_edit += 1;
        }
        else
          index_field_to_edit = 0;
      }
    }

    if(index_field_to_edit < m_Fields.size())
    {
      guint treemodel_column = 0;
      bool test = get_field_column_index(m_Fields[index_field_to_edit].get_name(), treemodel_column);
      if(test)
        m_AddDel.select_item(iter, treemodel_column, true /* start_editing */);
    }
    else
    {
      //The only keys are non-editable, so just add a row:
      on_adddel_user_added(iter);
      m_AddDel.select_item(iter); //without start_editing.
      //g_warning("Box_Data_List::on_adddel_user_requested_add(): index_field_to_edit does not exist: %d", index_field_to_edit);
    }
  }
}

void Box_Data_List::on_adddel_user_requested_edit(const Gtk::TreeModel::iterator& row)
{
  Gnome::Gda::Value primary_key_value = m_AddDel.get_value_key(row); //The primary key is in the key.

  signal_user_requested_details().emit(primary_key_value);
}

void Box_Data_List::on_adddel_user_requested_layout()
{
  show_layout_dialog();
}

void Box_Data_List::on_adddel_user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator&  /* rowEnd TODO */)
{
  if(rowStart)
  {
    //Ask the user for confirmation:
    Gtk::MessageDialog dialog(gettext("Are you sure that you would like to delete this record? The data in this record will then be permanently lost."), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::DELETE, Gtk::RESPONSE_OK);
    
    int response = dialog.run();
    if(response == Gtk::RESPONSE_OK)
    {
      record_delete( get_primary_key_value(rowStart) );

      //Remove the row:
      m_AddDel.remove_item(rowStart);
    }
  }
}

void Box_Data_List::on_adddel_user_added(const Gtk::TreeModel::iterator& row)
{
  Gnome::Gda::Value primary_key_value;

  const Glib::ustring& strPrimaryKeyName = get_primary_key_name();
  Field field;
  bool found = get_field(strPrimaryKeyName, field);
  if(!found)
    g_warning("Box_Data_List::on_adddel_user_added(): primary key %s not found.", strPrimaryKeyName.c_str());

  guint generated_id = 0;
  if(field.get_field_info().get_auto_increment())
  {
    //Auto-increment is awkward (we can't get the last-generated ID) with postgres, so we auto-generate it ourselves;
    generated_id = generate_next_auto_increment(m_strTableName, strPrimaryKeyName);  //TODO: return a Gnome::Gda::Value of an appropriate type.
    Glib::ustring strPrimaryKeyValue = util_string_from_decimal(generated_id);

    bool parsed = false;
    primary_key_value = GlomConversions::parse_value(field.get_glom_type(), strPrimaryKeyValue, parsed);
  }
  else
  {
    primary_key_value = get_primary_key_value(row);
  }

  sharedptr<SharedConnection> sharedconnection = connect_to_server(); //Keep it alive while we need the data_model.
  if(sharedconnection)
  {      
    Glib::RefPtr<Gnome::Gda::DataModel> data_model = record_new(true /* use entered field data*/, primary_key_value);
    if(data_model)
    {
      guint primary_key_model_col_index = 0;
      bool test = get_field_column_index(field.get_name(), primary_key_model_col_index);
      if(test)
      {
        const Gnome::Gda::FieldAttributes fieldInfo = field.get_field_info();
        //If it's an auto-increment, then get the value and show it:
        if(fieldInfo.get_auto_increment())
        {
          m_AddDel.set_value_key(row, primary_key_value); //The AddDel key is always a string.
          m_AddDel.set_value(row, primary_key_model_col_index, primary_key_value);
        }

        //Allow derived class to respond to record addition.
        on_record_added(primary_key_value);
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

void Box_Data_List::on_adddel_user_reordered_columns()
{
  Document_Glom* pDoc = dynamic_cast<Document_Glom*>(get_document());
  if(pDoc)
  {
    LayoutGroup group;
    group.set_name("toplevel");
    
    AddDel::type_vecStrings vec_field_names = m_AddDel.get_columns_order();

    guint index = 0;
    for(AddDel::type_vecStrings::iterator iter = vec_field_names.begin(); iter != vec_field_names.end(); ++iter)
    {
      LayoutItem_Field layout_item;
      layout_item.set_name(*iter);
      layout_item.m_sequence = index;
      group.add_item(layout_item, index); 
      ++index;
    }

    Document_Glom::type_mapLayoutGroupSequence mapGroups;
    mapGroups[1] = group;
    
    pDoc->set_data_layout_groups("list", m_strTableName, mapGroups);  
  }
}

void Box_Data_List::on_adddel_user_changed(const Gtk::TreeModel::iterator& row, guint col)
{ 
  const Gnome::Gda::Value primary_key_value = get_primary_key_value(row);
  if(!GlomConversions::value_is_empty(primary_key_value)) //If the record's primary key is filled in:
  {
    //Just update the record:
    try
    {
      guint primary_key_field_index = 0;
      bool bPresent = get_field_primary_key(primary_key_field_index);
      if(bPresent)
      {
        Field field_primary_key = m_Fields[primary_key_field_index];

        const Glib::ustring field_name = m_AddDel.get_column_field(col);
        Field field;
        bool test = get_fields_for_table_one_field(m_strTableName, field_name, field);
        if(test)
        {        
          const Gnome::Gda::Value field_value = m_AddDel.get_value(row, col);
             
          Glib::ustring strQuery = "UPDATE " + m_strTableName;
          strQuery += " SET " +  field.get_name() + " = " + field.sql(field_value);

          strQuery += " WHERE " + field_primary_key.get_name() + " = " + field_primary_key.sql(primary_key_value);
          Query_execute(strQuery);  //TODO: Handle errors

          //Get-and-set values for lookup fields, if this field triggers those relationships:
          do_lookups(row, field, field_value, field_primary_key, primary_key_value);
        }
        else
        {
          g_warning("Box_Data_List::on_adddel_user_changed(): get_fields_for_table_one_field() failed: m_strTableName=%s, field_name=%s", m_strTableName.c_str(), field_name.c_str());
        }
      }
      else
      {
        g_warning("Box_Data_List::on_adddel_user_changed(): primary key not found");
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

    bool found = get_field(get_primary_key_name(), field_primary_key);
    if(found && field_primary_key.get_field_info().get_auto_increment())
    {
      on_adddel_user_added(row);

       const Gnome::Gda::Value primaryKeyValue = get_primary_key_value(row); //TODO_Value
       if(!(GlomConversions::value_is_empty(primaryKeyValue))) //If the Add succeeeded:
         on_adddel_user_changed(row, col); //Change this field in the new record.
    }
  }

}

void Box_Data_List::do_lookups(const Gtk::TreeModel::iterator& row, const Field& field_changed, const Gnome::Gda::Value& field_value, const Field& primary_key, const Gnome::Gda::Value& primary_key_value)
{
   //Get values for lookup fields, if this field triggers those relationships:
   //TODO_performance: There is a LOT of iterating and copying here.
   const Glib::ustring strFieldName = field_changed.get_name();
   type_list_lookups lookups = get_lookup_fields(strFieldName);
   for(type_list_lookups::const_iterator iter = lookups.begin(); iter != lookups.end(); ++iter)
   {
     const Relationship relationship = iter->second;
     const Field field_lookup = iter->first;
     const Glib::ustring field_lookup_name = field_lookup.get_name();

     Field field_source;
     bool test = get_fields_for_table_one_field(relationship.get_to_table(), field_lookup.get_lookup_field(), field_source);
     if(test)
     {
       Gnome::Gda::Value value = get_lookup_value(iter->second /* relationship */,  field_source /* the field to look in to get the value */, field_value /* Value of to and from fields */);

       //Add it to the view:
       guint column_index = 0;
       bool test = get_field_column_index(field_lookup_name, column_index);
       if(test)
       {
         m_AddDel.set_value(row, column_index, value);
       }

       //Add it to the database (even if it is not shown in the view)
       Glib::ustring strQuery = "UPDATE " + m_strTableName;
       strQuery += " SET " + field_lookup_name + " = " + field_lookup.sql(value);
       strQuery += " WHERE " + primary_key.get_name() + " = " + primary_key.sql(primary_key_value);
       Query_execute(strQuery);  //TODO: Handle errors

       //TODO: Handle lookups triggered by these fields (recursively)? TODO: Check for infinitely looping lookups.
     }
   }

}

void Box_Data_List::on_details_nav_first()
{
  m_AddDel.select_item(m_AddDel.get_model()->children().begin());

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

      m_AddDel.select_item(iter);
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
      m_AddDel.select_item(iter);

      signal_user_requested_details().emit(m_AddDel.get_value_key_selected());
    }
  }
}

void Box_Data_List::on_details_nav_last()
{
  Gtk::TreeModel::iterator iter = m_AddDel.get_last_row();
  if(iter)
  {
    m_AddDel.select_item(iter);
    signal_user_requested_details().emit(m_AddDel.get_value_key_selected());
  }
}

void Box_Data_List::on_Details_record_deleted(Gnome::Gda::Value primary_key_value)
{
  //Find out which row is affected:
  Gtk::TreeModel::iterator iter = m_AddDel.get_row(primary_key_value);
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
      on_adddel_user_requested_edit(iterNext);
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

Gnome::Gda::Value Box_Data_List::get_primary_key_value(const Gtk::TreeModel::iterator& row)
{
  return m_AddDel.get_value_key(row);
}

Gnome::Gda::Value Box_Data_List::get_primary_key_value_selected()
{
  return Gnome::Gda::Value(m_AddDel.get_value_key_selected());
}

Gnome::Gda::Value Box_Data_List::get_entered_field_data(const Field& field) const
{
  //Get text from widget:

  guint index = 0;
  bool test = get_field_column_index(field.get_name(), index);
  if(test)
    return m_AddDel.get_value_selected(index);
  else
    return Gnome::Gda::Value(); //null.
}

void Box_Data_List::set_entered_field_data(const Field& field, const Gnome::Gda::Value& value)
{
  guint index = 0;
  bool test = get_field_column_index(field.get_name(), index);
  if(test)
    return m_AddDel.set_value_selected(index, value);
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
    for(type_vecFields::const_iterator iter =  listFieldsToShow.begin(); iter != listFieldsToShow.end(); ++iter)
    {
      m_AddDel.add_column(*iter);

      if(iter->get_field_info().get_primary_key())
        m_AddDel.set_key_field(*iter);
    }
  }
}

void Box_Data_List::on_record_added(const Gnome::Gda::Value& /* strPrimaryKey */)
{
  //Overridden by Box_Data_List_Related.
  //m_AddDel.add_item(strPrimaryKey); //Add blank row.
}

Box_Data_List::type_signal_user_requested_details Box_Data_List::signal_user_requested_details()
{
  return m_signal_user_requested_details;
}

bool Box_Data_List::get_field_column_index(const Glib::ustring& field_name, guint& index) const
{
  //Initialize output parameter:
  index = 0;
  
  //Get the index of the field with this name:
  guint i = 0;
  for(type_vecFields::const_iterator iter = m_Fields.begin(); iter != m_Fields.end(); ++iter)
  {
    if(iter->get_name() == field_name)
    {
      return m_AddDel.get_model_column_index(i, index); //Add the extra model columns to get the model column index from the field column index
    }
      
    ++i;
  }

  g_warning("Box_Data_List::get_field_column_index(): field not found.");
  return false; //failure.
}

Glib::ustring Box_Data_List::get_primary_key_name()
{
  return m_AddDel.get_key_field().get_name();
}
