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

#ifndef DIALOG_FIELDDEFINITION_H
#define DIALOG_FIELDDEFINITION_H

#include "combo_textglade.h"
#include "combo_lookup_relationship.h"
#include "combo_fieldtype.h"
#include <gtkmm.h>
#include "../../utility_widgets/table_columns.h"
#include "../../utility_widgets/entry_numerical.h"
#include "../../utility_widgets/dialog_properties.h"
#include "../../utility_widgets/datawidget.h"
#include "../../data_structure/field.h"
#include "../../base_db.h"


class Dialog_FieldDefinition
 : public Dialog_Properties,
   public Base_DB //Give this class access to the current document, and to some utility methods.
{
public: 
  Dialog_FieldDefinition(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_FieldDefinition();

  virtual void set_field(const Field& field, const Glib::ustring& table_name);
  virtual Field get_field() const;

protected:

  //Signal handlers:
  void on_combo_type_changed();
  void on_combo_lookup_relationship_changed();
  void on_check_lookup_toggled();
  void on_check_calculate_toggled();
  void on_button_edit_calculation();
  
  //void on_foreach(Gtk::Widget& widget);

  //Disable/enable other controls when a control is selected.
  virtual void enforce_constraints(); //override.

  Gtk::Entry* m_pEntry_Name;
  Combo_FieldType* m_pCombo_Type;
  Gtk::CheckButton* m_pCheck_Unique;
  Gtk::CheckButton* m_pCheck_NotNull;
  Gtk::HBox* m_pBox_DefaultValueSimple;
  Gtk::CheckButton* m_pCheck_PrimaryKey;
  Gtk::CheckButton* m_pCheck_AutoIncrement;

  Gtk::VBox* m_pBox_DefaultValue;
  Gtk::CheckButton* m_pCheck_Lookup;
  Gtk::Table* m_pTable_Lookup;
  Combo_LookupRelationship* m_pCombo_LookupRelationship;
  Combo_TextGlade* m_pCombo_LookupField;

  Gtk::CheckButton* m_pCheck_Calculate;
  Gtk::Alignment* m_pAlignment_Calculate;
  Gtk::TextView* m_pTextView_Calculation;
  Gtk::Button* m_pButton_EditCalculation;
     
  Gtk::Entry* m_pEntry_Title;

  DataWidget* m_pDataWidget_DefaultValueSimple;

  Field m_Field;
  Glib::ustring m_table_name;
};

#endif //DIALOG_FIELDDEFINITION_H
