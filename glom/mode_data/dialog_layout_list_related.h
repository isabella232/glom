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
#include "../utility_widgets/combo_textglade.h"
#include "../combobox_relationship.h"

namespace Glom
{

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
  virtual void set_document(const Glib::ustring& layout, Document_Glom* document, const sharedptr<const LayoutItem_Portal>& portal);
  virtual void update_ui(bool including_relationships_list = true);

  sharedptr<Relationship> get_relationship() const;
  sharedptr<LayoutItem_Portal>  get_portal_layout();

protected:

  //Enable/disable buttons, depending on treeview selection:
  virtual void enable_buttons();

  virtual void save_to_document();

  //signal handlers:
  void on_button_field_up();
  void on_button_field_down();
  void on_button_add_field();
  void on_button_delete();
  void on_button_edit_field();
  void on_button_field_formatting();
  void on_treeview_fields_selection_changed();
  void on_combo_relationship_changed();
  void on_cell_data_name(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
  
  void on_combo_navigation_specific_changed();

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

  ComboBox_Relationship* m_combo_relationship;
  sharedptr<LayoutItem_Portal> m_portal;

  Gtk::RadioButton* m_radio_navigation_automatic;
  Gtk::RadioButton* m_radio_navigation_specify;
  Gtk::Label* m_label_navigation_automatic;
  ComboBox_Relationship* m_combo_navigation_specify;

  Glib::RefPtr<Gtk::ListStore> m_model_fields;
};

} //namespace Glom

#endif //GLOM_MODE_DATA_DIALOG_LAYOUT_LIST_RELATED_H
