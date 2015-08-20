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

#include <libglom/data_structure/foundset.h>

namespace Glom
{

FoundSet::FoundSet()
{
}

FoundSet::FoundSet(const FoundSet& src) noexcept
:  m_table_name(src.m_table_name),
   m_extra_join(src.m_extra_join),
   m_where_clause(src.m_where_clause),
   m_sort_clause(src.m_sort_clause)
{
}

FoundSet::FoundSet(FoundSet&& src) noexcept
:  m_table_name(std::move(src.m_table_name)),
   m_extra_join(std::move(src.m_extra_join)),
   m_where_clause(std::move(src.m_where_clause)),
   m_sort_clause(std::move(src.m_sort_clause))
{
}

FoundSet& FoundSet::operator=(const FoundSet& src) noexcept
{
  m_table_name = src.m_table_name;
  m_extra_join = src.m_extra_join;
  m_where_clause = src.m_where_clause;
  m_sort_clause = src.m_sort_clause;

  return *this;
}

FoundSet& FoundSet::operator=(FoundSet&& src) noexcept
{
  m_table_name = std::move(src.m_table_name);
  m_extra_join = std::move(src.m_extra_join);
  m_where_clause = std::move(src.m_where_clause);
  m_sort_clause = std::move(src.m_sort_clause);

  return *this;
}

} //namespace Glom
