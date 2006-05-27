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

#ifndef GLOM_MODE_DATA_DIALOG_LAYOUT_LIST_H
#define GLOM_MODE_DATA_DIALOG_LAYOUT_LIST_H

#include "dialog_layout.h"

namespace Glom
{

class Dialog_Layout_List : public Dialog_Layout
{
public:
  Dialog_Layout_List(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_Layout_List();

  /**
   * @param layout "list" or "details"
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   * @param table_fields: The actual fields in the table, in case the document does not yet know about them all.
   */
  virtual void set_document(const Glib::ustring& layout, Document_Glom* document, const Glib::ustring& table_name, const type_vecLayoutFields& table_fields);

protected:

  //Enable/disable buttons, depending on treeview selection:
  virtual void enable_buttons();

  virtual void save_to_document();

  //signal handlers:
  virtual void on_button_field_up();
  virtual void on_button_field_down();
  virtual void on_button_add_field();
  virtual void on_button_delete();
  virtual void on_button_edit_field();
  virtual void on_button_field_formatting();
  virtual void on_treeview_fields_selection_changed();
  virtual void on_cell_data_name(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);

  void warn_about_images();

  //Tree model columns:
  class ModelColumns_Fields : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns_Fields()
    { add(m_col_layout_item); add(m_col_sequence); }

    Gtk::TreeModelColumn< sharedptr<LayoutItem_Field> > m_col_layout_item;
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

#endif //GLOM_MODE_DATA_DIALOG_LAYOUT_LIST_H
