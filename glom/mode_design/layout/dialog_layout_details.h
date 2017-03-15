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

#ifndef GLOM_MODE_DESIGN_DIALOG_LAYOUT_DETAILS_H
#define GLOM_MODE_DESIGN_DIALOG_LAYOUT_DETAILS_H

#include <glom/mode_design/layout/dialog_layout.h>
#include <glom/mode_design/layout/treestore_layout.h>
#include <gtkmm/frame.h>
#include <gtkmm/spinbutton.h>

namespace Glom
{

class Dialog_Layout_Details : public Dialog_Layout
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_Layout_Details(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  /**
   * @param layout_name "list" or "details"
   * @param layout_platform As in the document. Empty or "maemo".
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   * @param table_fields: The actual fields in the table, in case the document does not yet know about them all.
   */
  void init(const Glib::ustring& layout_name, const Glib::ustring& layout_platform, const std::shared_ptr<Document>& document, const Glib::ustring& table_name, const type_vecConstLayoutFields& table_fields) override;

protected:

  void add_group(const Gtk::TreeModel::iterator& parent, const std::shared_ptr<const LayoutGroup>& group);
  void fill_group(const Gtk::TreeModel::iterator& iter, std::shared_ptr<LayoutGroup>& group);

  //Enable/disable buttons, depending on treeview selection:
  void enable_buttons() override;

  void save_to_document() override;

  std::shared_ptr<Relationship> offer_relationship_list();
  std::shared_ptr<Relationship> offer_relationship_list(const std::shared_ptr<const Relationship>& relationship);

  Gtk::TreeModel::iterator get_selected_group_parent() const;
  std::shared_ptr<LayoutItem_Button> offer_button_script_edit(const std::shared_ptr<const LayoutItem_Button>& button);

  /** Get the table that the fields belong to.
   * This is usually the regular table name (m_table_name),
   * but for related records portals (Dialog_Layout_List_Related),
   * it's the to table of the relationship.
   */
  virtual Glib::ustring get_fields_table() const;

  //signal handlers:
  void on_button_up();
  void on_button_down();
  void on_button_field_delete();
  virtual void on_button_add_field(); //overridden in derived class.
  void on_button_add_group();
  void on_button_add_notebook();
  void on_button_add_related();
  void on_button_add_related_calendar();
  void on_button_add_button();
  void on_button_add_text();
  void on_button_add_image();
  void on_button_formatting();
  virtual void on_button_edit(); //overridden in derived class
  void on_treeview_fields_selection_changed();

  void on_cell_data_name(Gtk::CellRenderer* renderer, const Gtk::TreeModel::const_iterator& iter);
  void on_cell_data_title(Gtk::CellRenderer* renderer, const Gtk::TreeModel::const_iterator& iter);
  void on_cell_data_group_columns(Gtk::CellRenderer* renderer, const Gtk::TreeModel::const_iterator& iter);
  void on_cell_data_column_width(Gtk::CellRenderer* renderer, const Gtk::TreeModel::const_iterator& iter);

  void on_treeview_cell_edited_name(const Glib::ustring& path_string, const Glib::ustring& new_text);
  void on_treeview_cell_edited_title(const Glib::ustring& path_string, const Glib::ustring& new_text);
  void on_treeview_cell_edited_group_columns(const Glib::ustring& path_string, const Glib::ustring& new_text);
  void on_treeview_cell_edited_column_width(const Glib::ustring& path_string, const Glib::ustring& new_text);

  Gtk::TreeModel::iterator append_appropriate_row();

  Gtk::TreeView* m_treeview_fields;
  Gtk::TreeView::Column* m_treeview_column_title;
  Gtk::TreeView::Column* m_treeview_column_group_columns;
  Gtk::TreeView::Column* m_treeview_column_column_width;

  // Only one of these boxes should be shown:
  Gtk::Box* m_box_table_widgets;
  Gtk::Box* m_box_related_table_widgets;
  Gtk::Frame* m_box_related_navigation;
  Gtk::Frame* m_box_frame_lines;

  Gtk::Button* m_button_up;
  Gtk::Button* m_button_down;
  Gtk::Button* m_button_add_field;
  Gtk::Button* m_button_add_group;
  Gtk::Button* m_button_add_notebook;
  Gtk::Button* m_button_add_related;
  Gtk::Button* m_button_add_related_calendar;
  Gtk::Button* m_button_add_button;
  Gtk::Button* m_button_add_text;
  Gtk::Button* m_button_add_image;
  Gtk::Button* m_button_field_delete;
  Gtk::Button* m_button_formatting;
  Gtk::Button* m_button_edit;
  Gtk::Label* m_label_table_name;

  Gtk::Box* m_hbox_rows_count;
  Gtk::SpinButton* m_spinbutton_rows_count_min;
  Gtk::SpinButton* m_spinbutton_rows_count_max;

  Glib::RefPtr<TreeStore_Layout> m_model_items;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_LAYOUT_DETAILS_H
