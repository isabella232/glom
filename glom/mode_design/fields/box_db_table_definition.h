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

/**
  *@author Murray Cumming
  */


class Box_DB_Table_Definition : public Box_DB_Table
{
public: 
  Box_DB_Table_Definition();
  Box_DB_Table_Definition(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Box_DB_Table_Definition();

protected:
  void init(); //Avoid duplicating code in constructors.
  virtual void fill_from_database();
  virtual void fill_fields();

  Field get_field_definition(guint row);

  virtual void change_definition(const Field& fieldOld, Field field);

  //Signal handlers:
  virtual void on_AddDel_add(guint row);
  virtual void on_AddDel_delete(guint rowStart, guint rowEnd);
  virtual void on_AddDel_changed(guint row, guint col);
  virtual void on_AddDel_edit(guint row);

  virtual void on_Properties_apply();

  //Postgres needs some complex stuff:

    /** @param not_extras If this is true, then do not set extra details, such as NOT NULL. You should do that later, when you are ready.
   */
  virtual void postgres_add_column(const Field& field, bool not_extras = false);
  virtual void postgres_change_column(const Field& field_old, const Field& field);
  virtual void postgres_change_column_type(const Field& field_old, const Field& field);
  
  /** @param set_anyway If this is true, then set the extra details even if @field_old has the same properties.
   */
  virtual void postgres_change_column_extras(const Field& field_old, const Field& field, bool set_anyway = false);
  
  mutable AddDel_WithButtons m_AddDel; //mutable because its get_ methods aren't const.
 
  guint m_colName, m_colTitle, m_colType, m_colUnique, m_colPrimaryKey;

  Dialog_FieldDefinition* m_pDialog;
  Field m_Field_BeingEdited;
};

#endif
