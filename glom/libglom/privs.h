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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_PRIVS_H
#define GLOM_PRIVS_H

#include <libglom/glom_postgres.h>

namespace Glom
{

/** A class to get user/group information for a database.
 */
class Privs : public GlomPostgres
{
public:

  /** This is apparently undocumented in PostgreSQL,
   * but if we try to create a user or group
   * with more characters (or bytes?) than this then
   * a truncated version of it will be read back.
   */
  static const guint MAX_ROLE_SIZE = 63;

  /** Get the groups with access to the database.
   */
  static type_vec_strings get_database_groups();

  /** Get users with access to the database.
   * If the resuilt is empty even when @a group_name is empty then it was not possible to get the list of users, probably due to a permissions problem.
   * @param group_name Get only users in this group.
   */
  static type_vec_strings get_database_users(const Glib::ustring& group_name = Glib::ustring());


  /** Discover whether there is already a user with a real password,
   * instead of just a default glom user and default password,
   * The user should be forced to choose a user/password when network sharing is active.
   */
  static bool get_developer_user_exists_with_password(Document::HostingMode hosting_mode);

  /** Discover whether the default developer user exists (which has a known password).
   * The user should be forced to choose a user/password when network sharing is active,
   * and this default user should no longer exist in that case.
   */
  static bool get_default_developer_user_exists(Document::HostingMode hosting_mode);

  /** Get the standard username and password used for the no-password user,
   * which should only be used when network sharing is not active.
   *
   * @param hosting_mode So we can use a shorter name for MySQL to avoid an error about the name being too long.
   */
  static Glib::ustring get_default_developer_user_name(Glib::ustring& password, Document::HostingMode hosting_mode);

  static Privileges get_table_privileges(const Glib::ustring& group_name, const Glib::ustring& table_name);
  static bool set_table_privileges(const Glib::ustring& group_name, const Glib::ustring& table_name, const Privileges& privs, bool developer_privs = false);

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

  typedef std::unordered_map<Glib::ustring, Privileges, std::hash<std::string>> type_map_privileges;

  //A map of table names to cached privileges:
  static type_map_privileges m_privileges_cache;

  // Store the cache for a few seconds in case it
  // is immediately requested again, to avoid
  // introspecting again, which is slow.
  typedef std::unordered_map<Glib::ustring, sigc::connection, std::hash<std::string>> type_map_cache_timeouts;
  static type_map_cache_timeouts m_map_cache_timeouts;
};

} //namespace Glom

#endif //GLOM_PRIVS_H

