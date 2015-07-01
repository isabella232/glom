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

static bool test_add_group(const Glom::Document& document, const Glib::ustring& group)
{
  if(!Glom::DbUtils::add_group(&document, group))
  {
    std::cerr << G_STRFUNC << ": DbUtils::add_group() failed." << std::endl;
    return false;
  }

  const auto group_list = Glom::Privs::get_database_groups();
  if(!contains(group_list, group))
  {
    std::cerr << G_STRFUNC << ": Privs::get_database_groups() does not contain the expected group." << std::endl;
    std::cerr << G_STRFUNC << ":   group: " << group << std::endl;
    return false;
  }

  const auto user_list = Glom::Privs::get_database_users(group);
  if(!user_list.empty())
  {
    std::cerr << G_STRFUNC << ": The user list is not empty as expected.." << std::endl;
    return false;
  }

  return true;
}

static bool test_add_user(const Glom::Document& document, const Glib::ustring& user, const Glib::ustring& group)
{
  //Add an operator user, adding it to the group:
  if(!Glom::DbUtils::add_user(&document, user, "somepassword", group))
  {
    std::cerr << G_STRFUNC << ": DbUtils::add_user() failed." << std::endl;
    test_selfhosting_cleanup();
    return false;
  }

  const auto user_list = Glom::Privs::get_database_users(group);
  if(!contains(user_list, user))
  {
    std::cerr << G_STRFUNC << ": Privs::get_database_users() does not contain the expected user:" << std::endl;
    std::cerr << G_STRFUNC << ":   group: " << group << std::endl;
    std::cerr << G_STRFUNC << ":   user: " << user << std::endl;
    return false;
  }

  return true;
}


static bool change_privileges(const Glib::ustring& group_name, const Glib::ustring& table_name, bool view, bool edit, bool create, bool del)
{
  //Change the privs and make sure that it worked:
  Glom::Privileges privs_new;
  privs_new.m_view = view;
  privs_new.m_edit = edit;
  privs_new.m_create = create;
  privs_new.m_delete = del;
  if(!Glom::Privs::set_table_privileges(group_name, table_name, privs_new, false))
  {
    std::cerr << G_STRFUNC << ": Privs::set_table_privileges() failed for group=" << group_name << ", table_name=" << table_name << std::endl;
    return false;
  }

  const auto privs_changed = Glom::Privs::get_table_privileges(group_name, table_name);
  if( (privs_changed.m_view != privs_new.m_view) ||
    (privs_changed.m_edit != privs_new.m_edit) ||
    (privs_changed.m_create != privs_new.m_create) ||
    (privs_changed.m_delete != privs_new.m_delete) )
  {
    std::cerr << G_STRFUNC << ": Changing and re-reading privileges failed for group=" << group_name << ", table_name=" << table_name << std::endl;
    return false;
  }

  return true;
}

static bool test(Glom::Document::HostingMode hosting_mode)
{
  /* SQLite does not have user/group access levels,
   * so the SQL queries would fail.
   */
  if(hosting_mode == Glom::Document::HOSTING_MODE_SQLITE)
  {
    return true;
  }

  //Create and self-host the document:
  Glom::Document document;
    if(!(test_create_and_selfhost_new_database(document, hosting_mode, "test_db")))
  {
    std::cerr << G_STRFUNC << ": test_create_and_selfhost_new_database() failed" << std::endl;
    return false;
  }


  typedef std::vector<Glib::ustring> type_vec_strings;
  type_vec_strings table_names;
  table_names.push_back("sometable");
  table_names.push_back("SomeTableWithUpperCase");
  table_names.push_back("sometable with space characters");
  table_names.push_back("sometable with a \" doublequote character");
  table_names.push_back("sometable with a ' quote character");

  //MySQL has a 64-character limit on SQL identifiers:
  if(hosting_mode != Glom::Document::HOSTING_MODE_MYSQL_SELF)
  {
    table_names.push_back("sometablewithaverylongnameyaddayaddayaddayaddayaddyaddayaddayaddayaddayaddayaddayaddayaddayaddayaddayaddayadda");
  }

  //Add some tables, for the groups to have rights for:
  for(const auto& table_name : table_names)
  {
    if(!Glom::DbUtils::create_table_with_default_fields(&document, table_name))
    {
      std::cerr << G_STRFUNC << ": Failure: create_table_with_default_fields() failed." << std::endl;
      return false;
    }
  }


  //TODO_MySQL: Implement groups/users code.
  if(hosting_mode == Glom::Document::HOSTING_MODE_MYSQL_SELF)
  {
    test_selfhosting_cleanup(false /* do not delete the file. */);
    return true;
  }


  //Check that only one group exists (the developer group):
  const auto group_list_original = Glom::Privs::get_database_groups();
  if(group_list_original.empty())
  {
    std::cerr << G_STRFUNC << ": Privs::get_database_groups() returned an empty list." << std::endl;
    return false;
  }

  if(!contains(group_list_original, GLOM_STANDARD_GROUP_NAME_DEVELOPER))
  {
    std::cerr << G_STRFUNC << ": Privs::get_database_groups() does not contain the developers group." << std::endl;
    return false;
  }


  //Add groups:
  type_vec_strings group_names;
  group_names.push_back("somegroup1");
  group_names.push_back("somegroup with space characters");
  group_names.push_back("somegroup with a \" doublequote character");
  group_names.push_back("somegroup with a ' quote character");
  group_names.push_back("somegroupwithaverylongnameyaddayaddayaddayaddayaddyaddayaddayad"); //Almost too big.
  //We expect this to fail because of an apparently-undocumented max pg_user size of 63 characters in PostgreSQL:
  //group_names.push_back("somegroupwithaverylongnameyaddayaddayaddayaddayaddyaddayaddayadd");

  //Add groups:
  for(const auto& group_name : group_names)
  {
    //Add groups:
    if(!test_add_group(document, group_name))
      return false;
  }


  //Add users:
  //TODO: Test strange passwords.
  type_vec_strings user_names;
  user_names.push_back("someuser1");
  user_names.push_back("someuser with space characters");
  user_names.push_back("someuser with a \" doublequote character");
  user_names.push_back("someuser with a ' quote character");
  user_names.push_back("someuserwithaverylongnameyaddayaddayaddayaddayaddyaddayadda"); //Almost too big, with space for the numeric suffix below.
  //We expect this to fail because of an apparently-undocumented max pg_user size of 63 characters in PostgreSQL:
  //user_names.push_back("someuserwithaverylongnameyaddayaddayaddayaddayaddyaddayaddayadd");

  guint i = 0;
  for(const auto& user_name : user_names)
  {
    for(const auto& group_name : group_names)
    {
      const auto unique_user_name = Glib::ustring::compose("%1%2", user_name, i); //Make sure the username is unique.
      if(!test_add_user(document, unique_user_name, group_name))
        return false;

      for(const auto& table_name : table_names)
      {
        const auto privs = Glom::Privs::get_table_privileges(group_name, table_name);
        if(!privs.m_view)
        {
          std::cerr << G_STRFUNC << ": Privs::get_table_privileges() returned an unexpected view privilege for group=" << group_name << ", table_name=" << table_name << std::endl;
          return false;
        }

        if(!privs.m_edit)
        {
          std::cerr << G_STRFUNC << ": Privs::get_table_privileges() returned an unexpected edit privilege for group=" << group_name << ", table_name=" << table_name << std::endl;
          return false;
        }

        //We default to create and delete being false:
        /*
        if(privs.m_create)
        {
          std::cerr << G_STRFUNC << ": Privs::get_table_privileges() returned an unexpected create privilege for group=" << group_name << ", table_name=" << table_name << std::endl;
          return false;
        }

        if(privs.m_delete)
        {
          std::cerr << G_STRFUNC << ": Privs::get_table_privileges() returned an unexpected delete privilege for group=" << group_name << ", table_name=" << table_name << std::endl;
          return false;
        }
        */

	      if(!change_privileges(group_name, table_name, true, true, true, false))
          return false;
      }

      if(!Glom::DbUtils::remove_user_from_group(unique_user_name, group_name))
      {
        std::cerr << G_STRFUNC << ": DbUtils::remove_user() failed for user=" << unique_user_name << ", group=" << group_name << std::endl;
        return false;
      }

      if(!Glom::DbUtils::remove_user(unique_user_name))
      {
        std::cerr << G_STRFUNC << ": DbUtils::remove_user() failed for user=" << unique_user_name << std::endl;
        return false;
      }

      ++i;
    }
  }

  //Test get_table_privileges().
 


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
