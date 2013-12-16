/* Glom
 *
 * Copyright (C) 2013 Openismus GmbH
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

#include <glom/libglom/init.h>
#include <glom/libglom/data_structure/glomconversions.h>
#include <iostream>
#include <cstdlib>

static bool test_string_value(const Glib::ustring& str)
{
  const Glom::Field::glom_field_type field_type = Glom::Field::TYPE_TEXT;

  bool success = false;
  const Gnome::Gda::Value value =
    Glom::Conversions::parse_value(field_type, str, success, true);
  if(!success)
  {
    std::cerr << G_STRFUNC << ": parse_value() failed." << std::endl;
    return false;
  }

  const Glib::ustring retrieved = value.get_string();

  if(str != retrieved)
  {
    std::cerr << G_STRFUNC << ": Got string=" << retrieved << ", instead of string=" << str << std::endl;
    return false;
  }

  return true;
}


static bool test_numeric_value(double num)
{
  const Gnome::Gda::Value value =
    Glom::Conversions::parse_value(num);

  const double retrieved =
    Glom::Conversions::get_double_for_gda_value_numeric(value);

  if(num != retrieved)
  {
    std::cerr << G_STRFUNC << ": Got number=" << retrieved << ", instead of number=" << num << std::endl;
    return false;
  }

  return true;
}

int main()
{
  Glom::libglom_init();

  const Glib::ustring str = " Some value or other with a quote \" and leading space."; //Just to be awkward.
  if(!test_string_value(str))
    return EXIT_FAILURE;

  /* TODO:
  const Glib::Date date(11, Glib::Date::MAY, 1973);
  if(!test_value(Glom::Field::TYPE_DATE, Gnome::Gda::Value(date)))
    return EXIT_FAILURE;

  Gnome::Gda::Time time = {10, 20, 30, 0, 0};
  if(!test_value(Glom::Field::TYPE_TIME, Gnome::Gda::Value(time)))
    return EXIT_FAILURE;
  */

  if(!test_numeric_value((double)3.50f))
    return EXIT_FAILURE;


  //TODO: Image.

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
