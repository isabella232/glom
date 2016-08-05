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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */


#include <libglom/libglom_config.h> // For HAVE_STRPTIME

#include <libglom/data_structure/glomconversions.h>
#include <libglom/utils.h>
#include <libglom/string_utils.h>

#include <glibmm/convert.h>
#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl
#include <iomanip>
#include <string.h> // for strlen, memset, strcmp

#include <glibmm/i18n-lib.h>

namespace Glom
{

Glib::ustring Conversions::format_time(const tm& tm_data)
{
  try
  {
    return format_time( tm_data, std::locale("") /* the user's current locale */ ); //Get the current locale.
  }
  catch(const std::runtime_error& ex)
  {
    std::cerr << G_STRFUNC << ": exception from std::locale(\"\")): " << ex.what() << std::endl;
    std::cerr << G_STRFUNC << ":   This can happen if the locale is not properly installed or configured.\n";
    return Glib::ustring();
  }
}


Glib::ustring Conversions::format_time(const tm& tm_data, const std::locale& locale, bool iso_format)
{
  if(iso_format)
  {
    return format_tm(tm_data, locale, "%T" /* I see no iso-format time in the list, but this looks like the standard C-locale format. murrayc*/);
  }
  else
    return format_tm(tm_data, locale, "%X" /* time */);
}

Glib::ustring Conversions::format_date(const tm& tm_data)
{
  try
  {
    return format_date( tm_data, std::locale("") /* the user's current locale */ ); //Get the current locale.
  }
  catch(const std::runtime_error& ex)
  {
    std::cerr << G_STRFUNC << ": exception from std::locale(\"\")): " << ex.what() << std::endl;
    std::cerr << G_STRFUNC << ":   This can happen if the locale is not properly installed or configured.\n";
    return Glib::ustring();
  }
}

#define GLOM_NON_TRANSLATED_LOCALE_DATE_FORMAT "%x"

// The % format to use to print and interpret dates, with 4-digit years:
static const gchar* c_locale_date_format = nullptr;

static inline const char* glom_get_locale_date_format()
{
  if(!c_locale_date_format)
  {
    //Get the current LC_TIME value:

    //Unset LANGUAGE because it affects what setlocale(LC_TIME, NULL) returns,
    //though it doesn't affect what, for instance, printf() uses. So unsetting
    //it lets us get the correct LC_TIME locale.
    const char* ENV_LANGUAGE = "LANGUAGE";
    char* language = getenv(ENV_LANGUAGE);
    if(language)
      language = g_strdup(language);

    setenv(ENV_LANGUAGE, "", 1 /* replace */);

    //We copy the string because setlocale() probably returns the same array
    //each time.
    //Even when the LC_TIME environment variable is not set, we still seem
    //to get a useful value here, based on LC_ALL, for instance.
    char* lc_time = setlocale(LC_TIME, NULL);
    if(lc_time)
      lc_time = g_strdup(lc_time);

    //std::cout << "DEBUG: LC_TIME: " << lc_time << std::endl;

    char* old_lc_messages = nullptr;
    bool changed_lc_messages = false;

    if(lc_time)
    {
      //We call setlocale() so we get the %x translation that's appropriate for the
      //value of LC_TIME's locale,
      //Otherwise _() would fail if, for instance, gettext() just looked for the
      //translation for en_US.UTF-8, but LC_NUMERIC was set to en_GB.UTF-8
      //(which uses 2 digits when using %x).

      //Get the current LC_MESSAGES value:
      old_lc_messages = g_strdup(setlocale(LC_MESSAGES, NULL));
      if(old_lc_messages)
        old_lc_messages = g_strdup(old_lc_messages);

      //Change LC_MESSAGES to change how gettext works():
      if(g_strcmp0(lc_time, old_lc_messages) != 0) {
        setlocale(LC_MESSAGES, lc_time);
        changed_lc_messages = true;
      }
    }

    /* TRANSLATORS: Please only translate this string if you know that strftime()
     * shows only 2 year digits when using format "x". We want to always display
     * 4 year digits. For instance, en_GB should translate it to "%d/%m/%Y".
     * Glom will show a warning in the terminal at startup if this is necessary
     * and default to %d/%m/%Y" if it detects a problem, but that might not be
     * correct for your locale.
     * Thanks.
     * xgettext:no-c-format */
    c_locale_date_format = _("%x");


    if(changed_lc_messages)
    {
      //Change LC_MESSAGES back:
      setlocale(LC_MESSAGES, old_lc_messages);
    }

    //Restore this:
    if(language) { //setenv() can't take NULL for this.
      setenv(ENV_LANGUAGE, language, 1 /* replace */);
   }

    g_free(old_lc_messages);
    g_free(lc_time);
    g_free(language);
  }

  //std::cout << G_STRFUNC << ": c_locale_date_format=" << c_locale_date_format << std::endl;
  return c_locale_date_format;
}

Glib::ustring Conversions::format_date(const tm& tm_data, const std::locale& locale, bool iso_format)
{
  if(iso_format)
    return format_tm(tm_data, locale, "%F" /* iso-format date */);
  else
  {
    /* TRANSLATORS: Please only translate this string if you know that strftime() shows only 2 year digits when using format "x". We want to always display 4 year digits. For instance, en_GB should translate it to "%d/%m/%Y". To discover if your locale has this problem, try the testdateput_allformats.cc test case in http://bugzilla.gnome.org/show_bug.cgi?id=334648. Thanks. */
    return format_tm(tm_data, locale, glom_get_locale_date_format() /* date */);
  }
}

bool Conversions::sanity_check_date_parsing()
{
  //A date that is really really the date that we mean:
  tm the_c_time;
  memset(&the_c_time, 0, sizeof(the_c_time));

  //We mean 22nd November 2008:
  the_c_time.tm_year = 2008 - 1900; //C years start are the AD year - 1900. So, 01 is 1901.
  the_c_time.tm_mon = 11 - 1; //C months start at 0.
  the_c_time.tm_mday = 22; //starts at 1

  //Get the current locale's text representation:
  const auto date_text = format_date(the_c_time);
  //std::cout << "DEBUG: 22nd November 2008 in this locale has this text represention: " << date_text << std::endl;

  //Try to parse it:
  bool success = false;
  tm parsed_date = parse_date(date_text, success);

  if(success)
  {
    //std::cout << "  DEBUG: parsed date: year=" << parsed_date.tm_year + 1900 << ", month=" << parsed_date.tm_mon + 1 << ", day=" << parsed_date.tm_mday << std::endl;
  }

  if(!success ||
     parsed_date.tm_year != the_c_time.tm_year ||
     parsed_date.tm_mon != the_c_time.tm_mon ||
     parsed_date.tm_mday != the_c_time.tm_mday)
  {
    //Note to translators: If you see this error in the terminal at startup then you need to translate the %x elsewhere.
    std::cerr << _("ERROR: sanity_check_date_parsing(): Sanity check failed: Glom could not parse a date's text representation that it generated itself, in this locale.") << " (" << std::locale("").name() << ")\n";

    //If translators cannot be relied upon to do this, maybe we should default to "%d/%m/%Y" when "%x" fails this test.

    return false;
  }

  return true;
}

bool Conversions::sanity_check_date_text_representation_uses_4_digit_years(bool debug_output)
{
  //A date that is really really the date that we mean:
  tm the_c_time;
  memset(&the_c_time, 0, sizeof(the_c_time));

  //We mean 22nd November 2008:
  the_c_time.tm_year = 2008 - 1900; //C years start are the AD year - 1900. So, 01 is 1901.
  the_c_time.tm_mon = 11 - 1; //C months start at 0.
  the_c_time.tm_mday = 22; //starts at 1

  //Get the current locale's text representation:
  const auto date_text = format_date(the_c_time);

  if(debug_output)
  {
    std::cout << "DEBUG: 22nd November 2008 in this locale (" << std::locale("").name() << ") has this text represention: " << date_text << std::endl;
  }

  //See if the year appears in full in that date.
  //There are probably some locales for which this fails.
  //Please tell us if there are.
  const auto pos = date_text.find("2008");
  if(pos == Glib::ustring::npos)
  {
    //Note to translators: If you see this error in the terminal at startup then you need to translate the %x elsewhere.
    std::cerr << _("ERROR: sanity_check_date_text_representation_uses_4_digit_year(): Sanity check failed: Glom does not seem to use 4 digits to display years in a date's text representation, in this locale. Defaulting to dd/mm/yyyy though this might be incorrect for your locale. This needs attention from a translator. Please file a bug - see http://www.glom.org") << std::endl;

    std::cout << "  Unexpected date text: " << date_text << std::endl;
    std::cout << "  Current locale: " << std::locale("").name() << std::endl;

    //Do not depend on translators to do what we ask.
    //Default to a common format, though this would be incorrect in some
    //locales, such as German.
    c_locale_date_format = "%d/%m/%Y";

    return false;
  }

  return true;
}


Glib::ustring Conversions::format_tm(const tm& tm_data, const std::locale& locale, const char* format)
{
  //This is based on docs found here:
  //http://www.roguewave.com/support/docs/sourcepro/stdlibref/time-put.html

  //Format it into this stream:
  typedef std::stringstream type_stream;
  type_stream the_stream;
  the_stream.imbue(locale); //Make it format things for this locale. (Actually, I don't know if this is necessary, because we mention the locale in the time_put<> constructor.

  // Get a time_put face:
  typedef std::time_put<char> type_time_put;
  const auto& tp = std::use_facet<type_time_put>(locale);

  //type_iterator begin(the_stream);
  tp.put(the_stream /* iter to beginning of stream */, the_stream, ' ' /* fill */, &tm_data, format, format + strlen(format) /* 'E' */ /* use locale's alternative format */);

  Glib::ustring text = the_stream.str();
  //std::cout << "debug: " << G_STRFUNC << ": result from tp.put: " << text << std::endl;

  try
  {
    if(locale == std::locale("") /* The user's current locale */)
    {
      // Converts from the user's current locale to utf8. I would prefer a generic conversion from any locale,
      // but there is no such function, and it's hard to do because I don't know how to get the encoding name from the std::locale()
      text = Glib::locale_to_utf8(text);
    }
  }
  catch(const std::runtime_error& ex)
  {
    std::cerr << G_STRFUNC << ": exception from std::locale(\"\")): " << ex.what() << std::endl;
    std::cerr << G_STRFUNC << ":   This can happen if the locale is not properly installed or configured.\n";
  }

  //std::cout << "debug: " << G_STRFUNC << ": returning: " << text << std::endl;
  return text; //TODO: Use something like Glib::locale_to_utf8()?

  /*
  //This is based on the code in Glib::Date::format_string(), which only deals with dates, but not times:

  const auto locale_format = Glib::locale_from_utf8("%X"); //%x means "is replaced by the locale's appropriate time representation".
  gsize bufsize = std::max<gsize>(2 * locale_format.size(), 128);

  do
  {
    const auto buf = Glib::make_unique_ptr_gfree(static_cast<char*>(g_malloc(bufsize)));

    // Set the first byte to something other than '\0', to be able to
    // recognize whether strftime actually failed or just returned "".
    buf.get()[0] = '\1';
    const auto len = std::strftime(buf.get(), bufsize, locale_format.c_str(), &tm_data);

    if(len != 0 || buf.get()[0] == '\0')
    {
      g_assert(len < bufsize);
      return Glib::locale_to_utf8(std::string(buf.get(), len));
    }
  }
  while((bufsize *= 2) <= 65536);

  // This error is quite unlikely (unless strftime is buggy).
  std::cerr << G_STRFUNC << ": maximum size of strftime buffer exceeded. Giving up.\n";

  return Glib::ustring();
  */
}


namespace //anonymous
{

/*
class numpunct_thousands_separator: public std::numpunct<char>
{
  //Override
  std::string do_grouping() const
  {
    return "\3";
  };
};
*/

class numpunct_no_thousands_separator: public std::numpunct<char>
{
  std::string do_grouping() const override
  {
    return "";
  };
};

} //anonymous namespace


Glib::ustring Conversions::get_text_for_gda_value(Field::glom_field_type glom_type, const Gnome::Gda::Value& value, const NumericFormat& numeric_format)
{
  try
  {
    return get_text_for_gda_value(glom_type, value, std::locale("") /* the user's current locale */, numeric_format); //Get the current locale.
  }
  catch(const std::runtime_error& ex)
  {
    std::cerr << G_STRFUNC << ": exception from std::locale(\"\")): " << ex.what() << std::endl;
    std::cerr << G_STRFUNC << ":   This can happen if the locale is not properly installed or configured.\n";
    return Glib::ustring();
  }
}

double Conversions::get_double_for_gda_value_numeric(const Gnome::Gda::Value& value)
{
  const auto vtype = value.get_value_type();
  if(vtype != GDA_TYPE_NUMERIC)
  {
    // Note that in case the database system does not support GdaNumeric
    // (such as sqlite) we fall back to double (see
    // FieldTypes::get_string_name_for_gdavaluetype), so try this as well.
    switch(vtype)
    {
      case G_TYPE_DOUBLE:
        return value.get_double();
      case G_TYPE_INT:
        return value.get_int();
      case G_TYPE_UINT:
        return value.get_uint();
      case G_TYPE_LONG:
        return value.get_long();
      case G_TYPE_ULONG:
        return value.get_ulong();
      case G_TYPE_INT64:
        return value.get_int64();
      case G_TYPE_UINT64:
        return value.get_uint64();
      default:
        std::cerr << G_STRFUNC << ": expected NUMERIC but GdaValue type is: " << g_type_name(value.get_value_type()) << std::endl;
        return 0;
    }
  }

  const auto numeric = value.get_numeric();
  return numeric.get_double();
}

Glib::ustring Conversions::get_text_for_gda_value(Field::glom_field_type glom_type, const Gnome::Gda::Value& value, const std::locale& locale, const NumericFormat& numeric_format, bool iso_format)
{
  if(value.is_null()) //The type can be null for any of the actual field types.
  {
    return Glib::ustring();
  }

  if(glom_type == Field::glom_field_type::DATE)
  {
    tm the_c_time;
    memset(&the_c_time, 0, sizeof(the_c_time));

    if(value.get_value_type() == G_TYPE_STRING)
    {
      // If a date is contained in a string type instead of a date type,
      // which can happen if the database system does not support dates
      // natively, then the string representation is always in ISO format.
      bool success;
      the_c_time = parse_date(value.get_string(), std::locale::classic(), success);
      if(!success)
        std::cerr << G_STRFUNC << ": Failed to convert string-represented date value\n";
    }
    else if(value.get_value_type() == G_TYPE_DATE)
    {
      Glib::Date gda_date = value.get_date();
      //Gnome::Gda::Date gda_date = value.get_date();

      //tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

      the_c_time.tm_year = gda_date.get_year() - 1900; //C years start are the AD year - 1900. So, 01 is 1901.
      the_c_time.tm_mon = gda_date.get_month() - 1; //C months start at 0.
      the_c_time.tm_mday = gda_date.get_day(); //starts at 1
    }
    else
    {
      std::cerr << G_STRFUNC << ": glom field type is DATE but GdaValue type is: " << g_type_name(value.get_value_type()) << std::endl;

      // Default
      the_c_time.tm_mday = 1;
    }

    return format_date(the_c_time, locale, iso_format);

    /*

    Glib::Date date((Glib::Date::Day)gda_date.day, (Glib::Date::Month)gda_date.month, (Glib::Date::Year)gda_date.year);
    return date.format_string("%x"); //%x means "is replaced by the locale's appropriate date representation".
    */
  }
  else if(glom_type == Field::glom_field_type::TIME)
  {
    tm the_c_time;
    memset(&the_c_time, 0, sizeof(the_c_time));
    if(value.get_value_type() == G_TYPE_STRING)
    {
      // If a time is contained in a string type instead of a gda time type,
      // which can happen if the database system does not support times
      // natively, then the string representation is always in ISO format.
      bool success;
      the_c_time = parse_time(value.get_string(), std::locale::classic(), success);
      if(!success)
        std::cerr << G_STRFUNC << ": Failed to convert string-represented time value\n";
    }
    else if(value.get_value_type() == GDA_TYPE_TIME)
    {
      Gnome::Gda::Time gda_time = value.get_time();

      //tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

      the_c_time.tm_hour = gda_time.hour;
      the_c_time.tm_min = gda_time.minute;
      the_c_time.tm_sec = gda_time.second;
    }
    else
    {
      std::cerr << G_STRFUNC << ": glom field type is TIME but GdaValue type is: " << g_type_name(value.get_value_type()) << std::endl;
    }

    return format_time(the_c_time, locale, iso_format);
  }
  else if(glom_type == Field::glom_field_type::NUMERIC)
  {
    const auto value_type = value.get_value_type();
    if(value_type != GDA_TYPE_NUMERIC
      && value_type != G_TYPE_DOUBLE
      && value_type != G_TYPE_INT) //SQLite uses int for summary field results.
    {
      std::cerr << G_STRFUNC << ": glom field type is NUMERIC but GdaValue type is: " << g_type_name(value.get_value_type()) << std::endl;
      return value.to_string();
    }

    const auto number = get_double_for_gda_value_numeric(value);

    //Get the locale-specific text representation, in the required format:
    //TODO: Use std::to_string(), but with the correct locale.
    std::stringstream another_stream;
    another_stream.imbue(locale); //Tell it to parse stuff as per this locale.

    constexpr auto precision = std::numeric_limits<decltype(number)>::max_digits10;
    another_stream << std::setprecision(precision);

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

        //precision means number of decimal places when using std::fixed:
        another_stream << std::setprecision(numeric_format.m_decimal_places);
      }
      else
      {
        //Increase the number of digits (even before the decimal point) we can
        //have until it uses the awkward e syntax. The default seems to be 7.
        another_stream << std::setprecision( NumericFormat::get_default_precision() );
      }

      if(!(numeric_format.m_currency_symbol.empty()))
      {
        std::string charset;
        Glib::get_charset(charset);
        Glib::ustring currency_symbol = Glib::convert_with_fallback(numeric_format.m_currency_symbol, charset, "UTF-8");
        // Uses convert_with_fallback(.) for the curreny symbol to avoid an
        // exception where the operator<<'s automatic conversion fails.
        // Incompatible encodings are possible since the currency symbol itself
        // is stored in the Glom document (UTF-8), whereas the stream encoding
        // depends on the user's locale (needed for the numeric value
        // representation).
        another_stream << currency_symbol << " ";
      }
    }

    another_stream << number;
    Glib::ustring text = another_stream.str();  //Avoid using << because Glib::ustring does implicit character conversion with that.

    try
    {
      if(locale == std::locale("") /* The user's current locale */)
      {
        // Converts from the user's current locale to utf8. I would prefer a generic conversion from any locale,
        // but there is no such function, and it's hard to do because I don't know how to get the encoding name from the std::locale()
        text = Glib::locale_to_utf8(text);
      }
    }
    catch(const std::runtime_error& ex)
    {
      std::cerr << G_STRFUNC << ": exception from std::locale(\"\")): " << ex.what() << std::endl;
      std::cerr << G_STRFUNC << ":   This can happen if the locale is not properly installed or configured.\n";
    }

    //std::cout << "debug: " << G_STRFUNC << ": number=" << number << ", text=" << text << std::endl;
    return text; //Do something like Glib::locale_to_utf(), but with the specified locale instead of the current locale.
  }
  else if(glom_type == Field::glom_field_type::TEXT)
  {
     return value.get_string();
  }
  else if(glom_type == Field::glom_field_type::BOOLEAN)
  {
    //This is used only by Field::to_file_format(),
    //and should never be shown in the UI.
    if(G_VALUE_TYPE(value.gobj()) == G_TYPE_BOOLEAN)
      return (value.get_boolean() ? "TRUE" : "FALSE" );
    else
      return "FALSE";
  }
  else if(glom_type == Field::glom_field_type::IMAGE)
  {
    //This function is only used for :
    //- UI-visible strings, but images should never be shown as text in the UI.
    //- Values in SQL queries, but we only do that for clauses (where/sort/order)
    //  which should never use image values.
    std::cerr << G_STRFUNC << ": Unexpected enumType::IMAGE field type: " << Utils::to_utype(glom_type) << std::endl;
    return Glib::ustring();
  }
  else
  {
    std::cerr << G_STRFUNC << ": Unexpected glom field type: " << Utils::to_utype(glom_type) << std::endl;
    return value.to_string();
  }
}

Gnome::Gda::Value Conversions::parse_value(double number)
{
  //This is just a way to get a NUMERIC Gnome::Gda::Value from a numeric type:
  Gnome::Gda::Numeric numeric;
  numeric.set_double(number);
  return Gnome::Gda::Value(numeric);
}

Gnome::Gda::Value Conversions::parse_value(Field::glom_field_type glom_type, const Glib::ustring& text, bool& success, bool iso_format)
{
  NumericFormat ignore_format;
  return parse_value(glom_type, text, ignore_format, success, iso_format);
}

Gnome::Gda::Value Conversions::parse_value(Field::glom_field_type glom_type, const Glib::ustring& text, const NumericFormat& numeric_format, bool& success, bool iso_format)
{
  std::locale the_locale;
  try
  {
    the_locale = (iso_format ? std::locale::classic() :  std::locale("") /* The user's current locale */);
  }
  catch(const std::runtime_error& ex)
  {
    std::cerr << G_STRFUNC << ": exception from std::locale(\"\")): " << ex.what() << std::endl;
    std::cerr << G_STRFUNC << ":   This can happen if the locale is not properly installed or configured.\n";
  }

  //Put a NULL in the database for empty dates, times, and numerics, because 0 would be an actual value.
  //But we use "" for strings, because the distinction between NULL and "" would not be clear to users.
  if(text.empty())
  {
    if( (glom_type == Field::glom_field_type::DATE) || (glom_type ==  Field::glom_field_type::TIME) || (glom_type ==  Field::glom_field_type::NUMERIC) )
    {
      Gnome::Gda::Value null_value;
      success = true;
      return null_value;
    }
  }

  if(glom_type == Field::glom_field_type::DATE)
  {
    tm the_c_time = parse_date(text, the_locale, success);

    // The C time starts at 1900 and the C month starts at 0.
    Glib::Date gda_date(the_c_time.tm_mday, static_cast<Glib::Date::Month>(the_c_time.tm_mon + 1), the_c_time.tm_year + 1900);

    return Gnome::Gda::Value(gda_date);
  }
  else if(glom_type == Field::glom_field_type::TIME)
  {
    tm the_c_time = parse_time(text, the_locale, success);

    if(!success)
    {
      //Fall back to trying both the current locale and C locale:
      the_c_time = parse_time(text, success);
    }

    Gnome::Gda::Time gda_time = {0, 0, 0, 0, 0};
    gda_time.hour = the_c_time.tm_hour;
    gda_time.minute = the_c_time.tm_min;
    gda_time.second = the_c_time.tm_sec;

    return Gnome::Gda::Value(gda_time);
  }
  else if(glom_type == Field::glom_field_type::NUMERIC)
  {
    Glib::ustring text_to_parse = Utils::trim_whitespace(text);

    if(!(numeric_format.m_currency_symbol.empty()))
    {
      //Remove the currency symbol:
      const auto prefix = text_to_parse.substr(0, numeric_format.m_currency_symbol.size());
      if(text_to_parse.substr(0, numeric_format.m_currency_symbol.size()) == numeric_format.m_currency_symbol)
      {
        text_to_parse = text_to_parse.substr(numeric_format.m_currency_symbol.size());
        text_to_parse = Utils::trim_whitespace(text_to_parse); //remove any whitespace between the currency symbol and the number.
      }
    }


    //text_to_parse = Base_DB::string_trim(text_to_parse, " ");

    //Try to parse the inputted number, according to the current locale.
    std::stringstream the_stream;
    the_stream.imbue( the_locale ); //Parse it as per the specified locale.
    the_stream.str(text_to_parse); //Avoid << because it does implicit character conversion (though that might not be a problem here. Not sure). murrayc
    double the_number = 0;
    the_stream >> the_number;  //TODO: Does this throw any exception if the text is an invalid number?

    //std::cout << "debug: " << G_STRFUNC << ": text=" << text_to_parse << ", number=" << the_number << std::endl;


    Gnome::Gda::Numeric numeric;
    numeric.set_double(the_number);

    success = true; //Can this ever fail?
    return Gnome::Gda::Value(numeric);
  }
  else if(glom_type == Field::glom_field_type::BOOLEAN)
  {
    success = true;
    return Gnome::Gda::Value(text.uppercase() == "TRUE"); //TODO: Internationalize this, but it should never be used anyway.
  }
  else if(glom_type == Field::glom_field_type::IMAGE)
  {
    //This function is only used for :
    //- UI-visible strings, but images should never be entered as text in the UI.
    std::cerr << G_STRFUNC << ": Unexpected enumType::IMAGE field type: " << Utils::to_utype(glom_type) << std::endl;
    return Gnome::Gda::Value();
  }

  success = true;
  return Gnome::Gda::Value(text);

}

tm Conversions::parse_date(const Glib::ustring& text, bool& success)
{
  try
  {
    return parse_date( text, std::locale("") /* the user's current locale */, success ); //Get the current locale.
  }
  catch(const std::runtime_error& ex)
  {
    std::cerr << G_STRFUNC << ": exception from std::locale(\"\")): " << ex.what() << std::endl;
    std::cerr << G_STRFUNC << ":   This can happen if the locale is not properly installed or configured.\n";

    tm the_c_time;
    memset(&the_c_time, 0, sizeof(the_c_time));
    return the_c_time;
  }
}

tm Conversions::parse_date(const Glib::ustring& text, const std::locale& locale, bool& success)
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
  //tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  tm the_c_time;
  memset(&the_c_time, 0, sizeof(the_c_time));

  //If the standard locale date format is not appropriate for display then it's also not appropriate for parsing,
  //because std::get_time() stupidly parses _only_ that format.
  //Some implementations parse extra formats, in unspecified/non-standard ways, but not g++'s libstdc++
  //So just use the fallback instead. It's probably good enough.
  const auto is_iso_locale = (locale == std::locale::classic());
  const auto skip_time_get = !is_iso_locale && (strcmp(GLOM_NON_TRANSLATED_LOCALE_DATE_FORMAT, glom_get_locale_date_format()) != 0);

  std::ios_base::iostate err = std::ios_base::goodbit;  //The initialization is essential because time_get seems to a) not initialize this output argument and b) check its value.

  if(!skip_time_get)
  {
    //For some reason, the stream must be instantiated after we get the facet. This is a worryingly strange "bug".
    type_stream the_stream;
    the_stream.imbue(locale); //Make it format things for this locale. (Actually, I don't know if this is necessary, because we mention the locale in the time_put<> constructor.
    the_stream << text;

    // Get a time_get facet:

    typedef std::time_get<char> type_time_get;
    typedef type_time_get::iter_type type_iterator;

    const auto& tg = std::use_facet<type_time_get>(locale);

    type_iterator the_begin(the_stream);
    type_iterator the_end;

    tg.get_date(the_begin, the_end, the_stream, err, &the_c_time);
  }
  else
  {
    //std::cout << "DEBUG: Skipping std::time_get<>  because it is incapable of parsing 4-digit years in the current locale.\n";
  }

  if(!skip_time_get && err != std::ios_base::failbit)
  {
    success = true;
  }
  else
  {
    //time_get can fail just because you have entered "1/2/1903" instead of "01/02/1903",
    //or maybe we chose to skip it because it is not useful in this locale,
    //so let's try another, more liberal, way:
    Glib::Date date;
    date.set_parse(text); //I think this uses the current locale. murrayc.
    if(date.valid())
    {
      //tm null_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
      tm null_c_time;
      memset(&null_c_time, 0, sizeof(null_c_time));

      the_c_time = null_c_time;

      if(date.get_year() != Glib::Date::BAD_YEAR)
        the_c_time.tm_year = date.get_year() - 1900; //C years start are the AD year - 1900. So, 01 is 1901.

      if(date.get_month() != Glib::Date::BAD_MONTH)
        the_c_time.tm_mon = date.get_month() - 1; //C months start at 0.

      if(date.get_day() != Glib::Date::BAD_DAY)
        the_c_time.tm_mday = date.get_day(); //starts at 1

      success = true;
    }
    else //It really really failed.
    {
      //Note that 0 would be invalid for some of these.
      //tm blank_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
      tm blank_time;
      memset(&blank_time, 0, sizeof(blank_time));

      blank_time.tm_mday = 1;
      blank_time.tm_mon = 1;
      the_c_time = blank_time;
      success = false;
    }
  }

  //Prevent some nonsense values:
  //tm_day starts from 1.
  if(the_c_time.tm_mday == 0)
    the_c_time.tm_mday = 1;

  //tm_mon starts from 0, so 0 is an acceptable value.
  //if(the_c_time.tm_mon == 0)
  //    the_c_time.tm_mon = 1;

  return the_c_time;
}


tm Conversions::parse_time(const Glib::ustring& text, bool& success)
{
  //Initialize output parameter:
  success = false;

  //return parse_time( text, std::locale("") /* the user's current locale */ ); //Get the current locale.

  //time_get() does not seem to work with non-C locales.  TODO: Try again.
  tm the_time;

  try
  {
    the_time = parse_time( text, std::locale("") /* the user's current locale */, success );
  }
  catch(const std::runtime_error& ex)
  {
    std::cerr << G_STRFUNC << ": exception from std::locale(\"\")): " << ex.what() << std::endl;
    std::cerr << G_STRFUNC << ":   This can happen if the locale is not properly installed or configured.\n";
  }

  if(success)
  {
    return the_time;
  }
  else
  {
    //Fallback:
    //Try interpreting it as the C locale instead.
    //For instance, time_get::get_time() does not seem to be able to parse any time in a non-C locale (even "en_US" or "en_US.UTF-8").
    return parse_time( text, std::locale::classic() /* the C locale */, success );
  }
}


tm Conversions::parse_time(const Glib::ustring& text, const std::locale& locale, bool& success)
{
  //std::cout << "debug: " << G_STRFUNC << ": text=" << text << std::endl;
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
  //tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  tm the_c_time;
  memset(&the_c_time, 0, sizeof(the_c_time));

  std::ios_base::iostate err = std::ios_base::goodbit; //The initialization is essential because time_get seems to a) not initialize this output argument and b) check its value.

  type_stream the_stream;
  the_stream.imbue(locale); //Make it format things for this locale. (Actually, I don't know if this is necessary, because we mention the locale in the time_put<> constructor.

  the_stream << text;

  // Get a time_get facet:
  typedef std::istreambuf_iterator<char, std::char_traits<char> > type_iterator;
  typedef std::time_get<char, type_iterator> type_time_get;
  const auto& tg = std::use_facet<type_time_get>(locale);

  type_iterator the_begin(the_stream);
  type_iterator the_end;

  tg.get_time(the_begin, the_end, the_stream, err, &the_c_time);

  if(err != std::ios_base::failbit)
  {
    success = true;
    return the_c_time;
  }

#ifdef HAVE_STRPTIME
  //Fall back to strptime():
  //This fallback will be used in most cases. TODO: Remove the useless? time_get<> code then?
  //It seems to be well known that time_get<> can parse much less than what time_put<> can generate:
  // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2070.html
  //
  //Try various formats:

  memset(&the_c_time, 0, sizeof(the_c_time));
  char* lastchar = strptime(text.c_str(), "%r" /* 12-hour clock time using the AM/PM notation */, &the_c_time);
  if(lastchar)
  {
    success = true;
    return the_c_time;
  }

  memset(&the_c_time, 0, sizeof(the_c_time));
  lastchar = strptime(text.c_str(), "%X" /* The time, using the locale's time format. */, &the_c_time);
  if(lastchar)
  {
    //std::cout << "debug: " << G_STRFUNC << ": %X: text=" << text << " was parsed as: hour=" << the_c_time.tm_hour << ", min=" << the_c_time.tm_min  << ", sec=" << the_c_time.tm_sec << std::endl;
    success = true;
    return the_c_time;
  }

  //Note: strptime() with "%OI" parses "01:00 PM" incorrectly as 01:00, though it claims to parse successfully.

  memset(&the_c_time, 0, sizeof(the_c_time));
  lastchar = strptime(text.c_str(), "%c" /* alternative 12-hour clock */, &the_c_time);
  if(lastchar)
  {
    //std::cout << "debug: " << G_STRFUNC << ": %c: text=" << text << " was parsed as: hour=" << the_c_time.tm_hour << ", min=" << the_c_time.tm_min  << ", sec=" << the_c_time.tm_sec << std::endl;
    success = true;
    return the_c_time;
  }

  //This seems to be the only one that can parse "01:00 PM":
  memset(&the_c_time, 0, sizeof(the_c_time));
  lastchar = strptime(text.c_str(), "%I : %M %p" /* 12 hours clock with AM/PM, without seconds. */, &the_c_time);
  if(lastchar)
  {
    //std::cout << "debug: " << G_STRFUNC << ": %I : %M %p: text=" << text << " was parsed as: hour=" << the_c_time.tm_hour << ", min=" << the_c_time.tm_min  << ", sec=" << the_c_time.tm_sec << std::endl;
    success = true;
    return the_c_time;
  }

  //std::cout << "  DEBUG: strptime(%c) failed on text=" << text << std::endl;

#endif // HAVE_STRPTIME
  //Nothing worked:
  //tm blank_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  tm blank_time ;
  memset(&blank_time , 0, sizeof(blank_time ));

  success = false;
  return blank_time;
}



/* This requires an extension to the standard - time_get<>.get().:
tm Conversions::parse_tm(const Glib::ustring& text, const std::locale& locale, char format)
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
  const auto& tg = std::use_facet<type_time_get>(locale);

  the_stream << text;

  //tm the_c_time = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  tm the_c_time;
  memset(&the_c_time, 0, sizeof(the_c_time));

  type_iterator the_begin(the_stream);
  type_iterator the_end;
  std::ios_base::iostate state;  // Unused
  std::ios_base::iostate err;
  tg.get(the_begin, the_end, the_stream, state, err, &the_c_time, format, 0);

  return the_c_time;
}
*/

bool Conversions::value_is_empty(const Gnome::Gda::Value& value)
{
  if(value.is_null())
    return true;

  switch(value.get_value_type())
  {
    case(0):
      return true; //Empty and invalid. It has not been initalized with a type.
    case(G_TYPE_STRING):
      return value.get_string().empty();
    default:
      return false; //None of the other types can be empty. (An empty numeric, date, or time type shows up as a GDA_TYPE_NULL).
  }
}

Gnome::Gda::Value Conversions::get_empty_value(Field::glom_field_type field_type)
{
  switch(field_type)
  {
    case(Field::glom_field_type::TEXT):
      return Gnome::Gda::Value( Glib::ustring() ); //Use an empty string instead of a null for text fields, because the distinction is confusing for users, and gives no advantages.
    default:
      return Gnome::Gda::Value(); //A NULL instance, because there is no suitable empty value for numeric, date, or time fields.
  }
}

Gnome::Gda::Value Conversions::get_example_value(Field::glom_field_type field_type)
{
  switch(field_type)
  {
    case(Field::glom_field_type::BOOLEAN):
      return Gnome::Gda::Value(true);
    case(Field::glom_field_type::DATE):
    {
      bool success = false;
      return parse_value(field_type, "01/02/03", success, true /* iso_format */);
    }
    case(Field::glom_field_type::NUMERIC):
    {
      bool success = false;
      return parse_value(field_type, "1", success, true /* iso_format */);
    }
    case(Field::glom_field_type::TEXT):
      return Gnome::Gda::Value( Glib::ustring("example") ); //Use an empty string instead of a null for text fields, because the distinction is confusing for users, and gives no advantages.
    case(Field::glom_field_type::TIME):
    {
      bool success = false;
      return parse_value(field_type, "01:02", success, true /* iso_format */);
    }
    default:
      return Gnome::Gda::Value();
  }
}

static bool vtype_is_numeric(GType vtype)
{
  switch(vtype)
  {
    case G_TYPE_DOUBLE:
    case G_TYPE_INT:
    case G_TYPE_UINT:
    case G_TYPE_LONG:
    case G_TYPE_ULONG:
    case G_TYPE_INT64:
    case G_TYPE_UINT64:
      return true;
    default:
      return false;
  }
}

Gnome::Gda::Value Conversions::convert_value(const Gnome::Gda::Value& value, Field::glom_field_type target_glom_type)
{
  const auto gvalue_type_target = Field::get_gda_type_for_glom_type(target_glom_type);
  const auto gvalue_type_source = value.get_value_type();
  if(gvalue_type_source == gvalue_type_target)
    return value; //No conversion necessary, and no loss of precision.

  const auto source_glom_type = Field::get_glom_type_for_gda_type(gvalue_type_source);
  if(source_glom_type == target_glom_type)
  {
    //Try to return the canonical type,
    //instead of just something of a similar GType:
    if((target_glom_type == Field::glom_field_type::NUMERIC) &&
      (vtype_is_numeric(gvalue_type_source)))
    {
      const auto number = get_double_for_gda_value_numeric(value);
      return parse_value(number);
    }
  }

  //Fallback for other conversions:
  const auto text = get_text_for_gda_value(source_glom_type, value, std::locale::classic(), NumericFormat(), true /* iso_format */);
  bool test = false;
  return parse_value(target_glom_type, text, test, true /* iso_format */);
}

} //namespace Glom

