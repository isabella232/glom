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

#ifndef GLOM_MODE_DESIGN_DIALOG_DEFAULTFORMATTING_H
#define GLOM_MODE_DESIGN_DIALOG_DEFAULTFORMATTING_H

#include <gtkmm/dialog.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/alignment.h>
#include <gtkmm/entry.h>
#include <glom/mode_design/layout/combobox_relationship.h>
//#include "../../utility_widgets/entry_numerical.h"
#include "../../utility_widgets/dialog_properties.h"
#include <glom/mode_data/datawidget/datawidget.h>
#include <libglom/data_structure/field.h>
#include <glom/mode_design/layout/layout_item_dialogs/box_formatting.h>
#include <glom/base_db.h>

namespace Glom
{

class Dialog_DefaultFormatting
 : public Dialog_Properties,
   public Base_DB //Give this class access to the current document, and to some utility methods.
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_DefaultFormatting(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_DefaultFormatting();

  virtual void set_field(const std::shared_ptr<const Field>& field, const Glib::ustring& table_name);
  virtual std::shared_ptr<Field> get_field() const; //TODO_FieldShared

private:
  Gtk::Box* m_box_formatting_placeholder;
  Box_Formatting* m_box_formatting;

  std::shared_ptr<Field> m_Field;
  Glib::ustring m_table_name;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_DEFAULTFORMATTING_H
