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

#ifndef GLOM_MODE_DESIGN_BOX_DB_TABLE_DEFINITION_H
#define GLOM_MODE_DESIGN_BOX_DB_TABLE_DEFINITION_H

#include <glom/box_db_table.h>
#include <glom/mode_design/fields/dialog_fielddefinition.h>
#include <glom/mode_design/fields/dialog_defaultformatting.h>

namespace Glom
{

class Box_DB_Table_Definition : public Box_DB_Table
{
public: 
  Box_DB_Table_Definition();
  Box_DB_Table_Definition(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Box_DB_Table_Definition();

private:
  void init(); //Avoid duplicating code in constructors.
  bool fill_from_database() override;
  virtual void fill_fields();

  void fill_field_row(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const Field>& field);

  std::shared_ptr<Field> get_field_definition(const Gtk::TreeModel::iterator& row);

  std::shared_ptr<Field> change_definition(const std::shared_ptr<const Field>& fieldOld, const std::shared_ptr<const Field>& field);

  bool field_has_null_values(const std::shared_ptr<const Field>& field);
  bool field_has_non_unique_values(const std::shared_ptr<const Field>& field);

  //Signal handlers:
  void on_adddel_add(const Gtk::TreeModel::iterator& row);
  void on_adddel_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd);
  void on_adddel_changed(const Gtk::TreeModel::iterator& row, guint col);
  void on_adddel_edit(const Gtk::TreeModel::iterator& row);
  void on_adddel_extra(const Gtk::TreeModel::iterator& row);

  void on_field_definition_apply();
  void on_default_formatting_apply();

  bool check_field_change(const std::shared_ptr<const Field>& field_old, const std::shared_ptr<const Field>& field_new);

  mutable AddDel_WithButtons m_AddDel; //mutable because its get_ methods aren't const.

  guint m_colName, m_colTitle, m_colType, m_colUnique, m_colPrimaryKey;

  Dialog_FieldDefinition* m_dialog_field_definition;
  std::shared_ptr<const Field> m_Field_BeingEdited; //TODO_FieldShared
  Dialog_DefaultFormatting* m_dialog_default_formatting;

  type_vec_fields m_vecFields;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_BOX_DB_TABLE_DEFINITION_H

