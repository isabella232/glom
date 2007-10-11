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

#include "canvas_item_movable.h"
#include "canvas_rect_movable.h"
#include "canvas_text_movable.h"
#include "canvas_line_movable.h"
#include "canvas_group_movable.h"
#include <libgoocanvasmm/canvas.h>
#include <goocanvasrect.h>
#include <goocanvasgroup.h>
#include <gdkmm/cursor.h>
#include <iostream>

namespace Glom
{


CanvasItemMovable::CanvasItemMovable()
: m_dragging(false),
  m_dragging_vertical_only(false), m_dragging_horizontal_only(false),
  m_drag_start_cursor_x(0.0), m_drag_start_cursor_y(0.0),
  m_drag_start_position_x(0.0), m_drag_start_position_y(0.0),
  m_grid(0),
  m_allow_vertical_movement(true), m_allow_horizontal_movement(true)
{
   //TODO: Remove this when goocanvas is fixed, so the libgoocanvasmm constructor can connect default signal handlers:
  /*
  signal_motion_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_motion_notify_event));
  signal_button_press_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_button_press_event));
  signal_button_release_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_button_release_event));

  signal_enter_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_enter_notify_event));
  signal_leave_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_leave_notify_event));
  */
}

CanvasItemMovable::~CanvasItemMovable()
{
}


bool CanvasItemMovable::on_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event)
{
  switch(event->button)
  {
    case 1:
    {
      if(!m_allow_vertical_movement && !m_allow_horizontal_movement)
        return false; // Not handled. Let it be handled by an item lower in the z order, or a parent group, if any.

      Glib::RefPtr<Goocanvas::Item> item = target;
    
      m_drag_start_cursor_x = event->x;
      m_drag_start_cursor_y = event->y;

      get_xy(m_drag_start_position_x, m_drag_start_position_y);
    
      Goocanvas::Canvas* canvas = get_parent_canvas_widget();
      if(canvas)
      {
        canvas->pointer_grab(item, Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_RELEASE_MASK, m_drag_cursor, event->time);
      }

      m_dragging = true;
      return true; /* Handled. */
      break;
    }
    case 3:
    {
      std::cout << "CanvasItemMovable::on_button_press_event(): type=" << typeid(this).name() << std::endl;
      m_signal_show_context.emit(event->button, event->time);
      return false; /* Not fully Handled. */
      break;
    }
    default:
      break;
  }

  
  return false; /* Not handled. Pass it to an item lower in the z order, if any. */
}

bool CanvasItemMovable::on_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event)
{ 
  if(!m_allow_vertical_movement && !m_allow_horizontal_movement)
    return false; // Not handled. Let it be handled by an item lower in the z order, or a parent group, if any.

  Glib::RefPtr<Goocanvas::Item> item = target;
  
  if(item && m_dragging && (event->state & Gdk::BUTTON1_MASK))
  {
    const double offset_x = event->x - m_drag_start_cursor_x;
    const double offset_y = event->y - m_drag_start_cursor_y;

    // Inkscape uses the Ctrl key to restrict movement to horizontal or vertical, 
    // so let's do that too.
    if( (event->state & Gdk::CONTROL_MASK) && !m_dragging_vertical_only && !m_dragging_horizontal_only )
    {
      //Decide whether to restrict to vertical or horizontal movement:
      //Whichever has the greatest offset already will be the axis that we restrict movement to.
      if(offset_x > offset_y)
      {
        m_dragging_horizontal_only = true;
        m_dragging_vertical_only = false;
      }
      else
      {
        m_dragging_vertical_only = true;
        m_dragging_horizontal_only = false;
      }
    }
    else if ( !(event->state & Gdk::CONTROL_MASK) && (m_dragging_vertical_only || m_dragging_horizontal_only))
    {
      //Ctrl was released, so allow full movement again:
      m_dragging_vertical_only = false;
      m_dragging_horizontal_only = false;
    }

    double new_x = m_drag_start_position_x + offset_x;
    double new_y = m_drag_start_position_y + offset_y;

    snap_position(new_x, new_y);

    if(!m_allow_vertical_movement || m_dragging_horizontal_only)
      new_y = m_drag_start_position_y;

    if(!m_allow_horizontal_movement || m_dragging_vertical_only)
      new_x = m_drag_start_position_x;

    move(new_x, new_y);

    m_signal_moved.emit();

    return true;
  }

  return false;
}

bool CanvasItemMovable::on_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event)
{
  if(!m_allow_vertical_movement && !m_allow_horizontal_movement)
    return false; // Not handled. Let it be handled by an item lower in the z order, or a parent group, if any.

  Goocanvas::Canvas* canvas = get_parent_canvas_widget();
  if(canvas)
    canvas->pointer_ungrab(target, event->time);

  m_dragging = false;

  return true;
}

bool CanvasItemMovable::on_enter_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event)
{
  set_cursor(m_drag_cursor);

  return true;
}


bool CanvasItemMovable::on_leave_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event)
{
  unset_cursor();

  return true;
}

CanvasItemMovable::type_signal_moved CanvasItemMovable::signal_moved()
{
  return m_signal_moved;
}

CanvasItemMovable::type_signal_show_context CanvasItemMovable::signal_show_context()
{
  return m_signal_show_context;
}

void CanvasItemMovable::set_drag_cursor(const Gdk::Cursor& cursor)
{
  m_drag_cursor = cursor;
}

void CanvasItemMovable::set_drag_cursor(Gdk::CursorType cursor)
{
  m_drag_cursor = Gdk::Cursor(cursor);
}

void CanvasItemMovable::set_cursor(const Gdk::Cursor& cursor)
{
   Goocanvas::Canvas* canvas = get_parent_canvas_widget();
   if(canvas)
   {
     Glib::RefPtr<Gdk::Window> window = canvas->get_window();
     if(window)
       window->set_cursor(cursor);
   }
}

void CanvasItemMovable::unset_cursor()
{
   Goocanvas::Canvas* canvas = get_parent_canvas_widget();
   if(canvas)
   {
     Glib::RefPtr<Gdk::Window> window = canvas->get_window();
     if(window)
       window->set_cursor();
   }
}

void CanvasItemMovable::set_grid(const Glib::RefPtr<const CanvasGroupGrid>& grid)
{
  m_grid = grid;
}

void CanvasItemMovable::snap_position(double& x, double& y) const
{
  //Override this to snap on a part other than the arbitrary part used by get_xy() and move().
  //For instance, you may want to snap on the bottom-left corner of a rectangle rather than the top-left.

  if(m_grid)
    m_grid->snap_position(x, y);
}

void CanvasItemMovable::set_movement_allowed(bool vertical, bool horizontal)
{
  m_allow_vertical_movement = vertical;
  m_allow_horizontal_movement = horizontal;
}

//static:
Glib::RefPtr<CanvasItemMovable> CanvasItemMovable::cast_to_movable(const Glib::RefPtr<Goocanvas::Item>& item)
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

} //namespace Glom

