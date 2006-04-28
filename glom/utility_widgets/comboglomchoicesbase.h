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

#ifndef GLOM_UTILITY_WIDGETS_COMBO_GLOM_CHOICESBASE_H
#define GLOM_UTILITY_WIDGETS_COMBO_GLOM_CHOICESBASE_H

#include <gtkmm.h>
#include <glom/libglom/data_structure/field.h>
#include "layoutwidgetfield.h"

class ComboGlomChoicesBase : public LayoutWidgetField
{
public:
  ///You must call set_layout_item() to specify the field type and formatting of the main column.
  explicit ComboGlomChoicesBase();

  ///You must call set_layout_item() to specify the field type and formatting of the main column.
  explicit ComboGlomChoicesBase(const sharedptr<LayoutItem_Field>& field_second);

  virtual ~ComboGlomChoicesBase();

  void set_choices(const FieldFormatting::type_list_values& list_values);

  typedef std::list< std::pair<Gnome::Gda::Value, Gnome::Gda::Value> > type_list_values_with_second;
  void set_choices_with_second(const type_list_values_with_second& list_values);

protected:
  void init();

  //Tree model columns:
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns()
    { add(m_col_first); add(m_col_second); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_first; //The data to choose - this must be text.
    Gtk::TreeModelColumn<Glib::ustring> m_col_second;
  };

  ModelColumns m_Columns;

  Glib::RefPtr<Gtk::ListStore> m_refModel;

  bool m_with_second;
  sharedptr<const LayoutItem_Field> m_layoutitem_second;
  //Gnome::Gda::Value m_value; //The last-stored value. We have this because the displayed value might be unparseable.
};

#endif //GLOM_UTILITY_WIDGETS_COMBO_GLOM_CHOICESBASE_H

