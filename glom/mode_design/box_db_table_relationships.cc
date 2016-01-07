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

//#include <gtkmm/builder.h>
#include "box_db_table_relationships.h"
#include <glom/appwindow.h>
#include <libglom/db_utils.h>
#include <algorithm>
#include <glibmm/i18n.h>

namespace Glom
{

Box_DB_Table_Relationships::Box_DB_Table_Relationships()
{
  init();
}

Box_DB_Table_Relationships::Box_DB_Table_Relationships(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Box_DB_Table(cobject, builder)
{
  init();
}

void Box_DB_Table_Relationships::init()
{
  pack_start(m_AddDel);
  m_colName = m_AddDel.add_column(_("Name"));
  m_AddDel.prevent_duplicates(m_colName); //Don't allow a relationship to be added twice.
  m_AddDel.set_prevent_duplicates_warning(_("This relationship already exists. Please choose a different relationship name"));

  m_colTitle = m_AddDel.add_column(_("Title"));

  //Translators: FROM as in SQL's FROM
  m_colFromField = m_AddDel.add_column(_("From Field"), AddDelColumnInfo::enumStyles::Choices);
  m_colToTable = m_AddDel.add_column(_("Table"), AddDelColumnInfo::enumStyles::Choices);
  m_colToField = m_AddDel.add_column(_("To Field"), AddDelColumnInfo::enumStyles::Choices);
  m_colAllowEdit = m_AddDel.add_column(_("Allow Editing"),  AddDelColumnInfo::enumStyles::Boolean);
  m_colAutoCreate = m_AddDel.add_column(_("Automatic Creation"),  AddDelColumnInfo::enumStyles::Boolean);

  m_colTitleSingular = m_AddDel.add_column(_("Title (Singular Form)"));


  //Connect signals:
  m_AddDel.signal_user_activated().connect(sigc::mem_fun(*this, &Box_DB_Table_Relationships::on_adddel_user_activated));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_DB_Table_Relationships::on_adddel_user_changed));
  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_DB_Table_Relationships::on_adddel_user_added));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_DB_Table_Relationships::on_adddel_user_requested_delete));

  show_all_children();
}

bool Box_DB_Table_Relationships::fill_from_database()
{
  BusyCursor busy_cursor(get_app_window());

  bool result = Box_DB_Table::fill_from_database();

  const auto document = get_document();
  if(!document)
    return false;

  //Get relationships from the document:
  const auto vecRelationships = document->get_relationships(m_table_name);

  m_AddDel.remove_all();

  auto sharedconnection = connect_to_server(get_app_window());
  if(sharedconnection)
  {
    auto connection = sharedconnection->get_gda_connection();

    //Set combo choices:
    m_AddDel.set_column_choices(m_colFromField, util_vecStrings_from_Fields(
      DbUtils::get_fields_for_table(document, m_table_name)));

    type_vec_strings vecTableNames = document->get_table_names(false /* ignore_system_tables */);
    type_vec_strings vecTableNames_ustring(vecTableNames.begin(), vecTableNames.end());
    m_AddDel.set_column_choices(m_colToTable, vecTableNames_ustring);

    //To Field choices are different for each row: set in on_adddel_signal_user_activated.

    //Add the relationships:
    for(const auto& relationship : vecRelationships)
    {
      if(relationship)
      {
        //Name:
        auto iterTree = m_AddDel.add_item(relationship->get_name());
        m_AddDel.set_value(iterTree, m_colName, relationship->get_name());

        //Title:
        m_AddDel.set_value(iterTree, m_colTitle, item_get_title(relationship));
        m_AddDel.set_value(iterTree, m_colTitleSingular, relationship->get_title_singular(AppWindow::get_current_locale()));

        //From Field:
        m_AddDel.set_value(iterTree, m_colFromField, relationship->get_from_field());

        //To Table:
        const auto strToTable = relationship->get_to_table();
        m_AddDel.set_value(iterTree, m_colToTable, strToTable);

        //To Field:
        m_AddDel.set_value(iterTree, m_colToField, relationship->get_to_field());

        m_AddDel.set_value(iterTree, m_colAllowEdit, relationship->get_allow_edit());
        m_AddDel.set_value(iterTree, m_colAutoCreate, relationship->get_auto_create());
      }
    }
  }

  return result;
}

void Box_DB_Table_Relationships::save_to_document()
{
  //Build relationships from AddDel:
  Document::type_vec_relationships vecRelationships;

  const auto document = get_document();

  for(const auto& item : m_AddDel.get_model()->children())
  {
    const auto old_name = m_AddDel.get_value_key(item);

    const auto name = m_AddDel.get_value(item, m_colName);
    if(!name.empty())
    {
      //If it is a rename:
      if(!old_name.empty() && (old_name != name))
        get_document()->change_relationship_name(m_table_name, old_name, name); //Update layouts and reports.

      if(find_if_same_name_exists(vecRelationships, name)) //Don't add 2 relationships with the same name.
      {
        auto relationship = document->get_relationship(m_table_name, name); //Preserve other information, such as translations.
        if(!relationship)
          relationship = std::make_shared<Relationship>();

        relationship->set_name(name);
        relationship->set_title(m_AddDel.get_value(item, m_colTitle), AppWindow::get_current_locale());
        relationship->set_title_singular(m_AddDel.get_value(item, m_colTitleSingular), AppWindow::get_current_locale());
        relationship->set_from_table(m_table_name);
        relationship->set_from_field(m_AddDel.get_value(item, m_colFromField));
        relationship->set_to_table(m_AddDel.get_value(item, m_colToTable));
        relationship->set_to_field(m_AddDel.get_value(item, m_colToField));
        relationship->set_allow_edit(m_AddDel.get_value_as_bool(item, m_colAllowEdit));
        relationship->set_auto_create(m_AddDel.get_value_as_bool(item, m_colAutoCreate));

        vecRelationships.push_back(relationship);
      }
    }
  }

  //Update the Document with these relationships.
  get_document()->set_relationships(m_table_name, vecRelationships);

  //Call base:
  Box_DB_Table::save_to_document();
}

void Box_DB_Table_Relationships::on_adddel_user_added(const Gtk::TreeModel::iterator& row)
{
  const guint col_with_first_value = m_colName;

  if(col_with_first_value == m_colName)
  {
    //The name is the key:
    Glib::ustring new_name = m_AddDel.get_value(row, m_colName);
    if(!new_name.empty())
      m_AddDel.set_value_key(row, new_name);

    //Set a suitable starting title, if there is none already:
    Glib::ustring title = m_AddDel.get_value(row, m_colTitle);
    if(title.empty())
    {
      title = Utils::title_from_string(new_name);
      m_AddDel.set_value(row, m_colTitle, title);
    }
  }
}

void Box_DB_Table_Relationships::on_adddel_user_changed(const Gtk::TreeModel::iterator& row, guint col)
{
  if(col == m_colName)
  {
    //The name is the key, so If the name has been changed then the key must change too.
    Glib::ustring new_name = m_AddDel.get_value(row, m_colName);
    //if(!new_name.empty()) //Don't do this, so we can use the key as the old name to detect a rename later.
    //  m_AddDel.set_value_key(row, new_name);

    //Update the title, if there is none already:
    Glib::ustring title = m_AddDel.get_value(row, m_colTitle);
    if(title.empty())
    {
      title = Utils::title_from_string(new_name);
      m_AddDel.set_value(row, m_colTitle, title);
    }
  }
  else if(col == m_colToTable)
  {
    //User chose a new table so we need to wipe the field
    //because it might not be a valid field for that table:
    m_AddDel.set_value(row, m_colToField, Glib::ustring("")); //Plain "" goes to bool override.
  }

  set_modified();
}

void Box_DB_Table_Relationships::on_adddel_user_activated(const Gtk::TreeModel::iterator& row, guint col)
{
  if(col == m_colToField)
  {
    BusyCursor busy_cursor(get_app_window());

    const auto old_to_field = m_AddDel.get_value(row, m_colToField);

    const auto table_name = m_AddDel.get_value(row, m_colToTable);

    if(!table_name.empty())
    {
      //Set list of 'To' fields depending on table:
      m_AddDel.set_value(row, m_colToField, Glib::ustring(""));

      auto sharedconnection = connect_to_server(get_app_window());
      if(sharedconnection)
      {
        auto connection = sharedconnection->get_gda_connection();

        auto document = get_document();
        type_vec_strings vecFields = util_vecStrings_from_Fields(DbUtils::get_fields_for_table(document, table_name));

        //This would cause a lot of tedious re-filling:
        //m_AddDel.set_column_choices(m_colToField, vecFields);
        //fill_from_database();

        m_AddDel.set_column_choices(m_colToField, vecFields);

        //Attempt to set the previous value:
        m_AddDel.set_value(row, m_colToField, old_to_field);
      }
    }

  }
}

void Box_DB_Table_Relationships::on_adddel_user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& /* rowEnd */)
{
  //auto iterAfter = rowEnd;
  //++iterAfter;

  //for(auto iter = rowStart; iter != iterAfter; ++iter)
  //{ //AddDel::remove_item() can't cope with this.
    auto iter = rowStart;
    const auto relationship_name = m_AddDel.get_value_key(iter);

    //Remove the row:
    m_AddDel.remove_item(iter);

    //Remove it from any layouts, reports, field lookups, etc:
    auto document = get_document();
    if(document)
    {
      auto relationship = document->get_relationship(m_table_name, relationship_name);
      if(relationship)
      {
        document->remove_relationship(relationship);
      }
    }
  //}

  set_modified();
}

} //namespace Glom
