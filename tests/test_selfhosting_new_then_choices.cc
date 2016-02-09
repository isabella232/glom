/* Glom
 *
 * Copyright (C) 2010 Openismus GmbH
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
#include "tests/test_utils.h"
#include <libglom/init.h>
#include <libglom/utils.h>
#include <libglom/db_utils.h>
#include <libglom/connectionpool.h>
#include <libglom/data_structure/glomconversions.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
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
  
  const Glib::ustring table_name = "invoice_lines";
 
  const auto field_with_choice = 
    get_field_on_layout(document, table_name, table_name, "product_id");
  if(!field_with_choice)
  {
    std::cerr << G_STRFUNC << ": Failure: Could not get the field with choice from the layout.\n";
    return false;
  }

  const Glom::Utils::type_list_values_with_second values_with_second = 
    Glom::Utils::get_choice_values_all(document, field_with_choice);
  if(values_with_second.size() != 3)
  {
    std::cerr << G_STRFUNC << ": Failure: There were an unexpected number of choices.\n";
    return false;
  }
  
  const std::pair<Gnome::Gda::Value, Glom::Utils::type_list_values> pair_values 
    = *(values_with_second.begin());
  if(pair_values.second.size() != 1)
  {
    std::cerr << G_STRFUNC << ": Failure: There were an unexpected number of field values in each choice.\n";
    return false;
  }

  test_selfhosting_cleanup();
 
  return true; 
}

int main()
{
  Glom::libglom_init();
  
  const auto result = test_all_hosting_modes(sigc::ptr_fun(&test));

  Glom::libglom_deinit();

  return result;
}
