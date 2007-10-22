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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "canvas_text_movable.h"
#include <goocanvasmm/canvas.h>
#include <goocanvastext.h>
#include <iostream>

namespace Glom
{


CanvasTextMovable::CanvasTextMovable(const Glib::ustring& text, double x, double y, double width, Gtk::AnchorType anchor)
: Goocanvas::Text(text, x, y, width, anchor), 
  CanvasItemMovable(),
  m_snap_corner(CORNER_TOP_LEFT), //arbitrary default.
  m_fake_height(0)
{
  init();
}

CanvasTextMovable::~CanvasTextMovable()
{
}

void CanvasTextMovable::init()
{
  signal_motion_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_motion_notify_event));
  signal_button_press_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_button_press_event));
  signal_button_release_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_button_release_event));

  signal_enter_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_enter_notify_event));
  signal_leave_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_leave_notify_event));
}

Glib::RefPtr<CanvasTextMovable> CanvasTextMovable::create(const Glib::ustring& string, double x, double y, double width, Gtk::AnchorType anchor)
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
  
  //We don't use this because it's only useful when you force a width, instead of allowing _enough_ width: 
  //height = property_height();

  width = property_width();

  Goocanvas::Bounds bounds = get_bounds();
 
  if(width == -1) //-1 means unlimited.
    width = bounds.get_x2() - bounds.get_x1();

  if(m_fake_height == 0)
    height = bounds.get_y2() - bounds.get_y1();
  else
    height = m_fake_height;
}

void CanvasTextMovable::set_width_height(double width, double height)
{
  property_width() = width;

  //There is no height property:
  //property_height() = height;
  m_fake_height = height;
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
    case CORNER_TOP_LEFT:
      corner_x_offset = 0;
      corner_y_offset = 0;
      break;
    case CORNER_TOP_RIGHT:
      corner_x_offset = property_width();
      corner_y_offset = 0;
      break;
    case CORNER_BOTTOM_LEFT:
      corner_x_offset = 0;
      corner_y_offset = height;
      break;
    case CORNER_BOTTOM_RIGHT:
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

} //namespace Glom

