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

#include <libglom/data_structure/layout/layoutitem.h>

namespace Glom
{

LayoutItem::PrintLayoutPosition::PrintLayoutPosition()
: m_x(0),
  m_y(0),
  m_width(0),
  m_height(0),
  m_split_across_pages(false)
{
}

LayoutItem::PrintLayoutPosition::PrintLayoutPosition(const LayoutItem::PrintLayoutPosition& src)
: m_x(src.m_x),
  m_y(src.m_y),
  m_width(src.m_width),
  m_height(src.m_height),
  m_split_across_pages(src.m_split_across_pages)
{
}

LayoutItem::PrintLayoutPosition& LayoutItem::PrintLayoutPosition::operator=(const LayoutItem::PrintLayoutPosition& src)
{
  m_x = src.m_x;
  m_y = src.m_y;
  m_width = src.m_width;
  m_height = src.m_height;
  m_split_across_pages = src.m_split_across_pages;

  return *this;
}

bool LayoutItem::PrintLayoutPosition::operator==(const LayoutItem::PrintLayoutPosition& src) const
{
  return (m_x == src.m_x) &&
         (m_y == src.m_y) &&
         (m_width == src.m_width) &&
         (m_height == src.m_height) &&
         (m_split_across_pages == src.m_split_across_pages);
}


LayoutItem::LayoutItem()
: m_editable(true),
  m_display_width(0),
  m_positions(0)
{
  m_translatable_item_type = TRANSLATABLE_TYPE_LAYOUT_ITEM;
}

LayoutItem::LayoutItem(const LayoutItem& src)
: TranslatableItem(src),
  m_editable(src.m_editable),
  m_display_width(src.m_display_width),
  m_positions(0)
{
  if(src.m_positions)
    m_positions = new PrintLayoutPosition(*(src.m_positions));
}

LayoutItem::~LayoutItem()
{
  delete m_positions;
}

LayoutItem& LayoutItem::operator=(const LayoutItem& src)
{
  if(this == &src)
    return *this;

  TranslatableItem::operator=(src);

  m_editable = src.m_editable;
  m_display_width = src.m_display_width;

  delete m_positions;
  m_positions = 0;

  if(src.m_positions)
    m_positions = new PrintLayoutPosition(*(src.m_positions));

  return *this;
}

//Is this used?
bool LayoutItem::operator==(const LayoutItem& src) const
{
  bool equal = (TranslatableItem::operator==(src)) &&
          (m_editable == src.m_editable) &&
          (m_display_width == src.m_display_width);

  if(m_positions && src.m_positions)
  {
    //compare them:
    equal = equal && (*m_positions == *(src.m_positions));
  }
  else if(!m_positions && !src.m_positions)
  {
    //no change.
  }
  else
  {
    equal = false;
  }

  return equal;
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

guint LayoutItem::get_display_width() const
{
  return m_display_width;
}

void LayoutItem::set_display_width(guint value)
{
  m_display_width = value;
}

void LayoutItem::instantiate_positions() const
{
  if(!m_positions)
    m_positions = new PrintLayoutPosition();
}

void LayoutItem::get_print_layout_position(double& x, double& y, double& width, double& height) const
{
  if(!m_positions)
  {
    x = 0;
    y = 0;
    width = 0;
    height = 0;
  }
  else
  {
    x = m_positions->m_x;
    y = m_positions->m_y;
    width = m_positions->m_width;
    height = m_positions->m_height;
  }
}

void LayoutItem::set_print_layout_position(double x, double y, double width, double height)
{
  if(!m_positions && (x == 0) && (y == 0) && (width == 0) && (height == 0))
    return; //Don't bother instantiating the positions instance if everything is still 0.

  instantiate_positions();

  m_positions->m_x = x;
  m_positions->m_y = y;
  m_positions->m_width = width;
  m_positions->m_height = height;
}

void LayoutItem::set_print_layout_position_y(double y)
{
  if(!m_positions && (y == 0))
    return; //Don't bother instantiating the positions instance if everything is still 0.

  instantiate_positions();

  m_positions->m_y = y;
}

void LayoutItem::set_print_layout_split_across_pages(bool split)
{
  if(!m_positions && !split)
    return; //Don't bother instantiating the positions instance if everything is still 0.

  instantiate_positions();

  m_positions->m_split_across_pages = split;
}


bool LayoutItem::get_print_layout_split_across_pages() const
{
  if(!m_positions)
    return false;
  else
    return m_positions->m_split_across_pages;
}

} //namespace Glom

