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

#include <libglom/data_structure/groupinfo.h>

namespace Glom
{

GroupInfo::GroupInfo()
: m_developer(false)
{
}

GroupInfo::GroupInfo(const GroupInfo& src)
: TranslatableItem(src),
  m_developer(src.m_developer),
  m_map_privileges(src.m_map_privileges)
{
}

GroupInfo::GroupInfo(GroupInfo&& src)
: TranslatableItem(src),
  m_developer(std::move(src.m_developer)),
  m_map_privileges(std::move(src.m_map_privileges))
{
}

GroupInfo& GroupInfo::operator=(const GroupInfo& src)
{
  TranslatableItem::operator=(src);

  m_developer = src.m_developer;
  m_map_privileges = src.m_map_privileges;

  return *this;
}

GroupInfo& GroupInfo::operator=(GroupInfo&& src)
{
  TranslatableItem::operator=(src);

  m_developer = std::move(src.m_developer);
  m_map_privileges = std::move(src.m_map_privileges);

  return *this;
}

bool GroupInfo::operator==(const GroupInfo& src) const
{
  return TranslatableItem::operator==(src) &&
         (m_developer == src.m_developer) &&
         (m_map_privileges == src.m_map_privileges);
}

bool GroupInfo::operator!=(const GroupInfo& src) const
{
  return !(operator==(src));
}

} //namespace Glom
