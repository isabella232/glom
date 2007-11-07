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

#include "canvas_table_movable.h"
#include <goocanvasmm/canvas.h>
#include <gdkmm/cursor.h>
#include <iostream>

namespace Glom
{


CanvasTableMovable::CanvasTableMovable()
: m_x(0), m_y(0)
{
  signal_motion_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_motion_notify_event));
  signal_button_press_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_button_press_event));
  signal_button_release_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_button_release_event));

  signal_enter_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_enter_notify_event));
  signal_leave_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_leave_notify_event));
}

CanvasTableMovable::~CanvasTableMovable()
{
}

Glib::RefPtr<CanvasTableMovable> CanvasTableMovable::create()
{
  return Glib::RefPtr<CanvasTableMovable>(new CanvasTableMovable());
}

void CanvasTableMovable::get_xy(double& x, double& y) const
{
  x = m_x;
  y = m_y;

  // Or we could use the child at the top-left:
  /*
  Glib::RefPtr<const Goocanvas::Item> first_child = get_child(0);
  if(!first_child)
    return;

  Glib::RefPtr<const CanvasItemMovable> movable = CanvasItemMovable::cast_const_to_movable(first_child);
  if(movable)
     movable->get_xy(x, y);
  */
}

void CanvasTableMovable::set_xy(double x, double y)
{
  //Discover the offset:
  double old_x = 0;
  double old_y = 0;
  get_xy(old_x, old_y);

  const double offset_x = x - old_x;
  const double offset_y = y - old_y;

  //Apply the offset:
  translate(offset_x, offset_y);

  //Remember the position, because GooCanvasTable does not:
  m_x = x;
  m_y = y;
}

void CanvasTableMovable::get_width_height(double& width, double& height) const
{
  width = property_width();
  height = property_height();

  if(width == -1) //Means "default width" - presumably the width demanded by the children. But we don't use that.
    width = 0;

  if(height == -1) //Means "default height" - presumably the height demanded by the children. But we don't use that.
    height = 0;
}

void CanvasTableMovable::set_width_height(double width, double height)
{
  if(width == -1)
  {
    std::cout << "CanvasTableMovable::set_width_height(): width is -1" << std::endl;
  }

  property_width() = width;
  property_height() = height;
}

void CanvasTableMovable::set_grid(const Glib::RefPtr<const CanvasGroupGrid>& grid)
{
  //Call the base class:
  CanvasItemMovable::set_grid(grid);

  //Apply the grid to all children:
  const int count = get_n_children();
  for(int i = 0; i < count; ++i)
  {
    Glib::RefPtr<Goocanvas::Item> child = get_child(i);
    Glib::RefPtr<CanvasItemMovable> movable = CanvasItemMovable::cast_to_movable(child);
    if(movable)
    {
      movable->set_grid(grid);
    }
  }
}

void CanvasTableMovable::snap_position_one_corner(Corners corner, double& x, double& y) const
{
  Goocanvas::Bounds bounds = get_bounds();
  const double width = std::abs(bounds.get_x2() - bounds.get_x1());
  const double height = std::abs(bounds.get_y2() - bounds.get_y1());

  //Choose the offset of the part to snap to the grid:
  double corner_x_offset = 0;
  double corner_y_offset = 0;
  switch(corner)
  {
    case CORNER_TOP_LEFT:
      corner_x_offset = 0;
      corner_y_offset = 0;
      break;
    case CORNER_TOP_RIGHT:
      corner_x_offset = width;
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

void CanvasTableMovable::snap_position(double& x, double& y) const
{
  //std::cout << "CanvasTableMovable::snap_position" << std::endl;

  double offset_x_min = 0;
  double offset_y_min = 0;

  //Try snapping each corner, to choose the one that snapped closest:
  for(int i = CORNER_TOP_LEFT; i < CORNER_COUNT; ++i)
  {
    const Corners corner = (Corners)i;
    double temp_x = x;
    double temp_y = y;
    snap_position_one_corner(corner, temp_x, temp_y);

    const double offset_x = temp_x -x;
    const double offset_y = temp_y - y;

    //Use the smallest offset, preferring some offset to no offset:
    if(offset_x && ((std::abs(offset_x) < std::abs(offset_x_min)) || !offset_x_min))
      offset_x_min = offset_x;

    if(offset_y && ((std::abs(offset_y) < std::abs(offset_y_min)) || !offset_y_min))
      offset_y_min = offset_y;
  }

  x += offset_x_min;
  y += offset_y_min;
}

Goocanvas::Canvas* CanvasTableMovable::get_parent_canvas_widget()
{
  return get_canvas();
}


} //namespace Glom

