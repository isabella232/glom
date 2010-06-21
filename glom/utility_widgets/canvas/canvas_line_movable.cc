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
#include <goocanvasmm/canvas.h>
#include <gdkmm/cursor.h>
#include <iostream>

namespace Glom
{


CanvasLineMovable::CanvasLineMovable()
: Goocanvas::Polyline(0.0, 0.0, 0.0, 0.0),
  CanvasItemMovable()
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

void CanvasLineMovable::get_xy(double& x, double& y) const
{
  x = property_x();
  y = property_y();
}

void CanvasLineMovable::set_xy(double x, double y)
{
  property_x() = x;
  property_y() = y;
}

void CanvasLineMovable::get_width_height(double& width, double& height) const
{
  width = property_width();
  height = property_height();

  //std::cout << "debug: " << G_STRFUNC << ": width=" << width << std::endl;
}

void CanvasLineMovable::set_width_height(double width, double height)
{
  property_width() = width;
  property_height() = height;

  //std::cout << "debug: " << G_STRFUNC << ": end x=" << x1+width << std::endl;
}

Goocanvas::Canvas* CanvasLineMovable::get_parent_canvas_widget()
{
  return get_canvas();
}

} //namespace Glom

