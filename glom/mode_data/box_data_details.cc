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

#include "box_data_details.h"
#include "../data_structure/field.h"
#include "../data_structure/relationship.h"
#include <sstream> //For stringstream
#include <libintl.h>

Box_Data_Details::Box_Data_Details(bool bWithNavButtons /* = true */)
: m_HBox(false, 6),
  m_Button_New(Gtk::Stock::ADD),
  m_Button_Del(Gtk::Stock::DELETE),
  m_Button_Nav_First(Gtk::Stock::GOTO_FIRST),
  m_Button_Nav_Prev(Gtk::Stock::GO_BACK),
  m_Button_Nav_Next(Gtk::Stock::GO_FORWARD),
  m_Button_Nav_Last(Gtk::Stock::GOTO_LAST),
  m_bDoNotRefreshRelated(false),
  m_ignore_signals(true)
{
  m_layout_name = "details";

  m_FlowTable.set_columns_count(2);
  m_FlowTable.set_padding(6);
  
  m_strHint = gettext("When you change the data in a field the database is updated immediately.\n Click [New] to add a new record.\n Leave automatic ID fields empty - they will be filled for you.");


  //m_Paned.set_border_width(6);
  m_Paned.set_position(200); //Set size of top pane.
  pack_start(m_Paned);
  m_Paned.add(m_FlowTable);

  //Related records:
  m_Label_Related.set_text(gettext("<b>Related Records</b>"));
  m_Label_Related.set_use_markup(true);
  m_Frame_Related.set_label_widget(m_Label_Related);
  m_Frame_Related.set_shadow_type(Gtk::SHADOW_NONE);  
  m_Frame_Related.add(m_Alignment_Related);
  m_Alignment_Related.set_padding(0, 0, 12, 0);
  
  m_Notebook_Related.set_border_width(6);
  m_Alignment_Related.add(m_Notebook_Related);
  m_Paned.add(m_Frame_Related);

  //Add or delete record:
  m_HBox.pack_start(m_Button_New, Gtk::PACK_SHRINK);
  m_HBox.pack_start(m_Button_Del,  Gtk::PACK_SHRINK);

   //Link buttons to handlers:
  m_Button_New.signal_clicked().connect(sigc::mem_fun(*this, &Box_Data_Details::on_button_new));
  m_Button_Del.signal_clicked().connect(sigc::mem_fun(*this, &Box_Data_Details::on_button_del));

  //Navigation:
  if(bWithNavButtons)
  {
    m_HBox.pack_end(m_Button_Nav_Last, Gtk::PACK_SHRINK);
    m_HBox.pack_end(m_Button_Nav_Next, Gtk::PACK_SHRINK);
    m_HBox.pack_end(m_Button_Nav_Prev, Gtk::PACK_SHRINK);
    m_HBox.pack_end(m_Button_Nav_First, Gtk::PACK_SHRINK);
  }

  //Link buttons to handlers:
  m_Button_Nav_First.signal_clicked().connect(sigc::mem_fun(*this, &Box_Data_Details::on_button_nav_first));
  m_Button_Nav_Prev.signal_clicked().connect(sigc::mem_fun(*this, &Box_Data_Details::on_button_nav_prev));
  m_Button_Nav_Next.signal_clicked().connect(sigc::mem_fun(*this, &Box_Data_Details::on_button_nav_next));
  m_Button_Nav_Last.signal_clicked().connect(sigc::mem_fun(*this, &Box_Data_Details::on_button_nav_last));

  pack_start(m_HBox, Gtk::PACK_SHRINK);

  m_FlowTable.signal_field_edited().connect( sigc::mem_fun(*this,  &Box_Data_Details::on_flowtable_field_edited) );
  show_all();

  m_ignore_signals = false;
}

Box_Data_Details::~Box_Data_Details()
{
}

Glib::ustring Box_Data_Details::get_primary_key_value() const
{
  return m_strPrimaryKeyValue;
}

void Box_Data_Details::init_db_details(const Glib::ustring& strDatabaseName, const Glib::ustring& strTableName, const Glib::ustring& strPrimaryKeyValue)
{
  m_strPrimaryKeyValue = strPrimaryKeyValue;

  Box_DB_Table::init_db_details(strDatabaseName, strTableName);
}

void Box_Data_Details::init_db_details(const Glib::ustring& strPrimaryKeyValue)
{
  init_db_details(get_database_name(), get_table_name(), strPrimaryKeyValue);
}

void Box_Data_Details::fill_from_database()
{
  Bakery::BusyCursor(*get_app_window());
        
  Box_DB_Table::fill_from_database();

  type_vecFields listFieldsToShow = get_fields_to_show();
  m_Fields =  listFieldsToShow;
  
  const Glib::ustring strPrimaryKeyName = get_PrimaryKey_Name();

  if(strPrimaryKeyName.size() && m_strPrimaryKeyValue.size()) //If there is a record to show:
  {
    try
    {
      sharedptr<SharedConnection> sharedconnection = connect_to_server();
     
      if(sharedconnection)
      {
        Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();
        
        Glib::ustring sql_part_fields;
        for(type_vecFields::const_iterator iter =  listFieldsToShow.begin(); iter != listFieldsToShow.end(); ++iter)
        {
          if(iter != listFieldsToShow.begin())
            sql_part_fields += ",";

          sql_part_fields += iter->get_name();
        }

        std::stringstream query;
        query << "SELECT " << sql_part_fields << " FROM " << m_strTableName << " WHERE " << get_table_name() + "." + get_PrimaryKey_Name() << " = " << m_strPrimaryKeyValue;
        Glib::RefPtr<Gnome::Gda::DataModel> result = connection->execute_single_command(query.str());

        if(result && result->get_n_rows())
        {
          const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
          if(pDoc)
          {
            //Get glom-specific field info:
            Document_Glom::type_vecFields vecFields = pDoc->get_table_fields(m_strTableName);

            //Remove existing child widgets:
            m_FlowTable.remove_all();
            
            const int row_number = 0; //The only row.
            const int cols_count = result->get_n_columns();
            for(guint i = 0; i < cols_count; i ++)
            {
              //Field name:
              Glib::ustring strFieldName = m_Fields[i].get_name();
              Glib::ustring strFieldTitle = strFieldName;
                
              EntryGlom* pEntry = m_FlowTable.get_field(strFieldName);        
              if(!pEntry) //Create a widget for the field if one does not exist already:
              {
                //See if a field title has been specified:
                Document_Glom::type_vecFields::iterator iterFind = std::find_if( vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(strFieldName) );
                if(iterFind != vecFields.end()) //if it is there:
                  strFieldTitle = iterFind->get_title_or_name();

                m_FlowTable.add_field(strFieldTitle, strFieldName);
                pEntry = m_FlowTable.get_field(strFieldName);
              }

              if(pEntry)
              {
                //Field value:
                Gnome::Gda::Value value = result->get_value_at(i, row_number);
                const Glib::ustring& strValue = m_Fields[i].value_to_string(value); //TODO: Handle the actual value type.

                pEntry->set_text(strValue);
              }
            }
          }
        }
      }
    }
    catch(std::exception& ex)
    {
      handle_error(ex);
    }
  }
  else
  {
    //Show blank record:
    //TODO
  }

  fill_related();

  fill_end();
}

void Box_Data_Details::fill_related()
{
  if(!m_bDoNotRefreshRelated)
  {
    //Clear existing pages:
    {
      Gtk::Notebook_Helpers::PageList& pages = m_Notebook_Related.pages();
      for(Gtk::Notebook_Helpers::PageList::iterator iter = pages.begin(); iter != pages.end(); iter++)
      {
        Gtk::Notebook_Helpers::Page page = *iter;
        Box_Data_List_Related* pWidget = dynamic_cast<Box_Data_List_Related*>(page.get_child());
        if(pWidget)
          remove_view(pWidget);
        else
          std::cout << "Box_Data_Details::fill_related(): unexpected child widget." << std::endl;
      }

      pages.clear(); //deletes Gtk::manage()d boxes.
    }

    //Get relationships from the document:
    Document_Glom::type_vecRelationships vecRelationships = m_pDocument->get_relationships(m_strTableName);

    //Add the relationships:
    for(Document_Glom::type_vecRelationships::iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); iter++)
    {
       const Relationship& relationship = *iter;

       bool bMakeRelatedTab = true;
       guint rowKey = 0;

       Glib::ustring from_field = relationship.get_from_field();
       if(!from_field.empty())
       {
         const Glib::ustring to_table_name =  relationship.get_to_table();
         const Glib::ustring to_field_name = relationship.get_to_field();

         //Check that the to_field still exists in the to_table structure:
         //(note that the to_field will not necessarily be visible in the related records list, though it must be in the database
         //TODO_performance:
         type_vecFields vecRelatedFields = get_fields_for_table_from_database(to_table_name); //Fields in the database;
         type_vecFields::const_iterator iterFind = std::find_if(vecRelatedFields.begin(), vecRelatedFields.end(), predicate_FieldHasName<Field>(to_field_name));
         if(iterFind != vecRelatedFields.end() ) //If it was found
         {    
           Box_Data_List_Related* pBox = Gtk::manage(new Box_Data_List_Related());
           add_view(pBox); //So that it knows about the Document.
           m_Notebook_Related.pages().push_back( Gtk::Notebook_Helpers::TabElem(*pBox, relationship.get_name()) );

           Gtk::Entry* pEntry = m_FlowTable.get_field(from_field);
           if(pEntry)
           {
             Glib::ustring strKeyValue = pEntry->get_text();

             Field field = get_fields_for_table_one_field(m_strTableName,  from_field);

             if(strKeyValue.size()) //Don't use NULL as a key value.
               strKeyValue = field.sql(strKeyValue); //Quote/Escape it if necessary.

             pBox->init_db_details(get_database_name(), relationship, strKeyValue, m_strPrimaryKeyValue);
             pBox->show_all();

             //Connect signals:
             pBox->signal_record_added.connect( sigc::bind(
               sigc::mem_fun(*this, &Box_Data_Details::on_related_record_added), relationship.get_from_field() )
             );

             pBox->signal_user_requested_details().connect( sigc::bind(
               sigc::mem_fun(*this, &Box_Data_Details::on_related_user_requested_details), relationship.get_to_table() )
             );
           }
         }
       }
    }
  }

}

void Box_Data_Details::on_button_new()
{
  if(confirm_discard_unstored_data())
  {
    const Glib::ustring strPrimaryKeyName = get_PrimaryKey_Name();
    Field field;
    bool test = get_field(strPrimaryKeyName, field);

    Gnome::Gda::FieldAttributes fieldinfo = field.get_field_info();
    if(fieldinfo.get_auto_increment()) //If the primary key is an auto-increment:
    {
      //Just make a new record, and show it:
      guint generated_id = generate_next_auto_increment(m_strTableName, strPrimaryKeyName);
      Glib::ustring strPrimaryKeyValue = util_string_from_decimal(generated_id);

      record_new(strPrimaryKeyValue);
      init_db_details(strPrimaryKeyValue);
    }
    else
    {
      //It's not an auto-increment primary key,
      //so just blank the fields ready for a primary key later.
      init_db_details(""); //shows blank record.
    }

  } //if(confirm_discard_unstored_data())
}

void Box_Data_Details::on_button_del()
{
  if(get_primary_key_value().size() == 0)
  {
    //Tell user that a primary key is needed to delete a record:
    Gtk::MessageDialog dialog(gettext("This record can not be deleted because there is no primary key."));
    dialog.run();
  }
  else
  {
    bool bTest = record_delete(m_strPrimaryKeyValue);

    if(bTest)
    {
      //Tell the list that it has been deleted:
      //It will go to the next (or last) record,
      signal_record_deleted().emit(m_strPrimaryKeyValue);
    }
  }
}

void Box_Data_Details::on_button_nav_first()
{
  if(confirm_discard_unstored_data())
    signal_nav_first().emit();
}

void Box_Data_Details::on_button_nav_prev()
{
  if(confirm_discard_unstored_data())
    signal_nav_prev().emit();
}

void Box_Data_Details::on_button_nav_next()
{
  if(confirm_discard_unstored_data())
    signal_nav_next().emit();
}

void Box_Data_Details::on_button_nav_last()
{
  if(confirm_discard_unstored_data())
    signal_nav_last().emit();
}

Field Box_Data_Details::get_Entered_Field(guint index) const
{
  //Get Gnome::Gda::FieldAttributes from base:
  Field FieldResult = Box_Data::get_Entered_Field(index);
 /* TODO_port
  //Get text from widget:
  if(index < get_Entered_Field_count())
  {
    const Glib::ustring& strData = m_AddDel.get_value(index);
    FieldResult.set_data(strData);
  }
  */
  return FieldResult;
}

void Box_Data_Details::on_AddDel_user_changed(guint row, guint col)
{
/*
  if(row < m_Fields.size())
  {
    const Glib::ustring& strPrimaryKey_Name = get_PrimaryKey_Name();
    const Glib::ustring& strFieldName = m_Fields[row].get_name();
    const Glib::ustring& strFieldValue = m_AddDel.get_value(row);

    if(strPrimaryKey_Name.size())
    {
      Gnome::Gda::FieldAttributes fieldInfoPK;
      bool test = get_field(strPrimaryKey_Name, fieldInfoPK);

      if(get_primary_key_value().size()) //If there is a primary key value:
      {
        //Update the field in the record (the record with this primary key):
        try
        {
          Glib::ustring strQuery = "UPDATE " + m_strTableName;
          strQuery += " SET " +  get_table_name() + "." + strFieldName + " = " + Field::sql(strFieldValue);
          strQuery += " WHERE " + get_table_name() + "." + strPrimaryKey_Name + " = " + Field::sql(get_primary_key_value());
          bool bTest = Query_execute(strQuery);

          if(!bTest)
          {
            //Update failed.
            fill_from_database(); //Replace with correct values.
          }
          else
          {
            //If this is a foreign key then refresh the related records:
            bool bIsForeignKey = false;
            Document_Glom::type_vecRelationships vecRelationships = m_pDocument->get_relationships(m_strTableName);
            for(Document_Glom::type_vecRelationships::iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); iter++)
            {
              const Relationship& relationship = *iter;

              if(relationship.get_from_field() == strFieldName)
              {
                bIsForeignKey = true;
                break;
              }
            }

            if(bIsForeignKey)
              fill_related();
          }

        }
        catch(const std::exception& ex)
        {
          handle_error(ex);
        }
      }
      else
      {
        //There is no current primary key:

        if(fieldInfoPK.get_auto_increment()) //If the primary key is an auto-increment:
        {
          if(strFieldName == strPrimaryKey_Name) //If edited field is the primary key.
          {
            //Warn user that they can't choose their own primary key:
            Gtk::MessageDialog dialog ("The primary key is auto-incremented.\n You may not enter your own primary key value.");
            dialog.run();
          }
          else
          {
            //The edited field is not the primary key:

            //Create new record:
            bool bTest = record_new();

            if(bTest)
            {
              try
              {
                //Get new auto-generated primary key:
                sharedptr<SharedConnection> sharedconnection = connect_to_server();
                if(sharedconnection)
                {
                  Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();
                  unsigned long ulAutoID = get_last_auto_increment_value();

                  if(ulAutoID)
                  {
                    gchar pchAutoID[10] = {0};
                    sprintf(pchAutoID, "%d", (guint)ulAutoID);
                    Glib::ustring strAutoID(pchAutoID);

                    //Show the new record:
                    init_db_details(strAutoID);

                    //Set the edited field in the new record:
                    m_AddDel.set_value(row, col, strFieldValue);

                    //Update the field value in the database:
                    on_AddDel_user_changed(row, col);
                  }
                }
              }
              catch(const std::exception& ex)
              {
                handle_error(ex);
              }
            }
          } //if(strFieldName == strPrimaryKey_Name)
        }
        else
        {
          //It is not auto-generated:

          if(strFieldName == strPrimaryKey_Name) //if it is the primary key that' being edited.
          {
            //Create new record with this primary key,
            //and all the other field values too.
            //see comments after 'else':
            record_new_from_entered();
          }
          else
          {
            //The record does not exist yet.
            //The values in the other fields will have to wait
            //until the primary key is set by the user.

            set_unstored_data(true); //Cause a warning if this is never put into the database.
          }
        }
      } //if(get_primary_key_value().size())
    } //if(strPrimaryKey_Name.size())
  }
  */
}

Glib::ustring Box_Data_Details::get_primary_key_value_selected()
{

  Glib::ustring strResult;
/* TODO_port.
  guint uiRow = 0;
  bool bPresent = get_field_primary_key(uiRow);
  if(bPresent)
    strResult = m_AddDel.get_value(uiRow);

*/
  return strResult;
}

void Box_Data_Details::on_related_user_requested_details(Glib::ustring strKeyValue, Glib::ustring strTableName)
{
  signal_user_requested_related_details().emit(strTableName, strKeyValue);
}

void Box_Data_Details::on_related_record_added(Glib::ustring strKeyValue, Glib::ustring strFromKeyName)
{
  //Prevent deletion of Related boxes.
  //One of them emitted this signal, and is probably still being edited.
  //This prevents a crash.
  bool bDoNotRefreshRelated = m_bDoNotRefreshRelated;
  m_bDoNotRefreshRelated = true;

  //std::cout << "Box_Data_Details::on_related_record_added(): " << strKeyValue << ", " << strFromKeyName << std::endl;
  //Get current FromKey value:

 /* TODO_port
  guint iKey = 0;
  bool bTest = get_field_index(strFromKeyName, iKey);
  Glib::ustring strFromKeyValue = get_Entered_Field(iKey).get_data();

  if(strFromKeyValue.size() == 0)
  {
    //Set the From key value, to link the new related record (the first one so far) with the parent record.
    m_AddDel.set_value(iKey, m_ColumnValue, strKeyValue);
    on_AddDel_user_changed(iKey, m_ColumnValue); //Update the database.
  }
  */

  //Restore value:
  m_bDoNotRefreshRelated = bDoNotRefreshRelated;
}

Box_Data_Details::type_signal_void Box_Data_Details::signal_nav_first()
{
  return m_signal_nav_first;
}

Box_Data_Details::type_signal_void Box_Data_Details::signal_nav_prev()
{
  return m_signal_nav_prev;
}

Box_Data_Details::type_signal_void Box_Data_Details::signal_nav_next()
{
  return m_signal_nav_next;
}

Box_Data_Details::type_signal_void Box_Data_Details::signal_nav_last()
{
  return m_signal_nav_last;
}

Box_Data_Details::type_signal_record_deleted Box_Data_Details::signal_record_deleted()
{
  return m_signal_record_deleted; 
}

Box_Data_Details::type_signal_user_requested_related_details Box_Data_Details::signal_user_requested_related_details()
{
  return m_signal_user_requested_related_details;
}

void Box_Data_Details::on_flowtable_field_edited(Glib::ustring id)
{
  if(m_ignore_signals)
    return;
    
  Gtk::Entry* entry = m_FlowTable.get_field(id);
  if(entry)
  {
     const Glib::ustring& strPrimaryKey_Name = get_PrimaryKey_Name();
     const Glib::ustring& strFieldName = id;
     const Glib::ustring& strFieldValue = entry->get_text();

     if(strPrimaryKey_Name.size())
     {
       Field fieldInfoPK;
       bool test = get_field(strPrimaryKey_Name, fieldInfoPK);


       Glib::ustring primary_key_value = get_primary_key_value();
       if(!primary_key_value.empty()) //If there is a primary key value:
       {
         Field field;
         bool test = get_field(strFieldName, field);
       
         //Update the field in the record (the record with this primary key):
         try
         {
           Glib::ustring strQuery = "UPDATE " + m_strTableName;
           strQuery += " SET " +  /* get_table_name() + "." +*/ strFieldName + " = " + field.sql(strFieldValue);
           strQuery += " WHERE " + get_table_name() + "." + strPrimaryKey_Name + " = " + fieldInfoPK.sql(get_primary_key_value());
           bool bTest = Query_execute(strQuery);

           if(!bTest)
           {
             //Update failed.
             fill_from_database(); //Replace with correct values.
           }
           else
           {
             //If this is a foreign key then refresh the related records:
             bool bIsForeignKey = false;
             Document_Glom::type_vecRelationships vecRelationships = m_pDocument->get_relationships(m_strTableName);
             for(Document_Glom::type_vecRelationships::iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); iter++)
             {
               const Relationship& relationship = *iter;

               if(relationship.get_from_field() == strFieldName)
               {
                 bIsForeignKey = true;
                 break;
               }
             }

             if(bIsForeignKey)
               fill_related();
           }

         }
         catch(const std::exception& ex)
         {
           handle_error(ex);
         }
       }
       else
       {
         //There is no current primary key:

         if(fieldInfoPK.get_field_info().get_auto_increment()) //If the primary key is an auto-increment:
         {
           if(strFieldName == strPrimaryKey_Name) //If edited field is the primary key.
           {
             //Warn user that they can't choose their own primary key:
             Gtk::MessageDialog dialog ("The primary key is auto-incremented.\n You may not enter your own primary key value.");
             dialog.run();
           }
         }
         else
         {
           //It is not auto-generated:

           if(strFieldName == strPrimaryKey_Name) //if it is the primary key that' being edited.
           {
             //Create new record with this primary key,
             //and all the other field values too.
             //see comments after 'else':
             record_new_from_entered();
           }
           else
           {
             //The record does not exist yet.
             //The values in the other fields will have to wait
             //until the primary key is set by the user.

             set_unstored_data(true); //Cause a warning if this is never put into the database.
           }
         }
       } //if(get_primary_key_value().size())
     } //if(strPrimaryKey_Name.size())
  }
}



