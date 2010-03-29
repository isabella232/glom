/* Glom
 *
 * Copyright (C) 2001-2006 Murray Cumming
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "glom_privs.h"
#include <libglom/standard_table_prefs_fields.h>
#include <glom/application.h>

namespace Glom
{

Privs::type_map_privileges Privs::m_privileges_cache;

Privs::type_map_cache_timeouts Privs::m_map_cache_timeouts;

Privs::type_vec_strings Privs::get_database_groups()
{
  type_vec_strings result;

  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder =
      Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
  builder->select_add_field("groname", "pg_group");
  builder->select_add_target("pg_group");
  
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute_select(builder);
  if(data_model)
  {
    const int rows_count = data_model->get_n_rows();
    for(int row = 0; row < rows_count; ++row)
    {
#ifdef GLIBMM_EXCEPTIONS_ENABLED    
      const Gnome::Gda::Value value = data_model->get_value_at(0, row);
#else
      std::auto_ptr<Glib::Error> value_error;
      const Gnome::Gda::Value value = data_model->get_value_at(0, row, value_error); 
#endif           
      const Glib::ustring name = value.get_string();
      result.push_back(name);
    }
  }

  return result;
}

bool Privs::get_default_developer_user_exists()
{
  Glib::ustring default_password;
  const Glib::ustring default_user = get_default_developer_user_name(default_password);

  const type_vec_strings users = get_database_users();
  type_vec_strings::const_iterator iterFind = std::find(users.begin(), users.end(), default_user);
  if(iterFind != users.end())
    return true; //We assume that the password is what it should be and that it has developer rights.

  return false; //The default user is not there.
}

bool Privs::get_developer_user_exists_with_password()
{
  Glib::ustring default_password;
  const Glib::ustring default_user = get_default_developer_user_name(default_password);

  const type_vec_strings users = get_database_users();
  for(type_vec_strings::const_iterator iter = users.begin(); iter != users.end(); ++iter)
  {
    const Glib::ustring user = *iter;
    if(user == default_user)
      continue;

    if(get_user_is_in_group(user, GLOM_STANDARD_GROUP_NAME_DEVELOPER))
      return true;
  }

  return false;
}

Glib::ustring Privs::get_default_developer_user_name(Glib::ustring& password)
{
  password = "glom_default_developer_password";
  return "glom_default_developer_user";
}

Privs::type_vec_strings Privs::get_database_users(const Glib::ustring& group_name)
{
  BusyCursor cursor(Application::get_application());

  type_vec_strings result;

  if(group_name.empty())
  {
    //pg_shadow contains the users. pg_users is a view of pg_shadow without the password.
    Glib::RefPtr<Gnome::Gda::SqlBuilder> builder =
      Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
    builder->select_add_field("usename", "pg_shadow");
    builder->select_add_target("pg_shadow");
  
    Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute_select(builder);
    if(data_model)
    {
      const int rows_count = data_model->get_n_rows();
      for(int row = 0; row < rows_count; ++row)
      {
#ifdef GLIBMM_EXCEPTIONS_ENABLED      
        const Gnome::Gda::Value value = data_model->get_value_at(0, row);
#else
        std::auto_ptr<Glib::Error> value_error;       
        const Gnome::Gda::Value value = data_model->get_value_at(0, row, value_error);
#endif         
        const Glib::ustring name = value.get_string();        
        result.push_back(name);
      }
    }
  }
  else
  {
    Glib::RefPtr<Gnome::Gda::SqlBuilder> builder =
      Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
    builder->select_add_field("groname", "pg_group");
    builder->select_add_field("grolist", "pg_group");
    builder->select_add_target("pg_group");
    builder->set_where(
      builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_EQ,
        builder->add_id("groname"), //TODO: It would be nice to specify the table here too.
        builder->add_expr(group_name)));
    Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute_select(builder);
    if(data_model && data_model->get_n_rows())
    {
      const int rows_count = data_model->get_n_rows();
      for(int row = 0; row < rows_count; ++row)
      {
#ifdef GLIBMM_EXCEPTIONS_ENABLED      
        const Gnome::Gda::Value value = data_model->get_value_at(1, row); //Column 1 is the /* the user list.
#else
        std::auto_ptr<Glib::Error> value_error;
        const Gnome::Gda::Value value = data_model->get_value_at(1, row, value_error); //Column 1 is the /* the user list.
#endif        
        //pg_group is a string, formatted, bizarrely, like so: "{100, 101}".

        Glib::ustring group_list;
        if(!value.is_null())
          group_list = value.get_string();

        type_vec_strings vecUserIds = pg_list_separate(group_list);
        for(type_vec_strings::const_iterator iter = vecUserIds.begin(); iter != vecUserIds.end(); ++iter)
        {
          //TODO_Performance: Can we do this in one SQL SELECT?
          Glib::RefPtr<Gnome::Gda::SqlBuilder> builder =
            Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
          builder->select_add_field("usename", "pg_user");
          builder->select_add_target("pg_user");
          builder->set_where(
            builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_EQ,
              builder->add_id("usesysid"), //TODO: It would be nice to specify the table here too.
              builder->add_expr(*iter)));
          Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute_select(builder);
          if(data_model)
          {
#ifdef GLIBMM_EXCEPTIONS_ENABLED          
            const Gnome::Gda::Value value = data_model->get_value_at(0, 0);
#else
            std::auto_ptr<Glib::Error> value_error;
            const Gnome::Gda::Value value = data_model->get_value_at(0, 0, value_error);            
#endif            
            result.push_back(value.get_string());
          }
        }

      }
    }
  }

  return result;
}

void Privs::set_table_privileges(const Glib::ustring& group_name, const Glib::ustring& table_name, const Privileges& privs, bool developer_privs)
{
  if(group_name.empty() || table_name.empty())
    return;

  //Change the permission in the database:

  //Build the SQL statement:

  //Grant or revoke:
  Glib::ustring strQuery = "GRANT";
  //TODO: Revoke the ones that are not specified.

  //What to grant or revoke:
  Glib::ustring strPrivilege;

  if(developer_privs)
    strPrivilege = "ALL PRIVILEGES";
  else
  {
    if(privs.m_view)
      strPrivilege += "SELECT";

    if(privs.m_edit)
    {
      if(!strPrivilege.empty())
        strPrivilege += ", ";

      strPrivilege += "UPDATE";
    }

    if(privs.m_create)
    {
      if(!strPrivilege.empty())
      
        strPrivilege += ", ";

      strPrivilege += "INSERT";
    }

    if(privs.m_delete)
    {
      if(!strPrivilege.empty())
        strPrivilege += ", ";

      strPrivilege += "DELETE";
    }
  }

  strQuery += " " + strPrivilege + " ON \"" + table_name + "\" ";

  //This must match the Grant or Revoke:
  strQuery += "TO";

  strQuery += " GROUP \"" + group_name + "\"";

  const bool test = query_execute(strQuery);
  if(!test)
    std::cerr << "Privs::set_table_privileges(): GRANT failed." << std::endl;
  else
  {
    if( (table_name != GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME) && privs.m_create )
    {
      //To create a record, you will usually need write access to the autoincrements table,
      //so grant this too:
      Privileges priv_autoincrements;
      priv_autoincrements.m_view = true;
      priv_autoincrements.m_edit = true;
      set_table_privileges(group_name, GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME, priv_autoincrements);
    }
  }
}

Privileges Privs::get_table_privileges(const Glib::ustring& group_name, const Glib::ustring& table_name)
{
  Privileges result;

  if(group_name == GLOM_STANDARD_GROUP_NAME_DEVELOPER)
  {
    //Always give developers full access:
    result.m_view = true;
    result.m_edit = true;
    result.m_create = true;
    result.m_delete = true;
    return result;
  }

  //Get the permissions:
  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder =
    Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
  builder->select_add_field("relacl", "pg_class");
  builder->select_add_target("pg_class");
  builder->set_where(
    builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_EQ,
      builder->add_id("relname"), //TODO: It would be nice to specify the table here too.
      builder->add_expr(table_name)));
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute_select(builder);
  if(data_model && data_model->get_n_rows())
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    const Gnome::Gda::Value value = data_model->get_value_at(0, 0);
#else
    std::auto_ptr<Glib::Error> value_error;
    const Gnome::Gda::Value value = data_model->get_value_at(0, 0, value_error);
#endif
    
    Glib::ustring access_details;
    if(!value.is_null())
      access_details = value.get_string();

    //std::cout << "DEBUG: access_details:" << access_details << std::endl;

    //Parse the strange postgres permissions format:
    //For instance, "{murrayc=arwdxt/murrayc,operators=r/murrayc}" (Postgres 8.x)
    //For instance, "{murrayc=arwdxt/murrayc,group operators=r/murrayc}" (Postgres <8.x)
    const type_vec_strings vecItems = pg_list_separate(access_details);
    for(type_vec_strings::const_iterator iterItems = vecItems.begin(); iterItems != vecItems.end(); ++iterItems)
    {
      Glib::ustring item = *iterItems;
      //std::cout << "DEBUG: item:" << item << std::endl;

      item = Utils::string_trim(item, "\""); //Remove quotes from front and back.

      //std::cout << "DEBUG: item without quotes:" << item << std::endl;

      //Find group permissions, ignoring user permissions:
      //We need to find the role by name.
      // Previous versions of Postgres (8.1, or maybe 7.4) prefixed group names by "group ", 
      // but that doesn't work for recent versions of Postgres,
      // probably because the user and group concepts have been combined into "roles".
      //
      //const Glib::ustring strgroup = "group ";
      const Glib::ustring strgroup = group_name + "=";
      Glib::ustring::size_type posFind = item.find(strgroup);
      if(posFind != Glib::ustring::npos)
      {
        //It is the needed group permision:

        //Remove the "group " prefix (not needed for Postgres 8.x):
        //item = item.substr(strgroup.size());
        item = item.substr(posFind);
        //std::cout << "DEBUG: user permissions:" << item << std::endl;

        //Get the parts before and after the =:
        const type_vec_strings vecParts = Utils::string_separate(item, "=");
        if(vecParts.size() == 2)
        {
          const Glib::ustring this_group_name = vecParts[0];
          if(this_group_name == group_name) //Only look at permissions for the requested group->
          {
            Glib::ustring group_permissions = vecParts[1];

            //Get the part before the /user_who_granted_the_privileges:
            const type_vec_strings vecParts = Utils::string_separate(group_permissions, "/");
            if(!vecParts.empty())
              group_permissions = vecParts[0];

            //g_warning("  group=%s", group_name.c_str());
            //g_warning("  permisisons=%s", group_permissions.c_str());

            //Iterate through the characters:
            for(Glib::ustring::iterator iter = group_permissions.begin(); iter != group_permissions.end(); ++iter)
            {
              gunichar chperm = *iter;
              Glib::ustring perm(1, chperm);

              //See http://www.postgresql.org/docs/8.0/interactive/sql-grant.html
              if(perm == "r")
                result.m_view = true;
              else if(perm == "w")
                result.m_edit = true;
              else if(perm == "a")
                result.m_create = true;
              else if(perm == "d")
                result.m_delete = true;
            }
          }
        }
      }

    }
  }

  //g_warning("get_table_privileges(group_name=%s, table_name=%s) returning: %d", group_name.c_str(), table_name.c_str(), result.m_create);
  return result;
}


Glib::ustring Privs::get_user_visible_group_name(const Glib::ustring& group_name)
{
  Glib::ustring result = group_name;

  //Remove the special prefix:
  const Glib::ustring prefix = "glom_";
  if(result.substr(0, prefix.size()) == prefix)
    result = result.substr(prefix.size());

  return result;
}

Base_DB::type_vec_strings Privs::get_groups_of_user(const Glib::ustring& user)
{
  //TODO_Performance

  type_vec_strings result;

  //Look at each group:
  type_vec_strings groups = get_database_groups();
  for(type_vec_strings::const_iterator iter = groups.begin(); iter != groups.end(); ++iter)
  {
    //See whether the user is in this group:
    if(get_user_is_in_group(user, *iter))
    {
      //Add the group to the result:
      result.push_back(*iter);
    }
  }

  return result;
}

bool Privs::get_user_is_in_group(const Glib::ustring& user, const Glib::ustring& group)
{
  const type_vec_strings users = get_database_users(group);
  type_vec_strings::const_iterator iterFind = std::find(users.begin(), users.end(), user);
  return (iterFind != users.end());
}

bool Privs::on_privs_privileges_cache_timeout(const Glib::ustring& table_name)
{
  //std::cout << "DEBUG: Privs::on_privs_privileges_cache_timeou(): table=" << table_name << std::endl;  
      
  //Forget the cached privileges after a few seconds:
  type_map_privileges::iterator iter = m_privileges_cache.find(table_name);
  if(iter != m_privileges_cache.end())
  {
    //std::cout << "  DEBUG: Privs::on_privs_privileges_cache_timeou(): Cleared cache for table=" << table_name << std::endl;
    m_privileges_cache.erase(iter);
  }

  return false; //Don't call this again.
}

Privileges Privs::get_current_privs(const Glib::ustring& table_name)
{
  //TODO_Performance: There's lots of database access here.
  //We could maybe replace some with the postgres has_table_* function().

  BusyCursor cursor(Application::get_application());

  //Return a cached value if possible.
  //(If it is in the cache then it's fairly recent)
  type_map_privileges::const_iterator iter = m_privileges_cache.find(table_name);
  if(iter != m_privileges_cache.end())
  {
    //std::cout << "DEBUG: Privs::get_current_privs(): Returning cache." << std::endl;
    return iter->second;
  }

   
  //Get the up-to-date privileges from the database:
  Privileges result;

  //std::cout << "DEBUG: Privs::get_current_privs(): Getting non-cached." << std::endl;  

  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  const Glib::ustring current_user = connection_pool->get_user();

  //Is the user in the special developers group?
  /*
  type_vec_strings developers = get_database_users(GLOM_STANDARD_GROUP_NAME_DEVELOPER);
  type_vec_strings::const_iterator iterFind = std::find(developers.begin(), developers.end(), current_user);
  if(iterFind != developers.end())
  {
    result.m_developer = true;
  }
  */
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  sharedptr<SharedConnection> sharedconnection = connection_pool->connect();
#else
  std::auto_ptr<ExceptionConnection> ex;  
  sharedptr<SharedConnection> sharedconnection = connection_pool->connect(ex);
#endif  
  if(sharedconnection && sharedconnection->get_gda_connection()->supports_feature(Gnome::Gda::CONNECTION_FEATURE_USERS))
  {
    //Get the "true" rights for any groups that the user is in:
    type_vec_strings groups = get_groups_of_user(current_user);
    for(type_vec_strings::const_iterator iter = groups.begin(); iter != groups.end(); ++iter)
    {
      Privileges privs = get_table_privileges(*iter, table_name);

      if(privs.m_view)
        result.m_view = true;

      if(privs.m_edit)
        result.m_edit = true;

      if(privs.m_create)
        result.m_create = true;

      if(privs.m_delete)
        result.m_delete = true;
    }
  }
  else
  {
    // If the database doesn't support users we have privileges to do everything
    result.m_view = true;
    result.m_edit = true;
    result.m_create = true;
    result.m_delete = true;
  }

  m_privileges_cache[table_name] = result;


  //Connect a timeout to invalidate the cache after a short time:
  //In theory, the privileges could change (via another client)
  //during this time, but it is fairly unlikely.
  //TODO: Make sure we handle the failure well in that unlikely case.

  //Stop the existing timeout if it has not run yet.
  type_map_cache_timeouts::iterator iter_connection = m_map_cache_timeouts.find(table_name);
  if(iter_connection != m_map_cache_timeouts.end())
    iter_connection->second.disconnect();

  m_map_cache_timeouts[table_name] = 
    Glib::signal_timeout().connect_seconds(
      sigc::bind( sigc::ptr_fun(&Privs::on_privs_privileges_cache_timeout), table_name ), 
      30 /* seconds */);

  return result;
}

} //namespace Glom
