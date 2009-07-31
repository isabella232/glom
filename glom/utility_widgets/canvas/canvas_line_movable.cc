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
#ifdef GLIBMM_PROPERTIES_ENABLED
  x = property_x();
  y = property_y();
#else
  get_property("x", x);
  get_property("y", y);
#endif
}

void CanvasLineMovable::set_xy(double x, double y)
{
#ifdef GLIBMM_PROPERTIES_ENABLED
  property_x() = x;
  property_y() = y;
#else
  set_property("x", x);
  set_property("y", y);
#endif
}

void CanvasLineMovable::get_width_height(double& width, double& height) const
{
#ifdef GLIBMM_PROPERTIES_ENABLED
  width = property_width();
  height = property_height();
#else
  get_property("width", width);
  get_property("height", height);
#endif

  //std::cout << "CanvasLineMovable::get_width_height(): width=" << width << std::endl;
}

void CanvasLineMovable::set_width_height(double width, double height)
{
#ifdef GLIBMM_PROPERTIES_ENABLED
  property_width() = width;
  property_height() = height;
#else
  set_property("width", width);
  set_property("height", height);
#endif

  //std::cout << "CanvasLineMovable::set_width_height(): end x=" << x1+width << std::endl;
}

Goocanvas::Canvas* CanvasLineMovable::get_parent_canvas_widget()
{
  return get_canvas();
}

} //namespace Glom

