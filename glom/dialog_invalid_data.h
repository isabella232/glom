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

#ifndef GLOM_DIALOG_INVALIDDATA_H
#define GLOM_DIALOG_INVALIDDATA_H

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <libglom/data_structure/field.h>
#include <gtkmm/builder.h>

namespace Glom
{

/** Show the dialog.
 * @result true if the data in the field should be reverted.
 */
bool glom_show_dialog_invalid_data(Field::glom_field_type glom_type);

class Dialog_InvalidData : public Gtk::Dialog
{
public:
  static const char glade_id[];
  static const bool glade_developer;

  Dialog_InvalidData(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  /** Show appropriate example data for this field type.
   */
  void set_example_data(Field::glom_field_type glom_type);

private:
  Gtk::Label* m_label;
};

} //namespace Glom

#endif //GLOM_DIALOG_INVALIDDATA_H

