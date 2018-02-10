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

#ifndef GLOM_MODE_DESIGN_DIALOG_FIELDDEFINITION_H
#define GLOM_MODE_DESIGN_DIALOG_FIELDDEFINITION_H

#include <gtkmm/dialog.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/entry.h>
#include <glom/mode_design/layout/combobox_relationship.h>
#include "combo_fieldtype.h"
//#include "../../utility_widgets/entry_numerical.h"
#include "../../utility_widgets/dialog_properties.h"
#include <glom/mode_data/datawidget/datawidget.h>
#include <libglom/data_structure/field.h>
#include <glom/mode_design/layout/layout_item_dialogs/box_formatting.h>
#include <glom/base_db.h>
#include <gtksourceviewmm/view.h>

namespace Glom
{

class Dialog_FieldDefinition
 : public Dialog_Properties,
   public Base_DB //Give this class access to the current document, and to some utility methods.
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_FieldDefinition(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  void set_field(const std::shared_ptr<const Field>& field, const Glib::ustring& table_name);
  std::shared_ptr<Field> get_field() const; //TODO_FieldShared

private:

  //Signal handlers:
  void on_combo_type_changed();
  void on_combo_lookup_relationship_changed();
  void on_check_lookup_toggled();
  void on_radio_calculate_toggled();
  void on_radio_userentry_toggled();
  void on_button_edit_calculation();

  //void on_foreach(Gtk::Widget& widget);

  //Disable/enable other controls when a control is selected.
  void enforce_constraints() override;

  Gtk::Entry* m_entry_name;
  Combo_FieldType* m_combo_type;
  Gtk::CheckButton* m_check_unique;
  Gtk::Box* m_box_default_value_simple;
  Gtk::CheckButton* m_check_primary_key;
  Gtk::CheckButton* m_check_auto_increment;

  Gtk::Box* m_box_value_tab;

  Gtk::RadioButton* m_radio_user_entry;
  Gtk::Box* m_alignment_user_entry;
  Gtk::CheckButton* m_check_lookup;
  Gtk::Widget* m_table_lookup; //So we can make it insensitive.
  ComboBox_Relationship* m_combo_lookup_relationship;
  Gtk::ComboBoxText* m_combo_lookup_field;

  Gtk::RadioButton* m_radio_calculate;
  Gtk::Box* m_alignment_calculate;
  Gsv::View* m_textView_calculation;
  Gtk::Button* m_button_edit_calculation;

  Gtk::Entry* m_entry_title;

  DataWidget* m_data_idget_default_value_simple;

  std::shared_ptr<Field> m_Field;
  Glib::ustring m_table_name;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_FIELDDEFINITION_H
