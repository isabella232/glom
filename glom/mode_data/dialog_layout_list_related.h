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

#ifndef GLOM_MODE_DATA_DIALOG_LAYOUT_LIST_RELATED_H
#define GLOM_MODE_DATA_DIALOG_LAYOUT_LIST_RELATED_H

#include "dialog_layout.h"
#include "../mode_design/fields/combo_textglade.h"

class Dialog_Layout_List_Related : public Dialog_Layout
{
public:
  Dialog_Layout_List_Related(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_Layout_List_Related();

  /**
   * @param layout "list" or "details"
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   * @param table_fields: The actual fields in the table, in case the document does not yet know about them all.
   */
  virtual void set_document(const Glib::ustring& layout, Document_Glom* document, const Glib::ustring& parent_table_name, const Glib::ustring& relationship_name);
  virtual void update_ui(bool including_relationships_list = true);

  virtual Glib::ustring get_relationship_name() const;

protected:

  //Enable/disable buttons, depending on treeview selection:
  virtual void enable_buttons();

  virtual void save_to_document();

  //signal handlers:
  virtual void on_button_field_up();
  virtual void on_button_field_down();
  virtual void on_button_add_field();
  virtual void on_button_delete();
  virtual void on_treeview_fields_selection_changed();
  virtual void on_combo_relationship_changed();

  //Tree model columns:
  class ModelColumns_Fields : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns_Fields()
    { add(m_col_name); add(m_col_relationship_name); add(m_col_sequence); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    Gtk::TreeModelColumn<Glib::ustring> m_col_relationship_name;
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
  Combo_TextGlade* m_combo_relationship_name;
  Relationship m_relationship;

  Glib::RefPtr<Gtk::ListStore> m_model_fields;
};

#endif //GLOM_MODE_DATA_DIALOG_LAYOUT_LIST_RELATED_H
