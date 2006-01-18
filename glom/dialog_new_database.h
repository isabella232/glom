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

#ifndef GLOM_DIALOG_NEWDATABASE_H
#define GLOM_DIALOG_NEWDATABASE_H

#include <libglademm.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>

class Dialog_NewDatabase : public Gtk::Dialog
{
public:
  Dialog_NewDatabase(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_NewDatabase();

  //Set sensible defaults:
  virtual void set_input(const Glib::ustring& title);

  //Get the user input:
  virtual void get_input(Glib::ustring& title);

protected:
  virtual void on_entry_title_changed();

  Gtk::Entry* m_entry_title;
  Gtk::Button* m_button_ok;
};

#endif //GLOM_DIALOG_NEWDATABASE_H

