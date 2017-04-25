/* Glom
 *
 * Copyright (C) 2007 Murray Cumming
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

#include "canvas_text_movable.h"
#include <goocanvasmm/canvas.h>
#include <glibmm/markup.h>
#include <goocanvastext.h>
#include <iostream>

namespace Glom
{


CanvasTextMovable::CanvasTextMovable(const Glib::ustring& text, double x, double y, double width, Goocanvas::AnchorType anchor)
: Goocanvas::Text(text, x, y, width, anchor),
  m_snap_corner(Corners::TOP_LEFT) //arbitrary default.
{
  init();
}

void CanvasTextMovable::init()
{
  signal_motion_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_motion_notify_event), true /* connect after */);
  signal_button_press_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_button_press_event), true /* connect after */);
  signal_button_release_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_button_release_event), true /* connect after */);

  signal_enter_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_enter_notify_event), true /* connect after */);
  signal_leave_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_leave_notify_event), true /* connect after */);
}

Glib::RefPtr<CanvasTextMovable> CanvasTextMovable::create(const Glib::ustring& string, double x, double y, double width, Goocanvas::AnchorType anchor)
{
  return Glib::RefPtr<CanvasTextMovable>(new CanvasTextMovable(string, x, y, width, anchor));
}

void CanvasTextMovable::get_xy(double& x, double& y) const
{
  x = property_x();
  y = property_y();
}

void CanvasTextMovable::set_xy(double x, double y)
{
  property_x() = x;
  property_y() = y;
}

void CanvasTextMovable::get_width_height(double& width, double& height) const
{
  //TODO: This only works when it is on a canvas already,
  //and this is apparently incorrect when the "coordinate space" of the item changes, whatever that means. murrayc.

  width = property_width();
  height = property_height();
}

void CanvasTextMovable::set_width_height(double width, double height)
{
  property_width() = width;
  property_height() = height;
}

void CanvasTextMovable::snap_position(double& x, double& y) const
{
  double width = 0;
  double height = 0;
  get_width_height(width, height);

  //Choose the offset of the part to snap to the grid:
  double corner_x_offset = 0;
  double corner_y_offset = 0;
  switch(m_snap_corner)
  {
    case Corners::TOP_LEFT:
      corner_x_offset = 0;
      corner_y_offset = 0;
      break;
    case Corners::TOP_RIGHT:
      corner_x_offset = property_width();
      corner_y_offset = 0;
      break;
    case Corners::BOTTOM_LEFT:
      corner_x_offset = 0;
      corner_y_offset = height;
      break;
    case Corners::BOTTOM_RIGHT:
      corner_x_offset = width;
      corner_y_offset = height;
      break;
    default:
      break;
  }

  //Snap that point to the grid:
  const double x_to_snap = x + corner_x_offset;
  const double y_to_snap = y + corner_y_offset;
  double corner_x_snapped = x_to_snap;
  double corner_y_snapped = y_to_snap;
  CanvasItemMovable::snap_position(corner_x_snapped, corner_y_snapped);

  //Discover what offset the snapping causes:
  const double snapped_offset_x = corner_x_snapped - x_to_snap;
  const double snapped_offset_y = corner_y_snapped - y_to_snap;

  //Apply that offset to the regular position:
  x += snapped_offset_x;
  y += snapped_offset_y;
}

Goocanvas::Canvas* CanvasTextMovable::get_parent_canvas_widget()
{
  return get_canvas();
}

void CanvasTextMovable::set_snap_corner(Corners corner)
{
  m_snap_corner = corner;
}

void CanvasTextMovable::set_text(const Glib::ustring& text)
{
  m_text = text;
  reconstruct_markup();
}

void CanvasTextMovable::set_font_points(const Glib::ustring& font)
{
  Glib::ustring font_points = font;
  if(font_points.empty())
    font_points = "Serif 9";

  //Convert the size to mm, because GooCanvasText can only understand font sizes in terms of the canvas units,
  //but user will provide the size in points.
  //TODO: Discover the canvas units and do an appropriate conversion,
  //so that this works for other canvas units:
  Pango::FontDescription pango_font(font_points);
  pango_font.set_absolute_size( (int)((double)pango_font.get_size() * 0.375) ); //according to Wikipedia.
  //I don't know what it would be relative to, but it seems necessary to specify the absolute size.

  font_points = pango_font.to_string();

  m_font = font_points;
  //std::cout << "DEBUG: m_font=" << m_font << std::endl;

  reconstruct_markup();
}

void CanvasTextMovable::reconstruct_markup()
{
  if(m_font.empty())
  {
    property_text() = m_text;
    return;
  }

  char* markup = nullptr;
  if(!m_text.empty())
  {
    //We will use the text as markup, so remove anything that could be
    //interpreted as pango markup.
    //This is not really pango-specific, but it might just work:
    const auto text_escaped = Glib::Markup::escape_text(m_text);

    //We add px (meaning absolute points size).
    //Otherwise both GooCanvas and GTK+ scale the font up, making it too large.
    //This really seems like a bug in GooCanvas.
    //TODO: This might not be robust - it assumes that the font size is at the end of the font_desc
    //provided by GtkFontButton.
    markup = g_strdup_printf("<span font_desc=\"%s\">%s</span>", m_font.c_str(), text_escaped.c_str());
    //std::cout << "DEBUG: markup=" << markup << std::endl;
  }
  property_use_markup() = true;


  if(markup)
  {
    property_text() = Glib::ustring(markup); //TODO: Inefficient.
    g_free(markup);
  }
  else
  {
    property_text() = Glib::ustring();
  }
}


} //namespace Glom

