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

#include "canvas_group_resizable.h"
#include "canvas_rect_movable.h"
#include "canvas_line_movable.h"
#include <libgoocanvasmm/canvas.h>
#include <goocanvasrect.h>
#include <goocanvasgroup.h>
#include <iostream>

namespace Glom
{

const double manipulator_corner_size = 10;
const gchar* manipulator_corner_fill_color = "black";
const double manipulator_stroke_width = 2.0;
const gchar* manipulator_stroke_color = "black";

CanvasGroupResizable::CanvasGroupResizable()
: Goocanvas::Group((GooCanvasGroup*)goo_canvas_group_new(NULL, NULL)) //TODO: Remove this when goocanvas has been fixed.
{
  m_manipulator_corner_top_left = create_corner();
  m_manipulator_corner_top_right = create_corner();
  m_manipulator_corner_bottom_left = create_corner();
  m_manipulator_corner_bottom_right = create_corner();

  m_manipulator_edge_top = create_edge();
  m_manipulator_edge_bottom = create_edge();
  m_manipulator_edge_left = create_edge();
  m_manipulator_edge_right = create_edge();

  set_drag_cursor(Gdk::FLEUR);

  signal_enter_notify_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_enter_notify_event));
  signal_leave_notify_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_leave_notify_event));
}

CanvasGroupResizable::~CanvasGroupResizable()
{
}

Glib::RefPtr<CanvasGroupResizable> CanvasGroupResizable::create()
{
  return Glib::RefPtr<CanvasGroupResizable>(new CanvasGroupResizable());
}

void CanvasGroupResizable::position_manipulators()
{
  //Note that this only works after the child has been added to the canvas:
  //Goocanvas::Bounds bounds;
  //m_child->get_bounds(bounds);
 
  const double x1 = m_child->property_x();
  const double y1 = m_child->property_y();
  const double x2 = m_child->property_x() + m_child->property_width();
  const double y2 = m_child->property_y() + m_child->property_height();

  m_manipulator_corner_top_left->property_x() = x1;
  m_manipulator_corner_top_left->property_y() = y1;
  m_manipulator_corner_top_left->raise(m_child);

  m_manipulator_corner_top_right->property_x() = x2 - manipulator_corner_size;
  m_manipulator_corner_top_right->property_y() = y1;
  m_manipulator_corner_top_right->raise(m_child);

  m_manipulator_corner_bottom_left->property_x() = x1;
  m_manipulator_corner_bottom_left->property_y() = y2 - manipulator_corner_size;
  m_manipulator_corner_bottom_left->raise(m_child);

  m_manipulator_corner_bottom_right->property_x() = x2 - manipulator_corner_size;
  m_manipulator_corner_bottom_right->property_y() = y2 - manipulator_corner_size;
  m_manipulator_corner_bottom_right->raise(m_child);

  set_edge_points(m_manipulator_edge_top, x1, y1, x2, y1);
  m_manipulator_edge_top->raise(m_child);
  set_edge_points(m_manipulator_edge_bottom, x1, y2, x2, y2);
  m_manipulator_edge_bottom->raise(m_child);
  set_edge_points(m_manipulator_edge_left, x1, y1, x1, y2);
  m_manipulator_edge_left->raise(m_child);
  set_edge_points(m_manipulator_edge_right, x2, y1, x2, y2);
  m_manipulator_edge_right->raise(m_child);
}

void CanvasGroupResizable::set_child(const Glib::RefPtr<Goocanvas::Rect>& child)
{
  if(!child)
    return;

  m_child = child;
  add_child(child);

  //Allow drag to move:
  m_child->signal_motion_notify_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_child_motion_notify_event));
  m_child->signal_button_press_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_child_button_press_event));
  m_child->signal_button_release_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_child_button_release_event));

  //Manipulators:
  add_child(m_manipulator_corner_top_left);
  add_child(m_manipulator_corner_top_right);
  add_child(m_manipulator_corner_bottom_left);
  add_child(m_manipulator_corner_bottom_right);
  add_child(m_manipulator_edge_top);
  add_child(m_manipulator_edge_bottom);
  add_child(m_manipulator_edge_left);
  add_child(m_manipulator_edge_right);

  m_manipulator_corner_top_left->set_grid(m_grid);
  m_manipulator_corner_top_left->set_snap_corner(CanvasRectMovable::CORNER_TOP_LEFT);
  m_manipulator_corner_top_right->set_grid(m_grid);
  m_manipulator_corner_top_right->set_snap_corner(CanvasRectMovable::CORNER_TOP_RIGHT);
  m_manipulator_corner_bottom_left->set_grid(m_grid);
  m_manipulator_corner_bottom_left->set_snap_corner(CanvasRectMovable::CORNER_BOTTOM_LEFT);
  m_manipulator_corner_bottom_right->set_grid(m_grid);
  m_manipulator_corner_bottom_right->set_snap_corner(CanvasRectMovable::CORNER_BOTTOM_RIGHT);
  m_manipulator_edge_top->set_grid(m_grid);
  m_manipulator_edge_bottom->set_grid(m_grid);
  m_manipulator_edge_left->set_grid(m_grid);
  m_manipulator_edge_right->set_grid(m_grid);

  m_manipulator_corner_top_left->set_drag_cursor(Gdk::TOP_LEFT_CORNER);
  m_manipulator_corner_top_right->set_drag_cursor(Gdk::TOP_RIGHT_CORNER);
  m_manipulator_corner_bottom_left->set_drag_cursor(Gdk::BOTTOM_LEFT_CORNER);
  m_manipulator_corner_bottom_right->set_drag_cursor(Gdk::BOTTOM_RIGHT_CORNER);
  m_manipulator_edge_top->set_drag_cursor(Gdk::TOP_SIDE);
  m_manipulator_edge_bottom->set_drag_cursor(Gdk::BOTTOM_SIDE);
  m_manipulator_edge_left->set_drag_cursor(Gdk::LEFT_SIDE);
  m_manipulator_edge_right->set_drag_cursor(Gdk::RIGHT_SIDE);

  manipulator_connect_signals(m_manipulator_corner_top_left, MANIPULATOR_CORNER_TOP_LEFT);
  manipulator_connect_signals(m_manipulator_corner_top_right, MANIPULATOR_CORNER_TOP_RIGHT);
  manipulator_connect_signals(m_manipulator_corner_bottom_left, MANIPULATOR_CORNER_BOTTOM_LEFT);
  manipulator_connect_signals(m_manipulator_corner_bottom_right, MANIPULATOR_CORNER_BOTTOM_RIGHT);
  manipulator_connect_signals(m_manipulator_edge_top, MANIPULATOR_EDGE_TOP);
  manipulator_connect_signals(m_manipulator_edge_bottom, MANIPULATOR_EDGE_BOTTOM);
  manipulator_connect_signals(m_manipulator_edge_left, MANIPULATOR_EDGE_LEFT);
  manipulator_connect_signals(m_manipulator_edge_right, MANIPULATOR_EDGE_RIGHT);

  position_manipulators();

  set_manipulators_visibility(Goocanvas::CANVAS_ITEM_INVISIBLE);
}

void CanvasGroupResizable::manipulator_connect_signals(const Glib::RefPtr<Goocanvas::Item> manipulator, Manipulators manipulator_id)
{
  //Respond when the corner rectangles move (they implement their own dragging):

  //TODO: Use x and y property notification.

  Glib::RefPtr<CanvasRectMovable> rect = Glib::RefPtr<CanvasRectMovable>::cast_dynamic(manipulator);
  if(rect)
  {
    rect->signal_moved().connect(
      sigc::bind( sigc::mem_fun(*this, &CanvasGroupResizable::on_manipulator_corner_moved), rect, manipulator_id) );
  }
  else
  {
    Glib::RefPtr<CanvasLineMovable> line = Glib::RefPtr<CanvasLineMovable>::cast_dynamic(manipulator);
    if(line)
    {
      line->signal_moved().connect(
        sigc::bind( sigc::mem_fun(*this, &CanvasGroupResizable::on_manipulator_edge_moved), line, manipulator_id) );
    }
  }

  //manipulator->property_x().signal_changed().connect(
  //  sigc::bind( sigc::mem_fun(*this, &CanvasGroupResizable::on_manipulator_moved), manipulator_id) );

 // manipulator->property_y().signal_changed().connect(
  //  sigc::bind( sigc::mem_fun(*this, &CanvasGroupResizable::on_manipulator_moved), manipulator_id) );


  //manipulator->signal_button_press_event().connect(
  //  sigc::bind( sigc::mem_fun(*this, &CanvasGroupResizable::on_manipulator_button_press_event), manipulator_id) );

  //manipulator->signal_motion_notify_event().connect(
  //  sigc::bind( sigc::mem_fun(*this, &CanvasGroupResizable::on_manipulator_motion_notify_event), manipulator_id) );

  //manipulator->signal_button_release_event().connect(
  //  sigc::bind( sigc::mem_fun(*this, &CanvasGroupResizable::on_manipulator_button_release_event), manipulator_id) );
}

void CanvasGroupResizable::on_manipulator_corner_moved(const Glib::RefPtr<CanvasRectMovable>& manipulator, Manipulators manipulator_id)
{
  //std::cout << "CanvasGroupResizable::on_manipulator_corner_moved(): manipulator=" << manipulator_id << std::endl;

  switch(manipulator_id)
  {
    case(MANIPULATOR_CORNER_TOP_LEFT):
    {
      const double new_x = std::min(manipulator->property_x().get_value(), m_child->property_x() + m_child->property_width());
      const double new_y = std::min(manipulator->property_y().get_value(), m_child->property_y() + m_child->property_height());
      const double new_height = std::max(m_child->property_y() + m_child->property_height() - manipulator->property_y(), 0.0);
      const double new_width = std::max(m_child->property_x() + m_child->property_width() - manipulator->property_x(), 0.0);

      m_child->property_x() = new_x;
      m_child->property_y() = new_y;
      m_child->property_width() = new_width;
      m_child->property_height() = new_height;

      break;
    }
    case(MANIPULATOR_CORNER_TOP_RIGHT):
    {
      const double new_y = std::min(manipulator->property_y().get_value(), m_child->property_y() + m_child->property_height());
      const double new_height = std::max(m_child->property_y() + m_child->property_height() - manipulator->property_y(), 0.0);
      const double new_width = std::max(manipulator->property_x() + manipulator_corner_size - m_child->property_x(), 0.0);

      m_child->property_y() = new_y;
      m_child->property_width() = new_width;
      m_child->property_height() = new_height;

      break;
    }
    case(MANIPULATOR_CORNER_BOTTOM_LEFT):
    {
      const double new_x = std::min(manipulator->property_x().get_value(), m_child->property_x() + m_child->property_width());
      const double new_height = std::max(manipulator->property_y() + manipulator_corner_size - m_child->property_y(), 0.0);
      const double new_width = std::max(m_child->property_x() + m_child->property_width() - manipulator->property_x(), 0.0);

      m_child->property_x() = new_x;
      m_child->property_width() = new_width;
      m_child->property_height() = new_height;

      break;
    }
    case(MANIPULATOR_CORNER_BOTTOM_RIGHT):
    {
      const double new_height = std::max(manipulator->property_y() + manipulator_corner_size - m_child->property_y(), 0.0);
      const double new_width = std::max(manipulator->property_x() + manipulator_corner_size - m_child->property_x(), 0.0);

      m_child->property_width() = new_width;
      m_child->property_height() = new_height;

      break;
    }
    default:
      break;
  }

  position_manipulators();
}

void CanvasGroupResizable::on_manipulator_edge_moved(const Glib::RefPtr<CanvasLineMovable>& manipulator, Manipulators manipulator_id)
{
  //std::cout << "CanvasGroupResizable::on_manipulator_edge_moved(): manipulator=" << manipulator_id << std::endl;

  Goocanvas::Points points = manipulator->property_points();
  double x1 = 0;
  double y1 = 0;
  points.get_coordinate(0, x1, y1);
  double x2 = 0;
  double y2 = 0;
  points.get_coordinate(1, x2, y2);
  
  switch(manipulator_id)
  {
    case(MANIPULATOR_EDGE_TOP):
    {
      const double new_y = y1;
      const double new_height = std::max(m_child->property_y() + m_child->property_height() - y1, 0.0);

      m_child->property_y() = new_y;
      m_child->property_height() = new_height;

      break;
    }

    case(MANIPULATOR_EDGE_BOTTOM):
    {
      const double new_height = std::max(y1 - m_child->property_y(), 0.0);

      m_child->property_height() = new_height;

      break;
    }
    case(MANIPULATOR_EDGE_LEFT):
    {
      const double new_x = x1;
      const double new_width = std::max(m_child->property_x() + m_child->property_width() - x1, 0.0);

      m_child->property_x() = new_x;
      m_child->property_width() = new_width;

      break;
    }
    case(MANIPULATOR_EDGE_RIGHT):
    {
      const double new_width = std::max(x1 - m_child->property_x(), 0.0);

      m_child->property_width() = new_width;

      break;
    }
    default:
      break;
  }

  position_manipulators();
}


bool CanvasGroupResizable::on_child_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event)
{
  return CanvasItemMovable::on_button_press_event(target, event);
}

bool CanvasGroupResizable::on_child_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event)
{ 
  //std::cout << "CanvasGroupResizable::on_motion_notify_event()" << std::endl;

  CanvasItemMovable::on_motion_notify_event(target, event);
  
  position_manipulators();

  return true;
}

bool CanvasGroupResizable::on_child_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event)
{
  return CanvasItemMovable::on_button_release_event(target, event);
}



/*
bool CanvasGroupResizable::on_manipulator_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event, Manipulators manipulator)
{
  return true;
}

bool CanvasGroupResizable::on_manipulator_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event, Manipulators manipulator)
{
  return true;
}

bool CanvasGroupResizable::on_manipulator_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event, Manipulators manipulator)
{
  return true;
}
*/

void CanvasGroupResizable::set_manipulators_visibility(Goocanvas::ItemVisibility visibility)
{
  //TODO: Reenable this when we figure out how to know that we are still in the main box even when we are in resizing corner too.
  visibility = Goocanvas::CANVAS_ITEM_VISIBLE;

  m_manipulator_corner_top_left->property_visibility() = visibility;
  m_manipulator_corner_bottom_left->property_visibility() = visibility;
  m_manipulator_corner_top_right->property_visibility() = visibility;
  m_manipulator_corner_bottom_right->property_visibility() = visibility;
}

bool CanvasGroupResizable::on_enter_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event)
{
  //std::cout << "CanvasGroupResizable::on_enter_notify_event" << std::endl;
  CanvasItemMovable::on_enter_notify_event(target, event);

  set_manipulators_visibility(Goocanvas::CANVAS_ITEM_VISIBLE);

  return true;
}

bool CanvasGroupResizable::on_leave_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event)
{
  //std::cout << "CanvasGroupResizable::on_leave_notify_event" << std::endl;
  CanvasItemMovable::on_leave_notify_event(target, event);

  set_manipulators_visibility(Goocanvas::CANVAS_ITEM_INVISIBLE);

  return true;
}


Glib::RefPtr<CanvasRectMovable> CanvasGroupResizable::create_corner()
{
  Glib::RefPtr<CanvasRectMovable> result = CanvasRectMovable::create();
  result->property_fill_color() = manipulator_corner_fill_color; //This makes the whole area clickable, not just the outline stroke:
  result->property_line_width() = manipulator_stroke_width;
  result->property_stroke_color() = manipulator_stroke_color;

  result->property_height() = manipulator_corner_size;
  result->property_width() = manipulator_corner_size;
 
  return result;
}

Glib::RefPtr<CanvasLineMovable> CanvasGroupResizable::create_edge()
{
  Glib::RefPtr<Glom::CanvasLineMovable> line = Glom::CanvasLineMovable::create();
  line->property_line_width() = manipulator_stroke_width;
  line->property_stroke_color() = manipulator_stroke_color;

  return line;
}

void CanvasGroupResizable::set_edge_points(const Glib::RefPtr<Glom::CanvasLineMovable>& line, double x1, double y1, double x2, double y2)
{
  double points_coordinates[] = {x1, y1, x2, y2};
  Goocanvas::Points points(2, points_coordinates);
  line->property_points() = points;
}

void CanvasGroupResizable::get_xy(double& x, double& y)
{
  x = m_child->property_x();
  y = m_child->property_y();
}

void CanvasGroupResizable::move(double x, double y)
{
  m_child->property_x() = x;
  m_child->property_y() = y;

  position_manipulators();
}

void CanvasGroupResizable::snap_position(double& x, double& y) const
{
  double offset_x_min = 0;
  double offset_y_min = 0;

  //Try snapping each corner, to choose the one that snapped closest:
  for(int i = 0; i < CORNER_COUNT; ++i)
  {
    const Corners corner = (Corners)i;
    double temp_x = x;
    double temp_y = y;
    snap_position(corner, temp_x, temp_y);

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

void CanvasGroupResizable::snap_position(Corners corner, double& x, double& y) const
{
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
      corner_x_offset = m_child->property_width();
      corner_y_offset = 0;
      break;
    case CORNER_BOTTOM_LEFT:
      corner_x_offset = 0;
      corner_y_offset = m_child->property_height();
      break;
    case CORNER_BOTTOM_RIGHT:
      corner_x_offset = m_child->property_width();
      corner_y_offset = m_child->property_height();
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

Goocanvas::Canvas* CanvasGroupResizable::get_parent_canvas_widget()
{
  return get_canvas();
}

} //namespace Glom

