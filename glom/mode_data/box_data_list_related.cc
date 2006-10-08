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

#include "box_data_list_related.h"
#include "dialog_layout_list_related.h"
#include <glom/libglom/data_structure/glomconversions.h>
#include <glom/frame_glom.h> //For show_ok_dialog()
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>

namespace Glom
{

Box_Data_List_Related::Box_Data_List_Related()
: m_pDialogLayoutRelated(0)
{
  set_size_request(400, -1); //An arbitrary default.

  //m_Frame.set_label_widget(m_Label_Related);
  m_Frame.set_shadow_type(Gtk::SHADOW_NONE);

  m_Frame.add(m_Alignment);
  m_Frame.show();

  m_Frame.set_label_widget(m_Label);
  m_Label.show();

  m_Alignment.set_padding(6 /* top */, 0, 12 /* left */, 0);
  m_Alignment.show();

  remove(m_AddDel);
  m_Alignment.add(m_AddDel);
  m_AddDel.show();
  add(m_Frame);

  m_layout_name = "list_related"; //TODO: We need a unique name when 2 portals use the same table.

  //Delete the dialog from the base class, because we don't use it.
  if(m_pDialogLayout)
  {
    remove_view(m_pDialogLayout);
    delete m_pDialogLayout;
    m_pDialogLayout = 0;
  }

  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_data_layout_list_related");
  if(refXml)
  {
    Dialog_Layout_List_Related* dialog = 0;
    refXml->get_widget_derived("window_data_layout_list_related", dialog);
    if(dialog)
    {
      //Use the new dialog:
      m_pDialogLayoutRelated = dialog;
      add_view(m_pDialogLayoutRelated); //Give it access to the document.
      m_pDialogLayoutRelated->signal_hide().connect( sigc::mem_fun(*this, &Box_Data::on_dialog_layout_hide) );
    }
  }

}

Box_Data_List_Related::~Box_Data_List_Related()
{
  if(m_pDialogLayoutRelated)
  {
    remove_view(m_pDialogLayoutRelated);
    delete m_pDialogLayoutRelated;
    m_pDialogLayoutRelated = 0;
  }
}

void Box_Data_List_Related::enable_buttons()
{
  const bool view_details_possible = get_has_suitable_record_to_view_details();
  m_AddDel.set_allow_view_details(view_details_possible); //Don't allow the user to go to a record in a hidden table.
}

bool Box_Data_List_Related::init_db_details(const sharedptr<const LayoutItem_Portal>& portal, bool show_title)
{
  m_portal = glom_sharedptr_clone(portal);
  LayoutWidgetBase::m_table_name = m_portal->get_relationship()->get_to_table();
  Box_DB_Table::m_table_name = LayoutWidgetBase::m_table_name;

  sharedptr<const Relationship> relationship = m_portal->get_relationship();

  if(show_title)
  {
    m_Label.set_markup(Bakery::App_Gtk::util_bold_message( glom_get_sharedptr_title_or_name(relationship) ));
    m_Label.show();

    m_Alignment.set_padding(6 /* top */, 0, 12 /* left */, 0);
  }
  else
  {
    m_Label.set_markup(Glib::ustring());
    m_Label.hide();

    m_Alignment.set_padding(0, 0, 0, 0); //The box itself has padding of 6.
  }

  if(relationship)
    m_key_field = get_fields_for_table_one_field(relationship->get_to_table(), relationship->get_to_field());
  else
    m_key_field = sharedptr<Field>();

  enable_buttons();

  if(relationship)
  {
    FoundSet found_set;
    found_set.m_table_name = relationship->get_to_table();
    return Box_Data_List::init_db_details(found_set); //Calls create_layout() and fill_from_database().
  }
  else
    return false;
}

bool Box_Data_List_Related::refresh_data_from_database_with_foreign_key(const Gnome::Gda::Value& foreign_key_value)
{
  m_key_value = foreign_key_value;

  if(m_key_field)
  {
    if(!Conversions::value_is_empty(m_key_value))
    {
      FoundSet found_set = m_found_set;
      found_set.m_where_clause = "\"" + m_key_field->get_name() + "\" = " + m_key_field->sql(m_key_value);

      //g_warning("refresh_data_from_database(): where_clause=%s", where_clause.c_str());
      return Box_Data_List::refresh_data_from_database_with_where_clause(found_set);
    }
    else
    {
      //If there is no from key value then no records can be shown:
      refresh_data_from_database_blank();
      return true;
    }
  }
  else
  {
    //If there is no to field then this relationship specifies all records in the table.
    FoundSet found_set = m_found_set;
    found_set.m_where_clause = Glib::ustring();
    return Box_Data_List::refresh_data_from_database_with_where_clause(found_set);
  }
}

bool Box_Data_List_Related::fill_from_database()
{
  bool result = false;
  bool allow_add = true;

  if(m_key_field && m_found_set.m_where_clause.empty()) //There's a key field, but no value.
  {
    //No Foreign Key value, so just show the field names:

    result = Box_DB_Table::fill_from_database();

    //create_layout();

    fill_end();
  }
  else
  {
    result = Box_Data_List::fill_from_database();


    //Is there already one record here?
    if(m_has_one_or_more_records) //This was set by Box_Data_List::fill_from_database().
    {
      //Is the to_field unique? If so, there can not be more than one.
      if(m_key_field && m_key_field->get_unique_key()) //automatically true if it is a primary key
        allow_add = false;
    }

    //TODO: Disable add if the from_field already has a value and the to_field is auto-incrementing because
    //- we cannot override the auto-increment in the to_field.
    //- we cannot change the value in the from_field to the new auto_increment value in the to_field.
  }

  //Prevent addition of new records if that is what the relationship specifies:
  if(allow_add && m_portal->get_relationship())
    allow_add = m_portal->get_relationship()->get_auto_create();

  m_AddDel.set_allow_add(allow_add);

  return result;
}

void Box_Data_List_Related::on_record_added(const Gnome::Gda::Value& primary_key_value, const Gtk::TreeModel::iterator& row)
{
  //primary_key_value is a new autogenerated or human-entered key for the row.
  //It has already been added to the database.

  if(!row)
    return;

  Gnome::Gda::Value key_value;

  if(m_key_field)
  {
    //m_key_field is the field in this table that must match another field in the parent table.
    sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
    layout_item->set_full_field_details(m_key_field);
    key_value = m_AddDel.get_value(row, layout_item);
  }

  Box_Data_List::on_record_added(key_value, row); //adds blank row.

  //Make sure that the new related record is related,
  //by setting the foreign key:
  //If it's not auto-generated.
  if(!Conversions::value_is_empty(key_value)) //If there is already a value.
  {
    //It was auto-generated. Tell the parent about it, so it can make a link.
    signal_record_added.emit(key_value);
  }
  else if(Conversions::value_is_empty(m_key_value))
  {
    g_warning("Box_Data_List_Related::on_record_added(): m_key_value is NULL.");
  }
  else
  {
    sharedptr<Field> field_primary_key = m_AddDel.get_key_field();

    //Create the link by setting the foreign key
    if(m_key_field)
    {
      Glib::ustring strQuery = "UPDATE \"" + m_portal->get_relationship()->get_to_table() + "\"";
      strQuery += " SET \"" +  /* get_table_name() + "." +*/ m_key_field->get_name() + "\" = " + m_key_field->sql(m_key_value);
      strQuery += " WHERE \"" + get_table_name() + "\".\"" + field_primary_key->get_name() + "\" = " + field_primary_key->sql(primary_key_value);
      const bool test = query_execute(strQuery, get_app_window());
      if(test)
      {
        //Show it on the view, if it's visible:
        sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
        layout_item->set_full_field_details(field_primary_key);

        m_AddDel.set_value(row, layout_item, m_key_value);
      }
    }

    //on_adddel_user_changed(row, iKey); //Update the database.
  }
}

sharedptr<Relationship> Box_Data_List_Related::get_relationship() const
{
  return m_portal->get_relationship();
}

sharedptr<const Field> Box_Data_List_Related::get_key_field() const
{
  return m_key_field;
}

void Box_Data_List_Related::on_record_deleted(const Gnome::Gda::Value& /* primary_key_value */)
{
  //Allow the parent record (Details view) to recalculate aggregations:
  signal_record_changed().emit(m_portal->get_relationship_name());
}


void Box_Data_List_Related::on_adddel_user_changed(const Gtk::TreeModel::iterator& row, guint col)
{
  if(!(get_relationship()->get_allow_edit()))
  {
    std::cerr << "Box_Data_List_Related::on_adddel_user_added() called on non-editable portal. This should not happen." << std::endl;
   return;
  }

  //Call base class:
  Box_Data_List::on_adddel_user_changed(row, col);

  //Let parent respond:
  if(row)
    signal_record_changed().emit(m_portal->get_relationship_name());
}

void Box_Data_List_Related::on_adddel_user_added(const Gtk::TreeModel::iterator& row, guint col_with_first_value)
{
  sharedptr<const Relationship> relationship = get_relationship();
  if(!relationship->get_allow_edit())
  {
    std::cerr << "Box_Data_List_Related::on_adddel_user_added() called on non-editable portal. This should not happen." << std::endl;
   return;
  }

  //Like Box_Data_List::on_adddel_user_added(),
  //but it doesn't allow adding if the new record cannot be a related record.
  //This would happen if there is already one related record and the relationship uses the primary key in the related record.

  bool bAllowAdd = true;

  if(m_key_field && (m_key_field->get_unique_key() || m_key_field->get_primary_key()))
  {
    if(m_AddDel.get_count() > 0) //If there is already 1 record
      bAllowAdd = false;
  }

  if(bAllowAdd)
  {
    Box_Data_List::on_adddel_user_added(row, col_with_first_value);
  }
  else
  {
    //Tell user that they can't do that:
    Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Extra related records not possible.")), true, Gtk::MESSAGE_WARNING);
    dialog.set_secondary_text(_("You attempted to add a new related record, but there can only be one related record, because the relationship uses a unique key.")),
    dialog.set_transient_for(*get_app_window());
    dialog.run();

    //Replace with correct values:
    fill_from_database();
  }
}

Box_Data_List_Related::type_vecLayoutFields Box_Data_List_Related::get_fields_to_show() const
{
  const Document_Glom* document = get_document();
  if(document)
  {
    Document_Glom::type_mapLayoutGroupSequence mapGroups;
    mapGroups[0] = m_portal;

    sharedptr<const Relationship> relationship = m_portal->get_relationship();
    if(relationship)
    {
      type_vecLayoutFields result = get_table_fields_to_show_for_sequence(relationship->get_to_table(), mapGroups);

      //If the relationship does not allow editing, then mark all these fields as non-editable:
      if(!(get_relationship()->get_allow_edit()))
      {
        for(type_vecLayoutFields::iterator iter = result.begin(); iter != result.end(); ++iter)
        {
          sharedptr<LayoutItem_Field> item = *iter;
          if(item)
            item->set_editable(false);
        }
      }

      return result;
    }
  }

  return type_vecLayoutFields();
}

void Box_Data_List_Related::show_layout_dialog()
{
  if(m_pDialogLayoutRelated)
  {
    m_pDialogLayoutRelated->set_document(m_layout_name, get_document(), m_portal);
    m_pDialogLayoutRelated->show();
  }
}

Box_Data_List_Related::type_signal_record_changed Box_Data_List_Related::signal_record_changed()
{
  return m_signal_record_changed;
}

void Box_Data_List_Related::on_dialog_layout_hide()
{
  m_portal = m_pDialogLayoutRelated->get_portal_layout();


  //Update the UI:
  init_db_details(m_portal); 

  Box_Data::on_dialog_layout_hide();

  sharedptr<LayoutItem_Portal> pLayoutItem = sharedptr<LayoutItem_Portal>::cast_dynamic(get_layout_item());
  if(pLayoutItem)
  {
    *pLayoutItem = *m_portal;
    signal_layout_changed().emit(); //TODO: Check whether it has really changed.
  }
}

void Box_Data_List_Related::on_adddel_user_requested_add()
{
  //Prevent an add on a portal with no fields:
  //TODO: Warn the user instead of just doing nothing.
  if(!m_portal->m_map_items.empty())
    Box_Data_List::on_adddel_user_requested_add();
}




bool Box_Data_List_Related::get_has_suitable_record_to_view_details() const
{
  Glib::ustring navigation_table_name;
  sharedptr<const UsesRelationship> navigation_relationship;
  get_suitable_table_to_view_details(navigation_table_name, navigation_relationship);
  return !(navigation_table_name.empty());
}

void Box_Data_List_Related::get_suitable_table_to_view_details(Glib::ustring& table_name, sharedptr<const UsesRelationship>& relationship) const
{
  //Initialize output parameters:
  table_name = Glib::ustring();

  //Check whether a relationship was specified:
  bool navigation_relationship_main = false;
  sharedptr<const UsesRelationship> navigation_relationship = m_portal->get_navigation_relationship_specific(navigation_relationship_main);
  if(!navigation_relationship_main && !navigation_relationship)
  {
    //std::cout << "debug: decide automatically." << std::endl;
    //Decide automatically:
    navigation_relationship_main = false;
    navigation_relationship = get_portal_navigation_relationship_automatic(m_portal, navigation_relationship_main);
    //std::cout << "debug: auto main=" << navigation_relationship_main << ", navigation_relationship=" << (navigation_relationship ? navigation_relationship->get_name() : navigation_relationship->get_relationship()->get_name()) << std::endl;
  }
  //else
  //  std::cout << "debug: get_suitable_table_to_view_details(): Using specific nav." << std::endl;

  const Document_Glom* document = get_document();
  if(!document)
    return;


  //Get the navigation table name from the chosen relationship:
  const Glib::ustring directly_related_table_name = m_portal->get_relationship()->get_to_table();
  Glib::ustring navigation_table_name;
  if(navigation_relationship_main)
  {
     std::cout << "  debug: using main rel" << std::endl;
    navigation_table_name = directly_related_table_name;
  }
  else if(navigation_relationship)
  {
    //std::cout << "  debug: using rel: " << navigation_relationship->get_relationship_name() << std::endl;
    navigation_table_name = navigation_relationship->get_table_used(directly_related_table_name);
  }

  if(navigation_table_name.empty())
  {
    std::cerr << "Box_Data_List_Related::get_suitable_table_to_view_details(): navigation_table_name is empty." << std::endl;
    return;
  }

  if(document->get_table_is_hidden(navigation_table_name))
  {
    std::cerr << "Box_Data_List_Related::get_suitable_table_to_view_details(): navigation_table_name indicates a hidden table: " << navigation_table_name << std::endl;
    return;
  }

  table_name = navigation_table_name;
  relationship = navigation_relationship;
}

void Box_Data_List_Related::get_suitable_record_to_view_details(const Gnome::Gda::Value& primary_key_value, Glib::ustring& table_name, Gnome::Gda::Value& table_primary_key_value) const
{
  //Initialize output parameters:
  table_name = Glib::ustring();
  table_primary_key_value = Gnome::Gda::Value();

  Glib::ustring navigation_table_name;
  sharedptr<const UsesRelationship> navigation_relationship;
  get_suitable_table_to_view_details(navigation_table_name, navigation_relationship);

  if(navigation_table_name.empty())
    return;

  //Get the primary key of that table:
  sharedptr<Field> navigation_table_primary_key = get_field_primary_key_for_table(navigation_table_name);

  //Build a layout item to get the field's value:
  sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
  layout_item->set_full_field_details(navigation_table_primary_key);

  if(navigation_relationship)
  {
    layout_item->set_relationship( navigation_relationship->get_relationship() );
    //std::cout << "debug: navigation_relationship->get_relationship()= " << navigation_relationship->get_relationship()->get_name() << std::endl;
    layout_item->set_related_relationship( navigation_relationship->get_related_relationship() );
  }

  //Get the value of the navigation related primary key:
  type_vecLayoutFields fieldsToGet;
  fieldsToGet.push_back(layout_item);

  const Glib::ustring query = Utils::build_sql_select_with_key(m_portal->get_relationship()->get_to_table(), fieldsToGet, m_AddDel.get_key_field(), primary_key_value);
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(query);


  bool value_found = true;
  if(data_model && data_model->get_n_rows() && data_model->get_n_columns())
  {
    //Set the output parameters:
    table_name = navigation_table_name;
    table_primary_key_value = data_model->get_value_at(0, 0);

    //std::cout << "Box_Data_List_Related::get_suitable_record_to_view_details(): table_primary_key_value=" << table_primary_key_value.to_string() << std::endl;

    //The value is empty when there there is no record to match the key in the related table:
    //For instance, if an invoice lines record mentions a product id, but the product does not exist in the products table.
    if(Conversions::value_is_empty(table_primary_key_value))
    {
      value_found = false;
      std::cout << "debug: Box_Data_List_Related::get_suitable_record_to_view_details(): SQL query returned empty primary key." << std::endl;
    }
  }
  else
  {
    value_found = false;
    std::cout << "debug: Box_Data_List_Related::get_suitable_record_to_view_details(): SQL query returned no suitable primary key." << std::endl;
  }

  if(!value_found)
  {
    //Clear the output parameters:
    table_name = Glib::ustring();
    table_primary_key_value = Gnome::Gda::Value();

    Gtk::Window* window = const_cast<Gtk::Window*>(get_app_window());
    if(window)
      Frame_Glom::show_ok_dialog(_("No Corresponding Record Exists"), _("No record with this value exists. Therefore navigation to the related record is not possible."), *window, Gtk::MESSAGE_WARNING); //TODO: Make it more clear to the user exactly what record, what field, and what value, we are talking about.
  }
}

Document_Glom::type_mapLayoutGroupSequence Box_Data_List_Related::create_layout_get_layout()
{
  Document_Glom::type_mapLayoutGroupSequence result;

  if(m_portal)
    result[0] = m_portal;
  
  return result;
}


} //namespace Glom
