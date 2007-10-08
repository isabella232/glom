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
: Goocanvas::Polyline((GooCanvasPolyline*)goo_canvas_polyline_new_line(NULL, false /* don't close the path */, 0.0, 0.0, 0.0, 0.0, NULL)), //TODO: Remove this when goocanvas has been fixed.
  m_dragging(false),
  m_drag_start_cursor_x(0.0), m_drag_start_cursor_y(0.0)
{
   //TODO: Remove this when goocanvas is fixed, so the libgoocanvasmm constructor can connect default signal handlers:
  signal_motion_notify_event().connect(sigc::mem_fun(*this, &CanvasLineMovable::on_motion_notify_event));
  signal_button_press_event().connect(sigc::mem_fun(*this, &CanvasLineMovable::on_button_press_event));
  signal_button_release_event().connect(sigc::mem_fun(*this, &CanvasLineMovable::on_button_release_event));
}

CanvasLineMovable::~CanvasLineMovable()
{
}

Glib::RefPtr<CanvasLineMovable> CanvasLineMovable::create()
{
  return Glib::RefPtr<CanvasLineMovable>(new CanvasLineMovable());
}


bool CanvasLineMovable::on_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event)
{
  //std::cout << "CanvasLineMovable::on_button_press_event()" << std::endl;

  switch(event->button)
  {
    case 1:
    {
      Glib::RefPtr<Goocanvas::Item> item = target;
      
      item->raise();
    
      m_drag_start_cursor_x = event->x;
      m_drag_start_cursor_y = event->y;

      Glib::RefPtr<Goocanvas::Polyline> line = Glib::RefPtr<Goocanvas::Polyline>::cast_dynamic(item);
      if(line)
      {
        m_drag_start_points = line->property_points();
      }
    
      Goocanvas::Canvas* canvas = get_canvas();
      if(canvas)
      {
        canvas->pointer_grab(item, Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_RELEASE_MASK,
          m_drag_cursor, event->time);
      }

      m_dragging = true;
      break;
    }

    default:
      break;
  }
  
  return true;
}

bool CanvasLineMovable::on_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event)
{ 
  //std::cout << "CanvasLineMovable::on_motion_notify_event()" << std::endl;

  Glib::RefPtr<Goocanvas::Item> item = target;
  
  if(item && m_dragging && (event->state & Gdk::BUTTON1_MASK))
  {
    const double offset_x = event->x - m_drag_start_cursor_x;
    const double offset_y = event->y - m_drag_start_cursor_y;

    Glib::RefPtr<Goocanvas::Polyline> line = Glib::RefPtr<Goocanvas::Polyline>::cast_dynamic(item);
    if(line)
    {
      const int count = m_drag_start_points.get_num_points();
      Goocanvas::Points new_points(count);
      for(int i = 0; i < count; ++i)
      {
        double x = 0;
        double y = 0;
        m_drag_start_points.get_coordinate(i, x, y);
        new_points.set_coordinate(i, x + offset_x, y + offset_y);
      }
    
      line->property_points() = new_points;
    }

    m_signal_moved.emit();
  }

  return true;
}

bool CanvasLineMovable::on_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event)
{
  Goocanvas::Canvas* canvas = get_canvas();
  if(canvas)
    canvas->pointer_ungrab(target, event->time);

  m_dragging = false;

  return true;
}

CanvasLineMovable::type_signal_moved CanvasLineMovable::signal_moved()
{
  return m_signal_moved;
}

void CanvasLineMovable::set_drag_cursor(const Gdk::Cursor& cursor)
{
  m_drag_cursor = cursor;
}

void CanvasLineMovable::set_drag_cursor(Gdk::CursorType cursor)
{
  m_drag_cursor = Gdk::Cursor(cursor);
}




} //namespace Glom

