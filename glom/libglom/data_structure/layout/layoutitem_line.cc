/* Glom
 *
 * Copyright (C) 2006 Murray Cumming
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

#include <libglom/data_structure/layout/layoutitem_line.h>
#include <libglom/utils.h>
#include <glibmm/i18n-lib.h>

namespace Glom
{

LayoutItem_Line::LayoutItem_Line()
: m_start_x(0),
  m_start_y(0),
  m_end_x(0),
  m_end_y(0),
  m_line_width(0.5f), //Arbitrary default
  m_color("black") //Arbitrary default
{
  m_translatable_item_type = enumTranslatableItemType::INVALID; //There is no text in this to translate.
}

LayoutItem_Line::LayoutItem_Line(const LayoutItem_Line& src)
: LayoutItem(src),
  m_start_x(src.m_start_x),
  m_start_y(src.m_start_y),
  m_end_x(src.m_end_x),
  m_end_y(src.m_end_y),
  m_line_width(src.m_line_width),
  m_color(src.m_color)
{
}

LayoutItem* LayoutItem_Line::clone() const
{
  return new LayoutItem_Line(*this);
}

bool LayoutItem_Line::operator==(const LayoutItem_Line& src) const
{
  auto result = LayoutItem::operator==(src) &&
                (m_start_x == src.m_start_x) &&
                (m_start_y == src.m_start_y) &&
                (m_end_x == src.m_end_x) &&
                (m_end_y == src.m_end_y) &&
                (m_line_width == src.m_line_width) &&
                (m_color == src.m_color);

  return result;
}

//Avoid using this, for performance:
LayoutItem_Line& LayoutItem_Line::operator=(const LayoutItem_Line& src)
{
  LayoutItem::operator=(src);

  m_start_x = src.m_start_x;
  m_start_y = src.m_start_y;
  m_end_x = src.m_end_x;
  m_end_y = src.m_end_y;
  m_line_width = src.m_line_width;
  m_color = src.m_color;

  return *this;
}

Glib::ustring LayoutItem_Line::get_part_type_name() const
{
  //Translators: This is the name of a UI element (a layout part name).
  //This is a straight line, not a database row.
  return _("Line");
}

Glib::ustring LayoutItem_Line::get_report_part_id() const
{
  return "line"; //We reuse this for this node.
}

void LayoutItem_Line::get_coordinates(double& start_x, double& start_y, double& end_x, double& end_y) const
{
  start_x = m_start_x;
  start_y = m_start_y;
  end_x = m_end_x;
  end_y = m_end_y;
}

void LayoutItem_Line::set_coordinates(double start_x, double start_y, double end_x, double end_y)
{
  m_start_x = start_x;
  m_start_y = start_y;
  m_end_x = end_x;
  m_end_y = end_y;

  //Set the x,y,height,width too,
  //for generic code that deals with that API:
  set_print_layout_position(m_start_x, m_start_y, (m_end_x - m_start_x), (m_end_y - m_start_y));
}

double LayoutItem_Line::get_line_width() const
{
  return m_line_width;
}

void LayoutItem_Line::set_line_width(double line_width)
{
  m_line_width = line_width;
}

Glib::ustring LayoutItem_Line::get_line_color() const
{
  return m_color;
}

void LayoutItem_Line::set_line_color(const Glib::ustring& color)
{
  m_color = color;
}


} //namespace Glom

