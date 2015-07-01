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

#include <glom/mode_data/box_data_list_related.h>
#include <glom/mode_design/layout/dialog_layout_list_related.h>
#include <glom/appwindow.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/db_utils.h>
#include <libglom/privs.h>
#include <glom/glade_utils.h>
#include <glom/frame_glom.h> //For show_ok_dialog()
#include <glom/utils_ui.h> //For bold_message()).
#include <glibmm/main.h>
#include <glibmm/i18n.h>

namespace Glom
{

Box_Data_List_Related::Box_Data_List_Related()
{
  m_Frame.add(m_AddDel);
  add_view(&m_AddDel); //Give it access to the document.
  m_AddDel.show();
  m_AddDel.set_height_rows(6, 6);
  m_Frame.show();

  //Connect signals:
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Data_List_Related::on_adddel_user_requested_edit));
  m_AddDel.signal_record_changed().connect(sigc::mem_fun(*this, &Box_Data_List_Related::on_adddel_record_changed));

  m_AddDel.signal_script_button_clicked().connect(sigc::mem_fun(*this, &Box_Data_List_Related::on_adddel_script_button_clicked));
  m_AddDel.signal_record_added().connect(sigc::mem_fun(*this, &Box_Data_List_Related::on_adddel_record_added));

#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_AddDel.signal_user_requested_layout().connect(sigc::mem_fun(*this, &Box_Data_List_Related::on_adddel_user_requested_layout));
#endif // !GLOM_ENABLE_CLIENT_ONLY

  //We do not actually use this,
  //so it is a bug if this appears in the .glom file:
  m_layout_name = "NotUsedlist_related";
}

void Box_Data_List_Related::enable_buttons()
{
  const bool view_details_possible =
    get_has_suitable_record_to_view_details() &&
    (m_portal->get_navigation_type() != LayoutItem_Portal::NAVIGATION_NONE);

  // Don't allow the user to go to a record in a hidden table.
  // Unless we are on Maemo - then we want to allow editing in a separate window only.
  m_AddDel.set_allow_view_details(view_details_possible);
}

bool Box_Data_List_Related::init_db_details(const std::shared_ptr<const LayoutItem_Portal>& portal, bool show_title)
{
  //This calls the other method overload:
  return Box_Data_Portal::init_db_details(portal, show_title);
}

bool Box_Data_List_Related::init_db_details(const Glib::ustring& parent_table, bool show_title)
{
  m_parent_table = parent_table;

  if(m_portal)
    LayoutWidgetBase::m_table_name = m_portal->get_table_used(Glib::ustring() /* parent table_name, not used. */);
  else
    LayoutWidgetBase::m_table_name = Glib::ustring();

  if(LayoutWidgetBase::m_table_name.empty())
  {
    std::cerr << G_STRFUNC << ": LayoutWidgetBase::m_table_name is null" << std::endl;
  }
  
  Base_DB_Table::m_table_name = LayoutWidgetBase::m_table_name;

  if(show_title)
  {
    Glib::ustring title;
    if(m_portal)
      title = item_get_title(m_portal);

    m_Label.set_markup(UiUtils::bold_message(title));
    m_Label.show();

    if(!(m_Frame.get_label_widget()))
      m_Frame.set_label_widget(m_Label);

    m_AddDel.set_margin_start(UiUtils::DEFAULT_SPACING_LARGE);
    m_AddDel.set_margin_top(UiUtils::DEFAULT_SPACING_SMALL);

  }
  else
  {
    m_Label.set_markup(Glib::ustring());
    m_Label.hide();
    if(m_Frame.get_label_widget())
      m_Frame.unset_label(); //Otherwise the allocation is calculated wrong due to GtkFrame bug: https://bugzilla.gnome.org/show_bug.cgi?id=662915

    //The box itself has padding of 6:
    m_AddDel.set_margin_start(0);
    m_AddDel.set_margin_top(0);
  }

  if(m_portal)
  {
    m_key_field = DbUtils::get_fields_for_table_one_field(get_document(),
      LayoutWidgetBase::m_table_name, m_portal->get_to_field_used());
  }
  else
    m_key_field.reset();


  //Prevent impossible multiple related records:
  const bool single_related = (m_key_field && (m_key_field->get_unique_key() || m_key_field->get_primary_key()));
  m_AddDel.set_allow_only_one_related_record(single_related);

  enable_buttons();

  //TODO: Use m_found_set?
  FoundSet found_set;
  found_set.m_table_name = LayoutWidgetBase::m_table_name;

  const auto table_privs = Privs::get_current_privs(found_set.m_table_name);
  m_AddDel.set_allow_view(table_privs.m_view);

  m_AddDel.set_found_set(found_set);
  return Box_Data_ManyRecords::init_db_details(found_set, "" /* layout_platform */); //Calls create_layout() and fill_from_database().
}

bool Box_Data_List_Related::fill_from_database()
{
  bool result = false;
  bool allow_add = true;

  if(m_key_field && m_found_set.m_where_clause.empty()) //There's a key field, but no value.
  {
    //No Foreign Key value, so just show the field names:
    result = Base_DB_Table_Data::fill_from_database();
    if(!result)
    {
      std::cerr << G_STRFUNC << ": Base_DB_Table_Data::fill_from_database() failed." << std::endl;
    }

    //create_layout();
  }
  else
  {
    result = Box_Data_Portal::fill_from_database();
    if(!result)
    {
      std::cerr << G_STRFUNC << ": Box_Data_Portal::fill_from_database() failed." << std::endl;
    }

    //TODO: Disable add if the from_field already has a value and the to_field is auto-incrementing because
    //- we cannot override the auto-increment in the to_field.
    //- we cannot change the value in the from_field to the new auto_increment value in the to_field.
  }

  //Prevent addition of new records if that is what the relationship specifies:
  if(allow_add && m_portal && m_portal->get_relationship())
    allow_add = m_portal->get_relationship()->get_auto_create();

  m_AddDel.set_allow_add(allow_add);

  const auto table_privs = Privs::get_current_privs(m_found_set.m_table_name);
  m_AddDel.set_allow_view(table_privs.m_view);

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
  //Note that this is really an Open rather than an Edit.

  const auto primary_key_value = m_AddDel.get_value_key(row); //The primary key is in the key.
  
  if(!Conversions::value_is_empty(primary_key_value))
  {
    //std::cout << "debug: " << G_STRFUNC << ": Requesting edit for primary_key=" << primary_key_value.to_string() << std::endl;
    signal_user_requested_details().emit(primary_key_value);
  }
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


void Box_Data_List_Related::on_adddel_script_button_clicked(const std::shared_ptr<const LayoutItem_Button>& layout_item, const Gtk::TreeModel::iterator& row)
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
      sigc::mem_fun(*this, &Box_Data_List_Related::on_script_button_idle),
      layout_item,
      primary_key_value));
}

bool Box_Data_List_Related::on_script_button_idle(const std::shared_ptr<const LayoutItem_Button>& layout_item, const Gnome::Gda::Value& primary_key)
{
  execute_button_script(layout_item, primary_key);

  // Refill view from database as the script might have changed arbitrary records

#if 0
  // TODO: This is perhaps a better approach, but
  // DbTreeModel::refresh_from_database is protected
  Glib::RefPtr<Gtk::TreeModel> model = m_AddDel.get_model();
  Glib::RefPtr<DbTreeModel> db_model = Glib::RefPtr<DbTreeModel>::cast_dynamic(model);
  if(db_model)
    db_model->refresh_from_database(m_found_set);
#endif

  refresh_data_from_database();
  set_primary_key_value_selected(primary_key);
  return false;
}

void Box_Data_List_Related::on_adddel_record_added(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& primary_key_value)
{
  //Note that on_record_added() would only be called on the AddDel itself,
  //so we need to handle this AddDel signal.

  //primary_key_value is a new autogenerated or human-entered key for the row.
  //It has already been added to the database.
  //Gnome::Gda::Value primary_key_value = m_AddDel.get_value_key(row);
  //std::cout << "debug: " << G_STRFUNC << ": primary_key_value=" << primary_key_value.to_string() << std::endl;


  if(!row)
    return;

  Gnome::Gda::Value key_value;

  if(m_key_field)
  {
    //m_key_field is the field in this table that must match another field in the parent table.
    std::shared_ptr<LayoutItem_Field> layout_item = std::make_shared<LayoutItem_Field>();
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
    std::cerr << G_STRFUNC << ": m_key_value is NULL." << std::endl;
  }
  else
  {
    std::shared_ptr<Field> field_primary_key = m_AddDel.get_key_field();

    //Create the link by setting the foreign key
    if(m_key_field && m_portal)
    {
      make_record_related(primary_key_value);

      //Show it on the view, if it's visible:
      std::shared_ptr<LayoutItem_Field> layout_item = std::make_shared<LayoutItem_Field>();
      layout_item->set_full_field_details(m_key_field);

      //TODO: Although the to-field value is visible on the new related record, get_value() returns NULL so you can't immediately navigate to the new record:
      //std::cout << "debug: " << G_STRFUNC << ": setting field=" << layout_item->get_name() << "m_key_value=" << m_key_value.to_string() << std::endl;
      m_AddDel.set_value(row, layout_item, m_key_value);
    }
    else
      std::cerr << G_STRFUNC << ": m_key_field is NULL" << std::endl;


    //on_adddel_user_changed(row, iKey); //Update the database.
  }

  on_record_added(key_value, row);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Box_Data_List_Related::on_dialog_layout_hide()
{
  Dialog_Layout_List_Related* dialog_related = dynamic_cast<Dialog_Layout_List_Related*>(m_pDialogLayout);
  g_assert(dialog_related);
  m_portal = dialog_related->get_portal_layout();


  //Update the UI:
  init_db_details(m_portal);

  Box_Data::on_dialog_layout_hide();

  std::shared_ptr<LayoutItem_Portal> pLayoutItem = std::dynamic_pointer_cast<LayoutItem_Portal>(get_layout_item());
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
  Dialog_Layout_List_Related* dialog = 0;
  Glom::Utils::get_glade_widget_derived_with_warning(dialog);
  return dialog;
}

void Box_Data_List_Related::prepare_layout_dialog(Dialog_Layout* dialog)
{
  Dialog_Layout_List_Related* related_dialog = dynamic_cast<Dialog_Layout_List_Related*>(dialog);
  g_assert(related_dialog);

  related_dialog->init_with_portal(m_layout_name, m_layout_platform, get_document(), m_portal, m_parent_table);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Gnome::Gda::Value Box_Data_List_Related::get_primary_key_value_selected() const
{
  return m_AddDel.get_value_key_selected();
}

std::shared_ptr<Field> Box_Data_List_Related::get_field_primary_key() const
{
  return m_AddDel.get_key_field();
}

void Box_Data_List_Related::set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value)
{
  m_AddDel.set_value_key(row, value);
}


Document::type_list_layout_groups Box_Data_List_Related::create_layout_get_layout()
{
  Document::type_list_layout_groups result;

  //Do not use get_data_layout_groups(m_layout_name).
  //instead do this:
  if(m_portal)
    result.push_back(m_portal);

  return result;
}

//These create_layout*() methods are actually copy/pasted from Box_Data_List(),
//because we do not derived from Box_Data_List.
//TODO: Reduce the copy/pasting of these?
void Box_Data_List_Related::create_layout()
{
  Box_Data::create_layout(); //Fills m_TableFields.

  const Document* pDoc = dynamic_cast<const Document*>(get_document());
  if(!pDoc)
    return;


  //Field Names:
  m_AddDel.remove_all_columns();
  //m_AddDel.set_columns_count(m_Fields.size());

  m_AddDel.set_table_name(Base_DB_Table::m_table_name);
  
  if(m_portal)
  {
    gulong rows_count_min = 0;
    gulong rows_count_max = 0;
    m_portal->get_rows_count(rows_count_min, rows_count_max);
    if(rows_count_min) //0 is a silly value.
      m_AddDel.set_height_rows(rows_count_min, rows_count_max);
  }

  std::shared_ptr<Field> field_primary_key = get_field_primary_key_for_table(Base_DB_Table::m_table_name);
  if(!field_primary_key)
  {
    std::cerr << G_STRFUNC << ": primary key not found." << std::endl;
    return;
  }

   m_AddDel.set_key_field(field_primary_key);



  LayoutGroup::type_list_items items_to_use;

  //This map of layout groups will also contain the field information from the database:
  Document::type_list_layout_groups layout_groups = create_layout_get_layout();
  for(Document::type_list_layout_groups::const_iterator iter = layout_groups.begin(); iter != layout_groups.end(); ++iter)
  {
    const std::shared_ptr<LayoutGroup> layout_group = *iter;
    if(!layout_group)
      continue;

    const auto child_items = layout_group->get_items_recursive();
    for(LayoutGroup::type_list_items::const_iterator iterItems = child_items.begin(); iterItems != child_items.end(); ++iterItems)
    {
      std::shared_ptr<LayoutItem> child_item = *iterItems;

      //TODO: Set the whole thing as read-only instead:
      if(m_read_only)
        child_item->set_editable(false);

      std::shared_ptr<const LayoutItem_Field> child_field = std::dynamic_pointer_cast<const LayoutItem_Field>(child_item);
      
      //This check has already happened in Frame_Glom::update_table_in_document_from_database().
      //It is inefficient and unnecessary to do it here too.
      /*
      if(child_field)
      {
        //Check that the field really exists, to avoid SQL errors.
        //This could probably only happen if we have failed to rename something everywhere, when the user has renamed something.
        if(!DbUtils::get_field_exists_in_database(child_field->get_table_used(Base_DB_Table::m_table_name), child_field->get_name()))
        {
          std::cerr << G_STRFUNC << ": Field does not exist in database: table_name=" << child_field->get_table_used(Base_DB_Table::m_table_name) << ", field_name=" << child_field->get_name() << std::endl;
          continue;
        }
      }
      */

      items_to_use.push_back(child_item);
    }
  }


  //Add extra possibly-non-visible columns that we need:
  //TODO: Only add it if it is not already there.
  if(field_primary_key)
  {
    std::shared_ptr<LayoutItem_Field> layout_item = std::make_shared<LayoutItem_Field>();
    layout_item->set_hidden();
    layout_item->set_full_field_details(m_AddDel.get_key_field());
    m_FieldsShown.push_back(layout_item);

    items_to_use.push_back(layout_item);
  }

  const auto table_privs = Privs::get_current_privs(m_found_set.m_table_name);
  m_AddDel.set_allow_view(table_privs.m_view);
  
  m_AddDel.set_found_set(m_found_set);
  m_AddDel.set_columns(items_to_use);

  m_FieldsShown = get_fields_to_show();
}

void Box_Data_List_Related::set_find_mode(bool val)
{
  Box_Data_Portal::set_find_mode(val);
  m_AddDel.set_find_mode(val);
}

} //namespace Glom
