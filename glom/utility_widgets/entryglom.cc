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

#include "entryglom.h"


EntryGlom::EntryGlom()
{
}

EntryGlom::~EntryGlom()
{
}

void EntryGlom::check_for_change()
{
  Glib::ustring new_text = get_text();
  if(new_text != m_old_text)
  {
    m_old_text = new_text;
    m_signal_edited.emit(); //The text was edited, so tell the client code.
  }
}

  
EntryGlom::type_signal_edited EntryGlom::signal_edited()
{
  return m_signal_edited;
}

bool EntryGlom::on_focus_out_event(GdkEventFocus* event)
{
  //The user has finished editing.
  check_for_change();
  
  //Call base class:
  return Gtk::Entry::on_focus_out_event(event);
}

void EntryGlom::on_activate()
{ 
  //Call base class:
  Gtk::Entry::on_activate();

  //The user has finished editing.
  check_for_change();
}

void EntryGlom::on_changed()
{
  //The text is being edited, but the user has not finished yet.
  
  //Call base class:
  return Gtk::Entry::on_changed();
}

void EntryGlom::set_text(const Glib::ustring &text)
{
  m_old_text = text;

  //Call base class:
  Gtk::Entry::set_text(text);
}


  
