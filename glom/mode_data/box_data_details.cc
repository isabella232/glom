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
#include "../data_structure/glomconversions.h"
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

Gnome::Gda::Value Box_Data_Details::get_primary_key_value() const
{
  return m_primary_key_value;
}

void Box_Data_Details::init_db_details(const Glib::ustring& strDatabaseName, const Glib::ustring& strTableName, const Gnome::Gda::Value& primary_key_value)
{
  m_primary_key_value = primary_key_value;

  Box_DB_Table::init_db_details(strDatabaseName, strTableName);
}

void Box_Data_Details::init_db_details(const Gnome::Gda::Value& primary_key_value)
{
  init_db_details(get_database_name(), get_table_name(), primary_key_value);
}

void Box_Data_Details::init_db_details_blank()
{
  init_db_details( Gnome::Gda::Value() );
}

void Box_Data_Details::fill_from_database_layout()
{
  Bakery::BusyCursor(*get_app_window());

  //Remove existing child widgets:
  m_FlowTable.remove_all();
                          
  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  if(document)
  {          
    Document_Glom::type_mapLayoutGroupSequence layout_groups = document->get_data_layout_groups_plus_new_fields("details", m_strTableName);
    for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = layout_groups.begin(); iter != layout_groups.end(); ++iter)
    {
      const LayoutGroup& group = iter->second;
  
      if(!group.m_others)
      {
        FlowTableWithFields::type_map_field_sequence fields;

        //Get the fields:
        guint sequence = 0;
        for(LayoutGroup::type_map_items::const_iterator iterItems = group.m_map_items.begin(); iterItems != group.m_map_items.end(); ++iterItems)
        {
          Field field;
          bool found = get_fields_for_table_one_field(m_strTableName, iterItems->second.m_field_name, field);
          if(found)
          {
            fields[sequence] = field;
            ++sequence;
          }
        }
            
        m_FlowTable.add_group(group.m_group_name, group.m_title, fields);
      }
      else
      {
        //Add the extra fields:
        for(LayoutGroup::type_map_items::const_iterator iterItems = group.m_map_items.begin(); iterItems != group.m_map_items.end(); ++iterItems)
        {
          Field field;
          bool found = get_fields_for_table_one_field(m_strTableName, iterItems->second.m_field_name, field);
          if(found)
          {
            m_FlowTable.add_field(field); //This could add it to a sub-flowtable, or the main flow-table.
           
            //Do not allow editing of auto-increment fields:
            if(field.get_field_info().get_auto_increment())
              m_FlowTable.set_field_editable(field, false);
          }
        }
      }
    }
  }
      
}

void Box_Data_Details::fill_from_database()
{
  Bakery::BusyCursor(*get_app_window());

  Box_DB_Table::fill_from_database();
         
  type_vecFields listFieldsToShow = get_fields_to_show();
  m_Fields =  listFieldsToShow;

  fill_from_database_layout(); //TODO: Only do this when the layout has changed.

  Field field_primary_key;
  bool primary_key_found  = get_field_primary_key(field_primary_key);

  if(primary_key_found && !GlomConversions::value_is_empty(m_primary_key_value)) //If there is a record to show:
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
        query << "SELECT " << sql_part_fields << " FROM " << m_strTableName << " WHERE " << get_table_name() + "." + get_primarykey_name() << " = " << field_primary_key.sql(m_primary_key_value);
        Glib::RefPtr<Gnome::Gda::DataModel> result = connection->execute_single_command(query.str());

        if(result && result->get_n_rows())
        {
          const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
          if(pDoc)
          {
            //Get glom-specific field info:
            Document_Glom::type_vecFields vecFields = pDoc->get_table_fields(m_strTableName);

            const int row_number = 0; //The only row.
            const int cols_count = result->get_n_columns();
            for(int i = 0; i < cols_count; ++i)
            {
              const Field& field = m_Fields[i];

              //Field value:
              Gnome::Gda::Value value = result->get_value_at(i, row_number);
              m_FlowTable.set_field_value(field, value);
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

    if(vecRelationships.empty())
    {
      //Hide the relationships pane:
      m_Frame_Related.hide();
    }
    else
    {
      m_Frame_Related.show();
      
      //Add the relationships:
      for(Document_Glom::type_vecRelationships::iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); iter++)
      {
       const Relationship& relationship = *iter;

       //bool bMakeRelatedTab = true;
       //guint rowKey = 0;

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
              
           Gnome::Gda::Value value = m_FlowTable.get_field_value(from_field);

           Field field;
           bool test = get_fields_for_table_one_field(m_strTableName, from_field, field);
           if(test)
           {
             pBox->init_db_details(get_database_name(), relationship, value, m_primary_key_value);
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

}

void Box_Data_Details::on_button_new()
{
  if(confirm_discard_unstored_data())
  {
    Field field_primary_key;
    bool primary_key_found  = get_field_primary_key(field_primary_key);
    if(primary_key_found)
    {
      Gnome::Gda::FieldAttributes fieldinfo = field_primary_key.get_field_info();
      if(fieldinfo.get_auto_increment()) //If the primary key is an auto-increment:
      {
        //Just make a new record, and show it:
        guint generated_id = generate_next_auto_increment(m_strTableName, field_primary_key.get_name()); //TODO: This should return a Gda::Value

        //TODO: Create the Gda::Value directly from the integer.
        bool parsed = false; 
        Gnome::Gda::Value primary_key_value = GlomConversions::parse_value(field_primary_key.get_glom_type(), util_string_from_decimal(generated_id), parsed);
        if(parsed)
        {
          record_new(primary_key_value);
          init_db_details(primary_key_value);
        }
      }
      else
      {
        //It's not an auto-increment primary key,
        //so just blank the fields ready for a primary key later.
        init_db_details_blank(); //shows blank record.
      }
    }

  } //if(confirm_discard_unstored_data())
}

void Box_Data_Details::on_button_del()
{
  if( GlomConversions::value_is_empty(get_primary_key_value()) )
  {
    //Tell user that a primary key is needed to delete a record:
    Gtk::MessageDialog dialog(gettext("This record can not be deleted because there is no primary key."));
    dialog.run();
  }
  else
  {
    bool bTest = record_delete(m_primary_key_value);

    if(bTest)
    {
      //Tell the list that it has been deleted:
      //It will go to the next (or last) record,
      signal_record_deleted().emit(m_primary_key_value);
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

Gnome::Gda::Value Box_Data_Details::get_entered_field_data(const Field& field) const
{
  return m_FlowTable.get_field_value(field);
}

Gnome::Gda::Value Box_Data_Details::get_primary_key_value_selected()
{

  Glib::ustring strResult;
/* TODO_port.
  guint uiRow = 0;
  bool bPresent = get_field_primary_key(uiRow);
  if(bPresent)
    strResult = m_AddDel.get_value(uiRow);

*/
  return Gnome::Gda::Value(strResult);
}

void Box_Data_Details::on_related_user_requested_details(Gnome::Gda::Value key_value, Glib::ustring strTableName)
{
  signal_user_requested_related_details().emit(strTableName, key_value);
}

void Box_Data_Details::on_related_record_added(Gnome::Gda::Value /* strKeyValue */, Glib::ustring /* strFromKeyName */)
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
  Glib::ustring strFromKeyValue = get_entered_field_data(iKey).get_data();

  if(strFromKeyValue.size() == 0)
  {
    //Set the From key value, to link the new related record (the first one so far) with the parent record.
    m_AddDel.set_value(iKey, m_ColumnValue, strKeyValue);
    on_adddel_user_changed(iKey, m_ColumnValue); //Update the database.
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
    
   const Glib::ustring strPrimaryKey_Name = get_primarykey_name();
   const Glib::ustring strFieldName = id;
   const Gnome::Gda::Value field_value = m_FlowTable.get_field_value(id);

   if(!strPrimaryKey_Name.empty())
   {
     Field fieldInfoPK;
     bool test = get_field(strPrimaryKey_Name, fieldInfoPK);
     if(!test)
       g_warning("Box_Data_Details::on_flowtable_field_edited(): field not found");

     Gnome::Gda::Value primary_key_value = get_primary_key_value();
     if(!GlomConversions::value_is_empty(primary_key_value)) //If there is a primary key value:
     {
       Field field;
       bool test = get_field(strFieldName, field);
       if(!test)
         g_warning("Box_Data_Details::on_flowtable_field_edited: field not found");

       //Update the field in the record (the record with this primary key):
       try
       {
         Glib::ustring strQuery = "UPDATE " + m_strTableName;
         strQuery += " SET " +  /* get_table_name() + "." +*/ strFieldName + " = " + field.sql(field_value);
         strQuery += " WHERE " + get_table_name() + "." + strPrimaryKey_Name + " = " + fieldInfoPK.sql(get_primary_key_value());
         bool bTest = Query_execute(strQuery);

         if(!bTest)
         {
           //Update failed.
           fill_from_database(); //Replace with correct values.
         }
         else
         {
           //Get-and-set values for lookup fields, if this field triggers those relationships:
           do_lookups(field, field_value, fieldInfoPK, primary_key_value);

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

void Box_Data_Details::do_lookups(const Field& field_changed, const Gnome::Gda::Value& field_value, const Field& primary_key, const Gnome::Gda::Value& primary_key_value)
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
       m_FlowTable.set_field_value(field_lookup_name, value);

       //Add it to the database (even if it is not shown in the view)
       Glib::ustring strQuery = "UPDATE " + m_strTableName;
       strQuery += " SET " + field_lookup_name + " = " + field_lookup.sql(value);
       strQuery += " WHERE " + primary_key.get_name() + " = " + primary_key.sql(primary_key_value);
       Query_execute(strQuery);  //TODO: Handle errors

       //TODO: Handle lookups triggered by these fields (recursively)? TODO: Check for infinitely looping lookups.
     }
   }

}





