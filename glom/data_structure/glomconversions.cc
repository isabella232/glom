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

#include "glomconversions.h"
#include "../connectionpool.h"
#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl
#include <iomanip>


Glib::ustring GlomConversions::format_time(const tm& tm_data)
{
  return format_time( tm_data, std::locale("") /* the user's current locale */ ); //Get the current locale.
}


Glib::ustring GlomConversions::format_time(const tm& tm_data, const std::locale& locale, bool iso_format)
{
  if(iso_format)
  {
    return format_tm(tm_data, locale, 'T' /* I see no iso-format time in the list, but this looks like the standard C-locale format. murrayc*/);
  }
  else
    return format_tm(tm_data, locale, 'X' /* time */);
}

Glib::ustring GlomConversions::format_date(const tm& tm_data)
{
  return format_date( tm_data, std::locale("") /* the user's current locale */ ); //Get the current locale.
}


Glib::ustring GlomConversions::format_date(const tm& tm_data, const std::locale& locale, bool iso_format)
{
  if(iso_format)
    return format_tm(tm_data, locale, 'F' /* iso-format date */);
  else
    return format_tm(tm_data, locale, 'x' /* date */);
}


Glib::ustring GlomConversions::format_tm(const tm& tm_data, const std::locale& locale, char format)
{
  //This is based on docs found here:
  //http://www.roguewave.com/support/docs/sourcepro/stdlibref/time-put.html

  //Format it into this stream:
  typedef std::stringstream type_stream;
  type_stream the_stream;
  the_stream.imbue(locale); //Make it format things for this locale. (Actually, I don't know if this is necessary, because we mention the locale in the time_put<> constructor.

  // Get a time_put face:
  typedef std::time_put<char> type_time_put;
  typedef type_time_put::iter_type type_iterator;
  const type_time_put& tp = std::use_facet<type_time_put>(locale);

  //type_iterator begin(the_stream);
  tp.put(the_stream /* iter to beginning of stream */, the_stream, ' ' /* fill */, &tm_data, format, 0 /* 'E' */ /* use locale's alternative format */);

  Glib::ustring text = the_stream.str();

  if(locale == std::locale("") /* The user's current locale */)
  {
    //Converts from the user's current locale to utf8. I would prefer a generic conversion from any locale,
    // but there is no such function, and it's hard to do because I don't know how to get the encoding name from the std::locale()
    text = Glib::locale_to_utf8(text);
  }

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
  g_warning("GlomConversions::format_time(): maximum size of strftime buffer exceeded, giving up");

  return Glib::ustring();
  */
}

namespace //anonymous
{

class numpunct_thousands_separator: public std::numpunct<char>
{
  //Override
  std::string do_grouping() const
  {
    return "\3";
  };
};

class numpunct_no_thousands_separator: public std::numpunct<char>
{
  //Override
  std::string do_grouping() const
  {
    return "";
  };
};

} //anonymous namespace

Glib::ustring GlomConversions::get_text_for_gda_value(Field::glom_field_type glom_type, const Gnome::Gda::Value& value, const NumericFormat& numeric_format)
{
  return get_text_for_gda_value(glom_type, value, std::locale("") /* the user's current locale */, numeric_format); //Get the current locale.
}


Glib::ustring GlomConversions::get_text_for_gda_value(Field::glom_field_type glom_type, const Gnome::Gda::Value& value, const std::locale& locale, const NumericFormat& numeric_format, bool iso_format)
{
  if(value.get_value_type() == Gnome::Gda::VALUE_TYPE_NULL) //The type can be null for any of the actual field types.
  {
    return "";
  }

  if( (glom_type == Field::TYPE_DATE) && (value.get_value_type() == Gnome::Gda::VALUE_TYPE_DATE))
  {
    Gnome::Gda::Date gda_date = value.get_date();

    tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    the_c_time.tm_year = gda_date.year - 1900; //C years start are the AD year - 1900. So, 01 is 1901.
    the_c_time.tm_mon = gda_date.month - 1; //C months start at 0.
    the_c_time.tm_mday = gda_date.day; //starts at 1

    return format_date(the_c_time, locale, iso_format);

    /*

    Glib::Date date((Glib::Date::Day)gda_date.day, (Glib::Date::Month)gda_date.month, (Glib::Date::Year)gda_date.year);
    return date.format_string("%x"); //%x means "is replaced by the locale's appropriate date representation".
    */
  }
  else if((glom_type == Field::TYPE_TIME) && (value.get_value_type() == Gnome::Gda::VALUE_TYPE_TIME))
  {
    Gnome::Gda::Time gda_time = value.get_time();

    tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    the_c_time.tm_hour = gda_time.hour;
    the_c_time.tm_min = gda_time.minute;
    the_c_time.tm_sec = gda_time.second;

    return format_time(the_c_time, locale, iso_format);
  }
  else if( (glom_type == Field::TYPE_NUMERIC) && (value.get_value_type() == Gnome::Gda::VALUE_TYPE_NUMERIC))
  {
    const GdaNumeric* gda_numeric = value.get_numeric();
    std::string text_in_c_locale;
    if(gda_numeric && gda_numeric->number) //A char* - I assume that it formatted as per the C locale. murrayc. TODO: Do we need to look at the other fields?
      text_in_c_locale = gda_numeric->number; //What formatting does this use?

    //Get an actual numeric value, so we can get a locale-specific text representation:
    std::stringstream the_stream;
    the_stream.imbue( std::locale::classic() ); //The C locale.
    the_stream.str(text_in_c_locale); //Avoid using << because Glib::ustinrg does implicit character conversion with that.

    double number = 0;
    the_stream >> number;

    //Get the locale-specific text representation, in the required format:
    std::stringstream another_stream;
    another_stream.imbue(locale); //Tell it to parse stuff as per this locale.

    //Numeric formatting:
    if(!iso_format)
    {

      if(!numeric_format.m_use_thousands_separator)
      {
        std::locale locale_modified(locale, new numpunct_no_thousands_separator()); //The locale takes ownership.
        another_stream.imbue(locale_modified);
      }

      if(numeric_format.m_decimal_places_restricted)
      {
        another_stream << std::fixed;
        another_stream << std::setprecision(numeric_format.m_decimal_places); //precision means number of decimal places when using std::fixed.
      }

      if(!(numeric_format.m_currency_symbol.empty()))
      {
        another_stream << numeric_format.m_currency_symbol << " ";
      }
    }

    another_stream << number;
    Glib::ustring text = another_stream.str();  //Avoid using << because Glib::ustring does implicit character conversion with that.

    if(locale == std::locale("") /* The user's current locale */)
    {
      //Converts from the user's current locale to utf8. I would prefer a generic conversion from any locale,
      // but there is no such function, and it's hard to do because I don't know how to get the encoding name from the std::locale()
      text = Glib::locale_to_utf8(text); 
    }

    return text; //Do something like Glib::locale_to_utf(), but with the specified locale instead of the current locale.
  }
  else
  {
    return value.to_string();
  }
}

Gnome::Gda::Value GlomConversions::parse_value(double number)
{
  //This is just a way to get a NUMERIC Gda::Value from a numeric type:
  //Try to parse the inputted number, according to the current locale.

  GdaNumeric gda_numeric = {0, 0, 0};

  //Then generate a canonical representation of the number:
  std::stringstream clocale_stream;
  clocale_stream.imbue( std::locale::classic() ); //The C locale.
  clocale_stream << number;
  Glib::ustring number_canonical_text = clocale_stream.str(); //Avoid << because it does implicit character conversion (though that might not be a problem here. Not sure). murrayc

  //TODO: What about the precision and width values?
  /* From the postgres docs:
  *  The scale of a numeric is the count of decimal digits in the fractional part, to the right of the decimal point.
  * The precision of a numeric is the total count of significant digits in the whole number, that is, the number of digits to both sides of the decimal point.
  * So the number 23.5141 has a precision of 6 and a scale of 4. Integers can be considered to have a scale of zero.
  */
  gda_numeric.number = g_strdup(number_canonical_text.c_str());

  return Gnome::Gda::Value(&gda_numeric);
}

Gnome::Gda::Value GlomConversions::parse_value(Field::glom_field_type glom_type, const Glib::ustring& text, bool& success, bool iso_format)
{
  NumericFormat ignore_format;
  return parse_value(glom_type, text, ignore_format, success, iso_format);
}

Gnome::Gda::Value GlomConversions::parse_value(Field::glom_field_type glom_type, const Glib::ustring& text, const NumericFormat& numeric_format, bool& success, bool iso_format)
{
  std::locale the_locale = (iso_format ? std::locale::classic() :  std::locale("") /* The user's current locale */);

  //Put a NULL in the database for empty dates, times, and numerics, because 0 would be an actual value.
  //But we use "" for strings, because the distinction between NULL and "" would not be clear to users.
  if(text.empty())
  {
    if( (glom_type == Field::TYPE_DATE) || (glom_type ==  Field::TYPE_TIME) || (glom_type ==  Field::TYPE_NUMERIC) )
    {
      Gnome::Gda::Value null_value;
      success = true;
      return null_value;
    }
  }

  if(glom_type == Field::TYPE_DATE)
  {
    tm the_c_time = parse_date(text, the_locale, success);

    Gnome::Gda::Date gda_date = {0, 0, 0};
    gda_date.year = the_c_time.tm_year + 1900; //The C time starts at 1900.
    gda_date.month = the_c_time.tm_mon + 1; //The C month starts at 0.
    gda_date.day = the_c_time.tm_mday; //THe C mday starts at 1.

    return Gnome::Gda::Value(gda_date);
  }
  else if(glom_type == Field::TYPE_TIME)
  {
    tm the_c_time = parse_time(text, the_locale, success);

    Gnome::Gda::Time gda_time = {0, 0, 0, 0};
    gda_time.hour = the_c_time.tm_hour;
    gda_time.minute = the_c_time.tm_min;
    gda_time.second = the_c_time.tm_sec;

    return Gnome::Gda::Value(gda_time);
  }
  else if(glom_type == Field::TYPE_NUMERIC)
  {
    Glib::ustring text_to_parse = util_trim_whitespace(text);

    if(!(numeric_format.m_currency_symbol.empty()))
    {
      //Remove the currency symbol:
      const Glib::ustring prefix = text_to_parse.substr(0, numeric_format.m_currency_symbol.size());
      if(text_to_parse.substr(0, numeric_format.m_currency_symbol.size()) == numeric_format.m_currency_symbol)
      {
        text_to_parse = text_to_parse.substr(numeric_format.m_currency_symbol.size());
        text_to_parse = util_trim_whitespace(text_to_parse); //remove any whitespace between the currency symbol and the number.
      }
    }


    //text_to_parse = Base_DB::string_trim(text_to_parse, " ");

    //Try to parse the inputted number, according to the current locale.
    std::stringstream the_stream;
    the_stream.imbue( the_locale ); //Parse it as per the current locale.
    the_stream.str(text_to_parse); //Avoid << because it does implicit character conversion (though that might not be a problem here. Not sure). murrayc
    double the_number = 0;
    the_stream >> the_number;  //TODO: Does this throw any exception if the text is an invalid time?

    GdaNumeric gda_numeric = {0, 0, 0};

    //Then generate a canonical representation of the number:

    std::stringstream clocale_stream;
    clocale_stream.imbue( std::locale::classic() ); //The C locale.
    clocale_stream << the_number;
    Glib::ustring  number_canonical_text = clocale_stream.str(); //Avoid << because it does implicit character conversion (though that might not be a problem here. Not sure). murrayc

    //TODO: What about the precision and width values?
    /* From the postgres docs:
    *  The scale of a numeric is the count of decimal digits in the fractional part, to the right of the decimal point.
    * The precision of a numeric is the total count of significant digits in the whole number, that is, the number of digits to both sides of the decimal point.
    * So the number 23.5141 has a precision of 6 and a scale of 4. Integers can be considered to have a scale of zero.
    */
    gda_numeric.number = g_strdup(number_canonical_text.c_str());

    success = true; //Can this ever fail?
    return Gnome::Gda::Value(&gda_numeric);
  }
  else if(glom_type == Field::TYPE_BOOLEAN)
  {
    success = true;
    return Gnome::Gda::Value( (text == "TRUE" ? true : false) ); //TODO: Internationalize this, but it should never be used anyway.
  }

  success = true;
  return Gnome::Gda::Value(text);

}

tm GlomConversions::parse_date(const Glib::ustring& text, bool& success)
{
  return parse_date( text, std::locale("") /* the user's current locale */, success ); //Get the current locale.
}

tm GlomConversions::parse_date(const Glib::ustring& text, const std::locale& locale, bool& success)
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
 
  typedef std::time_get<char> type_time_get;
  typedef type_time_get::iter_type type_iterator;

  const type_time_get& tg = std::use_facet<type_time_get>(locale);

  type_iterator the_begin(the_stream);
  type_iterator the_end;

  tg.get_date(the_begin, the_end, the_stream, err, &the_c_time);

  if(err != std::ios_base::failbit)
  {
    success = true;
  }
  else
  {
    //time_get can fail just because you have entered "1/2/1903" instead "01/02/1903", so let's try another, more liberal, way:
    Glib::Date date;
    date.set_parse(text); //I think this uses the current locale. murrayc.
    if(date.valid())
    {
      tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
      the_c_time.tm_year = date.get_year() - 1900; //C years start are the AD year - 1900. So, 01 is 1901.
      the_c_time.tm_mon = date.get_month() - 1; //C months start at 0.
      the_c_time.tm_mday = date.get_day(); //starts at 1

      success = true;
    }
    else //It really really failed.
    {
      //Note that 0 would be invalid for some of these.
      tm blank_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
      blank_time.tm_mday = 1;
      blank_time.tm_mon = 1;
      the_c_time = blank_time;
      success = false;
    }
  }

  //Prevent some nonsense values:
  if(the_c_time.tm_mday == 0)
    the_c_time.tm_mday = 1;

  if(the_c_time.tm_mon == 0)
      the_c_time.tm_mon = 1;

  return the_c_time;
}


tm GlomConversions::parse_time(const Glib::ustring& text, bool& success)
{
  //return parse_time( text, std::locale("") /* the user's current locale */ ); //Get the current locale.

  //time_get() does not seem to work with non-C locales.  TODO: Try again.
  tm the_time = parse_time( text, std::locale("") /* the user's current locale */, success );
  if(success)
    return the_time;
  else
  {
    //Fallback:
    //Try interpreting it as the C locale instead.
    //For instance, time_get::get_time() does not seem to be able to parse any time in a non-C locale (even "en_US" or "en_US.UTF-8").
    return parse_time( text, std::locale::classic() /* the C locale */, success );
  }
}


tm GlomConversions::parse_time(const Glib::ustring& text, const std::locale& locale, bool& success)
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
  {
    success = true;
    return the_c_time;
  }
  else
  {
    tm blank_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    success = false;
    return blank_time;
  }
}



/* This requires an extension to the standard - time_get<>.get().:
tm GlomConversions::parse_tm(const Glib::ustring& text, const std::locale& locale, char format)
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

bool GlomConversions::value_is_empty(const Gnome::Gda::Value& value)
{
  switch(value.get_value_type())
  {
    case(Gnome::Gda::VALUE_TYPE_NULL):
      return true;
    case(Gnome::Gda::VALUE_TYPE_STRING):
      return value.get_string().empty();
    default:
      return false; //None of the other types can be empty. (An empty numeric, date, or time type shows up as a null.
  }
}

Gnome::Gda::Value GlomConversions::get_empty_value(Field::glom_field_type field_type)
{
  switch(field_type)
  {
    case(Field::TYPE_TEXT):
      return Gnome::Gda::Value( Glib::ustring() ); //Use an empty string instead of a null for text fields, because the distinction is confusing for users, and gives no advantages.
    default:
      return Gnome::Gda::Value(); //A NULL instance, because there is no suitable empty value for numeric, date, or time fields.
  }
}

Gnome::Gda::Value GlomConversions::get_example_value(Field::glom_field_type field_type)
{
  switch(field_type)
  {
    case(Field::TYPE_BOOLEAN):
      return Gnome::Gda::Value(true);
    case(Field::TYPE_DATE):
    {
      bool success = false;
      return parse_value(field_type, "01/02/03", success, true /* iso_format */);
    }
    case(Field::TYPE_NUMERIC):
    {
      bool success = false;
      return parse_value(field_type, "1", success, true /* iso_format */);
    }
    case(Field::TYPE_TEXT):
      return Gnome::Gda::Value( Glib::ustring("example") ); //Use an empty string instead of a null for text fields, because the distinction is confusing for users, and gives no advantages.
    case(Field::TYPE_TIME):
    {
      bool success = false;
      return parse_value(field_type, "01:02", success, true /* iso_format */);
    }
    default:
      return Gnome::Gda::Value();
  }
}

Glib::ustring GlomConversions::util_trim_whitespace(const Glib::ustring& text)
{
  //TODO_Performance:

  Glib::ustring result = text;

  //Find non-whitespace from front:
  Glib::ustring::size_type posFront = Glib::ustring::npos;
  Glib::ustring::size_type pos = 0;
  for(Glib::ustring::iterator iter = result.begin(); iter != result.end(); ++iter)
  {
    if(!Glib::Unicode::isspace(*iter))
    {
      posFront = pos;
      break;
    }

    ++pos;
  }

  //Remove the white space from the front:
  result = result.substr(posFront);


 //Find non-whitespace from back:
  Glib::ustring::size_type posBack = Glib::ustring::npos;
  pos = 0;
  for(Glib::ustring::reverse_iterator iter = result.rbegin(); iter != result.rend(); ++iter)
  {
    if(!Glib::Unicode::isspace(*iter))
    {
      posBack = pos;
      break;
    }

    ++pos;
  }

  //Remove the white space from the front:
  result = result.substr(0, result.size() - posBack);

  return result;
}



Glib::ustring util_build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const Glib::ustring& where_clause)
{
  Glib::ustring result;

  Glib::ustring sql_part_fields;

  typedef std::list<Relationship> type_list_relationships;
  type_list_relationships list_relationships;

  for(type_vecLayoutFields::const_iterator iter =  fieldsToGet.begin(); iter != fieldsToGet.end(); ++iter)
  {
    if(iter != fieldsToGet.begin())
      sql_part_fields += ", ";


    if(!iter->get_has_relationship_name())
    {
      sql_part_fields += ( table_name + "." );
    }
    else
    {
      Glib::ustring relationship_name = iter->get_relationship_name();
      if(!relationship_name.empty())
      {
        const Relationship relationship = iter->m_relationship;

        const Glib::ustring field_table_name = relationship.get_to_table();
        sql_part_fields += ( field_table_name + "." );

        //Add the relationship to the list:
        type_list_relationships::const_iterator iterFind = std::find_if(list_relationships.begin(), list_relationships.end(), predicate_FieldHasName<Relationship>( relationship_name ) );
        if(iterFind == list_relationships.end()) //If the table is not yet in the list:
          list_relationships.push_back(relationship);
      }
    }

    sql_part_fields += iter->get_name();
  }

  result =  "SELECT " + sql_part_fields +
    " FROM " + table_name;

  //LEFT OUTER JOIN will get the field values from the other tables, and give us our fields for this table even if there is no corresponding value in the other table.
  Glib::ustring sql_part_leftouterjoin; 
  for(type_list_relationships::const_iterator iter = list_relationships.begin(); iter != list_relationships.end(); ++iter)
  {
    const Relationship& relationship = *iter;
    sql_part_leftouterjoin += " LEFT OUTER JOIN " + relationship.get_to_table() +
      " ON (" + relationship.get_from_table() + "." + relationship.get_from_field() + " = " +
      relationship.get_to_table() + "." + relationship.get_to_field() +
      ")";
  }

  result += sql_part_leftouterjoin;

  if(!where_clause.empty())
    result += " WHERE " + where_clause;

  return result;
}


type_list_values_with_second get_choice_values(const LayoutItem_Field& field)
{
  Glib::ustring choice_relationship_name, choice_field, choice_second;
  field.get_choices(choice_relationship_name, choice_field, choice_second);

  const Relationship relationship = field.m_choices_related_relationship;
  const Glib::ustring to_table = relationship.get_to_table();

  const bool with_second = !choice_second.empty();
  const Glib::ustring sql_second = to_table + "." + choice_second;

  //Get possible values from database, sorted by the first column.
  Glib::ustring sql_query = "SELECT " + to_table + "." + choice_field;
  if(with_second)
    sql_query += ", " + sql_second;

  sql_query += " FROM " + relationship.get_to_table() + " ORDER BY " + to_table + "." + choice_field;

  //Connect to database:
  sharedptr<SharedConnection> connection = ConnectionPool::get_instance()->connect();


  std::cout << "get_choice_values: Executing SQL: " << sql_query << std::endl;
  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = connection->get_gda_connection()->execute_single_command(sql_query);

  type_list_values_with_second list_values;
  if(datamodel)
  {
    guint count = datamodel->get_n_rows();
    for(guint row = 0; row < count; ++row)
    {
      std::pair<Gnome::Gda::Value, Gnome::Gda::Value> itempair;
      itempair.first = datamodel->get_value_at(0, row);

      if(with_second)
        itempair.second = datamodel->get_value_at(1, row);

      list_values.push_back(itempair);
    }
  }

  return list_values;
}

