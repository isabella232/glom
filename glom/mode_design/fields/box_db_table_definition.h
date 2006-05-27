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

#ifndef BOX_DB_TABLE_DEFINITION_H
#define BOX_DB_TABLE_DEFINITION_H

#include "../../box_db_table.h"
#include "dialog_fielddefinition.h"

namespace Glom
{

class Box_DB_Table_Definition : public Box_DB_Table
{
public: 
  Box_DB_Table_Definition();
  Box_DB_Table_Definition(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Box_DB_Table_Definition();

protected:
  void init(); //Avoid duplicating code in constructors.
  virtual bool fill_from_database();
  virtual void fill_fields();

  void fill_field_row(const Gtk::TreeModel::iterator& iter, const sharedptr<const Field>& field);

  sharedptr<Field> get_field_definition(const Gtk::TreeModel::iterator& row);

  virtual sharedptr<Field> change_definition(const sharedptr<const Field>& fieldOld, const sharedptr<const Field>& field);

  //Signal handlers:
  virtual void on_adddel_add(const Gtk::TreeModel::iterator& row);
  virtual void on_adddel_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd);
  virtual void on_adddel_changed(const Gtk::TreeModel::iterator& row, guint col);
  virtual void on_adddel_edit(const Gtk::TreeModel::iterator& row);

  virtual void on_Properties_apply();

  //Postgres needs some complex stuff:

  virtual sharedptr<Field> postgres_change_column(const sharedptr<const Field>& field_old, const sharedptr<const Field>& field);
  virtual void postgres_change_column_type(const sharedptr<const Field>& field_old, const sharedptr<const Field>& field);

  mutable AddDel_WithButtons m_AddDel; //mutable because its get_ methods aren't const.

  guint m_colName, m_colTitle, m_colType, m_colUnique, m_colPrimaryKey;

  Dialog_FieldDefinition* m_pDialog;
  sharedptr<const Field> m_Field_BeingEdited; //TODO_FieldShared
  type_vecFields m_vecFields;
};

} //namespace Glom

#endif
