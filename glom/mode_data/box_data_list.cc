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
#include <glom/libglom/glade_utils.h>
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

  //m_strHint = _("When you change the data in a field the database is updated immediately.\n Click [Add] or enter data into the last row to add a new record.\n Leave automatic ID fields empty - they will be filled for you.\nOnly the first 100 records are shown.");

  pack_start(m_AddDel);
  add_view(&m_AddDel); //Give it access to the document.
  m_AddDel.set_rules_hint(); //Use alternating row colors when the theme does that.

  //Connect signals:
  //The Add and Delete buttons are handled by the DbAddDel widget itself.
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_requested_edit));
  m_AddDel.signal_script_button_clicked().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_script_button_clicked));
  
  //TODO: Re-add this signal if this is really wanted, but make it part of a complete drag-and-drop feature for list views:
  //m_AddDel.signal_user_reordered_columns().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_reordered_columns));

#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_AddDel.signal_user_requested_layout().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_requested_layout));
#endif // !GLOM_ENABLE_CLIENT_ONLY


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

  //Don't try to open a connection if there is no document,
  //for instance, during application destruction.
  if(!get_document())
    return false;

  Bakery::BusyCursor busy_cursor(get_app_window());

  sharedptr<SharedConnection> sharedconnection;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    sharedconnection = connect_to_server(get_app_window());
  }
  catch(const Glib::Exception& ex)
  {
    handle_error(ex);
    result = false;
  }
  catch(const std::exception& ex)
  {
    handle_error(ex);
    result = false;
  }
#else
  std::auto_ptr<ExceptionConnection> error;
  sharedconnection = connect_to_server(get_app_window(), error);
  if(error.get())
  {
    handle_error(*error);
    result = false;
  }
#endif

  if(sharedconnection)
  {
    Box_Data::fill_from_database();

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
  }

  return result;
}

void Box_Data_List::on_adddel_user_requested_edit(const Gtk::TreeModel::iterator& row)
{
  const Gnome::Gda::Value primary_key_value = m_AddDel.get_value_key(row); //The primary key is in the key.

  signal_user_requested_details().emit(primary_key_value);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Box_Data_List::on_adddel_user_requested_layout()
{
  show_layout_dialog();
}
#endif // !GLOM_ENABLE_CLIENT_ONLY


void Box_Data_List::set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value)
{
  m_AddDel.set_value_key(row, value);
}

void Box_Data_List::on_adddel_user_reordered_columns()
{
  Document_Glom* pDoc = dynamic_cast<Document_Glom*>(get_document());
  if(pDoc)
  {
    sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::create();
    group->set_name("toplevel");

    AddDel::type_vecStrings vec_field_names = m_AddDel.get_columns_order();

    for(AddDel::type_vecStrings::iterator iter = vec_field_names.begin(); iter != vec_field_names.end(); ++iter)
    {
      sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
      layout_item->set_name(*iter);
      group->add_item(layout_item); 
    }

    Document_Glom::type_list_layout_groups mapGroups;
    mapGroups[1] = group;

    pDoc->set_data_layout_groups("list", m_table_name, mapGroups);  
  }
}

void Box_Data_List::on_adddel_script_button_clicked(const sharedptr<const LayoutItem_Button>& layout_item, const Gtk::TreeModel::iterator& row)
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
  Glib::signal_idle().connect(sigc::bind(sigc::mem_fun(*this, &Box_Data_List::on_script_button_idle), primary_key_value));

  //refresh_data_from_database();
  //set_primary_key_value_selected(primary_key);
}

bool Box_Data_List::on_script_button_idle(const Gnome::Gda::Value& primary_key)
{
  refresh_data_from_database();
  set_primary_key_value_selected(primary_key);
  return false;
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
      std::cout << "DEBUG: Box_Data_List::on_details_nav_next(): The current row was not the last row." << std::endl;

      iter++;
      m_AddDel.select_item(iter);

      signal_user_requested_details().emit(m_AddDel.get_value_key_selected());
    }
    else
      std::cout << "DEBUG: Box_Data_List::on_details_nav_next(): Not going past the last row." << std::endl;
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
  
  //No, don't do this. When would that ever be a good idea? murrayc:
  //signal_user_requested_details().emit(Gnome::Gda::Value()); //Show a blank record if there are no records.
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

Gnome::Gda::Value Box_Data_List::get_primary_key_value(const Gtk::TreeModel::iterator& row) const
{
  return m_AddDel.get_value_key(row);
}

Gnome::Gda::Value Box_Data_List::get_primary_key_value_selected() const
{
  return m_AddDel.get_value_key_selected();
}

Gnome::Gda::Value Box_Data_List::get_primary_key_value_first() const
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
        if(!get_field_exists_in_database(child_field->get_table_used(m_table_name), child_field->get_name()))
        {
          std::cerr << "debug: Box_Data_List::create_layout_add_group(): Field does not exist in database: table_name=" << child_field->get_table_used(m_table_name) << ", field_name=" << child_field->get_name() << std::endl;
          continue;
        }
      }

      m_AddDel.add_column(child_item);
    }
  }
}

Document_Glom::type_list_layout_groups Box_Data_List::create_layout_get_layout()
{
  //This method is overriden in Box_Data_List_Related.

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

sharedptr<Field> Box_Data_List::get_field_primary_key() const
{
  return m_AddDel.get_key_field();
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

#ifndef GLOM_ENABLE_CLIENT_ONLY
//overridden, so we can change the column widths, so they are all visible:
void Box_Data_List::on_dialog_layout_hide()
{
  Box_Data::on_dialog_layout_hide();
}

Dialog_Layout* Box_Data_List::create_layout_dialog() const
{
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(Utils::get_glade_file_path("glom_developer.glade"), "window_data_layout"); //TODO: Use a generic layout dialog?
  if(refXml)
  {
    Dialog_Layout_List* dialog = 0;
    refXml->get_widget_derived("window_data_layout", dialog);
    return dialog;
  }

  return NULL;
}

void Box_Data_List::prepare_layout_dialog(Dialog_Layout* dialog)
{
  dialog->set_document(m_layout_name, get_document(), m_table_name, m_FieldsShown); //TODO: Use m_TableFields?
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

} //namespace Glom

