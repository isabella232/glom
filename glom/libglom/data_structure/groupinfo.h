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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLOM_DATA_STRUCTURE_GROUPINFO_H
#define GLOM_DATA_STRUCTURE_GROUPINFO_H

#include "translatable_item.h"
#include "privileges.h"
#include <map>

namespace Glom
{

class GroupInfo : public TranslatableItem
{
public:
  GroupInfo();
  GroupInfo(const GroupInfo& src);
  virtual ~GroupInfo();

  GroupInfo& operator=(const GroupInfo& src);

  bool operator==(const GroupInfo& src) const;
  bool operator!=(const GroupInfo& src) const;

  bool m_developer; //m_privs is ignored if this is true.

  typedef std::map<Glib::ustring, Privileges> type_map_table_privileges;
  type_map_table_privileges m_map_privileges;

  //typedef std::vector<Glib::ustring> type_users;
  //type_users m_users;
};

} //namespace Glom

#endif //GLOM_DATA_STRUCTURE_GROUPINFO_H
