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
#include "dialog_layout_details.h"
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

  add_view(&m_FlowTable); //Allow this to access the document too.

  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_data_layout_details"); //TODO: Use a generic layout dialog?
  if(refXml)
  {
    Dialog_Layout_Details* dialog = 0;
    refXml->get_widget_derived("window_data_layout_details", dialog);
    if(dialog)
    {
      m_pDialogLayout = dialog;
      m_pDialogLayout->signal_hide().connect( sigc::mem_fun(static_cast<Box_Data&>(*this), &Box_Data::on_dialog_layout_hide) );
    }
  }
  
  m_FlowTable.set_columns_count(1); //Sub-groups will have multiple columns (by default, there is one sub-group, with 2 columns).
  m_FlowTable.set_padding(6);
  
  m_strHint = gettext("When you change the data in a field the database is updated immediately.\n Click [New] to add a new record.\n Leave automatic ID fields empty - they will be filled for you.");


  //m_Paned.set_border_width(6);
  m_Paned.set_position(200); //Set size of top pane.
  pack_start(m_Paned);
  m_Paned.add(m_FlowTable);

  //Related records:
  /*
  m_Label_Related.set_text(gettext("<b>Related Records</b>"));
  m_Label_Related.set_use_markup(true);
  m_Frame_Related.set_label_widget(m_Label_Related);
  m_Frame_Related.set_shadow_type(Gtk::SHADOW_NONE);  
  m_Frame_Related.add(m_Alignment_Related);
  m_Alignment_Related.set_padding(0, 0, 12, 0);
  
  m_Notebook_Related.set_border_width(6);
  m_Alignment_Related.add(m_Notebook_Related);
  m_Paned.add(m_Frame_Related);
  */

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

  m_FlowTable.signal_layout_changed().connect( sigc::mem_fun(*this, &Box_Data_Details::on_flowtable_layout_changed) );

  m_ignore_signals = false;
}

Box_Data_Details::~Box_Data_Details()
{
}

Gnome::Gda::Value Box_Data_Details::get_primary_key_value() const
{
  return m_primary_key_value;
}

void Box_Data_Details::init_db_details(const Glib::ustring& strTableName, const Gnome::Gda::Value& primary_key_value)
{
  m_primary_key_value = primary_key_value;

  get_field_primary_key_for_table(strTableName, m_field_primary_key);

  Box_Data::init_db_details(strTableName);
}

void Box_Data_Details::refresh_db_details(const Gnome::Gda::Value& primary_key_value)
{
  init_db_details(get_table_name(), primary_key_value);
}

void Box_Data_Details::refresh_db_details_blank()
{
  refresh_db_details( Gnome::Gda::Value() );
}

void Box_Data_Details::fill_from_database_layout()
{
  Bakery::BusyCursor(*get_app_window());

  //Remove existing child widgets:
  m_FlowTable.remove_all();

  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  if(document)
  {
    m_FlowTable.set_table(m_strTableName); //This allows portals to get full Relationship information

    //This map of layout groups will also contain the field information from the database:
    Document_Glom::type_mapLayoutGroupSequence layout_groups = get_data_layout_groups("details");
    for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = layout_groups.begin(); iter != layout_groups.end(); ++iter)
    {
      m_FlowTable.add_layout_group(iter->second);
    }
  }

}

void Box_Data_Details::fill_from_database()
{
  Bakery::BusyCursor(*get_app_window());

  try
  {
    //TODO: This should keep the connection open, so we don't need to 
    //reconnect many times..
    sharedptr<SharedConnection> sharedconnection = connect_to_server();


    Box_DB_Table::fill_from_database();

    m_Fields = get_fields_to_show();
    type_vecLayoutFields fieldsToGet = m_Fields;

    if(!fieldsToGet.empty())
    {
      fill_from_database_layout(); //TODO: Only do this when the layout has changed.

      //Add extra possibly-non-visible columns that we need:
      LayoutItem_Field layout_item;
      layout_item.set_name(m_field_primary_key.get_name());
      layout_item.m_field = m_field_primary_key;
      fieldsToGet.push_back(layout_item);

      //g_warning("primary_key name = %s", m_field_primary_key.get_name().c_str());
      const int index_primary_key = fieldsToGet.size() - 1;

      const Glib::ustring query = build_sql_select(m_strTableName, fieldsToGet, m_field_primary_key, m_primary_key_value);
      Glib::RefPtr<Gnome::Gda::DataModel> result = Query_execute(query);

      if(result && result->get_n_rows())
      {
        const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
        if(pDoc)
        {
          //Get glom-specific field info:
          //Document_Glom::type_vecFields vecFields = pDoc->get_table_fields(m_strTableName);

          const int row_number = 0; //The only row.
          const int cols_count = result->get_n_columns();

          //Get special possibly-non-visible field values:
          if(index_primary_key < cols_count)
            m_primary_key_value = result->get_value_at(index_primary_key, row_number);

          //Get field values to show:
          for(int i = 0; i < cols_count; ++i)
          {
            const LayoutItem_Field& layout_item = fieldsToGet[i];

            //Field value:
            Gnome::Gda::Value value = result->get_value_at(i, row_number);
            m_FlowTable.set_field_value(layout_item, value);
          }
        }
      }
    } //if(!fieldsToGet.empty())

    //fill_related();

    fill_end();

  }
  catch(std::exception& ex)
  {
    handle_error(ex);
  }
}

/*
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
             pBox->init_db_details(relationship, value, m_primary_key_value);
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
*/

void Box_Data_Details::on_button_new()
{
  if(confirm_discard_unstored_data())
  {
    Gnome::Gda::FieldAttributes fieldinfo = m_field_primary_key.get_field_info();
    if(fieldinfo.get_auto_increment()) //If the primary key is an auto-increment:
    {
      //Just make a new record, and show it:
      Gnome::Gda::Value primary_key_value = generate_next_auto_increment(m_strTableName, m_field_primary_key.get_name()); //TODO: This should return a Gda::Value

      record_new(false /* use entered field data */, primary_key_value);
      refresh_db_details(primary_key_value);
    }
    else
    {
      //It's not an auto-increment primary key,
      //so just blank the fields ready for a primary key later.
      refresh_db_details_blank(); //shows blank record.
    }

  } //if(confirm_discard_unstored_data())
}

void Box_Data_Details::on_button_del()
{
  if( GlomConversions::value_is_empty(get_primary_key_value()) )
  {
    //Tell user that a primary key is needed to delete a record:
    Gtk::MessageDialog dialog(gettext("<b>No primary key value.</b>"), true);
    dialog.set_secondary_text(gettext("This record can not be deleted because there is no primary key."));
    dialog.set_transient_for(*get_app_window());
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

Gnome::Gda::Value Box_Data_Details::get_entered_field_data(const LayoutItem_Field& field) const
{
  return m_FlowTable.get_field_value(field);
}

void Box_Data_Details::set_entered_field_data(const LayoutItem_Field& field, const Gnome::Gda::Value& value)
{
  m_FlowTable.set_field_value(field, value);
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

/*
void Box_Data_Details::on_related_user_requested_details(Gnome::Gda::Value key_value, Glib::ustring strTableName)
{
  signal_user_requested_related_details().emit(strTableName, key_value);
}
*/

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

/*
Box_Data_Details::type_signal_user_requested_related_details Box_Data_Details::signal_user_requested_related_details()
{
  return m_signal_user_requested_related_details;
}
*/

void Box_Data_Details::on_flowtable_layout_changed()
{
  //Get new layout:
  Document_Glom::type_mapLayoutGroupSequence layout_groups;
  m_FlowTable.get_layout_groups(layout_groups);

  Document_Glom* document = get_document();
  if(document)
    document->set_data_layout_groups("details", m_strTableName, layout_groups);

  //Build the view again from the new layout:
  fill_from_database_layout();

  //And fill it with data:
  fill_from_database();
}

void Box_Data_Details::on_flowtable_field_edited(const LayoutItem_Field& layout_field, const Gnome::Gda::Value& field_value)
{
  if(m_ignore_signals)
    return;

  const Glib::ustring strFieldName = layout_field.get_name();

  Gnome::Gda::Value primary_key_value = get_primary_key_value();
  if(!GlomConversions::value_is_empty(primary_key_value)) //If there is a primary key value:
  {
    Glib::ustring table_name;
    Field primary_key_field;
    Gnome::Gda::Value primary_key_value;

    if(!layout_field.get_has_relationship_name())
    {
      table_name = get_table_name();
      primary_key_field = m_field_primary_key;
      primary_key_value = get_primary_key_value();
    }
    else
    {
      //If it's a related field then discover the actual table that it's in,
      //plus how to identify the record in that table.
      const Glib::ustring relationship_name = layout_field.get_relationship_name();

      Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());

      Relationship relationship;
      bool test = document->get_relationship(get_table_name(), relationship_name, relationship);
      if(test)
      {
        table_name = relationship.get_to_table();
        const Glib::ustring to_field_name = relationship.get_to_field();
        //Get the key field in the other table (the table that we will change)
        bool test = get_fields_for_table_one_field(table_name, to_field_name, primary_key_field); //TODO_Performance.
        if(test)
        {
          //Get the value of the corresponding key in the current table (that identifies the record in the table that we will change)
          LayoutItem_Field layout_item;
          layout_item.set_name(relationship.get_from_field());

          primary_key_value = get_entered_field_data(layout_item);

          //See whether the related record exists:
          const bool related_record_exists = get_related_record_exists(relationship, primary_key_field, primary_key_value);
          if(related_record_exists)
          {
            //No problem, the SQL command below will update this value in the related table.
          }
          else
          {
            //To store the entered data in the related field, we would first have to create a related record.
            if(!relationship.get_auto_create())
            {
              //Warn the user:
              //TODO: Make the field insensitive until it can receive data, so people never see this dialog.
              Gtk::MessageDialog dialog(gettext("<b>Related Record Does Not Exist</b>"), true);
              dialog.set_secondary_text(gettext("Data may not be entered into this related field, because the related record does not yet exist, and the relationship does not allow automatic creation of new related records."));
              dialog.run();

              //Clear the field again, discarding the entered data.
              set_entered_field_data(layout_field, Gnome::Gda::Value());

              return;
            }
            else
            {
              const bool key_is_auto_increment = primary_key_field.get_field_info().get_auto_increment();

              //If we would have to set an otherwise auto-increment key to add the record.
              if( key_is_auto_increment && !GlomConversions::value_is_empty(primary_key_value) )
              {
                //Warn the user:
                //TODO: Make the field insensitive until it can receive data, so people never see this dialog.
                Gtk::MessageDialog dialog(gettext("<b>Related Record Can Not Be Created</b>"), true);
                //TODO: This is a very complex error message:
                dialog.set_secondary_text(gettext("Data may not be entered into this related field, because the related record does not yet exist, and the key in the related record is auto-generated and therefore can not be created with the key value in this record."));
                dialog.run();

                //Clear the field again, discarding the entered data.
                set_entered_field_data(layout_field, Gnome::Gda::Value());

                return;
              }
              else
              {
                //TODO: Calculate values, and do lookups?

                //Create the related record:
                if(key_is_auto_increment)
                {
                  primary_key_value = generate_next_auto_increment(relationship.get_to_table(), primary_key_field.get_name());

                  //Generate the new key value;
                }

                const Glib::ustring strQuery = "INSERT INTO " + relationship.get_to_table() + " (" + primary_key_field.get_name() + ") VALUES (" + primary_key_field.sql(primary_key_value) + ")";
                bool test = Query_execute(strQuery);
                if(test)
                {
                  if(key_is_auto_increment)
                  {
                    //Set the key in the parent table
                    LayoutItem_Field item_from_key;
                    item_from_key.set_name(relationship.get_from_field());

                    //Show the new from key in the parent table's layout:
                    set_entered_field_data(item_from_key, primary_key_value);

                    //Set it in the database too:
                    Field field_from_key;
                    bool test = get_fields_for_table_one_field(relationship.get_from_table(), relationship.get_from_field(), field_from_key); //TODO_Performance.
                    if(test)
                    {
                      const Glib::ustring strQuery = "INSERT INTO " + relationship.get_from_table() + " (" + relationship.get_from_field() + ") VALUES (" + primary_key_field.sql(primary_key_value) + ")";
                      /* bool test = */ Query_execute(strQuery);
                    }
                  }

                  //Now that the related record exists, the following code to set the value of the other field in the related field can succeed.

                }
              }
            }

          }

        }
        else
        {
          g_warning("Box_Data_Details::on_flowtable_field_edited(): key not found for edited related field.");
        }
      }
    }

    const Field& field = layout_field.m_field;

    //Update the field in the record (the record with this primary key):
    try
    {
      Glib::ustring strQuery = "UPDATE " + table_name;
      strQuery += " SET " +  /* table_name + "." + postgres does not seem to like the table name here */ strFieldName + " = " + field.sql(field_value);
      strQuery += " WHERE " + table_name + "." + primary_key_field.get_name() + " = " + primary_key_field.sql(primary_key_value);
      bool bTest = Query_execute(strQuery);

      if(!bTest)
      {
        //Update failed.
        fill_from_database(); //Replace with correct values.
      }
      else
      {
        //Set the value in all instances of this field in the layout (The field might be on the layout more than once):
        m_FlowTable.set_field_value(layout_field, field_value);

        //Get-and-set values for lookup fields, if this field triggers those relationships:
        do_lookups(layout_field, field_value, primary_key_field, primary_key_value);

        //Show new values for related fields:
        refresh_related_fields(layout_field, field_value, primary_key_field, primary_key_value);

        //TODO: Display new values for related fields.

        //If this is a foreign key then refresh the related records:
        /*
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
        */
      }

    }
    catch(const std::exception& ex)
    {
      handle_error(ex);
    }
  }
  else
  {
    //There is no current primary key value:

    if(m_field_primary_key.get_field_info().get_auto_increment()) //If the primary key is an auto-increment:
    {
      if(strFieldName == m_field_primary_key.get_name()) //If edited field is the primary key.
      {
        //Warn user that they can't choose their own primary key:
        Gtk::MessageDialog dialog(gettext("<b>Primary key auto increments</b>"), true);
        dialog.set_secondary_text(gettext("The primary key is auto-incremented.\n You may not enter your own primary key value."));
        dialog.set_transient_for(*get_app_window());
        dialog.run();
      }
    }
    else
    {
      //It is not auto-generated:

      if(strFieldName == m_field_primary_key.get_name()) //if it is the primary key that' being edited.
      {
        //Create new record with this primary key,
        //and all the other field values too.
        //see comments after 'else':
        record_new(true /* use entered field data */);
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
}

void Box_Data_Details::refresh_related_fields(const LayoutItem_Field& field_changed, const Gnome::Gda::Value& /* field_value */, const Field& primary_key, const Gnome::Gda::Value& primary_key_value)
{
  if(field_changed.get_has_relationship_name())
    return; //TODO: Handle these too.

  //Get values for lookup fields, if this field triggers those relationships:
  //TODO_performance: There is a LOT of iterating and copying here.
  const Glib::ustring strFieldName = field_changed.get_name();
  type_vecLayoutFields fieldsToGet = get_related_fields(strFieldName);

  if(!fieldsToGet.empty())
  {
    const Glib::ustring query = build_sql_select(m_strTableName, fieldsToGet, primary_key, primary_key_value);

    Glib::RefPtr<Gnome::Gda::DataModel> result = Query_execute(query);
    if(!result)
      handle_error();
    else
    {
      //Field contents:
      if(result->get_n_rows())
      {
        type_vecLayoutFields::const_iterator iterFields = fieldsToGet.begin();

        guint cols_count = result->get_n_columns();
        for(guint uiCol = 0; uiCol < cols_count; uiCol++)
        {
          const Gnome::Gda::Value value = result->get_value_at(uiCol, 0 /* row */);
          const LayoutItem_Field& layout_item = *iterFields;

          //g_warning("list fill: field_name=%s", iterFields->get_name().c_str());
          //g_warning("  value_as_string=%s", value.to_string().c_str());

          m_FlowTable.set_field_value(layout_item, value);

          ++iterFields;
        }
      }
    }
  }
}

void Box_Data_Details::do_lookups(const LayoutItem_Field& field_changed, const Gnome::Gda::Value& field_value, const Field& primary_key, const Gnome::Gda::Value& primary_key_value)
{
   if(field_changed.get_has_relationship_name())
    return; //TODO: Handle these too.

   //Get values for lookup fields, if this field triggers those relationships:
   //TODO_performance: There is a LOT of iterating and copying here.
   const Glib::ustring strFieldName = field_changed.get_name();
   type_list_lookups lookups = get_lookup_fields(strFieldName);
   for(type_list_lookups::const_iterator iter = lookups.begin(); iter != lookups.end(); ++iter)
   {
     const LayoutItem_Field& layout_Item = iter->first;

     const Relationship relationship = iter->second;
     const Field& field_lookup = layout_Item.m_field;
     const Glib::ustring field_lookup_name = field_lookup.get_name();

     Field field_source;
     bool test = get_fields_for_table_one_field(relationship.get_to_table(), field_lookup.get_lookup_field(), field_source);
     if(test)
     {
       Gnome::Gda::Value value = get_lookup_value(iter->second /* relationship */,  field_source /* the field to look in to get the value */, field_value /* Value of to and from fields */);

       //Add it to the view:
       LayoutItem_Field layout_item;
       layout_item.set_name(field_lookup_name);
       //TODO? layout_item.set_relationship_name();
       m_FlowTable.set_field_value(layout_item, value);

       //Add it to the database (even if it is not shown in the view)
       Glib::ustring strQuery = "UPDATE " + m_strTableName;
       strQuery += " SET " + field_lookup_name + " = " + field_lookup.sql(value);
       strQuery += " WHERE " + primary_key.get_name() + " = " + primary_key.sql(primary_key_value);
       Query_execute(strQuery);  //TODO: Handle errors

       //TODO: Handle lookups triggered by these fields (recursively)? TODO: Check for infinitely looping lookups.
     }
   }

}

void Box_Data_Details::on_userlevel_changed(AppState::userlevels user_level)
{
  m_FlowTable.set_design_mode( user_level == AppState::USERLEVEL_DEVELOPER );
}

bool Box_Data_Details::get_field_primary_key(Field& field) const
{
  field = m_field_primary_key;
  return false;
}




