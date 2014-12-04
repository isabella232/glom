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

#include "canvas_item_movable.h"
#include "canvas_rect_movable.h"
#include "canvas_text_movable.h"
#include "canvas_image_movable.h"
#include "canvas_line_movable.h"
#include "canvas_group_movable.h"
#include "canvas_group_resizable.h"
#include "canvas_table_movable.h"
#include <goocanvasmm/canvas.h>
#include <goocanvasrect.h>
#include <goocanvasgroup.h>
#include <gdkmm/cursor.h>
#include <iostream>

namespace {

static Glib::RefPtr<Gdk::Cursor> create_drag_cursor(GdkEventAny* event, Gdk::CursorType cursor_type)
{
  Glib::RefPtr<Gdk::Window> window = Glib::wrap(event->window, true);
  Glib::RefPtr<Gdk::Display> display = window->get_display();
  return Gdk::Cursor::create(display, cursor_type);
}

} //anonymous namespace

namespace Glom
{

CanvasItemMovable::CanvasItemMovable()
: m_dragging(false),
  m_dragging_vertical_only(false), m_dragging_horizontal_only(false),
  m_drag_start_cursor_x(0.0), m_drag_start_cursor_y(0.0),
  m_drag_start_position_x(0.0), m_drag_start_position_y(0.0),
  m_drag_latest_position_x(0.0), m_drag_latest_position_y(0.0),
  m_drag_cursor_type(Gdk::FLEUR), //arbitrary default
  m_grid(0),
  m_allow_vertical_movement(true), m_allow_horizontal_movement(true),
  m_selected(false),
  m_shift_click(false)
{
   //TODO: Remove this when goocanvas is fixed, so the goocanvasmm constructor can connect default signal handlers:
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
  //std::cout << G_STRFUNC << ": DEBUG" << std::endl;

  m_shift_click = false;

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
      m_drag_latest_position_x = m_drag_start_position_x;
      m_drag_latest_position_y = m_drag_start_position_y; 
    
      Goocanvas::Canvas* canvas = get_parent_canvas_widget();
      if(canvas)
      {
        canvas->pointer_grab(item,
          Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_RELEASE_MASK,
          create_drag_cursor((GdkEventAny*)event, m_drag_cursor_type),
          event->time);
      }

      m_dragging = true;

      //Holding down shift when pressing the mouse down
      //means that any selection (decided later) will be a multiple selection.
      if(event->state & GDK_SHIFT_MASK)
        m_shift_click = true;

      return true; // Handled.
      break;
    }
    case 3:
    {
      m_signal_show_context.emit(event->button, event->time);
      return false; // Not fully Handled.
      break;
    }
    default:
      break;
  }

  
  return false; // Not handled. Pass it to an item lower in the z order, if any.
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
    else if( !(event->state & Gdk::CONTROL_MASK) && (m_dragging_vertical_only || m_dragging_horizontal_only))
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

    set_xy(new_x, new_y);

    Glib::RefPtr<CanvasItemMovable> refThis(this);
    refThis->reference();

    // A click with a move should always select:
    // We emit this before signal_moved,
    // so that its signal handler can know about the selection.
    const bool old_selected = get_selected();
    set_selected(true);
    if(!old_selected)
    {
      m_signal_selected.emit(refThis, m_shift_click);
    }

    const double this_move_offset_x = new_x - m_drag_latest_position_x;
    const double this_move_offset_y = new_y - m_drag_latest_position_y;
    m_drag_latest_position_x = new_x;
    m_drag_latest_position_y = new_y;
    m_signal_moved.emit(refThis, this_move_offset_x, this_move_offset_y);

    return true; //We handled this event.
  }

  return false; //We didn't handle this event.
}

bool CanvasItemMovable::on_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event)
{
  //std::cout << G_STRFUNC << ": DEBUG" << std::endl;

  if(!m_allow_vertical_movement && !m_allow_horizontal_movement)
    return false; // Not handled. Let it be handled by an item lower in the z order, or a parent group, if any.

  Goocanvas::Canvas* canvas = get_parent_canvas_widget();
  if(canvas)
    canvas->pointer_ungrab(target, event->time);

  m_dragging = false;


  // A click without a move should select or deselect:
  const bool old_selected = get_selected();
  bool selected = !old_selected;

  // A drag-to-move should always select and never deselect:
  if(!selected)
  {
    double x = 0;
    double y = 0;
    get_xy(x, y);
    if( (m_drag_start_position_x != x)
      || (m_drag_start_position_y != y) )
    {
      selected = true;
    }
  }

  //This will also ask derived classes to indicate it visually:

  set_selected(selected);

  //Notify of the selection change, if any:
  if(selected != old_selected)
  {
    Glib::RefPtr<CanvasItemMovable> refThis(this);
    refThis->reference();
    m_signal_selected.emit(refThis, m_shift_click);
  }

  return true;
}

bool CanvasItemMovable::on_enter_notify_event(const Glib::RefPtr<Goocanvas::Item>& /* target */, GdkEventCrossing* event)
{
  set_cursor(create_drag_cursor((GdkEventAny*)event, m_drag_cursor_type));

  return false; //We didn't fully handle this event - let other signal handlers (even for other items) handle it too.
}


bool CanvasItemMovable::on_leave_notify_event(const Glib::RefPtr<Goocanvas::Item>& /* target */, GdkEventCrossing* /* event */)
{
  unset_cursor();

  return false; //We didn't fully handle this event - let other signal handlers (even for other items) handle it too.
}

CanvasItemMovable::type_signal_moved CanvasItemMovable::signal_moved()
{
  return m_signal_moved;
}

CanvasItemMovable::type_signal_show_context CanvasItemMovable::signal_show_context()
{
  return m_signal_show_context;
}

CanvasItemMovable::type_signal_selected CanvasItemMovable::signal_selected()
{
  return m_signal_selected;
}

void CanvasItemMovable::set_drag_cursor(Gdk::CursorType cursor_type)
{
  m_drag_cursor_type = cursor_type;
}

void CanvasItemMovable::set_cursor(const Glib::RefPtr<Gdk::Cursor>& cursor)
{
   Goocanvas::Canvas* canvas = get_parent_canvas_widget();
   if(!canvas)
     return;
     
   Glib::RefPtr<Gdk::Window> window = canvas->get_window();
   if(window)
     window->set_cursor(cursor);
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
  //else
  //{
  //  std::cout << "debug: " << G_STRFUNC << ": m_grid is NULL" << std::endl;
  //}
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
        Glib::RefPtr<CanvasImageMovable> image = Glib::RefPtr<CanvasImageMovable>::cast_dynamic(item);
        if(image)
          movable = Glib::RefPtr<CanvasItemMovable>::cast_dynamic(image);
        else
        {
          Glib::RefPtr<CanvasGroupMovable> group = Glib::RefPtr<CanvasGroupMovable>::cast_dynamic(item);
          if(group)
            movable = Glib::RefPtr<CanvasItemMovable>::cast_dynamic(group);
          else
          {
            Glib::RefPtr<CanvasTableMovable> table = Glib::RefPtr<CanvasTableMovable>::cast_dynamic(item);
            if(table)
              movable = Glib::RefPtr<CanvasTableMovable>::cast_dynamic(table);
            else
            {
              Glib::RefPtr<CanvasGroupResizable> group_resizable = Glib::RefPtr<CanvasGroupResizable>::cast_dynamic(item);
              if(group_resizable)
                movable = Glib::RefPtr<CanvasItemMovable>::cast_dynamic(group_resizable);
            }
          }
        }
      }
    }
  }

  //Goocanvas::Item* debug = item.operator->();
  //std::cout << "CanvasItemMovable::cast_to_movable(" << typeid(*debug).name() << ") = " << movable << std::endl;

  return movable;
}

//static:
Glib::RefPtr<const CanvasItemMovable> CanvasItemMovable::cast_const_to_movable(const Glib::RefPtr<const Goocanvas::Item>& item)
{
  Glib::RefPtr<Goocanvas::Item> unconst = Glib::RefPtr<Goocanvas::Item>::cast_const(item);
  return cast_to_movable(unconst);
}


//static:
Glib::RefPtr<Goocanvas::Item> CanvasItemMovable::cast_to_item(const Glib::RefPtr<CanvasItemMovable>& item)
{
  Glib::RefPtr<Goocanvas::Item> result;
  if(!item)
    return result;

  //We can't cast directly to Item because each class derives from it separately.
  Glib::RefPtr<CanvasRectMovable> rect = Glib::RefPtr<CanvasRectMovable>::cast_dynamic(item);
  if(rect)
    result = Glib::RefPtr<Goocanvas::Item>::cast_dynamic(rect);
  else
  {
    Glib::RefPtr<CanvasLineMovable> line = Glib::RefPtr<CanvasLineMovable>::cast_dynamic(item);
    if(line)
      result = Glib::RefPtr<Goocanvas::Item>::cast_dynamic(line);
    else
    {
      Glib::RefPtr<CanvasTextMovable> text = Glib::RefPtr<CanvasTextMovable>::cast_dynamic(item);
      if(text)
        result = Glib::RefPtr<Goocanvas::Item>::cast_dynamic(text);
      else
      {
        Glib::RefPtr<CanvasImageMovable> image = Glib::RefPtr<CanvasImageMovable>::cast_dynamic(item);
        if(image)
          result = Glib::RefPtr<Goocanvas::Item>::cast_dynamic(image);
        else
        {
          Glib::RefPtr<CanvasGroupMovable> group = Glib::RefPtr<CanvasGroupMovable>::cast_dynamic(item);
          if(group)
            result = Glib::RefPtr<Goocanvas::Item>::cast_dynamic(group);
          else
          {
            Glib::RefPtr<CanvasTableMovable> table = Glib::RefPtr<CanvasTableMovable>::cast_dynamic(item);
            if(table)
              result = Glib::RefPtr<Goocanvas::Item>::cast_dynamic(table);
            else
            {
              Glib::RefPtr<CanvasGroupResizable> group_resizable = Glib::RefPtr<CanvasGroupResizable>::cast_dynamic(item);
              if(group_resizable)
                result = Glib::RefPtr<Goocanvas::Item>::cast_dynamic(group_resizable);
            }
          }
        }
      }
    }
  }

  return result;
}

//static:
Glib::RefPtr<const Goocanvas::Item> CanvasItemMovable::cast_const_to_item(const Glib::RefPtr<const CanvasItemMovable>& item)
{
  Glib::RefPtr<CanvasItemMovable> unconst = Glib::RefPtr<CanvasItemMovable>::cast_const(item);
  return cast_to_item(unconst);
}

void CanvasItemMovable::set_selected(bool selected)
{
  m_selected = selected;
  show_selected(); //Let derived classes indicate it visually,
}

bool CanvasItemMovable::get_selected() const
{
  return m_selected;
}

void CanvasItemMovable::show_selected()
{
  //Derived classes should override this.
}


} //namespace Glom

