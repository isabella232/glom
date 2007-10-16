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

#include "canvas_line_movable.h"
#include <libgoocanvasmm/canvas.h>
#include <goocanvaspolyline.h>
#include <goocanvasgroup.h>
#include <gdkmm/cursor.h>
#include <iostream>

namespace Glom
{


CanvasLineMovable::CanvasLineMovable()
: Goocanvas::Polyline(0.0, 0.0, 0.0, 0.0)
{
  signal_motion_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_motion_notify_event));
  signal_button_press_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_button_press_event));
  signal_button_release_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_button_release_event));

  signal_enter_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_enter_notify_event));
  signal_leave_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_leave_notify_event));
}

CanvasLineMovable::~CanvasLineMovable()
{
}

Glib::RefPtr<CanvasLineMovable> CanvasLineMovable::create()
{
  return Glib::RefPtr<CanvasLineMovable>(new CanvasLineMovable());
}

void CanvasLineMovable::get_xy(double& x, double& y)
{
  Goocanvas::Points points = property_points();
  points.get_coordinate(0, x, y);
}

void CanvasLineMovable::set_xy(double x, double y)
{
  //Discover the offset:
  double old_x = 0;
  double old_y = 0;
  Goocanvas::Points old_points = property_points();
  old_points.get_coordinate(0, old_x, old_y);

  const double offset_x = x - old_x;
  const double offset_y = y - old_y;

  //Apply the offset to all points:
  const int count = old_points.get_num_points();
  Goocanvas::Points new_points(count);
  for(int i = 0; i < count; ++i)
  {
    double this_x = 0;
    double this_y = 0;
    old_points.get_coordinate(i, this_x, this_y);
    new_points.set_coordinate(i, this_x + offset_x, this_y + offset_y);
  }
    
  property_points() = new_points;
}

void CanvasLineMovable::get_width_height(double& width, double& height)
{
  Goocanvas::Points points = property_points();
  double x1 = 0;
  double y1 = 0;
  points.get_coordinate(0, x1, y1);

  double x2 = 0;
  double y2 = 0;
  points.get_coordinate(1, x2, y2);

  width = x2 -x1;
  height = y2 - y1;
}

void CanvasLineMovable::set_width_height(double width, double height)
{
  Goocanvas::Points points = property_points();
  double x1 = 0;
  double y1 = 0;
  points.get_coordinate(0, x1, y1);
  points.set_coordinate(1, x1+width, y1+height);
}

Goocanvas::Canvas* CanvasLineMovable::get_parent_canvas_widget()
{
  return get_canvas();
}

} //namespace Glom

