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

#ifndef GLOM_MODE_DESIGN_COMBO_FIELDTYPE_H
#define GLOM_MODE_DESIGN_COMBO_FIELDTYPE_H

#include <libglom/data_structure/field.h>
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/builder.h>

namespace Glom
{

class Combo_FieldType : public Gtk::ComboBox
{
public:
  Combo_FieldType();
  Combo_FieldType(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  //set/get the text in terms of enumerated type:
  void set_field_type(Field::glom_field_type fieldType);
  Field::glom_field_type get_field_type() const;

private:

  void init();

  //Tree model columns:
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns()
    { add(m_col_name); add(m_col_type); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    Gtk::TreeModelColumn<Field::glom_field_type> m_col_type;
  };

  ModelColumns m_columns;

  Glib::RefPtr<Gtk::ListStore> m_tree_model;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_COMBO_FIELDTYPE_H
