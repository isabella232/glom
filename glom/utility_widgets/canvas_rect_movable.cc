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

#include "canvas_rect_movable.h"
#include <libgoocanvasmm/canvas.h>
#include <goocanvasrect.h>
#include <goocanvasgroup.h>
#include <gdkmm/cursor.h>
#include <iostream>

namespace Glom
{


CanvasRectMovable::CanvasRectMovable()
: Goocanvas::Rect((GooCanvasRect*)goo_canvas_rect_new(NULL, 0.0, 0.0, 0.0, 0.0, NULL)), //TODO: Remove this when goocanvas has been fixed.
  m_dragging(false),
  m_drag_start_cursor_x(0.0), m_drag_start_cursor_y(0.0),
  m_drag_start_position_x(0.0), m_drag_start_position_y(0.0)
{
   //TODO: Remove this when goocanvas is fixed, so the libgoocanvasmm constructor can connect default signal handlers:
  signal_motion_notify_event().connect(sigc::mem_fun(*this, &CanvasRectMovable::on_motion_notify_event));
  signal_button_press_event().connect(sigc::mem_fun(*this, &CanvasRectMovable::on_button_press_event));
  signal_button_release_event().connect(sigc::mem_fun(*this, &CanvasRectMovable::on_button_release_event));

  signal_enter_notify_event().connect(sigc::mem_fun(*this, &CanvasRectMovable::on_enter_notify_event));
  signal_leave_notify_event().connect(sigc::mem_fun(*this, &CanvasRectMovable::on_leave_notify_event));
}

CanvasRectMovable::~CanvasRectMovable()
{
}

Glib::RefPtr<CanvasRectMovable> CanvasRectMovable::create()
{
  return Glib::RefPtr<CanvasRectMovable>(new CanvasRectMovable());
}




bool CanvasRectMovable::on_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event)
{
  switch(event->button)
  {
    case 1:
    {
      Glib::RefPtr<Goocanvas::Item> item = target;
      
      item->raise();
    
      m_drag_start_cursor_x = event->x;
      m_drag_start_cursor_y = event->y;

      Glib::RefPtr<Goocanvas::Rect> rect = Glib::RefPtr<Goocanvas::Rect>::cast_dynamic(item);
      if(rect)
      {
        m_drag_start_position_x = rect->property_x();
        m_drag_start_position_y = rect->property_y();
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

bool CanvasRectMovable::on_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event)
{ 
  //std::cout << "CanvasRectMovable::on_motion_notify_event()" << std::endl;

  Glib::RefPtr<Goocanvas::Item> item = target;
  
  if(item && m_dragging && (event->state & Gdk::BUTTON1_MASK))
  {
    const double offset_x = event->x - m_drag_start_cursor_x;
    const double offset_y = event->y - m_drag_start_cursor_y;

    Glib::RefPtr<Goocanvas::Rect> rect = Glib::RefPtr<Goocanvas::Rect>::cast_dynamic(item);
    if(rect)
    {
      rect->property_x() = m_drag_start_position_x + offset_x;
      rect->property_y() = m_drag_start_position_y + offset_y;
    }

    m_signal_moved.emit();
  }

  return true;
}

bool CanvasRectMovable::on_enter_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event)
{
  set_cursor(m_drag_cursor);

  return true;
}

bool CanvasRectMovable::on_leave_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event)
{
  unset_cursor();

  return true;
}

bool CanvasRectMovable::on_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event)
{
  Goocanvas::Canvas* canvas = get_canvas();
  if(canvas)
    canvas->pointer_ungrab(target, event->time);

  m_dragging = false;

  return true;
}

CanvasRectMovable::type_signal_moved CanvasRectMovable::signal_moved()
{
  return m_signal_moved;
}

void CanvasRectMovable::set_drag_cursor(const Gdk::Cursor& cursor)
{
  m_drag_cursor = cursor;
}

void CanvasRectMovable::set_drag_cursor(Gdk::CursorType cursor)
{
  m_drag_cursor = Gdk::Cursor(cursor);
}

void CanvasRectMovable::set_cursor(const Gdk::Cursor& cursor)
{
   Goocanvas::Canvas* canvas = get_canvas();
   if(canvas)
   {
     Glib::RefPtr<Gdk::Window> window = canvas->get_window();
     if(window)
       window->set_cursor(cursor);
   }
}

void CanvasRectMovable::unset_cursor()
{
   Goocanvas::Canvas* canvas = get_canvas();
   if(canvas)
   {
     Glib::RefPtr<Gdk::Window> window = canvas->get_window();
     if(window)
       window->set_cursor();
   }
}

} //namespace Glom

