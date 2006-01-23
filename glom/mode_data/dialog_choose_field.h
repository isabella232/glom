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

#ifndef GLOM_MODE_DATA_DIALOG_CHOOSE_FIELD_H
#define GLOM_MODE_DATA_DIALOG_CHOOSE_FIELD_H

#include <gtkmm.h>
#include "../utility_widgets/dialog_properties.h"
#include "../document/document_glom.h"
#include "../box_db.h"
#include "../utility_widgets/combo_textglade.h"
#include "../utility_widgets/comboentry_currency.h"

class Dialog_ChooseField : public Gtk::Dialog
{
public:
  Dialog_ChooseField(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_ChooseField();

  /**
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   * @param field The starting field information.
   */
  virtual void set_document(Document_Glom* document, const Glib::ustring& table_name, const LayoutItem_Field& field);
  virtual void set_document(Document_Glom* document, const Glib::ustring& table_name);


  //void select_item(const sharedptr<const Field>& field);

  bool get_field_chosen(LayoutItem_Field& field) const;

protected:

  virtual void on_row_activated(const Gtk::TreePath& path, Gtk::TreeViewColumn* view_column);
  virtual void on_treeview_selection_changed();
  virtual void on_combo_relationship_changed();

  //Tree model columns:
  class ModelColumns_Fields : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns_Fields()
    { add(m_col_name); add(m_col_title); add(m_col_field); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    Gtk::TreeModelColumn<Glib::ustring> m_col_title;
    Gtk::TreeModelColumn< sharedptr<Field> > m_col_field;
  };

  ModelColumns_Fields m_ColumnsFields;

  Combo_TextGlade* m_combo_relationship;
  Gtk::Button* m_button_select;
  Gtk::CheckButton* m_checkbutton_editable;
  Gtk::TreeView* m_treeview;
  Glib::RefPtr<Gtk::ListStore> m_model;

  Glib::ustring m_table_name;

  Document_Glom* m_document;
};

#endif //GLOM_MODE_DATA_DIALOG_CHOOSE_FIELD_H
