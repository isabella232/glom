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
  /* SQLite does not have user/group access levels,
   * so the SQL queries woudl fail.
   */
  if(hosting_mode == Glom::Document::HOSTING_MODE_SQLITE)
  {
    return true;
  }

  Glib::ustring temp_file_uri;
  const Glib::ustring operator_user = "someoperator";
  const Glib::ustring operator_password = "somepassword";

  //Create and self-host the document:
  {
    Glom::Document document;
    const bool recreated = 
      test_create_and_selfhost_from_example("example_smallbusiness.glom", document, hosting_mode);
    if(!recreated)
    {
      std::cerr << G_STRFUNC << ": Recreation failed." << std::endl;
      return false;
    }

    temp_file_uri = test_get_temp_file_uri();
    if(temp_file_uri.empty())
    {
      std::cerr << G_STRFUNC << ": temp_file_uri is empty." << std::endl;
      return false;
    }

    //TODO_MySQL: Implement groups/users code.
    if(hosting_mode == Glom::Document::HOSTING_MODE_MYSQL_SELF)
    {
      test_selfhosting_cleanup(false /* do not delete the file. */);
      return true;
    }

    //Add an operator user:
    const Glib::ustring operator_group_name = "personnel_department";
    const Glom::DbUtils::type_vec_strings group_list = 
      Glom::Privs::get_database_groups();
    if(!contains(group_list, operator_group_name))
    {
      std::cerr << G_STRFUNC << ": The expected group was not found." << std::endl;
      return false;
    }

    if(!Glom::DbUtils::add_user(&document, operator_user, operator_password, operator_group_name))
    {
      std::cerr << G_STRFUNC << ": DbUtils::add_user() failed." << std::endl;
      test_selfhosting_cleanup();
      return false;
    }

    //Check that the developer user has access to database metadata:
    const Glom::DbUtils::type_vec_strings tables = 
      Glom::DbUtils::get_table_names_from_database(true /* ignore system tables */);
    if(tables.empty())
    {
      std::cerr << G_STRFUNC << ": get_table_names_from_database() failed for developer user." << std::endl;
      return false;
    }

    test_selfhosting_cleanup(false /* do not delete the file. */);
  }
  
  //Self-host the document again, this time as operator:
  {
    Glom::Document document;
    document.set_allow_autosave(false); //To simplify things and to not depend implicitly on autosave.
    document.set_file_uri(temp_file_uri);
    int failure_code = 0;
    const auto test = document.load(failure_code);
    //std::cout << "Document load result=" << test << std::endl;
    if(!test)
    {
      std::cerr << G_STRFUNC << ": Document::load() failed with failure_code=" << failure_code << std::endl;
      return false;
    }

    if(!test_selfhost(document, operator_user, operator_password))
    {
      std::cerr << G_STRFUNC << ": test_selfhost() failed." << std::endl;
      return false;
    }

    //Check that the operator user still has access to database metadata:
    Glom::ConnectionPool* connection_pool = Glom::ConnectionPool::get_instance();
    connection_pool->connect();
    const auto field_types = connection_pool->get_field_types();
    if(!field_types)
    {
      std::cerr << G_STRFUNC << ": get_field_types() returned null." << std::endl;
      return false;
    }

    if(field_types->get_types_count() == 0)
    {
      std::cerr << G_STRFUNC << ": get_field_types() returned no types." << std::endl;
      return false;
    }

    //std::cout << "field_types count=" << field_types->get_types_count() << std::endl;

    const Glom::DbUtils::type_vec_strings tables = 
      Glom::DbUtils::get_table_names_from_database(true /* ignore system tables */);
    if(tables.empty())
    {
      std::cerr << G_STRFUNC << ": get_table_names_from_database() failed for operator user." << std::endl;
      return false;
    }

    if(Glom::Privs::get_user_is_in_group(connection_pool->get_user(), GLOM_STANDARD_GROUP_NAME_DEVELOPER))
    {
      std::cerr << G_STRFUNC << ": The operator user is in the developer group, but should not be." << std::endl;
      return false;
    }


    test_selfhosting_cleanup(); //Delete the file this time.
  }
 
  return true; 
}

int main()
{
  Glom::libglom_init();

  const auto result = test_all_hosting_modes(sigc::ptr_fun(&test));

  Glom::libglom_deinit();

  return result;
}
