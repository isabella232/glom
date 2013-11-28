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

#ifndef GLOM_DATASTRUCTURE_GLOMCONVERSIONS_H
#define GLOM_DATASTRUCTURE_GLOMCONVERSIONS_H

#include <libglom/data_structure/field.h>
#include <libglom/data_structure/numeric_format.h>

#include <libglom/data_structure/layout/layoutitem_field.h>
#include <libxml++/libxml++.h>

namespace Glom
{

namespace Conversions
{
  ///Get text for display to the user.
  Glib::ustring get_text_for_gda_value(Field::glom_field_type glom_type, const Gnome::Gda::Value& value, const NumericFormat& numeric_format = NumericFormat());
  Glib::ustring get_text_for_gda_value(Field::glom_field_type glom_type, const Gnome::Gda::Value& value, const std::locale& locale, const NumericFormat& numeric_format = NumericFormat(), bool iso_format = false);
  Glib::ustring get_text_for_gda_value(Field::glom_field_type glom_type, const Gnome::Gda::Value& value, const Glib::ustring& locale, const NumericFormat& numeric_format = NumericFormat(), bool iso_format = false);

  //This is easier than using the GdaNumeric API,
  //which normally involves text-to-number parsing.
  double get_double_for_gda_value_numeric(const Gnome::Gda::Value& value);

  Glib::ustring format_time(const tm& tm_data);
  Glib::ustring format_time(const tm& tm_data, const std::locale& locale, bool iso_format = false);
  Glib::ustring format_date(const tm& tm_data);
  Glib::ustring format_date(const tm& tm_data, const std::locale& locale, bool iso_format = false);

  Gnome::Gda::Value parse_value(double number);  
  Gnome::Gda::Value parse_value(Field::glom_field_type glom_type, const Glib::ustring& text, bool& success, bool iso_format = false);
  Gnome::Gda::Value parse_value(Field::glom_field_type glom_type, const Glib::ustring& text, const NumericFormat& numeric_format, bool& success, bool iso_format = false);

  tm parse_date(const Glib::ustring& text, bool& success);
  tm parse_date(const Glib::ustring& text, const std::locale& locale, bool& success);
  tm parse_time(const Glib::ustring& text, bool& success);
  tm parse_time(const Glib::ustring& text, const std::locale& locale, bool& success);

  /** Check that Glom can parse text representations of dates for which is has 
   * itself created the text representation.
   * This may fail in some locales if a translation of the date format is missing.
   *
   * @result true if parsing is working.
   */
  bool sanity_check_date_parsing();

  /** Check that Glom uses 4 digits to show years in text representations of dates.
   * This may fail in some locales if a translation of the date format is missing.
   * If it fails then Glom will default to using a dd/mm/yy format, which 
   * might be incorrect for the locale.
   *
   * @result true if 4 digits are used.
   */
  bool sanity_check_date_text_representation_uses_4_digit_years(bool debug_output = false);

  Glib::ustring format_tm(const tm& tm_data, const std::locale& locale, const char* format);
  //static tm parse_tm(const Glib::ustring& text, const std::locale& locale, char format);

  bool value_is_empty(const Gnome::Gda::Value& value);
  Gnome::Gda::Value get_empty_value(Field::glom_field_type field_type);

  Gnome::Gda::Value get_example_value(Field::glom_field_type field_type);

  /** Convert the value to a different type, if necessary.
   * Any text-to-number or number-to-text conversions will be in the ISO format,
   * ignoring the current locale.
   */
  Gnome::Gda::Value convert_value(const Gnome::Gda::Value& value, Field::glom_field_type target_glom_type);
  
} //namespace Conversions

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_GLOMCONVERSIONS_H

