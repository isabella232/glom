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
#include <glom/libglom/glade_utils.h>
#include <glom/frame_glom.h> //For show_ok_dialog()
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>

namespace Glom
{

Box_Data_List_Related::Box_Data_List_Related()
{
  set_size_request(400, -1); //An arbitrary default.

  m_AddDel.set_rules_hint(); //Use alternating row colors when the theme does that.
  m_Alignment.add(m_AddDel);
  add_view(&m_AddDel); //Give it access to the document.
  m_AddDel.show();
  m_Alignment.show();
  
  //Connect signals:  
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Data_List_Related::on_adddel_user_requested_edit));
  m_AddDel.signal_record_changed().connect(sigc::mem_fun(*this, &Box_Data_List_Related::on_adddel_record_changed));

  m_AddDel.signal_script_button_clicked().connect(sigc::mem_fun(*this, &Box_Data_List_Related::on_adddel_script_button_clicked));
  m_AddDel.signal_record_added().connect(sigc::mem_fun(*this, &Box_Data_List_Related::on_adddel_record_added));
  
#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_AddDel.signal_user_requested_layout().connect(sigc::mem_fun(*this, &Box_Data_List_Related::on_adddel_user_requested_layout));
#endif // !GLOM_ENABLE_CLIENT_ONLY
  
  m_layout_name = "list_related"; //TODO: We need a unique name when 2 portals use the same table.
}

void Box_Data_List_Related::enable_buttons()
{
  const bool view_details_possible = get_has_suitable_record_to_view_details();
  m_AddDel.set_allow_view_details(view_details_possible); //Don't allow the user to go to a record in a hidden table.
}

bool Box_Data_List_Related::init_db_details(const sharedptr<const LayoutItem_Portal>& portal, bool show_title)
{
  m_portal = glom_sharedptr_clone(portal);

  LayoutWidgetBase::m_table_name = m_portal->get_table_used(Glib::ustring() /* parent table_name, not used. */); 
  Base_DB_Table::m_table_name = LayoutWidgetBase::m_table_name;

  const Glib::ustring relationship_title = m_portal->get_title_used(Glib::ustring() /* parent title - not relevant */);
  
  if(show_title)
  {
    m_Label.set_markup(Bakery::App_Gtk::util_bold_message(relationship_title));
    m_Label.show();

    m_Alignment.set_padding(Utils::DEFAULT_SPACING_SMALL /* top */, 0, Utils::DEFAULT_SPACING_LARGE /* left */, 0);
  }
  else
  {
    m_Label.set_markup(Glib::ustring());
    m_Label.hide();

    m_Alignment.set_padding(0, 0, 0, 0); //The box itself has padding of 6.
  }

  m_key_field = get_fields_for_table_one_field(LayoutWidgetBase::m_table_name, m_portal->get_to_field_used());

  //Prevent impossible multiple related records:
  const bool single_related = (m_key_field && (m_key_field->get_unique_key() || m_key_field->get_primary_key()));
  m_AddDel.set_allow_only_one_related_record(single_related);
  
  enable_buttons();

  FoundSet found_set;
  found_set.m_table_name = LayoutWidgetBase::m_table_name;
  m_AddDel.set_found_set(found_set);
  return Box_Data_ManyRecords::init_db_details(found_set); //Calls create_layout() and fill_from_database().
}

bool Box_Data_List_Related::fill_from_database()
{
  bool result = false;
  bool allow_add = true;

  if(m_key_field && m_found_set.m_where_clause.empty()) //There's a key field, but no value.
  {
    //No Foreign Key value, so just show the field names:

    result = Base_DB_Table_Data::fill_from_database();

    //create_layout();
  }
  else
  {
    result = Box_Data_Portal::fill_from_database();


    //Is there already one record here?
    if(m_has_one_or_more_records) //This was set by Box_Data_Portal::fill_from_database().
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
  
  m_AddDel.set_found_set(m_found_set);
  result = m_AddDel.refresh_from_database();

  return result;
}

Gnome::Gda::Value Box_Data_List_Related::get_primary_key_value(const Gtk::TreeModel::iterator& row) const
{
  return m_AddDel.get_value_key(row);
}

void Box_Data_List_Related::on_adddel_user_requested_edit(const Gtk::TreeModel::iterator& row)
{
  if(!(m_portal->get_relationship_used_allows_edit()))
  {
    std::cerr << "Box_Data_List_Related::on_adddel_user_requested_eidt() called on non-editable portal. This should not happen." << std::endl;
    return;
  }

  //Call base class:
  
  const Gnome::Gda::Value primary_key_value = m_AddDel.get_value_key(row); //The primary key is in the key.
  std::cout << "on_adddel_user_requested_edit(): Requesting edit for primary_key=" << primary_key_value.to_string() << std::endl;
  signal_user_requested_details().emit(primary_key_value);
}

void Box_Data_List_Related::on_adddel_record_changed()
{
  //Let parent respond:
  signal_portal_record_changed().emit(m_portal->get_relationship_name());
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Box_Data_List_Related::on_adddel_user_requested_layout()
{
  show_layout_dialog();
}
#endif // !GLOM_ENABLE_CLIENT_ONLY


void Box_Data_List_Related::on_adddel_script_button_clicked(const sharedptr<const LayoutItem_Button>& layout_item, const Gtk::TreeModel::iterator& row)
{
  if(!layout_item)
    return;
  
  const Gnome::Gda::Value primary_key_value = get_primary_key_value(row);
  execute_button_script(layout_item, primary_key_value);

  // Refill view from database as the script might have changed arbitrary records

#if 0
  // TODO: This is perhaps a better approach, but
  // DbTreeModel::refresh_from_database is protected
  Glib::RefPtr<Gtk::TreeModel> model = m_AddDel.get_model();
  Glib::RefPtr<DbTreeModel> db_model = Glib::RefPtr<DbTreeModel>::cast_dynamic(model);
  if(db_model)
    db_model->refresh_from_database(m_found_set);
#endif

  // TODO: Calling refresh_data_from_database() causes a crash somewhere
  // down in GTK+, so it is done in a handler here.
  // We are currently in a callback from the CellRendererButton_Text cell
  // renderer which is deleted by a call to refresh_data_from_database().
  // Probably this causes issues somewhere. 
  Glib::signal_idle().connect(sigc::bind(sigc::mem_fun(*this, &Box_Data_List_Related::on_script_button_idle), primary_key_value));

  //refresh_data_from_database();
  //set_primary_key_value_selected(primary_key);
}

bool Box_Data_List_Related::on_script_button_idle(const Gnome::Gda::Value& primary_key)
{
  refresh_data_from_database();
  set_primary_key_value_selected(primary_key);
  return false;
}

void Box_Data_List_Related::on_adddel_record_added(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& primary_key_value)
{
  //Note that on_record_added() would only be called on the AddDel itself, so we need ot handle this AddDel signal.
  
  //primary_key_value is a new autogenerated or human-entered key for the row.
  //It has already been added to the database.
  //Gnome::Gda::Value primary_key_value = m_AddDel.get_value_key(row);
  std::cout << "Box_Data_List_Related::on_adddel_record_added(): primary_key_value=" << primary_key_value.to_string() << std::endl;


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
    g_warning("Box_Data_List_Related::on_adddel_record_added(): m_key_value is NULL.");
  }
  else
  {
    sharedptr<Field> field_primary_key = m_AddDel.get_key_field();

    //Create the link by setting the foreign key
    if(m_key_field)
    {
      Glib::ustring strQuery = "UPDATE \"" + m_portal->get_table_used(Glib::ustring() /* not relevant */) + "\"";
      strQuery += " SET \"" +  /* get_table_name() + "." +*/ m_key_field->get_name() + "\" = " + m_key_field->sql(m_key_value);
      strQuery += " WHERE \"" + get_table_name() + "\".\"" + field_primary_key->get_name() + "\" = " + field_primary_key->sql(primary_key_value);
      std::cout << "Box_Data_List_Related::on_adddel_record_added(): setting value in db=" << primary_key_value.to_string() << std::endl;
      const bool test = query_execute(strQuery, get_app_window());
      if(test)
      {
        //Show it on the view, if it's visible:
        sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
        layout_item->set_full_field_details(m_key_field);

        //TODO: Although the to-field value is visible on the new related record, get_value() returns NULL so you can't immediately navigate to the new record: 
        //std::cout << "DEBUG: Box_Data_List_Related::on_record_added(): setting field=" << layout_item->get_name() << "m_key_value=" << m_key_value.to_string() << std::endl; 
        m_AddDel.set_value(row, layout_item, m_key_value);
      }
      else
        std::cerr << "Box_Data_List_Related::on_record_added(): SQL query failed: " << strQuery << std::endl;
    }
    else
      std::cerr << "Box_Data_List_Related::on_record_added(): m_key_field is NULL" << std::endl;
  

    //on_adddel_user_changed(row, iKey); //Update the database.
  }
  
  on_record_added(key_value, row);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Box_Data_List_Related::on_dialog_layout_hide()
{
  Dialog_Layout_List_Related* dialog_related = dynamic_cast<Dialog_Layout_List_Related*>(m_pDialogLayout);
  g_assert(dialog_related != NULL);
  m_portal = dialog_related->get_portal_layout();


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
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifndef GLOM_ENABLE_CLIENT_ONLY
Dialog_Layout* Box_Data_List_Related::create_layout_dialog() const
{
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(Utils::get_glade_file_path("glom_developer.glade"), "window_data_layout");
  if(refXml)
  {
    Dialog_Layout_List_Related* dialog = 0;
    refXml->get_widget_derived("window_data_layout", dialog);
    return dialog;
  }
  
  return NULL;
}

void Box_Data_List_Related::prepare_layout_dialog(Dialog_Layout* dialog)
{
  Dialog_Layout_List_Related* related_dialog = dynamic_cast<Dialog_Layout_List_Related*>(dialog);
  g_assert(related_dialog != NULL);
  related_dialog->set_document(m_layout_name, get_document(), m_portal);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Gnome::Gda::Value Box_Data_List_Related::get_primary_key_value_selected() const
{
  return m_AddDel.get_value_key_selected();
}

sharedptr<Field> Box_Data_List_Related::get_field_primary_key() const
{
  return m_AddDel.get_key_field();
}

void Box_Data_List_Related::set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value)
{
  m_AddDel.set_value_key(row, value);
}


Document_Glom::type_list_layout_groups Box_Data_List_Related::create_layout_get_layout()
{
  //Overriden in Box_Data_List_Related:
  return get_data_layout_groups(m_layout_name); 
}

//These create_layout*() methods are actually copy/pasted from Box_Data_List().
//TODO: Reduce the copy/pasting of these?
void Box_Data_List_Related::create_layout()
{
  Box_Data::create_layout(); //Fills m_TableFields.

  const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
  if(pDoc)
  {
    //Field Names:
    m_AddDel.remove_all_columns();
    //m_AddDel.set_columns_count(m_Fields.size());

    m_AddDel.set_table_name(Base_DB_Table::m_table_name);


    sharedptr<Field> field_primary_key = get_field_primary_key_for_table(Base_DB_Table::m_table_name);
    if(!field_primary_key)
    {
      //g_warning("%s: primary key not found.", __FUNCTION__);
    }
    else
    {
      m_AddDel.set_key_field(field_primary_key);
 
      //This map of layout groups will also contain the field information from the database:
      Document_Glom::type_list_layout_groups layout_groups = create_layout_get_layout();
      
      //int debug_count = 0;
      for(Document_Glom::type_list_layout_groups::const_iterator iter = layout_groups.begin(); iter != layout_groups.end(); ++iter)
      {
        //std::cout << "Box_Data_List::create_layout() group number=" << debug_count;
        //debug_count++;
        //iter->second->debug();

        create_layout_add_group(*iter);
      }
    }


    m_FieldsShown = get_fields_to_show();

    //Add extra possibly-non-visible columns that we need:
    //TODO: Only add it if it is not already there.
    if(field_primary_key)
    {
      sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
      layout_item->set_hidden();
      layout_item->set_full_field_details(m_AddDel.get_key_field());
      m_FieldsShown.push_back(layout_item);

      m_AddDel.add_column(layout_item);
    }

    m_AddDel.set_found_set(m_found_set);

    //Column-creation happens in fill_database() instead:
    //otherwise the treeview will be filled twice.
    //m_AddDel.set_columns_ready();
  }

}

void Box_Data_List_Related::create_layout_add_group(const sharedptr<LayoutGroup>& layout_group)
{
  if(!layout_group)
    return;

  LayoutGroup::type_list_items child_items = layout_group->get_items();
  for(LayoutGroup::type_list_items::const_iterator iter = child_items.begin(); iter != child_items.end(); ++iter)
  {
    sharedptr<LayoutItem> child_item = *iter;

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

      sharedptr<LayoutItem_Field> child_field = sharedptr<LayoutItem_Field>::cast_dynamic(child_item);
      if(child_field)
      {
        //Check that the field really exists, to avoid SQL errors.
        //This could probably only happen if we have failed to rename something everywhere, when the user has renamed something.
        if(!get_field_exists_in_database(child_field->get_table_used(Base_DB_Table::m_table_name), child_field->get_name()))
        {
          std::cerr << "debug: Box_Data_List_Related::create_layout_add_group(): Field does not exist in database: table_name=" << child_field->get_table_used(Base_DB_Table::m_table_name) << ", field_name=" << child_field->get_name() << std::endl;
          continue;
        }
      }

      //Sometimes we reset the column width so that new fields are easily visible:
      //if(m_reset_column_widths)
      //  child_item->set_display_width(0);

      m_AddDel.add_column(child_item);
    }
  }
}



} //namespace Glom
