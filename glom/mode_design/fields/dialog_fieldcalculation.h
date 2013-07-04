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

#ifndef GLOM_MODE_DESIGN_DIALOG_FIELDCALCULATION_H
#define GLOM_MODE_DESIGN_DIALOG_FIELDCALCULATION_H

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/builder.h>
#include <libglom/data_structure/field.h>
#include <glom/base_db.h>
#include <gtksourceviewmm/view.h>

namespace Glom
{

class Dialog_FieldCalculation
 : public Gtk::Dialog,
   public Base_DB //Give this class access to the current document, and to some utility methods.

{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_FieldCalculation(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_FieldCalculation();

  void set_field(const std::shared_ptr<const Field>& field, const Glib::ustring& table_name);
  std::shared_ptr<Field> get_field() const;

private:
  void on_button_test();
  bool check_for_return_statement(const Glib::ustring& calculation);

  Gsv::View* m_text_view;
  Gtk::Button* m_button_test;
  Gtk::Label* m_label_triggered_by;

  std::shared_ptr<Field> m_field;
  Glib::ustring m_table_name;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_FIELDCALCULATION_H
