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

#ifndef GLOM_DIALOG_CONNECTION_H
#define GLOM_DIALOG_CONNECTION_H

#include <libglademm.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include "base_db.h"
#include "connectionpool.h"

class Dialog_Connection
  : public Gtk::Dialog,
    public Base_DB
{
public:
  Dialog_Connection(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_Connection();

  sharedptr<SharedConnection> connect_to_server_with_connection_settings() const;

  virtual void load_from_document(); //override

  /** Use this to override the data from load_from_document().
   * For instance, if you want to try to connect to a renamed database.
   */
  void set_database_name(const Glib::ustring& name);

protected:
  Gtk::Entry* m_entry_host;
  Gtk::Entry* m_entry_user;
  Gtk::Entry* m_entry_password;
  Gtk::Label* m_label_database;
};

#endif //GLOM_DIALOG_CONNECTION_H

