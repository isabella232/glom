/* Glom
 *
 * Copyright (C) 2011 Openismus GmbH
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

#include "tests/test_utils.h"
#include "tests/test_utils_images.h"
#include <libglom/init.h>
#include <libglom/data_structure/field.h>
#include <iostream>

static void test_text_field()
{
  Glom::sharedptr<Glom::Field> field = Glom::sharedptr<Glom::Field>::create();
  field->set_glom_type(Glom::Field::TYPE_TEXT);

  const Gnome::Gda::Value value_original("text with \" double quote and ' single quote");
  const Glib::ustring str = field->to_file_format(value_original);
  g_assert(!str.empty());

  bool converted = false;
  const Gnome::Gda::Value value = field->from_file_format(str, converted);
  g_assert(converted);
  g_assert(value == value_original);
}

static void test_image_field()
{
  Glom::sharedptr<Glom::Field> field = Glom::sharedptr<Glom::Field>::create();
  field->set_glom_type(Glom::Field::TYPE_IMAGE);

  //TODO: Test an image too:
  const Gnome::Gda::Value value_original = get_value_for_image();
  g_assert(check_value_is_an_image(value_original));

  const Glib::ustring str = field->to_file_format(value_original);
  g_assert(!str.empty());

  bool converted = false;
  const Gnome::Gda::Value value = field->from_file_format(str, converted);
  g_assert(converted);
  g_assert(value == value_original);

  g_assert(check_value_is_an_image(value));
}

int main()
{
  Glom::libglom_init();

  test_text_field();
  test_image_field();

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
