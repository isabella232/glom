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
#include <glom/libglom/data_structure/glomconversions.h>
#include <glom/reports/report_builder.h>
#include "dialog_layout_list.h"
#include <glom/glom_privs.h>
#include <bakery/App/App_Gtk.h> //For util_bold_message().
//#include <../utility_widgets/db_adddel/glom_db_treemodel.h> //For DbTreeModel.
#include <sstream> //For stringstream
#include <glibmm/i18n.h>

namespace Glom
{

Box_Data_List::Box_Data_List()
: m_has_one_or_more_records(false),
  m_read_only(false)
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
      add_view(m_pDialogLayout); //Give it access to the document.
      m_pDialogLayout->signal_hide().connect( sigc::mem_fun(*this, &Box_Data::on_dialog_layout_hide) );
    }
  }

  //m_strHint = _("When you change the data in a field the database is updated immediately.\n Click [Add] or enter data into the last row to add a new record.\n Leave automatic ID fields empty - they will be filled for you.\nOnly the first 100 records are shown.");

  pack_start(m_AddDel);
  add_view(&m_AddDel); //Give it access to the document.
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
  remove_view(&m_AddDel);
}

void Box_Data_List::enable_buttons()
{
  const Privileges table_privs = Privs::get_current_privs(m_table_name);

    //Enable/Disable record creation and deletion:
  bool allow_create = !m_read_only;
  bool allow_delete = !m_read_only;
  if(!m_read_only)
  {
    allow_create = table_privs.m_create;
    allow_delete = table_privs.m_delete;
  }

  m_AddDel.set_allow_add(allow_create);
  m_AddDel.set_allow_delete(allow_delete);

  m_AddDel.set_allow_view_details(table_privs.m_view);
}

void Box_Data_List::refresh_data_from_database_blank()
{
  FoundSet found_set = m_found_set;
  found_set.m_where_clause = Glib::ustring();
  m_AddDel.set_found_set(found_set);

  std::cout << "debug: Box_Data_List::refresh_data_from_database_blank(): before refresh_from_database_blank()." << std::endl;
  m_AddDel.refresh_from_database_blank();
  m_found_set = found_set;
}

bool Box_Data_List::fill_from_database()
{
  bool result = false;

  Bakery::BusyCursor busy_cursor(get_app_window());

  try
  {
    sharedptr<SharedConnection> sharedconnection = connect_to_server(get_app_window());

    Box_Data::fill_from_database();

    m_AddDel.remove_all();

    //Field Names:
    //create_layout();

    //if(sharedconnection)
    //{
      //Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

    //Do not try to show the data if the user may not view it:
    const Privileges table_privs = Privs::get_current_privs(m_table_name);

    enable_buttons();

    m_AddDel.set_found_set(m_found_set);

    result = m_AddDel.refresh_from_database();

    if(table_privs.m_view)
    {
      //TODO: Don't show it if m_view is false.

      //Select first record:
      Glib::RefPtr<Gtk::TreeModel> refModel = m_AddDel.get_model();
      if(refModel)
        m_AddDel.select_item(refModel->children().begin());

    } //privs

    fill_end();
  }
  catch(const std::exception& ex)
  {
    handle_error(ex);
    result = false;
  }

  return result;
}

void Box_Data_List::on_adddel_user_requested_add()
{
 if(m_FieldsShown.empty())
    return; //Don't try to add a record to a list with no fields.

  Gtk::TreeModel::iterator iter = m_AddDel.get_item_placeholder();
  if(iter)
  {
    sharedptr<LayoutItem_Field> fieldToEdit;

    //Start editing in the primary key or the first cell if the primary key is auto-incremented (because there is no point in editing an auto-generated value)
    guint index_primary_key = 0;
    const bool bPresent = get_field_primary_key_index(index_primary_key); //If there is no primary key then the default of 0 is OK.
    if(bPresent)
    {
      sharedptr<Field> fieldPrimaryKey = get_field_primary_key();
      if(fieldPrimaryKey && fieldPrimaryKey->get_auto_increment())
      {
        //Start editing in the first cell that is not auto_increment:
        for(type_vecLayoutFields::const_iterator iter = m_FieldsShown.begin(); iter != m_FieldsShown.end(); ++iter)
        {
          sharedptr<LayoutItem_Field> layout_item = *iter;
          if(!(layout_item->get_full_field_details()->get_auto_increment()))
          {
            fieldToEdit = layout_item;
            break;
          }
        }
      }
      else
      {
        //The primary key is not auto-increment, so start by editing it:
        fieldToEdit = sharedptr<LayoutItem_Field>::create();
        fieldToEdit->set_full_field_details(fieldPrimaryKey);
      }
    }

    //std::cout << "debug: index_field_to_edit=" << index_field_to_edit << std::endl;

    if(fieldToEdit)
    {
      m_AddDel.select_item(iter, fieldToEdit, true /* start_editing */);
    }
    else
    {
      std::cout << "on_adddel_user_requested_add(): no editable rows." << std::endl;
      //The only keys are non-editable, so just add a row:
      on_adddel_user_added(iter, 0);
      m_AddDel.select_item(iter); //without start_editing.
      //g_warning("Box_Data_List::on_adddel_user_requested_add(): index_field_to_edit does not exist: %d", index_field_to_edit);
    }
  }
}

void Box_Data_List::on_adddel_user_requested_edit(const Gtk::TreeModel::iterator& row)
{
  const Gnome::Gda::Value primary_key_value = m_AddDel.get_value_key(row); //The primary key is in the key.

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
    if(confirm_delete_record())
    {
      const Gnome::Gda::Value primary_key_value = get_primary_key_value(rowStart);
      record_delete(primary_key_value);

      //Remove the row:
      m_AddDel.remove_item(rowStart);

      on_record_deleted(primary_key_value);
    }
  }
}


void Box_Data_List::set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value)
{
  m_AddDel.set_value_key(row, value);
}

void Box_Data_List::on_adddel_user_added(const Gtk::TreeModel::iterator& row, guint /* col_with_first_value */)
{
  //std::cout << "Box_Data_List::on_adddel_user_added" << std::endl;

  Gnome::Gda::Value primary_key_value;

  sharedptr<Field> primary_key_field = m_AddDel.get_key_field();

  //Get the new primary key value, if one is available now:
  if(primary_key_field->get_auto_increment())
  {
    //Auto-increment is awkward (we can't get the last-generated ID) with postgres, so we auto-generate it ourselves;
    const Glib::ustring& strPrimaryKeyName = primary_key_field->get_name();
    primary_key_value = generate_next_auto_increment(m_table_name, strPrimaryKeyName);  //TODO: return a Gnome::Gda::Value of an appropriate type.
  }
  else
  {
    //Use the user-entered primary key value:

    //This only works when the primary key is already stored: primary_key_value = get_primary_key_value(row);
    primary_key_value = get_entered_field_data_field_only(primary_key_field);
  }

  //If no primary key value is available yet, then don't add the record yet:
  if(!Conversions::value_is_empty(primary_key_value))
  {
    sharedptr<SharedConnection> sharedconnection = connect_to_server(get_app_window()); //Keep it alive while we need the data_model.
    if(sharedconnection)
    {
      sharedptr<LayoutItem_Field> layout_field = sharedptr<LayoutItem_Field>::create();
      layout_field->set_full_field_details(primary_key_field);
      if(!check_entered_value_for_uniqueness(m_table_name, layout_field, primary_key_value, get_app_window()))
      {
        //Revert to a blank value.
        primary_key_value = Conversions::get_empty_value(layout_field->get_full_field_details()->get_glom_type());
        set_entered_field_data(row, layout_field, primary_key_value);
        return;
      }

      Glib::RefPtr<Gnome::Gda::DataModel> data_model = record_new(true /* use entered field data*/, primary_key_value, row);
      if(data_model)
      {
        
        //Save the primary key value for later use:
        //record_new() did this: m_AddDel.set_value_key(row, primary_key_value);

        //Show the primary key in the row, if the primary key is visible:

        //If it's an auto-increment, then get the value and show it:
        if(primary_key_field->get_auto_increment())
        {
          sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
          layout_item->set_full_field_details(primary_key_field);
          m_AddDel.set_value(row, layout_item, primary_key_value);
        }

        on_record_added(primary_key_value, row);
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
}

void Box_Data_List::on_adddel_user_reordered_columns()
{
  Document_Glom* pDoc = dynamic_cast<Document_Glom*>(get_document());
  if(pDoc)
  {
    sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::create();
    group->set_name("toplevel");

    AddDel::type_vecStrings vec_field_names = m_AddDel.get_columns_order();

    guint index = 0;
    for(AddDel::type_vecStrings::iterator iter = vec_field_names.begin(); iter != vec_field_names.end(); ++iter)
    {
      sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
      layout_item->set_name(*iter);
      layout_item->m_sequence = index;
      group->add_item(layout_item, index); 
      ++index;
    }

    Document_Glom::type_mapLayoutGroupSequence mapGroups;
    mapGroups[1] = group;

    pDoc->set_data_layout_groups("list", m_table_name, mapGroups);  
  }
}

void Box_Data_List::on_adddel_user_changed(const Gtk::TreeModel::iterator& row, guint col)
{
  const Gnome::Gda::Value parent_primary_key_value = get_primary_key_value(row);
  sharedptr<const LayoutItem_Field> layout_field = m_AddDel.get_column_field(col);

  if(!Conversions::value_is_empty(parent_primary_key_value)) //If the record's primary key is filled in:
  {
    //Just update the record:
    try
    {
      
      Glib::ustring table_name = m_table_name;
      sharedptr<Field> primary_key_field;
      Gnome::Gda::Value primary_key_value;

      if(!layout_field->get_has_relationship_name())
      {
        table_name = m_table_name;
        primary_key_field = m_AddDel.get_key_field();
        primary_key_value = parent_primary_key_value;
      }
      else
      {
        //If it's a related field then discover the actual table that it's in,
        //plus how to identify the record in that table.
        const Glib::ustring relationship_name = layout_field->get_relationship_name();

        Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());

        sharedptr<Relationship> relationship = document->get_relationship(m_table_name, relationship_name);
        if(relationship)
        {
          table_name = relationship->get_to_table();
          const Glib::ustring to_field_name = relationship->get_to_field();
          //Get the key field in the other table (the table that we will change)
          primary_key_field = get_fields_for_table_one_field(table_name, to_field_name); //TODO_Performance.
          if(primary_key_field)
          {
            //Get the value of the corresponding key in the current table (that identifies the record in the table that we will change)
            sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
            layout_item->set_name(relationship->get_from_field());

            primary_key_value = get_entered_field_data(layout_item);

            const bool test = add_related_record_for_field(layout_field, relationship, primary_key_field, primary_key_value);
            if(!test)
              return;

            //Get the new primary_key_value if it has been created:
            primary_key_value = get_entered_field_data(layout_item);

            //Now that the related record exists, the following code to set the value of the other field in the related field can succeed.
          }
          else
          {
            g_warning("Box_Data_List::on_flowtable_field_edited(): key not found for edited related field.");
          }
        }
      }

      //Update the field in the record (the record with this primary key):
      const Gnome::Gda::Value field_value = m_AddDel.get_value(row, layout_field);
      //std::cout << "Box_Data_List::on_adddel_user_changed(): field_value = " << field_value.to_string() << std::endl;
      //const sharedptr<const Field>& field = layout_field->m_field;
      //const Glib::ustring strFieldName = layout_field->get_name();

      LayoutFieldInRecord field_in_record(layout_field, m_table_name /* parent */, primary_key_field, primary_key_value);

      //Check whether the value meets uniqueness constraints:
      Gtk::Window* window = get_app_window();
      if(!check_entered_value_for_uniqueness(m_table_name, row, layout_field, field_value, window))
      {
        //Revert to the value in the database:
        const Gnome::Gda::Value value_old = get_field_value_in_database(field_in_record, window);
        set_entered_field_data(row, layout_field, value_old);

        return; //The value has been reverted to the value in the database.
      }


      const bool bTest = set_field_value_in_database(field_in_record, row, field_value, false /* don't use current calculations */, window);

      //Glib::ustring strQuery = "UPDATE \"" + table_name + "\"";
      //strQuery += " SET " +  /* table_name + "." + postgres does not seem to like the table name here */ strFieldName + " = " + field.sql(field_value);
      //strQuery += " WHERE " + table_name + "." + primary_key_field.get_name() + " = " + primary_key_field.sql(primary_key_value);
      //bool bTest = query_execute(strQuery);
      if(!bTest)
      {
        //Update failed.
        fill_from_database(); //Replace with correct values.
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
    //Actually, on_adddel_user_added() is usually just called directly in response to the user_added signal.
    on_adddel_user_added(row, col);
    
    const Gnome::Gda::Value primaryKeyValue = get_primary_key_value(row); //TODO_Value
    if(!(Conversions::value_is_empty(primaryKeyValue))) //If the Add succeeeded:
    {
      if(!(layout_field->get_full_field_details()->get_primary_key())) //Don't try to re-set the primary key field, because we just inserted the record with it.
      {
        on_adddel_user_changed(row, col); //Change this field in the new record.
      }
    }
    else
    {
      //A field value was entered, but the record has not been added yet, because not enough information exists yet.
       g_warning("Box_Data_List::on_adddel_user_changed(): debug: record not yet added.");
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
  else
    signal_user_requested_details().emit(Gnome::Gda::Value()); //Show a blank record if there are no records.
}

void Box_Data_List::on_details_record_deleted(const Gnome::Gda::Value& primary_key_value)
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
  return m_AddDel.get_value_key_selected();
}

Gnome::Gda::Value Box_Data_List::get_primary_key_value_first()
{
  //std::cout << "Box_Data_List(): get_primary_key_value_first() records_count = " << m_AddDel.get_count() << std::endl;

  Glib::RefPtr<Gtk::TreeModel> model = m_AddDel.get_model();
  if(model)
  {
    Gtk::TreeModel::iterator iter = model->children().begin();
    while(iter != model->children().end())
    {
      Gnome::Gda::Value value = get_primary_key_value(iter);
      if(Conversions::value_is_empty(value))
      {
       //std::cout << "Box_Data_List(): get_primary_key_value_first() iter val is NULL" << std::endl;
        ++iter;
      }
      else
      {
         //std::cout << "Box_Data_List(): get_primary_key_value_first() returning: " << value.to_string() << std::endl;
        return value;
      }
    }
  }

 // std::cout << "Box_Data_List(): get_primary_key_value_first() return NULL" << std::endl;
  return Gnome::Gda::Value();
}

Gnome::Gda::Value Box_Data_List::get_entered_field_data(const sharedptr<const LayoutItem_Field>& field) const
{
  return m_AddDel.get_value_selected(field);
}

void Box_Data_List::set_entered_field_data(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  return m_AddDel.set_value_selected(field, value);
}

void Box_Data_List::set_entered_field_data(const Gtk::TreeModel::iterator& row, const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  return m_AddDel.set_value(row, field, value);
}

bool Box_Data_List::get_showing_multiple_records() const
{
  return m_AddDel.get_count() > 1;
}

void Box_Data_List::create_layout_add_group(const sharedptr<LayoutGroup>& layout_group)
{
  if(!layout_group)
    return;

  LayoutGroup::type_map_items child_items = layout_group->get_items();
  for(LayoutGroup::type_map_items::const_iterator iter = child_items.begin(); iter != child_items.end(); ++iter)
  {
    sharedptr<LayoutItem> child_item = iter->second;

    sharedptr<LayoutGroup> child_group = sharedptr<LayoutGroup>::cast_dynamic(child_item);
    if(child_group)
    {
      //std::cout << "debug: Start Adding child group." << std::endl;
      create_layout_add_group(child_group);
      //std::cout << "debug: End Adding child group." << std::endl;
    }
    else
    {
      if(m_read_only)
        child_item->set_editable(false);

      //std::cout << "debug: adding column: " << child_item->get_name() << std::endl;
      m_AddDel.add_column(child_item);
    }
  }
}

Document_Glom::type_mapLayoutGroupSequence Box_Data_List::create_layout_get_layout()
{
  //Overriden in Box_Data_List_Related:
  return get_data_layout_groups(m_layout_name); 
}

void Box_Data_List::create_layout()
{
  Box_Data::create_layout(); //Fills m_TableFields.

  const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
  if(pDoc)
  {
    //Field Names:
    m_AddDel.remove_all_columns();
    //m_AddDel.set_columns_count(m_Fields.size());

    m_AddDel.set_table_name(m_table_name);


    sharedptr<Field> field_primary_key = get_field_primary_key_for_table(m_table_name);
    if(!field_primary_key)
    {
      //g_warning("Box_Data_List::create_layout(): primary key not found.");
    }
    else
    {
      m_AddDel.set_key_field(field_primary_key);
 
      //This map of layout groups will also contain the field information from the database:
      Document_Glom::type_mapLayoutGroupSequence layout_groups = create_layout_get_layout();

      //int debug_count = 0;
      for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = layout_groups.begin(); iter != layout_groups.end(); ++iter)
      {
        //std::cout << "Box_Data_List::create_layout() group number=" << debug_count;
        //debug_count++;
        //iter->second->debug();

        create_layout_add_group(iter->second);
      }
    }


    m_FieldsShown = get_fields_to_show();

    //Add extra possibly-non-visible columns that we need:
    //TODO: Only add it if it is not already there.
    sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
    layout_item->set_hidden();
    layout_item->set_full_field_details(m_AddDel.get_key_field());
    m_FieldsShown.push_back(layout_item);

    m_AddDel.add_column(layout_item);

    m_AddDel.set_found_set(m_found_set);

    m_AddDel.set_columns_ready();
  }

}

void Box_Data_List::on_record_deleted(const Gnome::Gda::Value& /* primary_key_value */)
{
   //Overridden by Box_Data_List_Related.
}

void Box_Data_List::on_record_added(const Gnome::Gda::Value& /* strPrimaryKey */, const Gtk::TreeModel::iterator& /* row */)
{
  //Overridden by Box_Data_List_Related.
  //m_AddDel.add_item(strPrimaryKey); //Add blank row.
}

Box_Data_List::type_signal_user_requested_details Box_Data_List::signal_user_requested_details()
{
  return m_signal_user_requested_details;
}

sharedptr<Field> Box_Data_List::get_field_primary_key() const
{
  return m_AddDel.get_key_field();
}

bool Box_Data_List::get_field_primary_key_index(guint& field_column) const
{
  return Box_Data::get_field_primary_key_index_for_fields(m_FieldsShown, field_column);
}

void Box_Data_List::print_layout()
{
  const Privileges table_privs = Privs::get_current_privs(m_table_name);

  //Don't try to print tables that the user can't view.
  if(!table_privs.m_view)
  {
    //TODO: Warn the user.
  }
  else
  {
    //Create a simple report on the fly:
    sharedptr<Report> report_temp(new Report());
    report_temp->set_name("list");
    report_temp->set_title(_("List"));

    //Add all the fields from the layout:
    for(type_vecLayoutFields::const_iterator iter = m_FieldsShown.begin(); iter != m_FieldsShown.end(); ++iter)
    {
      report_temp->m_layout_group->add_item(*iter);
    }

    ReportBuilder report_builder;
    report_builder.set_document(get_document());
    report_builder.report_build(m_found_set, report_temp, get_app_window());
  }
}

void Box_Data_List::print_layout_group(xmlpp::Element* /* node_parent */, const sharedptr<const LayoutGroup>& /* group */)
{
}

void Box_Data_List::set_read_only(bool read_only)
{
  //This is useful when showing find results for the user to select one, without changing them.
  m_read_only = read_only;
  m_AddDel.set_allow_add(!read_only);
  m_AddDel.set_allow_delete(!read_only);
}

void Box_Data_List::set_open_button_title(const Glib::ustring& title)
{
  m_AddDel.set_open_button_title(title);
}

void Box_Data_List::set_primary_key_value_selected(const Gnome::Gda::Value& primary_key_value)
{
  Gtk::TreeModel::iterator iter = m_AddDel.get_row(primary_key_value);
  if(iter)
  {
    m_AddDel.select_item(iter);
  }
}

void Box_Data_List::get_record_counts(gulong& total, gulong& found) const
{
  //Initialize output parameters:
  total = 0;
  found = 0;

  Glib::RefPtr<Gtk::TreeModel> refModel = m_AddDel.get_model();
  Glib::RefPtr<DbTreeModel> refModelDerived = Glib::RefPtr<DbTreeModel>::cast_dynamic(refModel);
  
  if(refModelDerived)
    refModelDerived->get_record_counts(total, found);
}

} //namespace Glom



