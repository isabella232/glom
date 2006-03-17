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
  m_translatable_item_type = TRANSLATABLE_TYPE_LAYOUT_ITEM;
}

LayoutItem::LayoutItem(const LayoutItem& src)
: TranslatableItem(src),
  m_sequence(src.m_sequence),
  m_editable(src.m_editable)
{
}

LayoutItem::~LayoutItem()
{
}

LayoutItem& LayoutItem::operator=(const LayoutItem& src)
{
  TranslatableItem::operator=(src);

  m_sequence = src.m_sequence;
  m_editable = src.m_editable;

  return *this;
}

bool LayoutItem::operator==(const LayoutItem& src) const
{
  return (TranslatableItem::operator==(src)) &&
         (m_sequence == src.m_sequence) &&
         (m_editable == src.m_editable);
}

bool LayoutItem::get_editable() const
{
  return m_editable;
}

void LayoutItem::set_editable(bool val)
{
  m_editable = val;
}

Glib::ustring LayoutItem::get_layout_display_name() const
{
  return Glib::ustring();
}

Glib::ustring LayoutItem::get_report_part_id() const
{
  return "unexpected_report_part_id"; //This should never be used.
}
