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

#include <libglom/data_structure/tableinfo.h>

namespace Glom
{

TableInfo::TableInfo()
: m_hidden(false),
  m_default(false)
{
  m_translatable_item_type = TRANSLATABLE_TYPE_TABLE;
}

TableInfo::TableInfo(const TableInfo& src)
: TranslatableItem(src),
  HasTitleSingular(src),
  m_hidden(src.m_hidden),
  m_default(src.m_default)
{
}

TableInfo& TableInfo::operator=(const TableInfo& src)
{
  TranslatableItem::operator=(src);
  HasTitleSingular::operator=(src);

  m_hidden = src.m_hidden;
  m_default = src.m_default;

  return *this;
}

bool TableInfo::operator==(const TableInfo& src) const
{
  return TranslatableItem::operator==(src) &&
    HasTitleSingular::operator==(src) && 
    (m_hidden == src.m_hidden) &&
    (m_default == src.m_default);
}

bool TableInfo::operator!=(const TableInfo& src) const
{
  return !operator==(src);
}

bool TableInfo::get_hidden() const
{
  return m_hidden;
}

void TableInfo::set_hidden(bool val)
{
  m_hidden = val;
}

bool TableInfo::get_default() const
{
  return m_default;
}

void TableInfo::set_default(bool val)
{
  m_default = val;
}

} //namespace Glom
