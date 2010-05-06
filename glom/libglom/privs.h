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

#ifndef GLOM_PRIVS_H
#define GLOM_PRIVS_H

//#include <gtkmm.h>

#include <libglom/glom_postgres.h>

namespace Glom
{

/** A class to get user/group information for a database.
 */
class Privs : public GlomPostgres
{
public:

  /** Get the groups with access to the database.
   */
  static type_vec_strings get_database_groups();

  /** Get users with access to the database.
   * @param group_name Get only users in this group.
   */
  static type_vec_strings get_database_users(const Glib::ustring& group_name = Glib::ustring());


  /** Discover whether there is already a user with a real password,
   * instead of just a default glom user and default password,
   * The user should be forced to choose a user/password when network sharing is active.
   */
  static bool get_developer_user_exists_with_password();

  /** Discover whether the default developer user exists (which has a known password).
   * The user should be forced to choose a user/password when network sharing is active,
   * and this default user should no longer exist in that case.
   */
  static bool get_default_developer_user_exists();

  /** Get the standard username and password used for the no-password user,
   * which should only be used when network sharing is not active.
   */
  static Glib::ustring get_default_developer_user_name(Glib::ustring& password);

  static Privileges get_table_privileges(const Glib::ustring& group_name, const Glib::ustring& table_name);
  static void set_table_privileges(const Glib::ustring& group_name, const Glib::ustring& table_name, const Privileges& privs, bool developer_privs = false);

  static Glib::ustring get_user_visible_group_name(const Glib::ustring& group_name);

  /** Get the groups that the user is a member of.
   */
  static type_vec_strings get_groups_of_user(const Glib::ustring& user);

  static bool get_user_is_in_group(const Glib::ustring& user, const Glib::ustring& group);

  /** Get the privileges (access rights) that the current user has.
   */
  static Privileges get_current_privs(const Glib::ustring& table_name);

private:
  static bool on_privs_privileges_cache_timeout(const Glib::ustring& table_name);

  typedef std::map<Glib::ustring, Privileges> type_map_privileges;

  //A map of table names to cached privileges: 
  static type_map_privileges m_privileges_cache;

  // Store the cache for a few seconds in case it 
  // is immediately requested again, to avoid 
  // introspecting again, which is slow. 
  typedef std::map<Glib::ustring, sigc::connection> type_map_cache_timeouts;
  static type_map_cache_timeouts m_map_cache_timeouts;
};

} //namespace Glom

#endif //GLOM_PRIVS_H

