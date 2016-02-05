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

#include "box_db_table_definition.h"
#include <glom/frame_glom.h>
#include <glom/glade_utils.h>
#include <glom/utils_ui.h> //For bold_message()).
#include <glom/appwindow.h>
#include <libglom/libglom_config.h>
#include <libglom/db_utils.h>
#include <glibmm/i18n.h>

namespace Glom
{

Box_DB_Table_Definition::Box_DB_Table_Definition()
: m_dialog_field_definition(nullptr),
  m_dialog_default_formatting(nullptr)
{
  init();
}

Box_DB_Table_Definition::Box_DB_Table_Definition(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Box_DB_Table(cobject, builder)
{
  init();
}

void Box_DB_Table_Definition::init()
{
  //m_strHint = _("Click [Edit] to edit the field definition in more detail.\nUse the Mode menu to see Data or perform a Find.");

  Utils::get_glade_widget_derived_with_warning(m_dialog_field_definition);
  add_view(m_dialog_field_definition); //Give it access to the document.

  Utils::get_glade_widget_derived_with_warning(m_dialog_default_formatting);
  if(m_dialog_default_formatting)
  {
    add_view(m_dialog_default_formatting);
    m_dialog_default_formatting->signal_apply().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_field_definition_apply));
  }

  pack_start(m_AddDel);
  m_colName = m_AddDel.add_column(_("Name"));
  m_AddDel.prevent_duplicates(m_colName); //Don't allow adding of fields that already exist.
  m_AddDel.set_prevent_duplicates_warning(_("This field already exists. Please choose a different field name."));

  m_colTitle = m_AddDel.add_column(_("Title"));

  m_colType = m_AddDel.add_column(_("Type"), AddDelColumnInfo::enumStyles::Choices);
  m_AddDel.set_column_width(m_colType, 100); //TODO: Auto-size columns.


  //Setup the buttons:
  m_AddDel.set_edit_button_label(_("_Field Definition"));
  m_AddDel.set_extra_button_label(_("_Default Formatting"));

  //Set Type choices:

  Field::type_map_type_names mapFieldTypes = Field::get_usable_type_names();
  AddDel::type_vec_strings vecTypes;
  for(auto iter = mapFieldTypes.begin(); iter != mapFieldTypes.end();++iter)
  {
    const Glib::ustring& name = (*iter).second;
    vecTypes.emplace_back(name);
  }

  m_AddDel.set_column_choices(m_colType, vecTypes);

  m_colUnique = m_AddDel.add_column("Unique", AddDelColumnInfo::enumStyles::Boolean);
  m_colPrimaryKey = m_AddDel.add_column("Primary Key", AddDelColumnInfo::enumStyles::Boolean);

  //Connect signals:
  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_adddel_add));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_adddel_delete));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_adddel_changed));
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_adddel_edit));
  m_AddDel.signal_user_requested_extra().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_adddel_extra));

  //React to changes in the field properties:
  m_dialog_field_definition->signal_apply().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_field_definition_apply));
}

Box_DB_Table_Definition::~Box_DB_Table_Definition()
{
  if(m_dialog_field_definition)
  {
    remove_view(m_dialog_field_definition);
    delete m_dialog_field_definition;
  }

  if(m_dialog_default_formatting)
  {
    remove_view(m_dialog_default_formatting);
    delete m_dialog_default_formatting;
  }
}

void Box_DB_Table_Definition::fill_field_row(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const Field>& field)
{
  m_AddDel.set_value_key(iter, field->get_name());

  m_AddDel.set_value(iter, m_colName, field->get_name());

  const auto title = item_get_title(field);
  m_AddDel.set_value(iter, m_colTitle, title);

  //Type:
  //Field::glom_field_type fieldType = Field::get_glom_type_for_gda_type(field->get_field_info()->get_g_type()); //Could be INVALID if the gda type is not one of ours.
  // TODO: Why was this done by converting the field's gtype to a glom type
  // instead of using the glom type directly? This breaks numerical types in
  // sqlite which we store as double.
  Field::glom_field_type fieldType = field->get_glom_type();

  const auto strType = Field::get_type_name_ui( fieldType );
  m_AddDel.set_value(iter, m_colType, strType);

  //Unique:
  const auto bUnique = field->get_unique_key();

  m_AddDel.set_value(iter, m_colUnique, bUnique);

  //Primary Key:
  const auto bPrimaryKey = field->get_primary_key();
  m_AddDel.set_value(iter, m_colPrimaryKey, bPrimaryKey);
}

bool Box_DB_Table_Definition::fill_from_database()
{
  bool result = Box_DB_Table::fill_from_database();
  if(!result)
  {
    std::cerr << G_STRFUNC << ":  Box_DB_Table::fill_from_database() failed." << std::endl;
    return false;
  }

  if(!(ConnectionPool::get_instance()->get_ready_to_connect()))
    return false;

  fill_fields();

  try
  {
    //Fields:
    m_AddDel.remove_all();

    Field::type_map_type_names mapFieldTypes = Field::get_type_names_ui();

    for(const auto& field : m_vecFields)
    {
      //Name:
      auto tree_iter= m_AddDel.add_item(field->get_name());
      fill_field_row(tree_iter, field);
    }

    result = true;
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

  return result;
}

void Box_DB_Table_Definition::on_adddel_add(const Gtk::TreeModel::iterator& row)
{
  Glib::ustring name = m_AddDel.get_value(row, m_colName);
  if(!name.empty())
  {
    auto field = std::make_shared<Field>();
    field->set_name(name);
    field->set_title( Utils::title_from_string(name) , AppWindow::get_current_locale()); //Start with a title that might be useful.
    field->set_glom_type(Field::glom_field_type::NUMERIC);

    auto field_info = field->get_field_info();
    field_info->set_g_type( Field::get_gda_type_for_glom_type(Field::glom_field_type::NUMERIC) );
    field->set_field_info(field_info);

    //TODO: Warn about a delay before actually doing this when the backend
    //needs to recreate the whole table.
    const auto bTest = DbUtils::add_column(m_table_name, field, get_app_window()); //TODO: Get schema type for Field::glom_field_type::NUMERIC
    if(bTest)
    {
      //Store the generated title in the document:
      //on_adddel_changed(row, m_colTitle);

      // Don't call on_adddel_changed for this, since this does a lot of
      // unnecessary extra stuff just to get the field added into the
      // document:

      auto pDoc = std::static_pointer_cast<Document>(get_document());
      if(pDoc)
      {
        std::cout << Utils::to_utype(field->get_glom_type()) << std::endl;
        Document::type_vec_fields vecFields = pDoc->get_table_fields(m_table_name);
        vecFields.emplace_back(field);
        pDoc->set_table_fields(m_table_name, vecFields);
      }

      // Do this after having added the new field to the document, so
      // Base_DB::get_fields_for_table() can use it to compare the fields from
      // the database against.

      //Show the new field (fill in the other cells):

      //Update our list of database fields.
      fill_fields();

      //fill_from_database(); //We cannot change the structure in a cell renderer signal handler.

      fill_field_row(row, field);

      m_AddDel.select_item(field->get_name(), m_colName, false);

      //m_AddDel.select_item(row, m_colTitle, true); //Start editing the title
    }
  }
}

void Box_DB_Table_Definition::on_adddel_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd)
{
  auto iterAfterEnd = rowEnd;
  if(iterAfterEnd != m_AddDel.get_model()->children().end())
    ++iterAfterEnd;

  for(auto iter = rowStart; iter != iterAfterEnd; ++iter)
  {
    Glib::ustring name = m_AddDel.get_value_key(iter);
    if(!name.empty())
    {
      //TODO: Warn about a delay before actually doing this when the backend
      //needs to recreate the whole table.
      const auto test = DbUtils::drop_column(m_table_name, name);
      if(test)
      {
        //update_gda_metastore_for_table(m_table_name); // already done in drop_column().

        //Remove it from all layouts, reports, etc:
        get_document()->remove_field(m_table_name, name);
      }
      else
        std::cerr << G_STRFUNC << ": field deletion failed." << std::endl;
    }
  }

  fill_fields();
  fill_from_database();
}

bool Box_DB_Table_Definition::check_field_change(const std::shared_ptr<const Field>& field_old, const std::shared_ptr<const Field>& field_new)
{
  bool result = true; //Tells the caller whether to continue.

  auto parent_window = get_app_window();

  //Warn about a long slow recalculation, and offer the chance to cancel it:
  if(field_new->get_has_calculation())
  {
    if(field_new->get_calculation() != field_old->get_calculation())
    {
      //TODO: Only show this when there are > 100 records?
      Gtk::MessageDialog dialog(UiUtils::bold_message(_("Recalculation Required")), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
      dialog.set_secondary_text(_("You have changed the calculation used by this field so Glom must recalculate the value in all records. If the table contains many records then this could take a long time."));
      if(parent_window)
        dialog.set_transient_for(*parent_window);

      dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
      dialog.add_button(_("Recalculate"), Gtk::RESPONSE_OK);
      result = (dialog.run() == Gtk::RESPONSE_OK);
    }
  }

  //If we are changing a non-glom type:
  //Refuse to edit field definitions that were not created by glom:
  if(field_old->get_glom_type() == Field::glom_field_type::INVALID)
  {
    UiUtils::show_ok_dialog(_("Invalid database structure"),
      _("This database field was created or edited outside of Glom. It has a data type that is not supported by Glom. Your system administrator may be able to correct this."), parent_window, Gtk::MESSAGE_ERROR);

    return false;
  }

  //Refuse to have no primary key set:
  if(field_old->get_primary_key() && !field_new->get_primary_key()) //Was the primary key column unchecked?
  {
    UiUtils::show_ok_dialog(_("Primary key required"), 
      _("You may not unset the primary key because the table must have a primary key. You may set another field as the primary key instead."), parent_window, Gtk::MESSAGE_ERROR);

      return false;
  }


  //Setting a different primary key:
  if(field_new->get_primary_key() && !field_old->get_primary_key()) //Was the primary key column checked?
  {
    //Check for nulls:
    if(field_has_null_values(field_old)) //Use the fieldOld because we only use the name, and we want to check the _existing_ field:
    {
      UiUtils::show_ok_dialog(_("Field contains empty values."), _("The field may not yet be used as a primary key because it contains empty values."), parent_window, Gtk::MESSAGE_ERROR);

      //TODO: Offer to fill it in with generated ID numbers? That could give strange results if the existing values are human-readable.
      return false;
    }

    //Check that the values are unique:
    if(field_has_non_unique_values(field_old)) //Use the fieldOld because we only use the name, and we want to check the _existing_ field:
    {
      UiUtils::show_ok_dialog(_("Field contains non-unique values."), _("The field may not yet be used as a primary key because it contains values that are not unique."), parent_window, Gtk::MESSAGE_ERROR);
      return false;
    }

    //Ask the user to confirm this major change:
    Gtk::MessageDialog dialog(UiUtils::bold_message(_("Change primary key")), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
    dialog.set_secondary_text(_("Are you sure that you wish to set this field as the primary key, instead of the existing primary key?"));
    if(parent_window)
      dialog.set_transient_for(*parent_window);

    dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("Change Primary Key"), Gtk::RESPONSE_OK);
    if(dialog.run() != Gtk::RESPONSE_OK)
      return false; //Otherwise, continue, allowing the change.
  }

  //Refuse to change a field name to the same as an existing one:
  if( (field_new->get_name() != field_old->get_name()) &&
      (DbUtils::get_field_exists_in_database(m_table_name, field_new->get_name())) )
  {
    std::cout << "get_field_exists_in_database(" << m_table_name << ", " << field_new->get_name() << ") returned true" << std::endl;

    //Warn the user and refuse to make the change:
    UiUtils::show_ok_dialog(_("Field Name Already Exists"), 
      _("This field already exists. Please choose a different field name"), parent_window, Gtk::MESSAGE_ERROR);

    return false;
  }

  return result;
}

void Box_DB_Table_Definition::on_adddel_changed(const Gtk::TreeModel::iterator& row, guint /* col */)
{
  //Get old field definition:
  auto pDoc = std::static_pointer_cast<Document>(get_document());
  if(pDoc)
  {
    const auto strFieldNameBeingEdited = m_AddDel.get_value_key(row);

    auto constfield = pDoc->get_field(m_table_name, strFieldNameBeingEdited);
    m_Field_BeingEdited = constfield;

    //Get DB field info: (TODO: This might be unnecessary).
    const auto iterFind = find_if_same_name(m_vecFields, strFieldNameBeingEdited);
    if(iterFind == m_vecFields.end()) //If it was not found:
      std::cerr << G_STRFUNC << ": field not found: " << strFieldNameBeingEdited << std::endl;
    else
    {
      auto const_field_found = *iterFind;
      m_Field_BeingEdited = const_field_found;

      //Get new field definition:
      auto fieldNew = get_field_definition(row);

      //Change it:
      if(*m_Field_BeingEdited != *fieldNew) //If it has really changed.
      {
        const auto bcontinue = check_field_change(m_Field_BeingEdited, fieldNew);
        if(bcontinue)
        {
          auto fieldNewWithModifications = change_definition(m_Field_BeingEdited, fieldNew);

          //Update the row to show any extra changes (such as unique being set/unset whenever the primary key is set/unset) 
	  // TODO: When change_definition decides to unset another column from
	  // being primary key, then we also need to refill that row, so that
	  // the user interface does not show two primary keys.
          fill_field_row(row, fieldNewWithModifications);
        }
        else
        {
          //revert:
          fill_field_row(row, m_Field_BeingEdited);
        }
      }
    }
  }
}

void Box_DB_Table_Definition::on_adddel_edit(const Gtk::TreeModel::iterator& row)
{
  auto constfield = get_field_definition(row);
  m_Field_BeingEdited = constfield;

  m_dialog_field_definition->set_field(m_Field_BeingEdited, m_table_name);

  //m_dialog_field_definition->set_modified(false); //Disable [Apply] at start.

  auto parent_window = get_app_window();
  if(parent_window)
    m_dialog_field_definition->set_transient_for(*parent_window);

  m_dialog_field_definition->show();
}

void Box_DB_Table_Definition::on_adddel_extra(const Gtk::TreeModel::iterator& row)
{
  auto constfield = get_field_definition(row);
  m_Field_BeingEdited = constfield;

  m_dialog_default_formatting->set_field(m_Field_BeingEdited, m_table_name);

  //m_dialog_field_definition->set_modified(false); //Disable [Apply] at start.

  auto parent_window = get_app_window();
  if(parent_window)
    m_dialog_default_formatting->set_transient_for(*parent_window);

  m_dialog_default_formatting->show();
}

std::shared_ptr<Field> Box_DB_Table_Definition::get_field_definition(const Gtk::TreeModel::iterator& row)
{
  std::shared_ptr<Field> fieldResult;

  //Get old field definition (to preserve anything that the user doesn't have access to):

  const auto strFieldNameBeforeEdit = m_AddDel.get_value_key(row);

  //Glom field definition:
  auto pDoc = std::static_pointer_cast<Document>(get_document());
  if(pDoc)
  {
    Document::type_vec_fields vecFields= pDoc->get_table_fields(m_table_name);
    auto iterFind = find_if_same_name(vecFields, strFieldNameBeforeEdit);

    if((iterFind != vecFields.end()) && (*iterFind)) //If it was found:
    {
      fieldResult = glom_sharedptr_clone(*iterFind);
    }
    else
    {
      //Start with a default:
      fieldResult = std::make_shared<Field>();
    }
  }


  //DB field definition:

  //Start with original definitions, so that we preserve things like UNSIGNED.
  //TODO maybe use document's fieldinfo instead of m_vecFields.
  auto field_temp = 
    DbUtils::get_fields_for_table_one_field(pDoc, m_table_name, strFieldNameBeforeEdit);
  if(field_temp)
  {
    auto fieldInfo = field_temp->get_field_info()->copy();

    //Name:
    const auto name = m_AddDel.get_value(row, m_colName);
    fieldInfo->set_name(name);

    //Title:
    const auto title = m_AddDel.get_value(row, m_colTitle);
    fieldResult->set_title(title, AppWindow::get_current_locale());

    //Type:
    const auto strType = m_AddDel.get_value(row, m_colType);

    const Field::glom_field_type glom_type =  Field::get_type_for_ui_name(strType);
    GType fieldType = Field::get_gda_type_for_glom_type(glom_type);

    //Unique:
    const auto bUnique = m_AddDel.get_value_as_bool(row, m_colUnique);
    //TODO_gda: fieldInfo->set_unique_key(bUnique);

    //Primary Key:
    const auto bPrimaryKey = m_AddDel.get_value_as_bool(row, m_colPrimaryKey);
    ///TODO_gda: fieldInfo->set_primary_key(bPrimaryKey);

    fieldInfo->set_g_type(fieldType);
    // Reset old default value if the field type changes:
    if(fieldResult->get_glom_type() != glom_type)
      fieldInfo->set_default_value(Gnome::Gda::Value());

    //Put it together:
    fieldResult->set_glom_type(glom_type);
    fieldResult->set_field_info(fieldInfo);

    //TODO: Use the libgda functions above when we make them work:
    fieldResult->set_unique_key(bUnique);
    fieldResult->set_primary_key(bPrimaryKey);
  }

  return fieldResult;
}

void Box_DB_Table_Definition::on_field_definition_apply()
{
  auto field_New = m_dialog_field_definition->get_field();

  if(*m_Field_BeingEdited != *field_New)
  {
    const auto bcontinue = check_field_change(m_Field_BeingEdited, field_New);
    if(bcontinue)
    {
      change_definition(m_Field_BeingEdited, field_New);
      m_Field_BeingEdited = field_New;
    }

    //Update the list:
    fill_from_database();
  }

  m_dialog_field_definition->hide();
}

std::shared_ptr<Field> Box_DB_Table_Definition::change_definition(const std::shared_ptr<const Field>& fieldOld, const std::shared_ptr<const Field>& field)
{
  BusyCursor busy_cursor(get_app_window());

  //DB field definition:

  std::shared_ptr<Field> result;
  
  if(!fieldOld || !field)
    return result;

  type_vec_const_fields old_fields;
  type_vec_fields new_fields;

  if(fieldOld->get_primary_key() != field->get_primary_key())
  {
    //Note: We have already checked whether this change of primary key is likely to succeed.

    if(field->get_primary_key())
    {
      //Unset the current primary key:
      //(There should be one.)
      auto existing_primary_key = get_field_primary_key_for_table(m_table_name);
      if(existing_primary_key)
      {
        auto existing_primary_key_unset = glom_sharedptr_clone(existing_primary_key);
        existing_primary_key_unset->set_primary_key(false);
	old_fields.emplace_back(existing_primary_key);
	new_fields.emplace_back(existing_primary_key_unset);
      }
    }

    //Forget the remembered currently-viewed primary key value, 
    //because it will be useless with a different field as the primary key, or with no field as primary key:
    auto document = get_document();
    document->forget_layout_record_viewed(m_table_name);
  }

  old_fields.emplace_back(fieldOld);
  new_fields.emplace_back(glom_sharedptr_clone(field));

  //TODO: Warn about a delay, and possible loss of precision, before actually
  //changing types or when the backend needs to recreate the whole column or
  //table.
  // TODO: Don't call change_columns if only the field title has changed,
  // since the title is only stored in the document anyway.
  if(change_columns(m_table_name, old_fields, new_fields, get_app_window()))
  {
    result = new_fields.back();
  }
  else
  {
    //Give up. Don't update the document. Hope that we can read the current field properties from the database.
    fill_fields();
    //fill_from_database(); //We should not change the database definition in a cell renderer signal handler.

    //Select the same field again:
    m_AddDel.select_item(field->get_name(), m_colName, false);
    return glom_sharedptr_clone(old_fields.back());
  }

  //Extra Glom field definitions:
  auto pDoc = std::static_pointer_cast<Document>(get_document());
  if(pDoc)
  {
    //Get Table's fields:
    Document::type_vec_fields vecFields = pDoc->get_table_fields(m_table_name);

    for(unsigned int i = 0; i < old_fields.size(); ++ i)
    {
      //Find old field:
      const auto field_name_old = old_fields[i]->get_name();
      auto iterFind = find_if_same_name(vecFields, field_name_old);
      if(iterFind != vecFields.end()) //If it was found:
      {
        //Change it to the new Fields's value:
        *iterFind = glom_sharedptr_clone(new_fields[i]);
      }
      else
      {
        //Add it, because it's not there already:
        vecFields.emplace_back( glom_sharedptr_clone(new_fields[i]) );
      }

      // TODO_Performance: Can we do this at the end, after the loop? Or do
      // the following operations depend on this?
      pDoc->set_table_fields(m_table_name, vecFields);

      //Update field names where they are used in relationships or on layouts:
      if(field_name_old != new_fields[i]->get_name())
      {
        pDoc->change_field_name(m_table_name, field_name_old, new_fields[i]->get_name());
      }

      // TODO_Performance: Do we even need to do this if only the primary key
      // flag changed, such as for the first entry in the new_fields vector?
      //Recalculate if necessary:
      if(new_fields[i]->get_has_calculation())
      {
        const auto calculation = new_fields[i]->get_calculation();
        if(calculation != old_fields[i]->get_calculation())
          calculate_field_in_all_records(m_table_name, new_fields[i]);
      }
    }
  }

  //Update UI:

  fill_fields();
  //fill_from_database(); //We should not change the database definition in a cell renderer signal handler.

  //Select the same field again:
  m_AddDel.select_item(field->get_name(), m_colName, false);

  return result;
}

void Box_DB_Table_Definition::fill_fields()
{
  //std::cout << "DEBUG: Box_DB_Table_Definition::fill_fields()" << std::endl;

  //Update the fields (also checking the actual database):
  m_vecFields = DbUtils::get_fields_for_table(get_document(), m_table_name);
}

bool Box_DB_Table_Definition::field_has_null_values(const std::shared_ptr<const Field>& field)
{
  //Note that "= Null" doesn't work, though it doesn't error either.
  //Note also that SELECT COUNT always returns 0 if all the values are NULL, so we can't use that to be more efficient.
  auto builder =
    Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
  builder->select_add_field(field->get_name(), m_table_name);
  builder->select_add_target(m_table_name);
  builder->set_where(
    builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_ISNULL,
      builder->add_field_id(field->get_name(), m_table_name)));

  long null_count = 0;
  auto datamodel = DbUtils::query_execute_select(builder);
  if(datamodel)
  {
    if(datamodel->get_n_rows() && datamodel->get_n_columns())
    {
      null_count = datamodel->get_n_rows();
      //std::cout << "debug: null_count = " << null_count << std::endl;
    }
  }
  else
  {
    std::cerr << G_STRFUNC << ": query failed." << std::endl;
  }

  return null_count > 0; 
}

bool Box_DB_Table_Definition::field_has_non_unique_values(const std::shared_ptr<const Field>& field)
{
  long count_distinct = 0;
  long count_all = 0;

  //Count the distinct rows:  
  auto builder_query_distinct = Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_UPDATE);
  builder_query_distinct->select_set_distinct();
  builder_query_distinct->set_table(m_table_name);
  builder_query_distinct->select_add_field(field->get_name(), m_table_name);

  auto datamodel = DbUtils::query_execute_select(builder_query_distinct);
  if(datamodel)
  {
    count_distinct = datamodel->get_n_rows();
    //std::cout << "debug: count_distinct = " << count_distinct << std::endl;
  }
  else
  {
    std::cerr << G_STRFUNC << ": SELECT COUNT() failed." << std::endl;
  }

  //Count all rows, to compare. TODO_performance: Is there a more efficient way to do this? Maybe count(*), which apparently doesn't ignore NULL rows like count(somefield) would.
  auto builder_query_all =
    Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
  builder_query_all->select_add_field(field->get_name(), m_table_name);
  builder_query_all->select_add_target(m_table_name);
      
  datamodel = DbUtils::query_execute_select(builder_query_all);
  if(datamodel)
  {
    count_all = datamodel->get_n_rows();
    //std::cout << "debug: null_count = " << null_count << std::endl;
  }
  else
  {
    std::cerr << G_STRFUNC << ": SELECT COUNT() failed." << std::endl;
  }


  return count_distinct != count_all;
}


} //namespace Glom



