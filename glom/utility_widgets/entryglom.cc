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
#include "../data_structure/glomconversions.h"
#include <gtkmm/messagedialog.h>
#include "../dialog_invalid_data.h"
#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl

EntryGlom::EntryGlom(Field::glom_field_type glom_type)
: m_glom_type(glom_type)
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
    //Validate the input:
    bool success = false;
    Gnome::Gda::Value value = GlomConversions::parse_value(m_glom_type, get_text(), success);
    if(success)
    {
      //Actually show the canonical text:
      set_value(value);
      m_signal_edited.emit(); //The text was edited, so tell the client code.
    }
    else
    {
      //Tell the user and offer to revert or try again:
      bool revert = glom_show_dialog_invalid_date(m_glom_type);
      if(revert)
      {
        set_text(m_old_text);
      }
      else
        grab_focus(); //Force the user back into the same field, so that the field can be checked again and eventually corrected or reverted.
    }
  }
}

  
EntryGlom::type_signal_edited EntryGlom::signal_edited()
{
  return m_signal_edited;
}

bool EntryGlom::on_focus_out_event(GdkEventFocus* event)
{
  bool result = Gtk::Entry::on_focus_out_event(event);
  
  //The user has finished editing.
  check_for_change();
  
  //Call base class:
  return result;
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
  Gtk::Entry::on_changed();
}

void EntryGlom::on_insert_text(const Glib::ustring& text, int* position)
{
  Gtk::Entry::on_insert_text(text, position);
}  



void EntryGlom::set_value(const Gnome::Gda::Value& value)
{
  set_text(GlomConversions::get_text_for_gda_value(m_glom_type, value));
}

void EntryGlom::set_text(const Glib::ustring& text)
{
  m_old_text = text;

  //Call base class:
  Gtk::Entry::set_text(text);
}

Gnome::Gda::Value EntryGlom::get_value() const
{
  bool success = false;
  return GlomConversions::parse_value(m_glom_type, get_text(), success);
}



  
