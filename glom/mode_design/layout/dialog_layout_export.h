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

#ifndef GLOM_MODE_DESIGN_DIALOG_LAYOUT_EXPORT_H
#define GLOM_MODE_DESIGN_DIALOG_LAYOUT_EXPORT_H

#include <glom/mode_design/layout/dialog_layout.h>

namespace Glom
{

class Dialog_Layout_Export : public Dialog_Layout
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_Layout_Export(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_Layout_Export();

  /**
   * @param mapGroups The initial layout. (Note: This is only non-const for performance, to avoid an extra copy. This update with full field information.
   * @param document The document, so that the dialog can get extra information.
   * @param table_name The table name.
   * @param table_fields: The actual fields in the table, in case the document does not yet know about them all.
   */
  virtual void set_layout_groups(Document::type_list_layout_groups& mapGroups, Document* document, const Glib::ustring& table_name);

  void get_layout_groups(Document::type_list_layout_groups& layout_groups) const;

private:

  //Enable/disable buttons, depending on treeview selection:
  void enable_buttons() override;

  //signal handlers:
  virtual void on_button_up();
  virtual void on_button_down();
  virtual void on_button_add_field();
  virtual void on_button_delete();
  virtual void on_button_edit_field();
  virtual void on_treeview_fields_selection_changed();
  virtual void on_cell_data_name(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);

  //Tree model columns:
  class ModelColumns_Fields : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns_Fields()
    { add(m_col_layout_item); add(m_col_sequence); }

    Gtk::TreeModelColumn< std::shared_ptr<LayoutItem_Field> > m_col_layout_item;
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

  Glib::RefPtr<Gtk::ListStore> m_model_fields;

  Gtk::Label* m_label_table_name;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_LAYOUT_EXPORT_H
