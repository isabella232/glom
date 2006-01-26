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

#ifndef GLOM_BOX_FORMATTING_H
#define GLOM_BOX_FORMATTING_H

#include <gtkmm.h>
#include "../utility_widgets/dialog_properties.h"
#include "../document/document_glom.h"
#include "../box_db.h"
#include "../utility_widgets/combo_textglade.h"
#include "../utility_widgets/comboentry_currency.h"
#include "../combobox_relationship.h"

class Box_Formatting
 : public Gtk::VBox,
   public Base_DB
{
public:
  Box_Formatting(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Box_Formatting();

  /**
   * @param format The starting information.
   * @param table_name The field's table.
   * @param The field that will have this formatting, so we know what formatting options to allow.
   */
  virtual void set_formatting(const FieldFormatting& format, const Glib::ustring& table_name, const sharedptr<const Field>& field);
  bool get_formatting(FieldFormatting& format) const;

protected:
  //Signal handlers:
  void on_combo_choices_relationship_changed();

  void enforce_constraints();

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
  ComboBox_Relationship* m_combo_choices_relationship;
  Combo_TextGlade* m_combo_choices_field;
  Combo_TextGlade* m_combo_choices_field_second;

  mutable FieldFormatting m_format;

  Glib::ustring m_table_name;
  sharedptr<const Field> m_field;
};

#endif //GLOM_BOX_FORMATTING_H
