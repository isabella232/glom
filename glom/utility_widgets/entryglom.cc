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


//static
Glib::ustring EntryGlom::format_time(const tm& tm_data)
{
  return format_time( tm_data, std::locale(std::getenv("LANG")) ); //Get the current locale.
}

//static
Glib::ustring EntryGlom::format_time(const tm& tm_data, const std::locale& locale, bool iso_format)
{
  if(iso_format)
  {
    g_warning("formatting for iso");
    return format_tm(tm_data, locale, 'T' /* I see no iso-format time in the list, but this looks like the standard C-locale format. murrayc*/);
  }
  else
    return format_tm(tm_data, locale, 'X' /* time */);
}

//static
Glib::ustring EntryGlom::format_date(const tm& tm_data)
{
  return format_date( tm_data, std::locale(std::getenv("LANG")) ); //Get the current locale.
}

//static
Glib::ustring EntryGlom::format_date(const tm& tm_data, const std::locale& locale, bool iso_format)
{
  if(iso_format)
    return format_tm(tm_data, locale, 'F' /* iso-format date */);
  else
    return format_tm(tm_data, locale, 'x' /* date */);
}




//static
Glib::ustring EntryGlom::format_tm(const tm& tm_data, const std::locale& locale, char format)
{
  //This is based on docs found here:
  //http://www.roguewave.com/support/docs/sourcepro/stdlibref/time-put.html

  //Format it into this stream:
  typedef std::stringstream type_stream;
  type_stream the_stream;
  the_stream.imbue(locale); //Make it format things for this locale. (Actually, I don't know if this is necessary, because we mention the locale in the time_put<> constructor.
  
  // Get a time_put face:
  typedef std::ostreambuf_iterator<char, std::char_traits<char> > type_iterator;
  typedef std::time_put<char, type_iterator> type_time_put;
  const type_time_put& tp = std::use_facet<type_time_put>(locale);

  //type_iterator begin(the_stream);
  tp.put(the_stream /* iter to beginning of stream */, the_stream, ' ' /* fill */, &tm_data, format, 0 /* 'E' */ /* use locale's alternative format */);

  Glib::ustring text = the_stream.str();
  g_warning("EntryGlom::format_tm(): result=%s", text.c_str());
    
  return text; //TODO: Use something like Glib::locale_to_utf8()?
                                           
  /*
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
  */
}

//static:

void EntryGlom::set_value(const Gnome::Gda::Value& value)
{
  set_text(get_text_for_gda_value(m_glom_type, value));
}

//static:
Glib::ustring EntryGlom::get_text_for_gda_value(Field::glom_field_type glom_type, const Gnome::Gda::Value& value)
{
  return get_text_for_gda_value(glom_type, value, std::locale(std::getenv("LANG")) ); //Get the current locale.
}

//static:
Glib::ustring EntryGlom::get_text_for_gda_value(Field::glom_field_type glom_type, const Gnome::Gda::Value& value, const std::locale& locale, bool iso_format)
{
  if(value.get_value_type() == Gnome::Gda::VALUE_TYPE_NULL) //The type can be null for any of the actual field types.
  {
    return Glib::ustring();
  }
    
  if(glom_type == Field::TYPE_DATE)
  {
    Gnome::Gda::Date gda_date = value.get_date();

    tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    the_c_time.tm_year = gda_date.year - 1900; //C years start are the AD year - 1900. So, 01 is 1901.
    the_c_time.tm_mon = gda_date.month - 1; //C months start at 0.
    the_c_time.tm_mday = gda_date.day; //starts at 1

    g_warning("EntryGlom::get_text_for_gda_value() the_c_time: tm_year=%d, tm_mon=%d, tm_mday=%d",  the_c_time.tm_year, the_c_time.tm_mon, the_c_time.tm_mday);

    return format_date(the_c_time, locale, iso_format);
    
    /*

    Glib::Date date((Glib::Date::Day)gda_date.day, (Glib::Date::Month)gda_date.month, (Glib::Date::Year)gda_date.year);
    return date.format_string("%x"); //%x means "is replaced by the locale's appropriate date representation".
    */
  }
  else if(glom_type == Field::TYPE_TIME)
  {
    Gnome::Gda::Time gda_time = value.get_time();

    tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    the_c_time.tm_hour = gda_time.hour;
    the_c_time.tm_min = gda_time.minute;
    the_c_time.tm_sec = gda_time.second;
            
    return format_time(the_c_time, locale, iso_format);
  }
  else if(glom_type == Field::TYPE_NUMERIC)
  {
    const GdaNumeric* gda_numeric = value.get_numeric();
    Glib::ustring text_in_c_locale;
    if(gda_numeric && gda_numeric->number) //A char* - I assume that it formatted as per the C locale. murrayc. TODO: Do we need to look at the other fields?
      text_in_c_locale = gda_numeric->number; //What formatting does this use?

    //Get an actual numeric value, so we can get a locale-specific text representation:
    std::stringstream the_stream;
    the_stream.imbue( std::locale() ); //The C locale.
    the_stream << text_in_c_locale;
    float number = 0;
    the_stream >> number;

    //Get the locale-specific text representation:
    the_stream.clear();
    the_stream.imbue(locale); //Tell it to parse stuff as per this locale.
    the_stream << number;
    Glib::ustring text;
    the_stream >> text;
      
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

//static:
Gnome::Gda::Value EntryGlom::parse_value(Field::glom_field_type glom_type, const Glib::ustring& text)
{
  //Put a NULL in the database for empty dates, times, and numerics, because 0 would be an actual value.
  //But we use "" for strings, because the distinction between NULL and "" would not be clear to users.
  if(text.empty())
  {
    if( (glom_type ==  Field::TYPE_DATE) || (glom_type ==  Field::TYPE_TIME) || (glom_type ==  Field::TYPE_NUMERIC) )
    {
      Gnome::Gda::Value null_value;
      return null_value;
    }
  }

  if(glom_type == Field::TYPE_DATE)
  {
    tm the_c_time = parse_date(text);
    
    Gnome::Gda::Date gda_date = {0, 0, 0};
    gda_date.year = the_c_time.tm_year + 1900; //The C time starts at 1900.
    gda_date.month = the_c_time.tm_mon + 1; //The C month starts at 0.
    gda_date.day = the_c_time.tm_mday; //THe C mday starts at 1.
    g_warning("EntryGlom::get_value() date: year=%d, month=%d, day=%d",  gda_date.year, gda_date.month, gda_date.day);
    
    return Gnome::Gda::Value(gda_date);
  }
  else if(glom_type == Field::TYPE_TIME)
  {
    tm the_c_time = parse_time(text);

    Gnome::Gda::Time gda_time = {0, 0, 0, 0};
    gda_time.hour = the_c_time.tm_hour;
    gda_time.minute = the_c_time.tm_min;
    gda_time.second = the_c_time.tm_sec;
    
    return Gnome::Gda::Value(gda_time);
  }
  else if(glom_type == Field::TYPE_NUMERIC)
  {
    //Try to parse the inputted number, according to the current locale.
    std::stringstream the_stream;
    the_stream.imbue( std::locale(getenv("LANG")) ); //Parse it as per the current locale.
    the_stream << text;
    float the_number;
    the_stream >> the_number;  //TODO: Does this throw any exception if the text is an invalid time?

    return Gnome::Gda::Value(the_number);
  }

  //TODO: Do locale-specific, and strongly-typed conversion:
  return Gnome::Gda::Value(text);
  
}

//static
  
tm EntryGlom::parse_date(const Glib::ustring& text)
{
  return parse_date( text, std::locale(std::getenv("LANG")) ); //Get the current locale.
}

//static:
tm EntryGlom::parse_date(const Glib::ustring& text, const std::locale& locale)
{
  //return parse_tm(text, locale, 'x' /* date */);

  //This is based on docs found here:
  //http://www.roguewave.com/support/docs/sourcepro/stdlibref/time-get.html

  //Format it into this stream:
  typedef std::stringstream type_stream;


  //tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  /* This seems to be necessary - when I do this, as found here ( http://www.tacc.utexas.edu/services/userguides/pgi/pgC++_lib/stdlibcr/tim_0622.htm ) then the time is correctly parsed (it is changed).
   * When not, I get just zeros.
   */
  tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  std::ios_base::iostate err = std::ios_base::goodbit;  //The initialization is essential because time_get seems to a) not initialize this output argument and b) check its value.

  //For some reason, the stream must be instantiated after we get the facet. This is a worryingly strange "bug".
  type_stream the_stream;
  the_stream.imbue(locale); //Make it format things for this locale. (Actually, I don't know if this is necessary, because we mention the locale in the time_put<> constructor.
  the_stream << text;

  // Get a time_get facet:
  typedef std::istreambuf_iterator<char, std::char_traits<char> > type_iterator;
  typedef std::time_get<char, type_iterator> type_time_get;
  const type_time_get& tg = std::use_facet<type_time_get>(locale);
  
  type_iterator the_begin(the_stream);
  type_iterator the_end;

  tg.get_date(the_begin, the_end, the_stream, err, &the_c_time);

  if(err != std::ios_base::failbit)
    return the_c_time;
  else
  {
    tm blank_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    return blank_time;
  }
}

//static
tm EntryGlom::parse_time(const Glib::ustring& text)
{
  //return parse_time( text, std::locale(std::getenv("LANG")) ); //Get the current locale.

  //time_get() does not seem to work with non-C locales.
  return parse_time( text, std::locale() );
}

//static
tm EntryGlom::parse_time(const Glib::ustring& text, const std::locale& locale)
{
  //The sequence of statements here seems to be very fragile. If you move things then it stops working.
  
  //return parse_tm(text, locale, 'X' /* time */);

  //This is based on docs found here:
  //http://www.roguewave.com/support/docs/sourcepro/stdlibref/time-get.html

  //Format it into this stream:
  typedef std::stringstream type_stream;

   //tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  /* This seems to be necessary - when I do this, as found here ( http://www.tacc.utexas.edu/services/userguides/pgi/pgC++_lib/stdlibcr/tim_0622.htm ) then the time is correctly parsed (it is changed).
   * When not, I get just zeros.
   */
  tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  std::ios_base::iostate err = std::ios_base::goodbit; //The initialization is essential because time_get seems to a) not initialize this output argument and b) check its value.

    
  type_stream the_stream;
  the_stream.imbue(locale); //Make it format things for this locale. (Actually, I don't know if this is necessary, because we mention the locale in the time_put<> constructor.

  the_stream << text;

  // Get a time_get facet:
  typedef std::istreambuf_iterator<char, std::char_traits<char> > type_iterator;
  typedef std::time_get<char, type_iterator> type_time_get;
  const type_time_get& tg = std::use_facet<type_time_get>(locale);
    
  type_iterator the_begin(the_stream);
  type_iterator the_end;

  tg.get_time(the_begin, the_end, the_stream, err, &the_c_time);
  
  if(err != std::ios_base::failbit)
    return the_c_time;
  else
  {
    tm blank_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    return blank_time;
  }
}



/* This requires an extension to the standard - time_get<>.get().
//static:
tm EntryGlom::parse_tm(const Glib::ustring& text, const std::locale& locale, char format)
{
  //This is based on docs found here:
  //http://www.roguewave.com/support/docs/sourcepro/stdlibref/time-get.html

  //Format it into this stream:
  typedef std::stringstream type_stream;
  type_stream the_stream;
  the_stream.imbue(locale); //Make it format things for this locale. (Actually, I don't know if this is necessary, because we mention the locale in the time_put<> constructor.

  
  // Get a time_get facet:       
  typedef std::istreambuf_iterator<char, std::char_traits<char> > type_iterator;
  typedef std::time_get<char, type_iterator> type_time_get;
  const type_time_get& tg = std::use_facet<type_time_get>(locale);

  the_stream << text;

  tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  type_iterator the_begin(the_stream);
  type_iterator the_end;
  std::ios_base::iostate state;  // Unused
  std::ios_base::iostate err;
  tg.get(the_begin, the_end, the_stream, state, err, &the_c_time, format, 0);

  return the_c_time;    
}
*/

Gnome::Gda::Value EntryGlom::get_value() const
{
  return parse_value(m_glom_type, get_text());
}



  
