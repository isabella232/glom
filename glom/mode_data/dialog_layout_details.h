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

#ifndef GLOM_MODE_DATA_DIALOG_LAYOUT_DETAILS_H
#define GLOM_MODE_DATA_DIALOG_LAYOUT_DETAILS_H

#include "dialog_layout.h"
#include "treestore_layout.h"

class Dialog_Layout_Details : public Dialog_Layout
{
public:
  Dialog_Layout_Details(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_Layout_Details();

  /**
   * @param layout "list" or "details"
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   * @param table_fields: The actual fields in the table, in case the document does not yet know about them all.
   */
  virtual void set_document(const Glib::ustring& layout, Document_Glom* document, const Glib::ustring& table_name, const type_vecFields& table_fields);

protected:

  virtual void add_group(const Gtk::TreeModel::iterator& parent, const LayoutGroup& group);
  virtual void fill_group(const Gtk::TreeModel::iterator& iter, LayoutGroup& group);

   
  //Enable/disable buttons, depending on treeview selection:
  virtual void enable_buttons();

  virtual void save_to_document();

  bool offer_relationship_list(Relationship& relationship);
  bool offer_field_list(Field& field);
  Gtk::TreeModel::iterator get_selected_group_parent() const;
  
  //signal handlers:
  virtual void on_button_field_up();
  virtual void on_button_field_down();
  virtual void on_button_field_delete();
  virtual void on_button_field_add();  
  virtual void on_button_field_add_group();
  virtual void on_button_add_related();
  virtual void on_button_field_edit();  
  virtual void on_treeview_fields_selection_changed();

  virtual void on_cell_data_name(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
  virtual void on_cell_data_title(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
  virtual void on_cell_data_columns_count(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);

  virtual void on_treeview_cell_edited_text(const Glib::ustring& path_string, const Glib::ustring& new_text, const Gtk::TreeModelColumn<Glib::ustring>& model_column);
  virtual void on_treeview_cell_edited_numeric(const Glib::ustring& path_string, const Glib::ustring& new_text, const Gtk::TreeModelColumn<guint>& model_column);

       
  Gtk::TreeView* m_treeview_fields;
  Gtk::Button* m_button_field_up;
  Gtk::Button* m_button_field_down;
  Gtk::Button* m_button_field_add;
  Gtk::Button* m_button_field_add_group;
  Gtk::Button* m_button_add_related;
  Gtk::Button* m_button_field_delete;
  Gtk::Button* m_button_field_edit;

  Glib::RefPtr<TreeStore_Layout> m_model_items;
};

#endif //GLOM_MODE_DATA_DIALOG_LAYOUT_DETAILS_H
