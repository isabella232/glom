/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
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

#ifndef GLOM_DATA_STRUCTURE_GROUPINFO_H
#define GLOM_DATA_STRUCTURE_GROUPINFO_H

#include <libglom/data_structure/translatable_item.h>
#include <libglom/data_structure/privileges.h>
#include <unordered_map>

namespace Glom
{

class GroupInfo : public TranslatableItem
{
public:
  GroupInfo();
  GroupInfo(const GroupInfo& src);
  GroupInfo(GroupInfo&& src);

  GroupInfo& operator=(const GroupInfo& src);
  GroupInfo& operator=(GroupInfo&& src);

  bool operator==(const GroupInfo& src) const;
  bool operator!=(const GroupInfo& src) const;

  bool m_developer; //m_privs is ignored if this is true.

  std::unordered_map<Glib::ustring, Privileges, std::hash<std::string>> m_map_privileges;

  //typedef std::vector<Glib::ustring> type_users;
  //type_users m_users;
};

} //namespace Glom

#endif //GLOM_DATA_STRUCTURE_GROUPINFO_H
