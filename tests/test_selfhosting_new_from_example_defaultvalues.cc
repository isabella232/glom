/* Glom
 *
 * Copyright (C) 2010-2013 Openismus GmbH
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

#include "tests/test_selfhosting_utils.h"
#include <libglom/init.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/db_utils.h>
#include <glib.h> //For g_assert()
#include <iostream>
#include <cstdlib> //For EXIT_SUCCESS and EXIT_FAILURE

static bool test(Glom::Document::HostingMode hosting_mode)
{
  auto document = std::make_shared<Glom::Document>();
  const bool recreated =
    test_create_and_selfhost_from_example("example_smallbusiness.glom", document, hosting_mode);
  if(!recreated)
  {
    std::cerr << G_STRFUNC << ": Recreation failed.\n";
    return false;
  }



  //Check a numeric field:
  Glib::ustring table_name = "invoice_lines";
  if(!test_table_exists(table_name, document))
  {
    return false;
  }

  auto field = document->get_field(table_name, "count");
  if(!field)
  {
    std::cerr << G_STRFUNC << ": Failure: Could not get field.\n";
    return false;
  }

  Gnome::Gda::Value default_value = field->get_default_value();
  if(default_value.get_value_type() != GDA_TYPE_NUMERIC)
  {
    std::cerr << G_STRFUNC << ": Failure: Unexpected type of default value.\n";
    std::cerr << "  type=" << g_type_name(default_value.get_value_type()) << std::endl;
    return false;
  }

  const auto num = Glom::Conversions::get_double_for_gda_value_numeric(default_value);
  if(num != 1)
  {
    std::cerr << G_STRFUNC << ": Failure: Unexpected value of default value:" << num << std::endl;
    return false;
  }


  //Check a text field:
  table_name = "contacts";
  if(!test_table_exists(table_name, document))
  {
    return false;
  }

  field = document->get_field(table_name, "website");
  if(!field)
  {
    std::cerr << G_STRFUNC << ": Failure: Could not get field.\n";
    return false;
  }

  default_value = field->get_default_value();
  if(default_value.get_value_type() != G_TYPE_STRING)
  {
    std::cerr << G_STRFUNC << ": Failure: Unexpected type of default value.\n";
    std::cerr << "  type=" << g_type_name(default_value.get_value_type()) << std::endl;
    return false;
  }

  const auto str = default_value.get_string();
  if(str != "http://")
  {
    std::cerr << G_STRFUNC << ": Failure: Unexpected value of default value: " << str << std::endl;
    return false;
  }


  test_selfhosting_cleanup();

  return true;
}

int main()
{
  Glom::libglom_init();

  //We run this test in several locales via
  //test_selfhosting_new_from_example_in_locales.sh,
  //so we do this so the locale will really be used:
  setlocale(LC_ALL, "");

  const auto result = test_all_hosting_modes(sigc::ptr_fun(&test));

  Glom::libglom_deinit();

  return result;
}
