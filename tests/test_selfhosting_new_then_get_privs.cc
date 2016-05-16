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
#include <libglom/privs.h>
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

  const auto groups = Glom::Privs::get_database_groups();
  if(groups.empty())
  {
    std::cerr << G_STRFUNC << ": Failure: groups was empty.\n";
    return false;
  }

  for(const auto& group_name : groups)
  {
    if(group_name.empty())
    {
      std::cerr << G_STRFUNC << ": Failure: group_name was empty.\n";
      return false;
    }

    for(const auto& user_name : Glom::Privs::get_database_users(group_name))
    {
      if(user_name.empty())
      {
        std::cerr << G_STRFUNC << ": Failure: user_name was empty.\n";
        return false;
      }

      std::cout << "group: " << group_name << ", has user: " << user_name << std::endl;
    }
  }

  test_selfhosting_cleanup();

  return true;
}

int main()
{
  Glom::libglom_init();

  if(!test(Glom::Document::HostingMode::POSTGRES_SELF))
  {
    std::cerr << G_STRFUNC << ": Failed with PostgreSQL\n";
    test_selfhosting_cleanup();
    return EXIT_FAILURE;
  }

  /* SQLite does not have this feature:
  if(!test(Glom::Document::HostingMode::SQLITE))
  {
    std::cerr << G_STRFUNC << ": Failed with SQLite\n";
    test_selfhosting_cleanup();
    return EXIT_FAILURE;
  }
  */

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
