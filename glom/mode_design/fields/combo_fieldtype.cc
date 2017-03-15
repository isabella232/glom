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

#include <gtkmm/liststore.h>
#include "combo_fieldtype.h"
#include "../../box_db_table.h"

namespace Glom
{

Combo_FieldType::Combo_FieldType(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& /* builder */)
: Gtk::ComboBox(cobject)
{
  init();
}

Combo_FieldType::Combo_FieldType()
{
  init();
}

void Combo_FieldType::init()
{
  m_tree_model = Gtk::ListStore::create(m_columns);
  set_model(m_tree_model);

  //Set Type choices:
  for(const auto& the_pair : Field::get_usable_type_names())
  {
    auto iterModel = m_tree_model->append();
    if(iterModel)
    {
      auto row = *iterModel;
      row[m_columns.m_col_name] = the_pair.second;
      row[m_columns.m_col_type] = the_pair.first;

      //std::cout << "adding combo row: " << row[m_columns.m_col_name] << ", " << row[m_columns.m_col_type] << std::endl;
    }
  }

  pack_start(m_columns.m_col_name);  //Show the name, but hide the ID.


  //The value must be from the list, and it can't be empty:
  //set_value_in_list(true, false);
}

void Combo_FieldType::set_field_type(Field::glom_field_type fieldType)
{
   for(const auto& row : m_tree_model->children())
   {
     if( row[m_columns.m_col_type] == fieldType )
     {
       set_active(row);

       //TODO: What was this?
       //Glib::ustring temp = row[m_columns.m_col_name];
       //iter
     }
   }
}

Field::glom_field_type Combo_FieldType::get_field_type() const
{
  auto result = Field::glom_field_type::INVALID;

  //Get the active row:
  auto active_row  = get_active();
  if(active_row)
  {
    const auto row = *active_row;
    result = row[m_columns.m_col_type];

    // TODO: What was this?
    // Glib::ustring temp = row[m_columns.m_col_name];
  }

  return result;
}

} //namespace Glom
