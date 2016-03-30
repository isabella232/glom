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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "box_data_list.h"
#include <libglom/data_structure/glomconversions.h>
#include <glom/glade_utils.h>
#include <glom/mode_design/layout/dialog_layout_list.h>
#include <libglom/privs.h>
#include <libglom/layout_utils.h>
#include <libglom/utils.h> //For bold_message()).
#include <glibmm/main.h>

namespace Glom
{

Box_Data_List::Box_Data_List()
: m_read_only(false)
{
  m_layout_name = "list";

  //m_strHint = _("When you change the data in a field the database is updated immediately.\n Click [Add] or enter data into the last row to add a new record.\n Leave automatic ID fields empty - they will be filled for you.\nOnly the first 100 records are shown.");

  pack_start(m_AddDel);
  add_view(&m_AddDel); //Give it access to the document.

  //Connect signals:
  //The Add and Delete buttons are handled by the DbAddDel widget itself.
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_requested_edit));
  m_AddDel.signal_script_button_clicked().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_script_button_clicked));
  m_AddDel.signal_sort_clause_changed().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_sort_clause_changed));
  m_AddDel.signal_record_selection_changed().connect(m_signal_record_selection_changed.make_slot());

#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_AddDel.signal_user_requested_layout().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_requested_layout));
#endif // !GLOM_ENABLE_CLIENT_ONLY


  //Groups are not very helpful for a list view:
  //m_dialog_layout->set_show_groups(false);

  m_AddDel.show();
}

Box_Data_List::~Box_Data_List()
{
  remove_view(&m_AddDel);
}

void Box_Data_List::enable_buttons()
{
  const auto table_privs = Privs::get_current_privs(m_table_name);

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
  found_set.m_where_clause = Gnome::Gda::SqlExpr();
  m_AddDel.set_found_set(found_set);

  std::cout << "debug: " << G_STRFUNC << ": before refresh_from_database_blank().\n";
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

  BusyCursor busy_cursor(get_app_window());

  std::shared_ptr<SharedConnection> sharedconnection;

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

  if(sharedconnection)
  {
    Box_Data::fill_from_database();

    //Field Names:
    //create_layout();

    //if(sharedconnection)
    //{
      //Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

    //Do not try to show the data if the user may not view it:
    const auto table_privs = Privs::get_current_privs(m_table_name);

    enable_buttons();

    m_AddDel.set_allow_view(table_privs.m_view);
    m_AddDel.set_found_set(m_found_set);

    result = m_AddDel.refresh_from_database();

    if(table_privs.m_view)
    {
      //Select first record:
      auto refModel = m_AddDel.get_model();
      if(refModel)
        m_AddDel.select_item(refModel->children().begin());

    } //privs
  }

  return result;
}

void Box_Data_List::on_adddel_user_requested_edit(const Gtk::TreeModel::iterator& row)
{
  const auto primary_key_value = m_AddDel.get_value_key(row); //The primary key is in the key.

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

void Box_Data_List::on_adddel_script_button_clicked(const std::shared_ptr<const LayoutItem_Button>& layout_item, const Gtk::TreeModel::iterator& row)
{
  if(!layout_item)
    return;

  const auto primary_key_value = get_primary_key_value(row);

  // TODO: Calling refresh_data_from_database(),
  // or navigating to a different table from inside the Python script,
  // causes a crash somewhere down in GTK+, so it is done in an idle handler here.
  // We are currently in a callback from the CellRendererButton_Text cell
  // renderer which is deleted by a call to refresh_data_from_database().
  // Probably this causes issues somewhere.
  Glib::signal_idle().connect(
    sigc::bind(
      sigc::mem_fun(*this, &Box_Data_List::on_script_button_idle),
      layout_item,
      primary_key_value));
}

bool Box_Data_List::on_script_button_idle(const std::shared_ptr<const LayoutItem_Button>& layout_item, const Gnome::Gda::Value& primary_key)
{
  execute_button_script(layout_item, primary_key);

  // Refill view from database as the script might have changed arbitrary records

#if 0
  // TODO: This is perhaps a better approach, but
  // DbTreeModel::refresh_from_database is protected
  auto model = m_AddDel.get_model();
  auto db_model = Glib::RefPtr<DbTreeModel>::cast_dynamic(model);
  if(db_model)
    db_model->refresh_from_database(m_found_set);
#endif

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
  auto iter = m_AddDel.get_item_selected();
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
  auto iter = m_AddDel.get_item_selected();
  if(iter)
  {
    //Don't go past the last record:
    if( !m_AddDel.get_is_last_row(iter) )
    {
      //std::cout << "debug: " << G_STRFUNC << ": The current row was not the last row.\n";

      iter++;
      m_AddDel.select_item(iter);

      signal_user_requested_details().emit(m_AddDel.get_value_key_selected());
    }
    //else
    //  std::cout << "debug: " << G_STRFUNC << ": Not going past the last row.\n";
  }
}

void Box_Data_List::on_details_nav_last()
{
  auto iter = m_AddDel.get_last_row();
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
  auto iter = m_AddDel.get_row(primary_key_value);
  if(iter)
  {
    //Remove the row:
    auto iterNext = iter;
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
  //std::cout << "debug: " << G_STRFUNC << ": get_primary_key_value_first() records_count = " << m_AddDel.get_count() << std::endl;

  auto model = m_AddDel.get_model();
  if(model)
  {
    auto iter = model->children().begin();
    while(iter != model->children().end())
    {
      Gnome::Gda::Value value = get_primary_key_value(iter);
      if(Conversions::value_is_empty(value))
      {
       //std::cout << "debug: " << G_STRFUNC << ": get_primary_key_value_first() iter val is NULL\n";
        ++iter;
      }
      else
      {
         //std::cout << "debug: " << G_STRFUNC << ": get_primary_key_value_first() returning: " << value.to_string() << std::endl;
        return value;
      }
    }
  }

 // std::cout << "debug: " << G_STRFUNC << ": get_primary_key_value_first() return NULL\n";
  return Gnome::Gda::Value();
}

Gnome::Gda::Value Box_Data_List::get_entered_field_data(const std::shared_ptr<const LayoutItem_Field>& field) const
{
  return m_AddDel.get_value_selected(field);
}

void Box_Data_List::set_entered_field_data(const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  m_AddDel.set_value_selected(field, value);
}

void Box_Data_List::set_entered_field_data(const Gtk::TreeModel::iterator& row, const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  m_AddDel.set_value(row, field, value);
}

bool Box_Data_List::get_showing_multiple_records() const
{
  return m_AddDel.get_count() > 1;
}

Document::type_list_layout_groups Box_Data_List::create_layout_get_layout()
{
  //This method is overriden in Box_Data_List_Related.

  return get_data_layout_groups(m_layout_name, m_layout_platform);
}

void Box_Data_List::create_layout()
{
  Box_Data::create_layout(); //Fills m_TableFields.

  const auto pDoc = std::dynamic_pointer_cast<const Document>(get_document());
  if(!pDoc)
    return;


  //Field Names:
  m_AddDel.remove_all_columns();
  //m_AddDel.set_columns_count(m_Fields.size());

  m_AddDel.set_table_name(m_table_name);


  auto field_primary_key = get_field_primary_key_for_table(m_table_name);
  if(!field_primary_key)
  {
    std::cerr << G_STRFUNC << ": primary key not found for table: " << m_table_name << std::endl;
    return;
  }

   m_AddDel.set_key_field(field_primary_key);



  LayoutGroup::type_list_items items_to_use;

  //This map of layout groups will also contain the field information from the database:
  Document::type_list_layout_groups layout_groups = create_layout_get_layout();
  for(const auto& layout_group : layout_groups)
  {
    if(!layout_group)
      continue;

    const auto child_items = layout_group->get_items_recursive();
    for(const auto& child_item : child_items)
    {
      //TODO: Set the whole thing as read-only instead:
      if(m_read_only)
        child_item->set_editable(false);

      auto child_field = std::dynamic_pointer_cast<const LayoutItem_Field>(child_item);

      //This check has already happened in Frame_Glom::update_table_in_document_from_database().
      //It is inefficient and unnecessary to do it here too.
      /*
      if(child_field)
      {
        //Check that the field really exists, to avoid SQL errors.
        //This could probably only happen if we have failed to rename something everywhere, when the user has renamed something.
        if(!DbUtils::get_field_exists_in_database(child_field->get_table_used(m_table_name), child_field->get_name()))
        {
          std::cerr << G_STRFUNC << ": Field does not exist in database: table_name=" << child_field->get_table_used(m_table_name) << ", field_name=" << child_field->get_name() << std::endl;
          continue;
        }
      }
      */

      items_to_use.emplace_back(child_item);
    }
  }


  //Add extra possibly-non-visible columns that we need:
  //TODO: Only add it if it is not already there.
  items_to_use = Utils::get_layout_items_plus_primary_key(items_to_use, pDoc, m_table_name);
  if(field_primary_key)
  {
    auto layout_item = std::make_shared<LayoutItem_Field>();
    layout_item->set_hidden();
    layout_item->set_full_field_details(m_AddDel.get_key_field());

    m_FieldsShown.emplace_back(layout_item); //TODO: Do this only if it is not already present.
  }

  const auto table_privs = Privs::get_current_privs(m_found_set.m_table_name);
  m_AddDel.set_allow_view(table_privs.m_view);
    
  m_AddDel.set_found_set(m_found_set);
  m_AddDel.set_columns(items_to_use); //TODO: Use LayoutGroup::type_list_const_items instead?

  m_FieldsShown = get_fields_to_show();
}

std::shared_ptr<Field> Box_Data_List::get_field_primary_key() const
{
  return m_AddDel.get_key_field();
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
  auto iter = m_AddDel.get_row(primary_key_value);
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

  auto refModel = m_AddDel.get_model();
  auto refModelDerived = Glib::RefPtr<DbTreeModel>::cast_dynamic(refModel);

  if(refModelDerived)
    refModelDerived->get_record_counts(total, found);
}

void Box_Data_List::on_adddel_user_sort_clause_changed()
{
  //Remember details about the previously viewed table,
  //so we don't forget the sort order and where clause when
  //navigating back, which would annoy the user:

  m_found_set = m_AddDel.get_found_set();

  auto document = get_document();
  if(document)
    document->set_criteria_current(m_table_name, m_found_set);
}

Gtk::TreeModel::iterator Box_Data_List::get_row_selected()
{
  return m_AddDel.get_item_selected();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
//overridden, so we can change the column widths, so they are all visible:
void Box_Data_List::on_dialog_layout_hide()
{
  Box_Data::on_dialog_layout_hide();
}

Dialog_Layout* Box_Data_List::create_layout_dialog() const
{
  Dialog_Layout_List* dialog = nullptr;
  Glom::Utils::get_glade_widget_derived_with_warning(dialog);
  return dialog;
}

void Box_Data_List::prepare_layout_dialog(Dialog_Layout* dialog)
{
  dialog->init(m_layout_name, m_layout_platform, get_document(), m_table_name, m_FieldsShown); //TODO: Use m_TableFields?
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

} //namespace Glom
