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
 
#include "layoutitem.h"

LayoutItem::LayoutItem()
: m_sequence(0),
  m_editable(true)
{
}

LayoutItem::LayoutItem(const LayoutItem& src)
: m_sequence(src.m_sequence),
  m_name(src.m_name),
  m_editable(src.m_editable)
{
}

LayoutItem::~LayoutItem()
{
}

LayoutItem& LayoutItem::operator=(const LayoutItem& src)
{
  m_name = src.m_name;
  m_sequence = src.m_sequence;
  m_editable = src.m_editable;

  return *this;
}

bool LayoutItem::operator==(const LayoutItem& src) const
{
  return (m_name == src.m_name) &&
         (m_sequence == src.m_sequence) &&
         (m_editable == src.m_editable);
}

void LayoutItem::set_name(const Glib::ustring& name)
{
  m_name = name;
}

Glib::ustring LayoutItem::get_name() const
{
  return m_name;
}

bool LayoutItem::get_editable() const
{
  return m_editable;
}

void LayoutItem::set_editable(bool val)
{
  m_editable = val;
}


