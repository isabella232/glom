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
71 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "tests/test_selfhosting_utils.h"
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
  typename T_Container::const_iterator iter =
    std::find(container.begin(), container.end(), name);
  return iter != container.end();
}

static bool test(Glom::Document::HostingMode hosting_mode)
{
  //Create and self-host the document:
  Glom::Document document;
    if(!(test_create_and_selfhost_new_database(document, hosting_mode, "test_db")))
  {
    std::cerr << "test_create_and_selfhost_new_database() failed" << std::endl;
    return false;
  }

  //Check that only one group exists (the developer group):
  const Glom::Privs::type_vec_strings group_list_original = Glom::Privs::get_database_groups();
  if(group_list_original.empty())
  {
    std::cerr << "Privs::get_database_groups() returned an empty list." << std::endl;
    return false;
  }

  if(!contains(group_list_original, GLOM_STANDARD_GROUP_NAME_DEVELOPER))
  {
    std::cerr << "Privs::get_database_groups() does not contain the developers group." << std::endl;
    return false;
  }

  //Add a group:
  const Glib::ustring group_name = "somegroup";
  if(!Glom::DbUtils::add_group(&document, group_name))
  {
    std::cerr << "DbUtils::add_group() failed." << std::endl;
    test_selfhosting_cleanup();
    return false;
  }

  const Glom::Privs::type_vec_strings group_list_after = Glom::Privs::get_database_groups();
  if(group_list_after.size() <= group_list_original.size())
  {
    std::cerr << "The new group list is not larger than the old group list." << std::endl;
    return false;
  }

  const Glom::Privs::type_vec_strings user_list_original = Glom::Privs::get_database_users(group_name);
  if(!user_list_original.empty())
  {
    std::cerr << "The user list is not empty as expected.." << std::endl;
    return false;
  }

  //Add an operator user, adding it to the group:
  if(!Glom::DbUtils::add_user(&document, "someuser", "somepassword", group_name))
  {
    std::cerr << "DbUtils::add_user() failed." << std::endl;
    test_selfhosting_cleanup();
    return false;
  }

  const Glom::Privs::type_vec_strings user_list_after = Glom::Privs::get_database_users(group_name);
  if(user_list_after.size() != 1)
  {
    std::cerr << "The user list has an unexpected size: " << user_list_after.size() << std::endl;
    return false;
  }

  test_selfhosting_cleanup(false /* do not delete the file. */);

  return true; 
}

int main()
{
  Glom::libglom_init();
  
  if(!test(Glom::Document::HOSTING_MODE_POSTGRES_SELF))
  {
    std::cerr << "Failed with PostgreSQL" << std::endl;
    test_selfhosting_cleanup();
    return EXIT_FAILURE;
  }
  
  /* SQLite does not have user/group access levels,
   * so the SQL queries woudl fail.
  if(!test(Glom::Document::HOSTING_MODE_SQLITE))
  {
    std::cerr << "Failed with SQLite" << std::endl;
    test_selfhosting_cleanup();
    return EXIT_FAILURE;
  }
  */

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
