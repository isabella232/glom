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


#ifndef BASE_DB_TABLE_DATA_READONLY_H
#define BASE_DB_TABLE_DATA_READONLY_H

#include "base_db_table.h"
#include <libglom/data_structure/field.h>
#include <algorithm> //find_if used in various places.

namespace Glom
{

/** A base class some database functionality
 * for use with a specific database table, showing data from the table.
 */
class Base_DB_Table_Data_ReadOnly : public Base_DB_Table
{
public:
  Base_DB_Table_Data_ReadOnly();
  virtual ~Base_DB_Table_Data_ReadOnly();

  bool refresh_data_from_database();

protected:

  //TODO: Move these to Base_DB_Table_Data too?
  virtual std::shared_ptr<Field> get_field_primary_key() const = 0;

  //TODO: Document whether these get the primary key in an existing record,
  //or the primary key value as it is entered in the UI, even before a record exists,
  //or both.
  virtual Gnome::Gda::Value get_primary_key_value_selected() const = 0;
  virtual Gnome::Gda::Value get_primary_key_value(const Gtk::TreeModel::iterator& row) const = 0;

  FoundSet m_found_set;

  type_vec_fields m_TableFields; //A cache, so we don't have to repeatedly get them from the Document.
  type_vecConstLayoutFields m_FieldsShown; //And any extra keys needed by shown fields. //TODO: Move to the non-read-only class?
};

} //namespace Glom

#endif //BASE_DB_TABLE__DATA_READONLY_H
