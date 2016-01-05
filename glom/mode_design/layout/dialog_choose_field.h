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

#ifndef GLOM_MODE_DESIGN_DIALOG_CHOOSE_FIELD_H
#define GLOM_MODE_DESIGN_DIALOG_CHOOSE_FIELD_H

#include <gtkmm/dialog.h>
#include <gtkmm/checkbutton.h>
#include <glom/utility_widgets/dialog_properties.h>
#include <libglom/document/document.h>
#include <glom/box_withbuttons.h>
#include <glom/mode_design/layout/combobox_relationship.h>
#include <glom/mode_design/comboentry_currency.h>

namespace Glom
{

class Dialog_ChooseField : public Gtk::Dialog
{
public:
  static const char* glade_id;
  static const bool glade_developer;
  
  Dialog_ChooseField(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  /**
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   * @param field The starting field information.
   */
  void set_document(Document* document, const Glib::ustring& table_name, const std::shared_ptr<const LayoutItem_Field>& field);
  void set_document(Document* document, const Glib::ustring& table_name);


  //void select_item(const std::shared_ptr<const Field>& field);

  std::shared_ptr<LayoutItem_Field> get_field_chosen() const;
  
  typedef std::list< std::shared_ptr<LayoutItem_Field> > type_list_field_items;
  type_list_field_items get_fields_chosen() const;

private:

  void on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* view_column);
  void on_treeview_selection_changed();
  void on_combo_relationship_changed();
  void on_checkbutton_related_relationships_toggled();

  //Tree model columns:
  class ModelColumns_Fields : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns_Fields()
    { add(m_col_name); add(m_col_title); add(m_col_field); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    Gtk::TreeModelColumn<Glib::ustring> m_col_title;
    Gtk::TreeModelColumn< std::shared_ptr<Field> > m_col_field;
  };

  ModelColumns_Fields m_ColumnsFields;

  ComboBox_Relationship* m_combo_relationship;
  Gtk::Button* m_button_select;
  Gtk::CheckButton* m_checkbutton_show_related_relationships;
  Gtk::TreeView* m_treeview;
  Glib::RefPtr<Gtk::ListStore> m_model;

  Glib::ustring m_table_name;
  std::shared_ptr<LayoutItem_Field> m_start_field; //stored so we can preserve extra information that's not changed here.

  Document* m_document;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_CHOOSE_FIELD_H
