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

#include "dialog_add_related_table.h"
#include <glom/frame_glom.h> //For show_ok_dialog.h
#include <libglom/db_utils.h>
#include <libglom/utils.h>
#include <libglom/string_utils.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_AddRelatedTable::glade_id("dialog_add_related_table");
const bool Dialog_AddRelatedTable::glade_developer(true);

Dialog_AddRelatedTable::Dialog_AddRelatedTable(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  Base_DB(),
  m_entry_table_name(nullptr),
  m_entry_relationship_name(nullptr),
  m_combo_from_field(nullptr),
  m_button_edit_fields(nullptr),
  m_button_ok(nullptr)
{
  builder->get_widget("entry_related_table_name", m_entry_table_name);
  builder->get_widget("entry_relationship_name", m_entry_relationship_name);
  builder->get_widget("combobox_from_key", m_combo_from_field);

  //Connect signals:
  m_combo_from_field->signal_changed().connect( sigc::mem_fun(*this, &Dialog_AddRelatedTable::on_combo_field_name) );
  m_entry_table_name->signal_changed().connect( sigc::mem_fun(*this, &Dialog_AddRelatedTable::on_entry_table_name) );


  builder->get_widget("button_edit_fields", m_button_edit_fields);
  builder->get_widget("button_ok", m_button_ok);

  m_button_edit_fields->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_AddRelatedTable::on_button_edit_fields) );

}

void Dialog_AddRelatedTable::get_input(Glib::ustring& table_name, Glib::ustring& relationship_name, Glib::ustring& from_key_name)
{
  table_name = m_entry_table_name->get_text();
  relationship_name = m_entry_relationship_name->get_text();
  from_key_name = m_combo_from_field->get_active_text();
}

void Dialog_AddRelatedTable::set_fields(const Glib::ustring& table_name)
{
  m_table_name = table_name;

  const auto fields = DbUtils::get_fields_for_table_from_database(table_name);

  //Show the fields:
  m_combo_from_field->remove_all();
  for(const auto& item : fields)
  {
     if(item)
       m_combo_from_field->append(item->get_name());
  }

}

void Dialog_AddRelatedTable::on_entry_table_name()
{
  const auto table_name = m_entry_table_name->get_text();
  if(table_name.empty())
    return;

  //Guess a possible relationship name, based on the table name:
  Glib::ustring possible_relationship_name = table_name;

  //Discover whether a table with this name exists already,
  //and append a numerical prefix until we find one that doesn't exist.
  //TODO: A numerical prefix would look pretty stupid, but I suppose must do something. murrayc
  bool exists_already = true;
  Glib::ustring name_to_try = possible_relationship_name;
  int suffix_number = 1;
  while(exists_already)
  {
    if(!get_relationship_exists(m_table_name, name_to_try))
       exists_already = false; //Stop the while loop.
    else
    {
      //Append a numeric suffix and try again:
      name_to_try = possible_relationship_name + Utils::string_from_decimal(suffix_number);
      ++suffix_number;
    }
  }

  possible_relationship_name = name_to_try; //We found an unused relationship name.

  m_entry_relationship_name->set_text(possible_relationship_name);
}

void Dialog_AddRelatedTable::on_combo_field_name()
{
  const auto field_name = m_combo_from_field->get_active_text();
  if(field_name.empty())
    return;

  //Guess a possible related table name, based on the from key:
  Glib::ustring possible_table_name = field_name;

  //If the field has an _id prefix then remove that:
  possible_table_name = Utils::string_remove_suffix(possible_table_name, "_id", false /* not case sensitive */);

  //Add a "s" to the end, though this probably only makes sense in English. TODO: Don't do this?
  possible_table_name += 's';

  //Discover whether a table with this name exists already,
  //and append a numerical prefix until we find one that doesn't exist:
  bool exists_already = true;
  Glib::ustring name_to_try = possible_table_name;
  int suffix_number = 1;
  while(exists_already)
  {
    if(!DbUtils::get_table_exists_in_database(name_to_try))
       exists_already = false; //Stop the while loop.
    else
    {
      //Append a numeric suffix and try again:
      name_to_try = possible_table_name + Utils::string_from_decimal(suffix_number);
      ++suffix_number;
    }
  }

  possible_table_name = name_to_try; //We found an unused table name.

  m_entry_table_name->set_text(possible_table_name);
}

void Dialog_AddRelatedTable::on_button_edit_fields()
{
  m_signal_request_edit_fields.emit();
}

Dialog_AddRelatedTable::type_signal_request_edit_fields Dialog_AddRelatedTable::signal_request_edit_fields()
{
  return m_signal_request_edit_fields;
}



} //namespace Glom
