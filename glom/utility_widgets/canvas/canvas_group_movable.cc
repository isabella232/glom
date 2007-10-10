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

#include "canvas_group_movable.h"
#include "canvas_rect_movable.h"
#include "canvas_line_movable.h"
#include "canvas_text_movable.h"
#include <libgoocanvasmm/canvas.h>
#include <goocanvasgroup.h>
#include <gdkmm/cursor.h>
#include <iostream>

namespace Glom
{


CanvasGroupMovable::CanvasGroupMovable()
: Goocanvas::Group((GooCanvasGroup*)goo_canvas_group_new(NULL, NULL)) //TODO: Remove this when goocanvas has been fixed.
{
   //TODO: Remove this when goocanvas is fixed, so the libgoocanvasmm constructor can connect default signal handlers:
  signal_motion_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_motion_notify_event));
  signal_button_press_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_button_press_event));
  signal_button_release_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_button_release_event));

  signal_enter_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_enter_notify_event));
  signal_leave_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_leave_notify_event));
}

CanvasGroupMovable::~CanvasGroupMovable()
{
}

Glib::RefPtr<CanvasGroupMovable> CanvasGroupMovable::create()
{
  return Glib::RefPtr<CanvasGroupMovable>(new CanvasGroupMovable());
}

static Glib::RefPtr<CanvasItemMovable> cast_to_movable(const Glib::RefPtr<Goocanvas::Item>& item)
{
  Glib::RefPtr<CanvasItemMovable> movable;
  if(!item)
    return movable;

  //We can't cast directly to CanvasItemMovable because each class derives from it separately,
  //instead of it being a base class of Goocanvas::Item (the common base class):
  Glib::RefPtr<CanvasRectMovable> rect = Glib::RefPtr<CanvasRectMovable>::cast_dynamic(item);
  if(rect)
    movable = Glib::RefPtr<CanvasItemMovable>::cast_dynamic(rect);
  else
  {
    Glib::RefPtr<CanvasLineMovable> line = Glib::RefPtr<CanvasLineMovable>::cast_dynamic(item);
    if(line)
      movable = Glib::RefPtr<CanvasItemMovable>::cast_dynamic(line);
    else
    {
      Glib::RefPtr<CanvasTextMovable> text = Glib::RefPtr<CanvasTextMovable>::cast_dynamic(item);
      if(text)
        movable = Glib::RefPtr<CanvasItemMovable>::cast_dynamic(text);
      else
      {
        Glib::RefPtr<CanvasGroupMovable> group = Glib::RefPtr<CanvasGroupMovable>::cast_dynamic(item);
        if(group)
          movable = Glib::RefPtr<CanvasItemMovable>::cast_dynamic(group);
      }
    }
  }

  return movable;
}

void CanvasGroupMovable::get_xy(double& x, double& y)
{
  Glib::RefPtr<Goocanvas::Item> first_child = get_child(0);
  if(!first_child)
    return;

  Glib::RefPtr<CanvasItemMovable> movable = cast_to_movable(first_child);
  if(movable)
     movable->get_xy(x, y);
}

void CanvasGroupMovable::move(double x, double y)
{
  //Discover the offset:
  double old_x = 0;
  double old_y = 0;
  get_xy(old_x, old_y);

  const double offset_x = x - old_x;
  const double offset_y = y - old_y;

  //Apply the offset to all children:
  const int count = get_n_children();
  for(int i = 0; i < count; ++i)
  {
    Glib::RefPtr<Goocanvas::Item> child = get_child(i);
    Glib::RefPtr<CanvasItemMovable> movable = cast_to_movable(child);
    if(movable)
    {
      double this_x = 0;
      double this_y = 0;
      movable->get_xy(this_x, this_y);
      movable->move(this_x + offset_x, this_y + offset_y);
    }
  }
}

void CanvasGroupMovable::snap_position(double& x, double& y) const
{
  CanvasItemMovable::snap_position(x, y);
}

Goocanvas::Canvas* CanvasGroupMovable::get_parent_canvas_widget()
{
  return get_canvas();
}


} //namespace Glom

