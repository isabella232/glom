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

#include <gtkmm.h>

#include <glom/glom_postgres.h>

namespace Glom
{

/** A class to get user/group information for a database.
 */
class Privs : public GlomPostgres
{
public:

  static type_vecStrings get_database_groups();
  static type_vecStrings get_database_users(const Glib::ustring& group_name = Glib::ustring());
  static Privileges get_table_privileges(const Glib::ustring& group_name, const Glib::ustring& table_name);
  static void set_table_privileges(const Glib::ustring& group_name, const Glib::ustring& table_name, const Privileges& privs, bool developer_privs = false);
  static Glib::ustring get_user_visible_group_name(const Glib::ustring& group_name);

  static type_vecStrings get_groups_of_user(const Glib::ustring& user);
  static bool get_user_is_in_group(const Glib::ustring& user, const Glib::ustring& group);
  static Privileges get_current_privs(const Glib::ustring& table_name);

protected:
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

