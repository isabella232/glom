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

#ifndef GLOM_DIALOG_INVALIDDATA_H
#define GLOM_DIALOG_INVALIDDATA_H

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <glom/libglom/data_structure/field.h>
#include <libglademm.h>

/** Show the dialog.
 * @result true if the data in the field should be reverted.
 */
bool glom_show_dialog_invalid_data(Field::glom_field_type glom_type);

class Dialog_InvalidData : public Gtk::Dialog
{
public:
  Dialog_InvalidData(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_InvalidData();

  /** Show appropriate example data for this field type
   */
  virtual void set_example_data(Field::glom_field_type glom_type);

protected:
  Gtk::Label* m_label;
};

#endif //GLOM_DIALOG_INVALIDDATA_H

