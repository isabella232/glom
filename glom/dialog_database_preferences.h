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

#ifndef GLOM_DIALOG_DATABASE_PREFERENCES_H
#define GLOM_DIALOG_DATABASE_PREFERENCES_H

#include <libglademm.h>
#include <libglademm/variablesmap.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include "base_db.h"
#include "data_structure/system_prefs.h"
#include "connectionpool.h"

class Dialog_Database_Preferences
  : public Gtk::Dialog,
    public Base_DB
{
public:
  Dialog_Database_Preferences(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_Database_Preferences();

  sharedptr<SharedConnection> connect_to_server_with_connection_settings() const;

  virtual void load_from_document(); //override
  virtual void save_to_document(); //override

protected:
  void on_response(int response_id);

  Gnome::Glade::VariablesMap m_glade_variables_map;

  SystemPrefs m_system_prefs;
};

#endif //GLOM_DIALOG_DATABASE_PREFERENCES_H

