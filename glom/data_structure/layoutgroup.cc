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
 
#include "layoutgroup.h"

LayoutGroup::LayoutGroup()
: m_sequence(0),
  m_others(false)
{
}

LayoutGroup::LayoutGroup(const LayoutGroup& src)
: m_group_name(src.m_group_name),
  m_title(src.m_title),
  m_sequence(src.m_sequence),
  m_others(src.m_others),
  m_map_items(src.m_map_items)
{
}

LayoutGroup& LayoutGroup::operator=(const LayoutGroup& src)
{
  m_group_name = src.m_group_name;
  m_title = src.m_title;
  m_sequence = src.m_sequence;
  m_others = src.m_others;
  m_map_items = src.m_map_items;

  return *this;
}

bool LayoutGroup::has_field(const Glib::ustring& field_name) const
{
  for(type_map_items::const_iterator iter = m_map_items.begin(); iter != m_map_items.end(); ++iter)
  {
    if(iter->second.m_field_name == field_name)
      return true;
  }

  return false;
}

void LayoutGroup::add_item(const LayoutItem& item)
{
  //Get next available sequence:
  guint sequence = 0;
   if(!m_map_items.empty())
     sequence = m_map_items.rbegin()->first;
  ++sequence;

  m_map_items[sequence] = item;
  m_map_items[sequence].m_sequence = sequence;
}

