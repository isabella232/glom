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

#ifndef GLOM_MODE_DESIGN_DIALOG_DESIGN_H
#define GLOM_MODE_DESIGN_DIALOG_DESIGN_H

#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/builder.h>
#include <libglom/document/view.h>

namespace Glom
{

/** A dialog with a titled frame, a label for the table title, and a close button.
 */
class Dialog_Design
: public Gtk::Window,
  public View_Composite_Glom
{
public:
  Dialog_Design(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  virtual bool init_db_details(const Glib::ustring& table_name);

private:
  virtual void on_button_close();

protected:
  Gtk::Label* m_label_table;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_DESIGN_H
