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
 
#include "dialog_new_database.h"

Dialog_NewDatabase::Dialog_NewDatabase(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_entry_name(0),
  m_entry_title(0)
{
  refGlade->get_widget("entry_name", m_entry_name);
  refGlade->get_widget("entry_title", m_entry_title);
  refGlade->get_widget("button_ok", m_button_ok);
  
  m_entry_name->signal_changed().connect( sigc::mem_fun(*this, &Dialog_NewDatabase::on_entry_name_changed) );
}

Dialog_NewDatabase::~Dialog_NewDatabase()
{
}

void Dialog_NewDatabase::on_entry_name_changed()
{
  //Disable the OK button if the database name is not suitable.
  Glib::ustring name = m_entry_name->get_text();
  bool allow_ok = !name.empty();
  m_button_ok->set_sensitive(allow_ok);
}

void Dialog_NewDatabase::set_input(const Glib::ustring& name, const Glib::ustring& title)
{
  m_entry_name->set_text(name);
  m_entry_title->set_text(title);
}

void Dialog_NewDatabase::get_input(Glib::ustring& name, Glib::ustring& title)
{
  name = m_entry_name->get_text();
  title = m_entry_title->get_text();
}
