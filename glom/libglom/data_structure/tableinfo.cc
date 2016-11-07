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

TableInfo::TableInfo() noexcept
: m_hidden(false),
  m_default(false)
{
  m_translatable_item_type = enumTranslatableItemType::TABLE;
}

bool TableInfo::operator==(const TableInfo& src) const noexcept
{
  return TranslatableItem::operator==(src) &&
    HasTitleSingular::operator==(src) &&
    (m_hidden == src.m_hidden) &&
    (m_default == src.m_default);
}

bool TableInfo::operator!=(const TableInfo& src) const noexcept
{
  return !operator==(src);
}

bool TableInfo::get_hidden() const noexcept
{
  return m_hidden;
}

void TableInfo::set_hidden(bool val) noexcept
{
  m_hidden = val;
}

bool TableInfo::get_default() const noexcept
{
  return m_default;
}

void TableInfo::set_default(bool val) noexcept
{
  m_default = val;
}

} //namespace Glom
