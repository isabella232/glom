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

#ifndef GLOM_MODE_DATA_DIALOG_GROUPBY_SECONDARYFIELDS_HH
#define GLOM_MODE_DATA_DIALOG_GROUPBY_SECONDARYFIELDS_HH

#include "../base_db.h"
#include "../mode_data/dialog_layout.h"
#include <libglademm.h>

namespace Glom
{

class Dialog_GroupBy_SecondaryFields
 : public Dialog_Layout //It has some useful stuff
{
public:
  Dialog_GroupBy_SecondaryFields(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_GroupBy_SecondaryFields();


  void set_fields(const Glib::ustring& table_name, const LayoutGroup::type_map_items& table_fields);
  LayoutGroup::type_map_items get_fields() const;

protected:

  //Enable/disable buttons, depending on treeview selection:
  virtual void enable_buttons();

  //signal handlers:
  virtual void on_button_field_up();
  virtual void on_button_field_down();
  virtual void on_button_add_field();
  virtual void on_button_delete();
  virtual void on_button_edit_field();
  virtual void on_button_field_formatting();
  virtual void on_treeview_fields_selection_changed();
  virtual void on_cell_data_name(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);

  //Tree model columns:
  class ModelColumns_Fields : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns_Fields()
    { add(m_col_layout_item); add(m_col_sequence); }

    Gtk::TreeModelColumn< sharedptr<const LayoutItem_Field> > m_col_layout_item;
    Gtk::TreeModelColumn<guint> m_col_sequence;
  };

  ModelColumns_Fields m_ColumnsFields;

  //Tree model columns:
  Gtk::TreeView* m_treeview_fields;
  Gtk::Button* m_button_field_up;
  Gtk::Button* m_button_field_down;
  Gtk::Button* m_button_field_add;
  Gtk::Button* m_button_field_delete;
  Gtk::Button* m_button_field_edit;
  Gtk::Button* m_button_field_formatting;

  Glib::RefPtr<Gtk::ListStore> m_model_fields;

  Gtk::Label* m_label_table_name;
};

} //namespace Glom

#endif //GLOM_MODE_DATA_DIALOG_GROUPBY_SECONDARYFIELDS_HH
