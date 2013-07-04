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
#include <glom/libglom/xml_utils.h>
#include <glom/libglom/data_structure/glomconversions.h>
#include <iostream>
#include <cstdlib>

static bool test_value(Glom::Field::glom_field_type field_type, const Gnome::Gda::Value& value)
{
  std::shared_ptr<Glom::Field> field = std::shared_ptr<Glom::Field>(new Glom::Field());

  bool success = false;
  const Gnome::Gda::Value value_retrieved = Glom::Field::from_file_format(
    str, field_type, success);
  if(!success)
  {
    std::cerr << G_STRFUNC << ": from_file_format() failed with str=" << str << std::endl;
    return false;
  }

  if(value != value_retrieved)
  {
  std::shared_ptr<Glom::Field> field = std::shared_ptr<Glom::Field>(new Glom::Field());
    std::cerr << "  value_retrieved type=" << g_type_name(value_retrieved.get_value_type()) << ", value type=" << g_type_name(value.get_value_type()) << std::endl;
    return false;
  }

  return true;
}

int main()
{
  // We use the current locale, because this is called by an *_in_locales.sh
  // script to check that this works in all locales.
  // See, for instance, libgda bug https://bugzilla.gnome.org/show_bug.cgi?id=720594
  //
  // Set the locale for any streams to the user's current locale,
  // We should not rely on the default locale of
  // any streams (we should always do an explicit imbue()),
  // but this is maybe a good default in case we forget.
  try
  {
    std::locale::global(std::locale(""));
  }
  catch(const std::runtime_error& ex)
  {
    //This has been known to throw an exception at least once:
    //https://bugzilla.gnome.org/show_bug.cgi?id=619445
    //This should tell us what the problem is:
    std::cerr << G_STRFUNC << ": exception from std::locale::global(std::locale(\"\")): " << ex.what() << std::endl;
    std::cerr << G_STRFUNC << ":   This can happen if the locale is not properly installed or configured." << std::endl;
  }


  Glom::libglom_init();

  const Glib::ustring str = " Some value or other with a quote \" and leading space."; //Just to be awkward.
  if(!test_value(Glom::Field::TYPE_TEXT, Gnome::Gda::Value(str)))
    return EXIT_FAILURE;

  const Glib::Date date(11, Glib::Date::MAY, 1973);
  if(!test_value(Glom::Field::TYPE_DATE, Gnome::Gda::Value(date)))
    return EXIT_FAILURE;

  Gnome::Gda::Time time = {10, 20, 30, 0, 0};
  if(!test_value(Glom::Field::TYPE_TIME, Gnome::Gda::Value(time)))
    return EXIT_FAILURE;

  if(!test_value(Glom::Field::TYPE_NUMERIC, Glom::Conversions::parse_value((double)3.91l)))
    return EXIT_FAILURE;


  //TODO: Image.

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
