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
#include <libglom/init.h>
#include <libglom/db_utils_export.h>
#include <glib.h> //For g_assert()
#include <iostream>
#include <cstdlib> //For EXIT_SUCCESS and EXIT_FAILURE


static bool test(Glom::Document::HostingMode hosting_mode)
{
  auto document = std::make_shared<Glom::Document>();
  const bool recreated = 
    test_create_and_selfhost_from_example("example_music_collection.glom", document, hosting_mode);
  if(!recreated)
  {
    std::cerr << G_STRFUNC << ": Recreation failed.\n";
    return false;
  }
  
  Glom::FoundSet found_set; //TODO: Test a where clause.
  found_set.m_table_name = "albums";
  Glom::Document::type_list_layout_groups layout_groups;
  auto group = std::make_shared<Glom::LayoutGroup>();
  layout_groups.emplace_back(group);
  auto field_name = std::make_shared<Glom::LayoutItem_Field>();
  field_name->set_name("name");
  group->add_item(field_name);
  auto field_year = std::make_shared<Glom::LayoutItem_Field>();
  field_year->set_name("year");
  group->add_item(field_year);
  
  std::stringstream the_stream;
  Glom::DbUtils::export_data_to_stream(document, the_stream, found_set, layout_groups);
  const auto text = the_stream.str();
  std::cout << "debug: " << text << std::endl;

  if(text.empty())
  {
    std::cerr << G_STRFUNC << ": Failed: text was empty.\n";
    return false;
  }

  if(text.find("Superfly") == std::string::npos)
  {
    std::cerr << G_STRFUNC << ": Failed: text did not contain the expected text.\n";
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
