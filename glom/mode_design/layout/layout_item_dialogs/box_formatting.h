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
#include <glom/utility_widgets/dialog_properties.h>
#include <libglom/document/document.h>
#include <glom/box_withbuttons.h>
#include <glom/mode_design/comboentry_currency.h>
#include <glom/mode_design/layout/combobox_relationship.h>
#include <glom/mode_design/layout/combobox_fields.h>

namespace Glom
{

class Box_Formatting
 : public Gtk::VBox,
   public View_Composite_Glom
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Box_Formatting(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Box_Formatting();

  /**
   * @param format The starting information.
   */
  void set_formatting(const FieldFormatting& format, bool show_numeric = true, bool show_choices = true);
  
  /**
   * @param format The starting information.
   * @param table_name The field's table.
   * @param The field that will have this formatting, so we know what formatting options to allow.
   */
  void set_formatting(const FieldFormatting& format, const Glib::ustring& table_name, const sharedptr<const Field>& field);
  bool get_formatting(FieldFormatting& format) const;

  //When used for print layout items, 
  //we hide some stuff:
  void set_is_for_print_layout();

private:
  //Signal handlers:
  void on_combo_choices_relationship_changed();
  void on_checkbox();

  void enforce_constraints();

  Gtk::VBox* m_vbox_numeric_format;
  Gtk::CheckButton* m_checkbox_format_use_thousands;
  Gtk::CheckButton* m_checkbox_format_use_decimal_places;
  Gtk::Entry* m_entry_format_decimal_places;
  ComboEntry_Currency* m_entry_currency_symbol;
  Gtk::CheckButton* m_checkbox_format_color_negatives;

  Gtk::VBox* m_vbox_text_format;
  Gtk::ComboBox* m_combo_format_text_horizontal_alignment;
  Gtk::CheckButton* m_checkbox_format_text_multiline;
  Gtk::Label* m_label_format_text_multiline_height;
  Gtk::SpinButton* m_spinbutton_format_text_multiline_height;
  Gtk::HBox* m_hbox_font;
  Gtk::CheckButton* m_checkbox_format_text_font;
  Gtk::FontButton* m_fontbutton;
  Gtk::HBox* m_hbox_color_foreground;
  Gtk::CheckButton* m_checkbox_format_text_color_foreground;
  Gtk::ColorButton* m_colorbutton_foreground;
  Gtk::HBox* m_hbox_color_background;
  Gtk::CheckButton* m_checkbox_format_text_color_background;
  Gtk::ColorButton* m_colorbutton_background;

  Gtk::VBox* m_vbox_choices;
  Gtk::RadioButton* m_radiobutton_choices_custom;
  Gtk::RadioButton* m_radiobutton_choices_related;
  Gtk::CheckButton* m_checkbutton_choices_restricted;
  Gtk::CheckButton* m_checkbutton_choices_restricted_as_radio_buttons;
  AddDel_WithButtons* m_adddel_choices_custom;
  guint m_col_index_custom_choices;
  ComboBox_Relationship* m_combo_choices_relationship;
  ComboBox_Fields* m_combo_choices_field;
  ComboBox_Fields* m_combo_choices_field_second;

  mutable FieldFormatting m_format;

  Glib::ustring m_table_name;
  sharedptr<const Field> m_field;

  //We show different options when 
  //showing this on a print layout.
  bool m_for_print_layout;

  bool m_show_numeric;
  bool m_show_choices;


  class AlignmentColumns: public Gtk::TreeModelColumnRecord
  {
  public:
    AlignmentColumns()
    { add(m_col_alignment); add(m_col_title); }

    Gtk::TreeModelColumn<FieldFormatting::HorizontalAlignment> m_col_alignment;
    Gtk::TreeModelColumn<Glib::ustring> m_col_title;
  };

  AlignmentColumns m_columns_alignment;
  Glib::RefPtr<Gtk::ListStore> m_model_alignment;
};

} //namespace Glom

#endif //GLOM_BOX_FORMATTING_H
