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
#include <gtkmm/messagedialog.h>
#include <sstream> //For stringstream

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
  Gtk::Entry::on_changed();
}

void EntryGlom::on_insert_text(const Glib::ustring& text, int* position)
{
  Gtk::Entry::on_insert_text(text, position);
}  

//static
Glib::ustring EntryGlom::format_time(const tm& tm_data)
{
  //This is based on the code in Glib::Date::format_string(), which only deals with dates, but not times:

  const std::string locale_format = Glib::locale_from_utf8("%X"); //%x means "is replaced by the locale's appropriate time representation".
  gsize bufsize = std::max<gsize>(2 * locale_format.size(), 128);

  do
  {
    const Glib::ScopedPtr<char> buf (static_cast<char*>(g_malloc(bufsize)));

    // Set the first byte to something other than '\0', to be able to
    // recognize whether strftime actually failed or just returned "".
    buf.get()[0] = '\1';
    const gsize len = strftime(buf.get(), bufsize, locale_format.c_str(), &tm_data);

    if(len != 0 || buf.get()[0] == '\0')
    {
      g_assert(len < bufsize);
      return Glib::locale_to_utf8(std::string(buf.get(), len));
    }
  }
  while((bufsize *= 2) <= 65536);

  // This error is quite unlikely (unless strftime is buggy).
  g_warning("EntryGlom::format_time(): maximum size of strftime buffer exceeded, giving up");

  return Glib::ustring();
}

//static:

void EntryGlom::set_value(const Gnome::Gda::Value& value)
{
  set_text(get_text_for_gda_value(m_glom_type, value));
}

Glib::ustring EntryGlom::get_text_for_gda_value(Field::glom_field_type glom_type, const Gnome::Gda::Value& value)
{
  if(value.get_value_type() == Gnome::Gda::VALUE_TYPE_NULL) //The type can be null for any of the actual field types.
  {
    return Glib::ustring();
  }
    
  if(glom_type == Field::TYPE_DATE)
  {
    Gnome::Gda::Date gda_date = value.get_date();
    Glib::Date date((Glib::Date::Day)gda_date.day, (Glib::Date::Month)gda_date.month, (Glib::Date::Year)gda_date.year);
    return date.format_string("%x"); //%x means "is replaced by the locale's appropriate date representation".
  }
  else if(glom_type == Field::TYPE_TIME)
  {
    Gnome::Gda::Time gda_time = value.get_time();

    tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    the_c_time.tm_hour = gda_time.hour;
    the_c_time.tm_min = gda_time.minute;
    the_c_time.tm_sec = gda_time.second;
            
    return format_time( the_c_time );
  }
  else if(glom_type == Field::TYPE_NUMERIC)
  {
    const GdaNumeric* gda_numeric = value.get_numeric();
    Glib::ustring text;
    if(gda_numeric && gda_numeric->number) //A char*. TODO: Do we need to look at the other fields?
      text = gda_numeric->number; //What formatting does this use?

    return text;
  }
  else
  {
    return value.to_string();
  }
}

void EntryGlom::set_text(const Glib::ustring& text)
{
  m_old_text = text;

  //Call base class:
  Gtk::Entry::set_text(text);
}

bool EntryGlom::validate_text() const
{
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
  Glib::ustring text = get_text();

  //Put a NULL in the database for empty dates, times, and numerics, because 0 would be an actual value.
  //But we use "" for strings, because the distinction between NULL and "" would not be clear to users.
  if(text.empty())
  {
    if( (m_glom_type ==  Field::TYPE_DATE) || (m_glom_type ==  Field::TYPE_TIME) || (m_glom_type ==  Field::TYPE_NUMERIC) )
    {
      Gnome::Gda::Value null_value;
      return null_value;
    }
  }
  
  if(m_glom_type == Field::TYPE_DATE)
  {
    //Try to parse the inputted date, according to the current locale.
    Glib::Date date;
    date.set_parse(text);
    if(date.valid())
    {
      Gnome::Gda::Date gda_date = {0, 0, 0};

      gda_date.year = date.get_year();
      gda_date.month = date.get_month();
      gda_date.day = date.get_day();
      return Gnome::Gda::Value(gda_date);
    }
    else
    {
      text = ""; //The input is invalid, and should already have been checked with check_valid().
    }
  }
  else if(m_glom_type == Field::TYPE_TIME)
  {
    //Try to parse the inputted date, according to the current locale.
    std::stringstream the_stream; //TODO: Imbue it with the locale?
    the_stream << text;
    time_t the_time;
    the_stream >> the_time;  //TODO: Does this throw any exception if the text is an invalid time?
    tm tm_time = *(gmtime(&the_time)); //TODO: Or should we use localtime instead?

    //TODO: Check validity somhow.
    
    Gnome::Gda::Time gda_time = {0, 0, 0, 0};

    gda_time.hour = tm_time.tm_hour;
    gda_time.minute = tm_time.tm_min;
    gda_time.second = tm_time.tm_sec;
    return Gnome::Gda::Value(gda_time);
  }
  else if(m_glom_type == Field::TYPE_NUMERIC)
  {
    //Try to parse the inputted number, according to the current locale.
    std::stringstream the_stream; //TODO: Imbue it with the locale?
    the_stream << text;
    float the_number;
    the_stream >> the_number;  //TODO: Does this throw any exception if the text is an invalid time?
    g_warning("
    return Gnome::Gda::Value(the_number);
  }
    
  //TODO: Do locale-specific, and strongly-typed conversion:
  return Gnome::Gda::Value(text);
}



  
