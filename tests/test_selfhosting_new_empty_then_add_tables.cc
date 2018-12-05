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
#include <libglom/algorithms_utils.h>
#include <libglom/init.h>
#include <libglom/utils.h>
#include <libglom/db_utils.h>
#include <libglom/connectionpool.h>
#include <libglom/privs.h>
#include <glib.h> //For g_assert()
#include <iostream>
#include <cstdlib> //For EXIT_SUCCESS and EXIT_FAILURE

template<typename T_Container, typename T_Value>
bool contains(const T_Container& container, const T_Value& name)
{
  return Glom::Utils::find_exists(container, name);
}

static bool test(Glom::Document::HostingMode hosting_mode)
{
  //Create and self-host the document:
  auto document = std::make_shared<Glom::Document>();
    if(!(test_create_and_selfhost_new_database(document, hosting_mode, "test_db")))
  {
    std::cerr << G_STRFUNC << ": test_create_and_selfhost_new_database() failed\n";
    return false;
  }


  std::vector<Glib::ustring> table_names( {
    "sometable",
    } );

  //MySQL has a 64-character limit on SQL identifiers:
  /*
  if(hosting_mode != Glom::Document::HostingMode::MYSQL_SELF)
  {
    table_names.emplace_back("sometablewithaverylongnameyaddayaddayaddayaddayaddyaddayaddayaddayaddayaddayaddayaddayaddayaddayaddayaddayadda");
  }
  */

  //Add some tables, for the groups to have rights for:
  for(const auto& table_name : table_names)
  {
    if(!Glom::DbUtils::create_table_with_default_fields(document, table_name))
    {
      std::cerr << G_STRFUNC << ": Failure: create_table_with_default_fields() failed.\n";
      return false;
    }
  }


  // Check that the tables really exist.
  // If they don't then the newly-added groups won't have default arguments for those tables:
  const auto tables_created = Glom::DbUtils::get_table_names_from_database(false /* plus system prefs */);
  if(tables_created.empty())
  {
    std::cerr << "The added tables don't seem to exist (No tables exist).\n";
    return false;
  }

  for(const auto& table_name : table_names)
  {
    if(!contains(tables_created, table_name))
    {
      std::cerr << "The added table doesn't seem to exist (Not in list of tables): " << table_name << "\n";
      return false;
    }
  }

  test_selfhosting_cleanup(false /* do not delete the file. */);

  return true;
}

int main()
{
  Glom::libglom_init();

  const auto result = test_all_hosting_modes(sigc::ptr_fun(&test));

  Glom::libglom_deinit();

  return result;
}
