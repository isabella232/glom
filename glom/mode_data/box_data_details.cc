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
#include "../frame_glom.h" //For show_ok_dialog().
#include <glom/libglom/data_structure/field.h>
#include <glom/libglom/data_structure/relationship.h>
#include <glom/libglom/data_structure/glomconversions.h>
#include "dialog_layout_details.h"
#include <glom/libglom/utils.h>
#include <glom/glom_privs.h>
#include "../xsl_utils.h"
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include "../python_embed/glom_python.h"
#include <sstream> //For stringstream
#include <glibmm/i18n.h>

namespace Glom
{

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

  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_data_layout"); //TODO: Use a generic layout dialog?
  if(refXml)
  {
    Dialog_Layout_Details* dialog = 0;
    refXml->get_widget_derived("window_data_layout", dialog);
    if(dialog)
    {
      m_pDialogLayout = dialog;
      add_view(m_pDialogLayout); //Give it access to the document.
      m_pDialogLayout->signal_hide().connect( sigc::mem_fun(static_cast<Box_Data&>(*this), &Box_Data::on_dialog_layout_hide) );
    }
  }

  m_FlowTable.set_columns_count(1); //Sub-groups will have multiple columns (by default, there is one sub-group, with 2 columns).
  m_FlowTable.set_padding(6);

  //m_strHint = _("When you change the data in a field the database is updated immediately.\n Click [New] to add a new record.\n Leave automatic ID fields empty - they will be filled for you.");


  //m_Paned.set_border_width(6);
  m_Paned.set_position(200); //Set size of top pane.
  pack_start(m_Paned);
  m_Paned.add(m_FlowTable);

  //Related records:
  /*
  m_Label_Related.set_text(Bakery::App_Gtk::util_bold_message(_("Related Records")));
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
  m_FlowTable.signal_field_open_details_requested().connect( sigc::mem_fun(*this,  &Box_Data_Details::on_flowtable_field_open_details_requested) );
  show_all();

  m_FlowTable.signal_related_record_changed().connect( sigc::mem_fun(*this, &Box_Data_Details::on_flowtable_related_record_changed) );


  m_FlowTable.signal_layout_changed().connect( sigc::mem_fun(*this, &Box_Data_Details::on_flowtable_layout_changed) );

  m_FlowTable.signal_requested_related_details().connect( sigc::mem_fun(*this, &Box_Data_Details::on_flowtable_requested_related_details) );

  m_FlowTable.signal_script_button_clicked().connect( sigc::mem_fun(*this, &Box_Data_Details::on_flowtable_script_button_clicked) );

  m_ignore_signals = false;
}

Box_Data_Details::~Box_Data_Details()
{
  remove_view(&m_FlowTable);
}

Gnome::Gda::Value Box_Data_Details::get_primary_key_value() const
{
  return m_primary_key_value;
}

void Box_Data_Details::set_primary_key_value(const Gtk::TreeModel::iterator& /* row */, const Gnome::Gda::Value& value)
{
  m_primary_key_value = value;
}

bool Box_Data_Details::init_db_details(const FoundSet& found_set, const Gnome::Gda::Value& primary_key_value)
{
  //std::cout << "Box_Data_Details::init_db_details(): primary_key_value.to_string()=" << primary_key_value.to_string() << std::endl;

  m_primary_key_value = primary_key_value;

  m_field_primary_key = get_field_primary_key_for_table(found_set.m_table_name);

  return Box_Data::init_db_details(found_set); //Calls create_layout(), then fill_from_database()
}

bool Box_Data_Details::refresh_data_from_database_with_primary_key(const Gnome::Gda::Value& primary_key_value)
{
  //std::cout << "refresh_data_from_database_with_primary_key(): primary_key_value.to_string()=" << primary_key_value.to_string() << std::endl;
  m_primary_key_value = primary_key_value;
  return fill_from_database();
}

bool Box_Data_Details::refresh_data_from_database_blank()
{
  return refresh_data_from_database_with_primary_key( Gnome::Gda::Value() );
}

void Box_Data_Details::create_layout()
{
  Bakery::BusyCursor busy_cursor(get_app_window());

  Box_Data::create_layout(); //Fills m_TableFields.

  //Remove existing child widgets:
  m_FlowTable.remove_all();

  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  if(document)
  {
    m_FlowTable.set_table(m_table_name); //This allows portals to get full Relationship information

    //This map of layout groups will also contain the field information from the database:
    Document_Glom::type_mapLayoutGroupSequence layout_groups = get_data_layout_groups(m_layout_name);

    for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = layout_groups.begin(); iter != layout_groups.end(); ++iter)
    {
      m_FlowTable.add_layout_group(iter->second);
    }
  }

}

bool Box_Data_Details::fill_from_database()
{
  //std::cout << "Box_Data_Details::fill_from_database(): m_primary_key_value.to_string()=" << m_primary_key_value.to_string() << std::endl;

  bool bResult = false;

  Bakery::BusyCursor busy_cursor(get_app_window());

  const bool primary_key_is_empty = Conversions::value_is_empty(m_primary_key_value);
  if(!primary_key_is_empty)
    get_document()->set_layout_record_viewed(m_table_name, m_layout_name, m_primary_key_value);

  if(!m_field_primary_key)
  {
    //refresh_data_from_database_blank(); //shows blank record
    return false;
  }

  try
  {

    //TODO: This should keep the connection open, so we don't need to 
    //reconnect many times..
    sharedptr<SharedConnection> sharedconnection = connect_to_server(get_app_window());


    bResult = Box_Data::fill_from_database();

    m_FieldsShown = get_fields_to_show();
    type_vecLayoutFields fieldsToGet = m_FieldsShown;

    if(!fieldsToGet.empty())
    {
      //Do not try to show the data if the user may not view it:
      Privileges table_privs = Privs::get_current_privs(m_table_name);

      //Enable/Disable record creation and deletion:
      m_Button_New.set_sensitive(table_privs.m_create);
      m_Button_Del.set_sensitive(table_privs.m_delete);

      if(table_privs.m_view)
      {
        //Add extra possibly-non-visible columns that we need:
        sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
        layout_item->set_full_field_details(m_field_primary_key);
        fieldsToGet.push_back(layout_item);

        //g_warning("primary_key name = %s", m_field_primary_key->get_name().c_str());
        const int index_primary_key = fieldsToGet.size() - 1;

        const Glib::ustring query = Utils::build_sql_select_with_key(m_table_name, fieldsToGet, m_field_primary_key, m_primary_key_value);
        Glib::RefPtr<Gnome::Gda::DataModel> result;

        if(!primary_key_is_empty)
          result = query_execute(query, get_app_window());

        if((result && result->get_n_rows()) || primary_key_is_empty) //either a working result or no result needed.
        {
          const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
          if(pDoc)
          {
            //Get glom-specific field info:
            //Document_Glom::type_vecFields vecFields = pDoc->get_table_fields(m_table_name);

            const int row_number = 0; //The only row.
            int cols_count = 0;
            if(!primary_key_is_empty)
              cols_count = result->get_n_columns();
            else
              cols_count = fieldsToGet.size();

            //Get special possibly-non-visible field values:
            if(!primary_key_is_empty)
            {
              if(index_primary_key < cols_count)
                m_primary_key_value = result->get_value_at(index_primary_key, row_number);
            }

            //Get field values to show:
            for(int i = 0; i < cols_count; ++i)
            {
              sharedptr<LayoutItem_Field> layout_item = fieldsToGet[i];

              //Field value:
              Gnome::Gda::Value value;

              if(!primary_key_is_empty)
                value = result->get_value_at(i, row_number);
              else
              {
                value = Conversions::get_empty_value(layout_item->get_glom_type());
              }

              m_FlowTable.set_field_value(layout_item, value);
            }
          }
        }
        else
        {
          bResult = false; //There were no records.
        }
      }
    } //if(!fieldsToGet.empty())

    //fill_related();

    set_unstored_data(false);

    fill_end();

  }
  catch(const std::exception& ex)
  {
    handle_error(ex);
    bResult = false;
  }

  return bResult;
}

void Box_Data_Details::on_button_new()
{
  if(confirm_discard_unstored_data())
  {
    if(m_field_primary_key && m_field_primary_key->get_auto_increment()) //If the primary key is an auto-increment:
    {
      //Just make a new record, and show it:
      Gnome::Gda::Value primary_key_value = generate_next_auto_increment(m_table_name, m_field_primary_key->get_name()); //TODO: This should return a Gda::Value

      record_new(false /* use entered field data */, primary_key_value);
      refresh_data_from_database_with_primary_key(primary_key_value);
    }
    else
    {
      //It's not an auto-increment primary key,
      //so just blank the fields ready for a primary key later.
      refresh_data_from_database_blank(); //shows blank record.
    }

  } //if(confirm_discard_unstored_data())
}

void Box_Data_Details::on_button_del()
{
  if( Conversions::value_is_empty(get_primary_key_value()) )
  {
    //Tell user that a primary key is needed to delete a record:
    Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("No primary key value.")), true);
    dialog.set_secondary_text(_("This record cannot be deleted because there is no primary key."));
    dialog.set_transient_for(*get_app_window());
    dialog.run();
  }
  else
  {
    if(confirm_delete_record())
    {
      const bool bTest = record_delete(m_primary_key_value);
  
      if(bTest)
      {
        //Tell the list that it has been deleted:
        //It will go to the next (or last) record,
        signal_record_deleted().emit(m_primary_key_value);
      }
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

Gnome::Gda::Value Box_Data_Details::get_entered_field_data(const sharedptr<const LayoutItem_Field>& field) const
{
  return m_FlowTable.get_field_value(field);
}

void Box_Data_Details::set_entered_field_data(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  m_FlowTable.set_field_value(field, value);
}

void Box_Data_Details::set_entered_field_data(const Gtk::TreeModel::iterator& /* row */, const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  set_entered_field_data(field, value);
}

Gnome::Gda::Value Box_Data_Details::get_primary_key_value_selected()
{
  return m_primary_key_value;
}

void Box_Data_Details::recalculate_fields_for_related_records(const Glib::ustring& relationship_name)
{
  m_FieldsCalculationInProgress.clear();

  //Check all fields in the parent table:
  const Gnome::Gda::Value primary_key_value = get_primary_key_value();
  for(type_vecFields::iterator iter = m_TableFields.begin(); iter != m_TableFields.end(); ++iter)
  {
    const sharedptr<const Field> field = *iter;

    //Is this field triggered by this relationship?
    const Field::type_list_strings triggered_by = field->get_calculation_relationships();
    Field::type_list_strings::const_iterator iterFind = std::find(triggered_by.begin(), triggered_by.end(), relationship_name);
    if(iterFind != triggered_by.end()) //If it was found
    {
      sharedptr<Field> field = *iter;
      if(field)
      {
        sharedptr<LayoutItem_Field> layoutitem_field = sharedptr<LayoutItem_Field>::create();
        layoutitem_field->set_full_field_details(field);
        LayoutFieldInRecord field_in_record(layoutitem_field, m_table_name, m_field_primary_key, primary_key_value);
        calculate_field(field_in_record); //And any dependencies.

        //Calculate anything that depends on this.
        //sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
        //layout_item->set_full_field_details(field);

        do_calculations(field_in_record, false /* recurse, reusing m_FieldsCalculationInProgress */);
      }
    }
  }

   m_FieldsCalculationInProgress.clear();
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

Box_Data_Details::type_signal_requested_related_details Box_Data_Details::signal_requested_related_details()
{
  return m_signal_requested_related_details;
}

void Box_Data_Details::on_flowtable_layout_changed()
{
  //Get new layout:
  //Document_Glom::type_mapLayoutGroupSequence layout_groups;
  //m_FlowTable.get_layout_groups(layout_groups);

  //Store it in the document:
  Document_Glom* document = get_document();
  document->set_modified();
  //if(document)
  //  document->set_data_layout_groups(m_layout_name, m_table_name, layout_groups);

  //Build the view again from the new layout:
  create_layout();

  //And fill it with data:
  fill_from_database();
}

void Box_Data_Details::on_flowtable_requested_related_details(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value)
{
  if(Conversions::value_is_empty(primary_key_value))
    return; //Ignore empty ID fields.

  signal_requested_related_details().emit(table_name, primary_key_value);
}

void Box_Data_Details::on_flowtable_related_record_changed(const Glib::ustring& relationship_name)
{
  recalculate_fields_for_related_records(relationship_name);
}

void Box_Data_Details::on_flowtable_field_open_details_requested(const sharedptr<const LayoutItem_Field>& layout_field, const Gnome::Gda::Value& field_value)
{
  if(Conversions::value_is_empty(field_value))
    return; //Ignore empty ID fields.

  sharedptr<Relationship> relationship = get_document()->get_field_used_in_relationship_to_one(m_table_name, layout_field->get_name());
  if(relationship)
  {
    signal_requested_related_details().emit(relationship->get_to_table(), field_value);
  }
}

void Box_Data_Details::on_flowtable_script_button_clicked(const sharedptr<const LayoutItem_Button>& layout_item)
{
  if(layout_item)
  {
    const Gnome::Gda::Value primary_key_value = get_primary_key_value();
    const type_map_fields field_values = get_record_field_values_for_calculation(m_table_name, m_field_primary_key, primary_key_value);

   //We need the connection when we run the script, so that the script may use it.
   sharedptr<SharedConnection> sharedconnection = connect_to_server(0 /* parent window */);

    glom_execute_python_function_implementation(layout_item->get_script(), field_values, //TODO: Maybe use the field's type here.
    get_document(), get_table_name(), sharedconnection->get_gda_connection());

    //Refresh the view, in case the script changed any data:
    if(get_primary_key_is_in_foundset(m_found_set, m_primary_key_value)) //Check, because maybe the script deleted the current record, or changed something so that it should no longer be shown in the found set.
    {
      refresh_data_from_database_with_primary_key(m_primary_key_value);
    }
    else
    {
      //Tell the parent to do something appropriate, such as show another record:
      signal_record_deleted().emit(m_primary_key_value);
    }
  }
}

void Box_Data_Details::on_flowtable_field_edited(const sharedptr<const LayoutItem_Field>& layout_field, const Gnome::Gda::Value& field_value)
{
  if(m_ignore_signals)
    return;

  const Glib::ustring strFieldName = layout_field->get_name();

  Gtk::Window* window = get_app_window();

  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());

  Gnome::Gda::Value primary_key_value = get_primary_key_value();
  if(!Conversions::value_is_empty(primary_key_value)) //If there is not a primary key value:
  {
    Glib::ustring table_name;
    sharedptr<Field> primary_key_field;
    Gnome::Gda::Value primary_key_value;

    if(!layout_field->get_has_relationship_name())
    {
      table_name = get_table_name();
      primary_key_field = m_field_primary_key;
      primary_key_value = get_primary_key_value();
    }
    else
    {
      //If it's a related field then discover the actual table that it's in,
      //plus how to identify the record in that table.
      const Glib::ustring relationship_name = layout_field->get_relationship_name();

      sharedptr<Relationship> relationship = document->get_relationship(get_table_name(), relationship_name);
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
          layout_item->set_full_field_details( document->get_field(relationship->get_from_table(), relationship->get_from_field()) );

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
          g_warning("Box_Data_Details::on_flowtable_field_edited(): key not found for edited related field.");
        }
      }
    }


    LayoutFieldInRecord field_in_record(layout_field, m_table_name /* parent table */, primary_key_field, primary_key_value);

    //Check whether the value meets uniqueness constraints:
    if(!check_entered_value_for_uniqueness(m_table_name, layout_field, field_value, window))
    {
      //Revert to the value in the database:
      const Gnome::Gda::Value value_old = get_field_value_in_database(field_in_record, window);
      set_entered_field_data(layout_field, value_old);
   
      return; 
    }

    //Set the value in all instances of this field in the layout (The field might be on the layout more than once):
    //if(layout_field->get_glom_type() != Field::TYPE_IMAGE) //TODO For now, don't do this for images, because the ImageGlom widget expects a broken GdaValue, because gda_value_get_binary() needs to be fixed.
    m_FlowTable.set_field_value(layout_field, field_value);

    //Update the field in the record (the record with this primary key):
    try
    {
      const bool bTest = set_field_value_in_database(field_in_record, field_value, false /* don't use current calculations */, get_app_window());

      if(!bTest)
      {
        //Update failed.
        fill_from_database(); //Replace with correct values.
      }
      else
      {
        //TODO: Display new values for related fields.

        //If this is a foreign key then refresh the related records:
        /*
        bool bIsForeignKey = false;
        Document_Glom::type_vecRelationships vecRelationships = get_document()->get_relationships(m_table_name);
        for(Document_Glom::type_vecRelationships::iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); iter++)
        {
          const Relationship& relationship = *iter;

          if(relationship->get_from_field() == strFieldName)
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

    if(m_field_primary_key && m_field_primary_key->get_auto_increment()) //If the primary key is an auto-increment:
    {
      if(strFieldName == m_field_primary_key->get_name()) //If edited field is the primary key.
      {
        //Warn user that they can't choose their own primary key:
        Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Primary key auto increments")), true);
        dialog.set_secondary_text(_("The primary key is auto-incremented.\n You may not enter your own primary key value."));
        dialog.set_transient_for(*get_app_window());
        dialog.run();
      }
      else
      {
        //Make a new record, and show it:
        Gnome::Gda::Value primary_key_value = generate_next_auto_increment(m_table_name, m_field_primary_key->get_name());

        record_new(true /* use entered field data */, primary_key_value);
        refresh_data_from_database_with_primary_key(primary_key_value);
      }
    }
    else
    {
      //It is not auto-generated:

      if(strFieldName == m_field_primary_key->get_name()) //if it is the primary key that is being edited.
      {
        if(!check_entered_value_for_uniqueness(m_table_name, layout_field, field_value, window))
        {
          //Revert to a blank value:
          const Gnome::Gda::Value value_old = Conversions::get_empty_value(layout_field->get_full_field_details()->get_glom_type());
          set_entered_field_data(layout_field, value_old);
        }
        else
        {
          //Create new record with this primary key,
          //and all the other field values too.
          //see comments after 'else':
          record_new(true /* use entered field data */);
        }
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

void Box_Data_Details::on_userlevel_changed(AppState::userlevels user_level)
{
  m_FlowTable.set_design_mode( user_level == AppState::USERLEVEL_DEVELOPER );
}

sharedptr<Field> Box_Data_Details::get_field_primary_key() const
{
  return m_field_primary_key;
}

void Box_Data_Details::print_layout_group(xmlpp::Element* node_parent, const sharedptr<const LayoutGroup>& group)
{
  xmlpp::Element* nodeChildGroup = node_parent->add_child("group");
  nodeChildGroup->set_attribute("title", group->get_title());

  LayoutGroup::type_map_const_items items = group->get_items();
  for(LayoutGroup::type_map_const_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
  {
    sharedptr<const LayoutItem> layout_item = iter->second;

    sharedptr<const LayoutGroup> pLayoutGroup = sharedptr<const LayoutGroup>::cast_dynamic(layout_item);
    if(pLayoutGroup)
      print_layout_group(nodeChildGroup, pLayoutGroup); //recurse
    else
    {
      sharedptr<const LayoutItem_Field> pLayoutField = sharedptr<const LayoutItem_Field>::cast_dynamic(layout_item);
      if(pLayoutField)
      {
        xmlpp::Element* nodeField = nodeChildGroup->add_child("field");

        nodeField->set_attribute("title", pLayoutField->get_title_or_name());

        Gnome::Gda::Value value = m_FlowTable.get_field_value(pLayoutField);
        const Glib::ustring text_representation = Conversions::get_text_for_gda_value(pLayoutField->get_glom_type(), value,
          pLayoutField->get_formatting_used().m_numeric_format); //In the current locale.

        nodeField->set_attribute("value", text_representation);
      }
      else
      {
        sharedptr<const LayoutItem_Portal> pLayoutPortal = sharedptr<const LayoutItem_Portal>::cast_dynamic(layout_item);
        if(pLayoutPortal)
        {
          xmlpp::Element* nodePortal = nodeChildGroup->add_child("related_records");

          //Box_Data_List_Related* pPortalWidget = m_FlowTable.get_portals();

          sharedptr<Relationship> relationship = get_document()->get_relationship(m_table_name, pLayoutPortal->get_relationship_name());
          if(relationship)
          {
            nodePortal->set_attribute("title", relationship->get_title_or_name());

            //TODO:

            //TODO: Only print this if the user has access rights.
          }
        }
      }
    }
  }

}

void Box_Data_Details::print_layout()
{
  const Privileges table_privs = Privs::get_current_privs(m_table_name);

  //Don't try to print tables that the user can't view.
  if(!table_privs.m_view)
  {
    //TODO: Warn the user.
  }
  else
  {
    //Create a DOM Document with the XML:
    xmlpp::DomParser dom_parser;;

    xmlpp::Document* pDocument = dom_parser.get_document();
    xmlpp::Element* nodeRoot = pDocument->get_root_node();
    if(!nodeRoot)
    {
      //Add it if it isn't there already:
      nodeRoot = pDocument->create_root_node("details_print");
    }

    Glib::ustring table_title = get_document()->get_table_title(m_table_name);
    if(table_title.empty())
      table_title = m_table_name;

    nodeRoot->set_attribute("table", table_title);


    //The groups:
    xmlpp::Element* nodeParent = nodeRoot;

    Document_Glom::type_mapLayoutGroupSequence layout_groups = get_data_layout_groups(m_layout_name);
    for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = layout_groups.begin(); iter != layout_groups.end(); ++iter)
    {
      sharedptr<const LayoutGroup> layout_group = iter->second;
      print_layout_group(nodeParent, layout_group);
    }

    GlomXslUtils::transform_and_open(*pDocument, "print_details_to_html.xsl", get_app_window());
  }
}

} //namespace Glom
