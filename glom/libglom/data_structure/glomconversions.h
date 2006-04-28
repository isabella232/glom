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

#ifndef GLOM_DATASTRUCTURE_GLOMCONVERSIONS_H
#define GLOM_DATASTRUCTURE_GLOMCONVERSIONS_H

#include <glom/libglom/data_structure/field.h>
#include <glom/libglom/data_structure/numeric_format.h>

#include <glom/libglom/data_structure/layout/layoutitem_field.h>
#include <libxml++/libxml++.h>

namespace GlomConversions
{
  ///Get text for display to the user.
  Glib::ustring get_text_for_gda_value(Field::glom_field_type glom_type, const Gnome::Gda::Value& value, const NumericFormat& numeric_format = NumericFormat());
  Glib::ustring get_text_for_gda_value(Field::glom_field_type glom_type, const Gnome::Gda::Value& value, const std::locale& locale, const NumericFormat& numeric_format = NumericFormat(), bool iso_format = false);

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

  Glib::ustring format_tm(const tm& tm_data, const std::locale& locale, const char* format);
  //static tm parse_tm(const Glib::ustring& text, const std::locale& locale, char format);

  bool value_is_empty(const Gnome::Gda::Value& value);
  Gnome::Gda::Value get_empty_value(Field::glom_field_type field_type);

  /** This is like get_empty_value(), but it never returns a NULL-type object, 
   * which would be useless to calculations.
   */
  Gnome::Gda::Value get_empty_value_suitable_for_python(Field::glom_field_type field_type);

  Gnome::Gda::Value get_example_value(Field::glom_field_type field_type);

  Glib::ustring get_escaped_binary_data(guint8* buffer, size_t buffer_size);
  Gnome::Gda::Value parse_escaped_binary_data(const Glib::ustring& escaped_data);

  Gnome::Gda::Value convert_value(const Gnome::Gda::Value& value, Field::glom_field_type target_glom_type);
}

//Copied from Postgres's PQunescapeBytea() so I don't have the trouble of finding and linking to the 
//postgres libraries directly, without the benefit of a pkg-config .pc file. murrayc.
unsigned char *
Glom_PQunescapeBytea(const unsigned char *strtext, size_t *retbuflen);

#endif //GLOM_DATASTRUCTURE_GLOMCONVERSIONS_H

