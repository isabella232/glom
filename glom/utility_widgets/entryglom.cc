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
#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <time.h>     // for struct tm
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
    if(validate_text())
    {
      m_old_text = new_text;
      m_signal_edited.emit(); //The text was edited, so tell the client code.
    }
    else
    {
      //TODO: problems with focus-out-event: Gtk::MessageDialog dialog(gettext("The data has an incorrect format")); //TODO: Improve this warning. Mention the field name and what format it should have.

      /* TODO:
      Gtk::Window* pWindowApp = get_app_window();
      if(pWindowApp)
        dialog.set_transient_for(*pWindowApp);
      */

       //dialog.run();
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

bool EntryGlom::validate_text() const
{
  return true; //TODO: See whether the regular conversion functions have an effect.
  
  const Glib::ustring text = get_text();
  if(m_glom_type == Field::TYPE_DATE)
  {
    //Try to parse the inputted date, according to the current locale.
    Glib::Date date;
    date.set_parse(text);

    return date.valid();
  }
  else if(m_glom_type == Field::TYPE_TIME)
  {
      //TODO:
  }
  
  return true;
  
}


Gnome::Gda::Value EntryGlom::get_value() const
{
  return GlomConversions::parse_value(m_glom_type, get_text());
}



  
