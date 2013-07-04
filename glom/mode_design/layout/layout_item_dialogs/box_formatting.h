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

#ifndef GLOM_MODE_DESIGN_BOX_FORMATTING_H
#define GLOM_MODE_DESIGN_BOX_FORMATTING_H

#include <gtkmm/dialog.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/fontbutton.h>
#include <glom/utility_widgets/dialog_properties.h>
#include <libglom/document/document.h>
#include <glom/box_withbuttons.h>
#include <glom/mode_design/comboentry_currency.h>
#include <glom/mode_design/layout/combobox_relationship.h>
#include <glom/mode_design/layout/combobox_fields.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_fieldslist.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_sortfields.h>

namespace Glom
{

class Dialog_FieldsList;

class Box_Formatting
 : public Gtk::Box,
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
  void set_formatting_for_non_field(const Formatting& format, bool show_numeric = true);

  /**
   * @param format The starting information.
   * @param table_name The field's table.
   * @param The field that will have this formatting, so we know what formatting options to allow.
   */
  void set_formatting_for_field(const Formatting& format, const Glib::ustring& table_name, const std::shared_ptr<const Field>& field);
  bool get_formatting(Formatting& format) const;

  /** When used, for instance, for print layout items or choice lists,
   * where the user could not edit the field anyway.
   * This hides some UI.
   */
  void set_is_for_non_editable();

  typedef sigc::signal<void> type_signal_modified;
  type_signal_modified signal_modified();

private:
  //Signal handlers:
  void on_combo_choices_relationship_changed();
  void on_checkbox();
  void on_button_choices_extra();
  void on_button_choices_sortby();

  void enforce_constraints();

  Gtk::Box* m_vbox_numeric_format;
  Gtk::CheckButton* m_checkbox_format_use_thousands;
  Gtk::CheckButton* m_checkbox_format_use_decimal_places;
  Gtk::Entry* m_entry_format_decimal_places;
  ComboEntry_Currency* m_entry_currency_symbol;
  Gtk::CheckButton* m_checkbox_format_color_negatives;

  Gtk::Box* m_vbox_text_format;
  Gtk::ComboBox* m_combo_format_text_horizontal_alignment;
  Gtk::CheckButton* m_checkbox_format_text_multiline;
  Gtk::Label* m_label_format_text_multiline_height;
  Gtk::SpinButton* m_spinbutton_format_text_multiline_height;
  Gtk::Box* m_hbox_font;
  Gtk::CheckButton* m_checkbox_format_text_font;
  Gtk::FontButton* m_fontbutton;
  Gtk::Box* m_hbox_color_foreground;
  Gtk::CheckButton* m_checkbox_format_text_color_foreground;
  Gtk::ColorButton* m_colorbutton_foreground;
  Gtk::Box* m_hbox_color_background;
  Gtk::CheckButton* m_checkbox_format_text_color_background;
  Gtk::ColorButton* m_colorbutton_background;

  Gtk::Box* m_vbox_choices;
  Gtk::RadioButton* m_radiobutton_choices_custom;
  Gtk::RadioButton* m_radiobutton_choices_related;
  Gtk::CheckButton* m_checkbutton_choices_restricted;
  Gtk::CheckButton* m_checkbutton_choices_restricted_as_radio_buttons;
  AddDel_WithButtons* m_adddel_choices_custom;
  guint m_col_index_custom_choices;
  ComboBox_Relationship* m_combo_choices_relationship;
  ComboBox_Fields* m_combo_choices_field;
  Gtk::Label* m_label_choices_extra_fields;
  Gtk::Button* m_button_choices_extra_fields;
  Gtk::Label* m_label_choices_sortby;
  Gtk::Button* m_button_choices_sortby;
  Gtk::CheckButton* m_checkbutton_choices_related_show_all;

  Dialog_FieldsList* m_dialog_choices_extra_fields;
  Dialog_SortFields* m_dialog_choices_sortby;

  mutable Formatting m_format;

  Glib::ustring m_table_name;
  std::shared_ptr<const Field> m_field;

  //We show different options when
  //showing this on a print layout.
  bool m_for_print_layout;

  bool m_show_numeric;
  bool m_show_editable_options;

  type_signal_modified m_signal_modified;


  class AlignmentColumns: public Gtk::TreeModelColumnRecord
  {
  public:
    AlignmentColumns()
    { add(m_col_alignment); add(m_col_title); }

    Gtk::TreeModelColumn<Formatting::HorizontalAlignment> m_col_alignment;
    Gtk::TreeModelColumn<Glib::ustring> m_col_title;
  };

  AlignmentColumns m_columns_alignment;
  Glib::RefPtr<Gtk::ListStore> m_model_alignment;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_BOX_FORMATTING_H
