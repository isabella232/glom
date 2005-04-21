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

#ifndef GLOM_MODE_DATA_DIALOG_FIELD_LAYOUT_H
#define GLOM_MODE_DATA_DIALOG_FIELD_LAYOUT_H

#include <gtkmm.h>
#include "../utility_widgets/dialog_properties.h"
#include "../document/document_glom.h"
#include "../box_db.h"
#include "../utility_widgets/combo_textglade.h"
#include "../utility_widgets/comboentry_currency.h"

class Dialog_FieldLayout
 : public Gtk::Dialog,
   public View_Composite_Glom //Give it access to the document.
{
public:
  Dialog_FieldLayout(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_FieldLayout();

  /**
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param field The starting field information.
   * @param table_name The field's table.
   */
  virtual void set_field(const LayoutItem_Field& field, const Glib::ustring& table_name);

  //void select_item(const Field& field);

  bool get_field_chosen(LayoutItem_Field& field) const;

protected:
  //Signal handlers:
  void on_combo_choices_relationship_changed();

  Gtk::Label* m_label_field_name;
  Gtk::CheckButton* m_checkbutton_editable;

  Gtk::Frame* m_frame_numeric_format;
  Gtk::CheckButton* m_checkbox_format_use_thousands;
  Gtk::CheckButton* m_checkbox_format_use_decimal_places;
  Gtk::Entry* m_entry_format_decimal_places;
  ComboEntry_Currency* m_entry_currency_symbol;

  Gtk::Frame* m_frame_text_format;
  Gtk::CheckButton* m_checkbox_format_text_multiline;

  Gtk::RadioButton* m_radiobutton_choices_custom;
  Gtk::RadioButton* m_radiobutton_choices_related;
  Gtk::CheckButton* m_checkbutton_choices_restricted;
  AddDel_WithButtons* m_adddel_choices_custom;
  guint m_col_index_custom_choices;
  Combo_TextGlade* m_combo_choices_relationship;
  Combo_TextGlade* m_combo_choices_field;
  Combo_TextGlade* m_combo_choices_field_second;

  mutable LayoutItem_Field m_layout_item;

  Glib::ustring m_table_name;
};

#endif //GLOM_MODE_DATA_DIALOG_FIELD_LAYOUT_H
