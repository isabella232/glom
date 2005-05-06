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

#ifndef GLOM_DIALOG_LAYOUT_REPORT
#define GLOM_DIALOG_LAYOUT_REPORT

#include "mode_data/dialog_layout.h"

class Dialog_Layout_Report : public Dialog_Layout
{
public:
  Dialog_Layout_Report(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_Layout_Report();

  /**
   * @param layout "list" or "details"
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   * @param table_fields: The actual fields in the table, in case the document does not yet know about them all.
   */
  virtual void set_document(const Glib::ustring& layout, Document_Glom* document, const Glib::ustring& table_name, const type_vecLayoutFields& table_fields);

protected:

  class ModelColumnsGroups : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumnsGroups()
    { add(m_col_item); }

    Gtk::TreeModelColumn<LayoutItem*> m_col_item;
  };

  ModelColumnsGroups m_columns_parts;

  ModelColumnsGroups m_columns_available_parts;

  virtual void add_group(const Gtk::TreeModel::iterator& parent, const LayoutGroup& group);
  virtual void fill_group(const Gtk::TreeModel::iterator& iter, LayoutGroup& group);

  //Enable/disable buttons, depending on treeview selection:
  virtual void enable_buttons();

  virtual void save_to_document();

  bool offer_relationship_list(Relationship& relationship);
  bool offer_field_list(LayoutItem_Field& field);
  bool offer_field_layout(LayoutItem_Field& field);
  Gtk::TreeModel::iterator get_selected_group_parent() const;

  Gtk::TreeModel::iterator get_selected_available() const;

  //signal handlers:
  virtual void on_button_add();

  virtual void on_button_up();
  virtual void on_button_down();
  virtual void on_button_delete();

  virtual void on_button_edit();

  virtual void on_treeview_fields_selection_changed();

  void on_cell_data_part(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
  void on_cell_data_available_part(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);

  virtual void on_treeview_cell_edited_text(const Glib::ustring& path_string, const Glib::ustring& new_text, const Gtk::TreeModelColumn<Glib::ustring>& model_column);
  virtual void on_treeview_cell_edited_numeric(const Glib::ustring& path_string, const Glib::ustring& new_text, const Gtk::TreeModelColumn<guint>& model_column);

  Gtk::TreeView* m_treeview_parts;
  Gtk::TreeView* m_treeview_available_parts;

  Gtk::Button* m_button_up;
  Gtk::Button* m_button_down;
  Gtk::Button* m_button_add;
  Gtk::Button* m_button_delete;
  Gtk::Button* m_button_edit;
  Gtk::Label* m_label_table_name;
  Gtk::Entry* m_entry_name;
  Gtk::Entry* m_entry_title;

  Glib::RefPtr<Gtk::TreeStore> m_model_parts;
  Glib::RefPtr<Gtk::TreeStore> m_model_available_parts;
};

#endif //GLOM_DIALOG_LAYOUT_REPORT
