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

#include "canvas_group_resizable.h"
#include "canvas_rect_movable.h"
#include "canvas_line_movable.h"
#include <glom/utility_widgets/canvas/canvas_table_movable.h>
#include <libglom/utils.h>
#include <goocanvasmm/canvas.h>
#include <goocanvasrect.h>
#include <goocanvasgroup.h>
#include <iostream>

namespace Glom
{

static const double MANIPULATOR_CORNER_SIZE = 2;
static const char MANIPULATOR_CORNER_FILL_COLOR[] = "black";
static const double MANIPULATOR_STROKE_WIDTH = 0.5f; //mm (assuming that the canvas uses mm.
static const char MANIPULATOR_STROKE_COLOR[] = "black";
static const double OUTLINE_STROKE_WIDTH = MANIPULATOR_STROKE_WIDTH; //mm (assuming that the canvas uses mm.
static const char OUTLINE_STROKE_COLOR[] = "gray";

void CanvasGroupResizable::get_outline_stroke(Glib::ustring& color, double& width)
{
  color = OUTLINE_STROKE_COLOR;
  width = OUTLINE_STROKE_WIDTH;
}

CanvasGroupResizable::CanvasGroupResizable()
: m_in_manipulator(false),
  m_x(0), m_y(0), m_width(0), m_height(0)
{
  //property_pointer_events() =
  //    (Goocanvas::PointerEvents)(Goocanvas::EVENTS_VISIBLE_FILL & GOO_CANVAS_EVENTS_VISIBLE_STROKE);


  set_drag_cursor(Gdk::CursorType::FLEUR);
}

void CanvasGroupResizable::create_manipulators()
{
  //Remove any existing manipulators:
  if(m_group_edge_manipulators)
    m_group_edge_manipulators->remove();

  if(m_group_corner_manipulators)
    m_group_corner_manipulators->remove();

  m_group_edge_manipulators = Goocanvas::Group::create();
  add_child(m_group_edge_manipulators);

  m_group_corner_manipulators = Goocanvas::Group::create();
  add_child(m_group_corner_manipulators);

  if(get_is_line())
    create_line_manipulators();
  else
    create_rect_manipulators();
}

void CanvasGroupResizable::create_rect_manipulators()
{
  m_rect = Goocanvas::Rect::create(0, 0, 0, 0);
  m_rect->property_line_width() = 0;
  m_rect->property_fill_color_rgba() = 0xFFFFFF00; //Needed to make it react to drags. White but completely transparent.
  add_child(m_rect);

  //Allow dragging of the rect to move everything:
  m_rect->signal_motion_notify_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_child_motion_notify_event));
  m_rect->signal_button_press_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_child_button_press_event));
  m_rect->signal_button_release_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_child_button_release_event));

  //m_rect->property_pointer_events() =
  //    (Goocanvas::PointerEvents)(Goocanvas::EVENTS_VISIBLE_FILL & GOO_CANVAS_EVENTS_VISIBLE_STROKE);

  m_rect->signal_enter_notify_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_rect_enter_notify_event), false);
  m_rect->signal_leave_notify_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_rect_leave_notify_event), false);


  m_manipulator_corner_top_left = create_corner_manipulator();
  m_manipulator_corner_top_right = create_corner_manipulator();
  m_manipulator_corner_bottom_left = create_corner_manipulator();
  m_manipulator_corner_bottom_right = create_corner_manipulator();

  m_manipulator_edge_top = create_edge_manipulator();
  m_manipulator_edge_bottom = create_edge_manipulator();
  m_manipulator_edge_left = create_edge_manipulator();
  m_manipulator_edge_right = create_edge_manipulator();

  m_group_corner_manipulators->add_child(m_manipulator_corner_top_left);
  m_group_corner_manipulators->add_child(m_manipulator_corner_top_right);
  m_group_corner_manipulators->add_child(m_manipulator_corner_bottom_left);
  m_group_corner_manipulators->add_child(m_manipulator_corner_bottom_right);
  m_group_edge_manipulators->add_child(m_manipulator_edge_top);
  m_group_edge_manipulators->add_child(m_manipulator_edge_bottom);
  m_group_edge_manipulators->add_child(m_manipulator_edge_left);
  m_group_edge_manipulators->add_child(m_manipulator_edge_right);

  m_manipulator_corner_top_left->set_grid(m_grid);
  m_manipulator_corner_top_left->set_snap_corner(CanvasRectMovable::Corners::TOP_LEFT);
  m_manipulator_corner_top_right->set_grid(m_grid);
  m_manipulator_corner_top_right->set_snap_corner(CanvasRectMovable::Corners::TOP_RIGHT);
  m_manipulator_corner_bottom_left->set_grid(m_grid);
  m_manipulator_corner_bottom_left->set_snap_corner(CanvasRectMovable::Corners::BOTTOM_LEFT);
  m_manipulator_corner_bottom_right->set_grid(m_grid);
  m_manipulator_corner_bottom_right->set_snap_corner(CanvasRectMovable::Corners::BOTTOM_RIGHT);
  m_manipulator_edge_top->set_grid(m_grid);
  m_manipulator_edge_bottom->set_grid(m_grid);
  m_manipulator_edge_left->set_grid(m_grid);
  m_manipulator_edge_right->set_grid(m_grid);

  m_manipulator_corner_top_left->set_drag_cursor(Gdk::CursorType::TOP_LEFT_CORNER);
  m_manipulator_corner_top_right->set_drag_cursor(Gdk::CursorType::TOP_RIGHT_CORNER);
  m_manipulator_corner_bottom_left->set_drag_cursor(Gdk::CursorType::BOTTOM_LEFT_CORNER);
  m_manipulator_corner_bottom_right->set_drag_cursor(Gdk::CursorType::BOTTOM_RIGHT_CORNER);
  m_manipulator_edge_top->set_drag_cursor(Gdk::CursorType::TOP_SIDE);
  m_manipulator_edge_bottom->set_drag_cursor(Gdk::CursorType::BOTTOM_SIDE);
  m_manipulator_edge_left->set_drag_cursor(Gdk::CursorType::LEFT_SIDE);
  m_manipulator_edge_right->set_drag_cursor(Gdk::CursorType::RIGHT_SIDE);

  //Make sure that this is above the outline group:
  m_group_edge_manipulators->raise();//m_group_outline);
  m_group_corner_manipulators->raise();//m_group_outline);

  manipulator_connect_signals(m_manipulator_corner_top_left, Manipulators::CORNER_TOP_LEFT);
  manipulator_connect_signals(m_manipulator_corner_top_right, Manipulators::CORNER_TOP_RIGHT);
  manipulator_connect_signals(m_manipulator_corner_bottom_left, Manipulators::CORNER_BOTTOM_LEFT);
  manipulator_connect_signals(m_manipulator_corner_bottom_right, Manipulators::CORNER_BOTTOM_RIGHT);
  manipulator_connect_signals(m_manipulator_edge_top, Manipulators::EDGE_TOP);
  manipulator_connect_signals(m_manipulator_edge_bottom, Manipulators::EDGE_BOTTOM);
  manipulator_connect_signals(m_manipulator_edge_left, Manipulators::EDGE_LEFT);
  manipulator_connect_signals(m_manipulator_edge_right, Manipulators::EDGE_RIGHT);
}

void CanvasGroupResizable::create_line_manipulators()
{
  m_manipulator_start = create_corner_manipulator();
  m_manipulator_end = create_corner_manipulator();

  //We add these to the edge manipulators, though they look like
  //corner manipulators, because we want to use them to show selection.
  m_group_edge_manipulators->add_child(m_manipulator_start);
  m_group_edge_manipulators->add_child(m_manipulator_end);

  m_manipulator_start->set_grid(m_grid);
  //m_manipulator_corner_top_left->set_snap_corner(CanvasRectMovable::Corners::TOP_LEFT);
  m_manipulator_end->set_grid(m_grid);
  //m_manipulator_corner_top_right->set_snap_corner(CanvasRectMovable::Corners::TOP_RIGHT);

  m_manipulator_start->set_drag_cursor(Gdk::CursorType::TCROSS); //A rather arbitrary cursor.
  m_manipulator_end->set_drag_cursor(Gdk::CursorType::TCROSS);

  manipulator_connect_signals(m_manipulator_start, Manipulators::START);
  manipulator_connect_signals(m_manipulator_end, Manipulators::END);
}

Glib::RefPtr<CanvasLineMovable> CanvasGroupResizable::create_outline_line(double x1, double y1, double x2, double y2)
{
  auto line = Glom::CanvasLineMovable::create();
  line->property_line_width() = OUTLINE_STROKE_WIDTH;
  line->property_stroke_color() = OUTLINE_STROKE_COLOR;
  line->set_movement_allowed(false, false);
  m_group_outline->add_child(line);
  set_edge_points(line, x1, y1, x2, y2);
  return line;
}

void CanvasGroupResizable::create_outline_group()
{
  //Add something to indicate when the item is selected:
  if(m_group_outline)
    m_group_outline->remove();
  m_group_outline = Goocanvas::Group::create();
  add_child(m_group_outline);

  double x1 = 0;
  double y1 = 0;
  get_xy(x1, y1);

  double child_width = 0;
  double child_height = 0;
  get_width_height(child_width, child_height);

  const double x2 = x1 + child_width;
  const double y2 = y1 + child_height;

  m_outline_top = create_outline_line(x1, y1, x2, y1);
  m_outline_bottom = create_outline_line(x1, y2, x2, y2);
  m_outline_left = create_outline_line(x1, y1, x1, y2);
  m_outline_right = create_outline_line(x2, y1, x2, y2);
}

void CanvasGroupResizable::set_outline_visible(bool visible)
{
  if(!m_group_outline)
  {
    std::cerr << G_STRFUNC << ": m_group_outline was null.\n";
    return;
  }

  m_group_outline->property_visibility() =
    (visible ? Goocanvas::ItemVisibility::VISIBLE : Goocanvas::ItemVisibility::INVISIBLE);
}

Glib::RefPtr<CanvasGroupResizable> CanvasGroupResizable::create()
{
  return Glib::RefPtr<CanvasGroupResizable>(new CanvasGroupResizable());
}

void CanvasGroupResizable::position_extras()
{
  position_manipulators();
  position_outline();
}

void CanvasGroupResizable::position_manipulators()
{
  if(get_is_line())
    position_line_manipulators();
  else
    position_rect_manipulators();
}

void CanvasGroupResizable::position_rect_manipulators()
{
  if(!m_rect)
    return;

  //Note that this only works after the child has been added to the canvas:
  //Goocanvas::Bounds bounds;
  //m_child->get_bounds(bounds);

  double x1 = 0;
  double y1 = 0;
  get_xy(x1, y1);

  double child_x = 0;
  double child_y = 0;
  get_xy(child_x, child_y); //TODO: Remove duplicate get_xy() call?
  //std::cout << "debug: " << G_STRFUNC << ": child x=" << child_x << std::endl;

  double child_width = 0;
  double child_height = 0;
  get_width_height(child_width, child_height);
  //std::cout << "debug: " << G_STRFUNC << ": child width=" << child_width << std::endl;


  //Show the size of this item (not always the same as the child size):
  m_rect->property_x() = child_x;
  m_rect->property_y() = child_y;
  m_rect->property_width() = child_width;
  m_rect->property_height() = child_height;
  //m_rect->property_fill_color_rgba() = 0xFFFFFF00;

  const double x2 = child_x + child_width;
  const double y2 = child_y + child_height;

  auto item = CanvasItemMovable::cast_to_item(m_child);

  m_manipulator_corner_top_left->set_xy(x1, y1);
  m_manipulator_corner_top_right->set_xy(x2 - MANIPULATOR_CORNER_SIZE, y1);
  m_manipulator_corner_bottom_left->set_xy(x1, y2 - MANIPULATOR_CORNER_SIZE);
  m_manipulator_corner_bottom_right->set_xy(x2 - MANIPULATOR_CORNER_SIZE, y2 - MANIPULATOR_CORNER_SIZE);
  set_edge_points(m_manipulator_edge_top, x1, y1, x2, y1);
  set_edge_points(m_manipulator_edge_bottom, x1, y2, x2, y2);
  set_edge_points(m_manipulator_edge_left, x1, y1, x1, y2);
  set_edge_points(m_manipulator_edge_right, x2, y1, x2, y2);

  //Make sure that the bounds rect is below the item,
  //and the manipulators are above the item (and above the rect):
  if(item)
  {
    m_group_edge_manipulators->raise();
    m_group_corner_manipulators->raise();
    m_rect->lower(item);
  }
  else
  {
    m_group_edge_manipulators->raise();
    m_group_corner_manipulators->raise();
  }
}

void CanvasGroupResizable::position_line_manipulators()
{
  auto line = std::dynamic_pointer_cast<CanvasLineMovable>(m_child);
  if(!line)
    return;

  const Goocanvas::Points points = line->property_points();
  if(points.get_num_points() < 2)
    return;

  double start_x = 0;
  double start_y = 0;
  points.get_coordinate(0, start_x, start_y);

  const double half_size = MANIPULATOR_CORNER_SIZE / 2;
  m_manipulator_start->set_xy(start_x - half_size, start_y - half_size); //Center it over the point.

  double end_x = 0;
  double end_y = 0;
  points.get_coordinate(1, end_x, end_y);

  m_manipulator_end->set_xy(end_x - half_size, end_y - half_size); //Center it over the point.

  m_group_edge_manipulators->raise();
  m_group_corner_manipulators->raise();
}

void CanvasGroupResizable::position_outline()
{
  if(!m_rect)
    return;

  //Note that this only works after the child has been added to the canvas:
  //Goocanvas::Bounds bounds;
  //m_child->get_bounds(bounds);

  double x1 = 0;
  double y1 = 0;
  get_xy(x1, y1);

  double child_width = 0;
  double child_height = 0;
  get_width_height(child_width, child_height);
  //std::cout << "debug: " << G_STRFUNC << ": child width=" << child_width << std::endl;

  const double x2 = x1 + child_width;
  const double y2 = y1 + child_height;

  auto item = CanvasItemMovable::cast_to_item(m_child);

  set_edge_points(m_outline_top, x1, y1, x2, y1);
  set_edge_points(m_outline_bottom, x1, y2, x2, y2);
  set_edge_points(m_outline_left, x1, y1, x1, y2);
  set_edge_points(m_outline_right, x2, y1, x2, y2);

  /*
  //Make sure that the bounds rect is below the item,
  //and the manipulators are above the item (and above the rect):
  if(item)
  {
    m_group_edge_manipulators->raise(item);
    m_group_corner_manipulators->raise(item);
    m_rect->lower(item);
  }
  else
  {
    m_group_edge_manipulators->raise(m_rect);
    m_group_corner_manipulators->raise(m_rect);
  }
  */
}

void CanvasGroupResizable::set_child(const Glib::RefPtr<CanvasItemMovable>& child)
{
  //Remove the previous child, if any:
  if(m_child)
  {
    auto item = CanvasItemMovable::cast_to_item(m_child);
    item->remove();
  }

  //std::cout << "DEBUG: CanvasGroupResizable::set_child() start\n";
  if(!child)
    return;

  auto item = CanvasItemMovable::cast_to_item(child);
  if(!item)
    return;

  //Get any previously-set position
  //(after we set m_child, this would get the position from the child.)
  double x = 0;
  double y = 0;
  get_xy(x, y);
  double width = 0;
  double height = 0;
  get_width_height(width, height);

  m_child = child;
  add_child(item);

  //Do not use the child's own movable behaviour, because we want everything to move together:
  child->set_movement_allowed(false, false);

  //Allow drag to move:
  item->signal_motion_notify_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_child_motion_notify_event));
  item->signal_button_press_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_child_button_press_event));
  item->signal_button_release_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_child_button_release_event));

  item->signal_enter_notify_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_resizer_enter_notify_event), false);
  item->signal_leave_notify_event().connect(sigc::mem_fun(*this, &CanvasGroupResizable::on_resizer_leave_notify_event), false);

  create_manipulators(); //Potentially changing the type of manipulators used, if the item is of a different type.

  create_outline_group();

  //Set the child's position to match this parent resizable's position, if any was set:
  //(Note that the resizable should have its position set to that of the child,
  //if that is the position that is wanted.
  set_xy(x, y);
  set_width_height(width, height);

  position_extras();
  set_manipulators_visibility(Goocanvas::ItemVisibility::INVISIBLE);
  set_outline_visible(false);
}

Glib::RefPtr<CanvasItemMovable> CanvasGroupResizable::get_child()
{
  return m_child;
}


Glib::RefPtr<const CanvasItemMovable> CanvasGroupResizable::get_child() const
{
  return m_child;
}

void CanvasGroupResizable::manipulator_connect_signals(const Glib::RefPtr<Goocanvas::Item>& manipulator, Manipulators manipulator_id)
{
  //Respond when the corner rectangles move (they implement their own dragging):

  //TODO: Use x and y property notification.

  auto rect = std::dynamic_pointer_cast<CanvasRectMovable>(manipulator);
  if(rect)
  {
    if(get_is_line())
    {
      rect->signal_moved().connect(
        sigc::bind( sigc::mem_fun(*this, &CanvasGroupResizable::on_manipulator_line_end_moved), manipulator_id) );
    }
    else
    {
      rect->signal_moved().connect(
        sigc::bind( sigc::mem_fun(*this, &CanvasGroupResizable::on_manipulator_corner_moved), manipulator_id) );
    }
  }
  else
  {
    auto line = std::dynamic_pointer_cast<CanvasLineMovable>(manipulator);
    if(line)
    {
      line->signal_moved().connect(
        sigc::bind( sigc::mem_fun(*this, &CanvasGroupResizable::on_manipulator_edge_moved), manipulator_id) );
    }
  }

  manipulator->signal_enter_notify_event().connect(
     sigc::mem_fun(*this, &CanvasGroupResizable::on_manipulator_enter_notify_event), false);
  manipulator->signal_leave_notify_event().connect(
     sigc::mem_fun(*this, &CanvasGroupResizable::on_manipulator_leave_notify_event), false);


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

Glib::RefPtr<CanvasItemMovable> CanvasGroupResizable::get_manipulator(Manipulators manipulator_id)
{
  switch(manipulator_id)
  {
    //Rectangle manipulators:
    case(Manipulators::CORNER_TOP_LEFT):
      return m_manipulator_corner_top_left;
    case(Manipulators::CORNER_TOP_RIGHT):
      return m_manipulator_corner_top_right;
    case(Manipulators::CORNER_BOTTOM_LEFT):
      return m_manipulator_corner_bottom_left;
    case(Manipulators::CORNER_BOTTOM_RIGHT):
      return m_manipulator_corner_bottom_right;
    case(Manipulators::EDGE_TOP):
      return m_manipulator_edge_top;
    case(Manipulators::EDGE_BOTTOM):
      return m_manipulator_edge_bottom;
    case(Manipulators::EDGE_LEFT):
      return m_manipulator_edge_left;
    case(Manipulators::EDGE_RIGHT):
      return m_manipulator_edge_right;

    //Line manipulators:
    case(Manipulators::START):
      return m_manipulator_start;
    case(Manipulators::END):
      return m_manipulator_end;
    default:
      return Glib::RefPtr<CanvasItemMovable>();
  }
}

void CanvasGroupResizable::on_manipulator_corner_moved(const Glib::RefPtr<CanvasItemMovable>& /* item */, double /* x_offset */, double /* y_offset */, Manipulators manipulator_id)
{
  //Make sure that the manipulator is still visibile.
  //(if the user moves too fast then we get a leave-notify-event on the manipulator, rect, or item):
  set_manipulators_visibility(Goocanvas::ItemVisibility::VISIBLE);

  auto manipulator_base = get_manipulator(manipulator_id);
  auto manipulator = std::dynamic_pointer_cast<CanvasRectMovable>(manipulator_base);

  if(!manipulator)
    return;

  double manipulator_x = 0;
  double manipulator_y = 0;
  manipulator->get_xy(manipulator_x, manipulator_y);

  double child_x = 0;
  double child_y = 0;
  get_xy(child_x, child_y);
  double child_width = 0;
  double child_height = 0;
  get_width_height(child_width, child_height);

  switch(manipulator_id)
  {
    case(Manipulators::CORNER_TOP_LEFT):
    {
      const double new_x = std::min(manipulator_x, child_x + child_width);
      const double new_y = std::min(manipulator_y, child_y + child_height);
      const double new_height = std::max(child_y + child_height - manipulator->property_y(), 0.0);
      const double new_width = std::max(child_x + child_width - manipulator->property_x(), 0.0);
      set_xy(new_x, new_y);
      set_width_height(new_width, new_height);

      break;
    }
    case(Manipulators::CORNER_TOP_RIGHT):
    {
      const double new_y = std::min(manipulator_y, child_y + child_height);
      const double new_height = std::max(child_y + child_height - manipulator->property_y(), 0.0);
      const double new_width = std::max(manipulator->property_x() + MANIPULATOR_CORNER_SIZE - child_x, 0.0);

      set_xy(child_x, new_y);
      set_width_height(new_width, new_height);

      break;
    }
    case(Manipulators::CORNER_BOTTOM_LEFT):
    {
      const double new_x = std::min(manipulator_x, child_x + child_width);
      const double new_height = std::max(manipulator->property_y() + MANIPULATOR_CORNER_SIZE - child_y, 0.0);
      const double new_width = std::max(child_x + child_width - manipulator->property_x(), 0.0);
      set_xy(new_x, child_y);
      set_width_height(new_width, new_height);

      break;
    }
    case(Manipulators::CORNER_BOTTOM_RIGHT):
    {
      const double new_height = std::max(manipulator->property_y() + MANIPULATOR_CORNER_SIZE - child_y, 0.0);
      const double new_width = std::max(manipulator->property_x() + MANIPULATOR_CORNER_SIZE - child_x, 0.0);
      set_width_height(new_width, new_height);

      break;
    }
    default:
      break;
  }

  position_extras();
  m_signal_resized.emit();
}

void CanvasGroupResizable::on_manipulator_line_end_moved(const Glib::RefPtr<CanvasItemMovable>& /* item */, double /* x_offset */, double /* y_offset */, Manipulators manipulator_id)
{
  //Make sure that the manipulator is still visibile.
  //(if the user moves too fast then we get a leave-notify-event on the manipulator, rect, or item):
  set_manipulators_visibility(Goocanvas::ItemVisibility::VISIBLE);

  auto manipulator_base = get_manipulator(manipulator_id);
  auto manipulator = std::dynamic_pointer_cast<CanvasRectMovable>(manipulator_base);

  if(!manipulator)
    return;


  auto line = std::dynamic_pointer_cast<CanvasLineMovable>(m_child);
  if(!line)
    return;



  double manipulator_x = 0;
  double manipulator_y = 0;
  manipulator->get_xy(manipulator_x, manipulator_y);

  Goocanvas::Points points = line->property_points();

  if(points.get_num_points() < 2)
    return;

  const int point_index = (manipulator_id == Manipulators::START) ? 0 : 1;
  const double half_size = MANIPULATOR_CORNER_SIZE / 2;
  points.set_coordinate(point_index, manipulator_x + half_size, manipulator_y + half_size);
  line->property_points() = points; //TODO: Add a way to do this without getting and setting the points property.

  position_extras();
}

bool CanvasGroupResizable::on_manipulator_enter_notify_event(const Glib::RefPtr<Goocanvas::Item>& /* target */, GdkEventCrossing* /* event */)
{
  m_in_manipulator = true;
  set_manipulators_visibility(Goocanvas::ItemVisibility::VISIBLE);
  return false;
}

bool CanvasGroupResizable::on_manipulator_leave_notify_event(const Glib::RefPtr<Goocanvas::Item>& /* target */, GdkEventCrossing* /* event */)
{
  m_in_manipulator = false;
  set_manipulators_visibility(Goocanvas::ItemVisibility::INVISIBLE);
  return false;
}

void CanvasGroupResizable::on_manipulator_edge_moved(const Glib::RefPtr<CanvasItemMovable>& /* item */, double /* x_offset */, double /* y_offset */, Manipulators manipulator_id)
{
  //Make sure that the manipulator is still visibile.
  //(if the user moves too fast then we get a leave-notify-event on the manipulator, rect, or item):
  set_manipulators_visibility(Goocanvas::ItemVisibility::VISIBLE);

  auto manipulator_base = get_manipulator(manipulator_id);
  auto manipulator = std::dynamic_pointer_cast<CanvasLineMovable>(manipulator_base);

  //std::cout << "debug: " << G_STRFUNC << ": manipulator=" << manipulator_id << std::endl;

  Goocanvas::Points points = manipulator->property_points();
  double x1 = 0;
  double y1 = 0;
  points.get_coordinate(0, x1, y1);
  double x2 = 0;
  double y2 = 0;
  points.get_coordinate(1, x2, y2);

  double child_x = 0;
  double child_y = 0;
  get_xy(child_x, child_y);
  double child_width = 0;
  double child_height = 0;
  get_width_height(child_width, child_height);

  switch(manipulator_id)
  {
    case(Manipulators::EDGE_TOP):
    {
      const double new_y = y1;
      const double new_height = std::max(child_y + child_height - y1, 0.0);

      set_xy(child_x, new_y);
      set_width_height(child_width, new_height);

      break;
    }

    case(Manipulators::EDGE_BOTTOM):
    {
      const double new_height = std::max(y1 - child_y, 0.0);

      set_width_height(child_width, new_height);

      break;
    }
    case(Manipulators::EDGE_LEFT):
    {
      const double new_x = x1;
      const double new_width = std::max(child_x + child_width - x1, 0.0);

      set_xy(new_x, child_y);
      set_width_height(new_width, child_height);

      break;
    }
    case(Manipulators::EDGE_RIGHT):
    {
      const double new_width = std::max(x1 - child_x, 0.0);

      set_width_height(new_width, child_height);

      break;
    }
    default:
      break;
  }

  position_extras();

  m_signal_resized.emit();
}


bool CanvasGroupResizable::on_child_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event)
{
  return CanvasItemMovable::on_button_press_event(target, event);
}

bool CanvasGroupResizable::on_child_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event)
{
  //std::cout << "CanvasGroupResizable::on_motion_notify_event()\n";

  const bool result = CanvasItemMovable::on_motion_notify_event(target, event);

  position_extras();

  return result;
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
  if(!m_group_edge_manipulators || !m_group_corner_manipulators)
    return;

  //Make sure that edges stays visible if the item is selected,
  //because that is how we show selection:
  Goocanvas::ItemVisibility edge_visibility = visibility;
  if(get_selected())
    edge_visibility = Goocanvas::ItemVisibility::VISIBLE;

  //For testing: visibility = Goocanvas::ItemVisibility::VISIBLE;
  m_group_edge_manipulators->property_visibility() = edge_visibility;
  m_group_corner_manipulators->property_visibility() = visibility;

  //Also show grid lines in the portal table,
  //though these are not actually manipulatable.
  auto table =
    std::dynamic_pointer_cast<CanvasTableMovable>(get_child());
  if(table)
  {
    if(visibility == Goocanvas::ItemVisibility::VISIBLE)
      table->set_lines_visibility();
    else
      table->set_lines_visibility(false);
  }

}

bool CanvasGroupResizable::on_rect_enter_notify_event(const Glib::RefPtr<Goocanvas::Item>& /* target */, GdkEventCrossing* /* event */)
{
  set_manipulators_visibility(Goocanvas::ItemVisibility::VISIBLE);

  return true;
}

bool CanvasGroupResizable::on_rect_leave_notify_event(const Glib::RefPtr<Goocanvas::Item>& /* target */, GdkEventCrossing* /* event */)
{
  //std::cout << "CanvasGroupResizable::on_rect_leave_notify_event\n";

  //Glib::RefPtr<CanvasItemMovable> target_movable = CanvasItemMovable::cast_to_movable(target);
  //Hide the manipulators if we are outside of the main area,
  //but not just because we are instead inside the manipulator itself:
  //Doesn't seem useful: if(!m_in_manipulator && (target_movable == m_child))
  set_manipulators_visibility(Goocanvas::ItemVisibility::INVISIBLE);


  return false;
}

bool CanvasGroupResizable::on_resizer_enter_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event)
{
  CanvasItemMovable::on_enter_notify_event(target, event);

  set_manipulators_visibility(Goocanvas::ItemVisibility::VISIBLE);

  return true;
}

bool CanvasGroupResizable::on_resizer_leave_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event)
{
  CanvasItemMovable::on_leave_notify_event(target, event);

  //Glib::RefPtr<CanvasItemMovable> target_movable = CanvasItemMovable::cast_to_movable(target);
  //Hide the manipulators if we are outside of the main area,
  //but not just because we are instead inside the manipulator itself:
  //Doesn't seem useful: if(!m_in_manipulator && (target_movable == m_child))
  set_manipulators_visibility(Goocanvas::ItemVisibility::INVISIBLE);

  return false;
}


Glib::RefPtr<CanvasRectMovable> CanvasGroupResizable::create_corner_manipulator()
{
  auto result = CanvasRectMovable::create();
  result->property_fill_color() = MANIPULATOR_CORNER_FILL_COLOR; //This makes the whole area clickable, not just the outline stroke:
  result->property_line_width() = MANIPULATOR_STROKE_WIDTH;
  result->property_stroke_color() = MANIPULATOR_STROKE_COLOR;

  result->property_height() = MANIPULATOR_CORNER_SIZE;
  result->property_width() = MANIPULATOR_CORNER_SIZE;
  return result;
}

Glib::RefPtr<CanvasLineMovable> CanvasGroupResizable::create_edge_manipulator()
{
  auto line = Glom::CanvasLineMovable::create();
  line->property_line_width() = MANIPULATOR_STROKE_WIDTH;
  line->property_stroke_color() = MANIPULATOR_STROKE_COLOR;
  return line;
}

void CanvasGroupResizable::set_edge_points(const Glib::RefPtr<Glom::CanvasLineMovable>& line, double x1, double y1, double x2, double y2)
{
  double points_coordinates[] = {x1, y1, x2, y2};
  Goocanvas::Points points(2, points_coordinates);
  line->property_points() = points;
}

void CanvasGroupResizable::get_xy(double& x, double& y) const
{
  if(m_child)
    m_child->get_xy(x, y);
  else
  {
    x = m_x;
    y = m_y;
  }
}

void CanvasGroupResizable::set_xy(double x, double y)
{
  //std::cout << "debug: " << G_STRFUNC << ": " << x << ", " << y << std::endl;
  if(m_child)
    m_child->set_xy(x, y);

  //Store them for use when we have a child.
  m_x = x;
  m_y = y;

  position_extras();
}

void CanvasGroupResizable::get_width_height(double& width, double& height) const
{
  if(m_child)
    m_child->get_width_height(width, height);
  else
  {
    width = m_width;
    height = m_height;
  }

  //GooCanvasGroup allows height and width to be -1 to mean the "use the default",
  //but other GooCanvas* items reject that as out of range,
  //so prevent us from using it:
  if(width == -1)
    width = 100; //Arbitrary default.

  if(height == -1)
    height = 100; //Arbitrary default.
}

void CanvasGroupResizable::set_width_height(double width, double height)
{
  if(m_child)
    m_child->set_width_height(width, height);

  //Store them for use when we have a child.
  m_width = width;
  m_height = height;

  position_extras();
}

void CanvasGroupResizable::snap_position(double& x, double& y) const
{
  double offset_x_min = 0;
  double offset_y_min = 0;

  //Try snapping each corner, to choose the one that snapped closest:
  for(int i = 0; i < Utils::to_utype(Corners::COUNT); ++i)
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
  double child_width = 0;
  double child_height = 0;
  get_width_height(child_width, child_height);

  //Choose the offset of the part to snap to the grid:
  double corner_x_offset = 0;
  double corner_y_offset = 0;
  switch(corner)
  {
    case Corners::TOP_LEFT:
      corner_x_offset = 0;
      corner_y_offset = 0;
      break;
    case Corners::TOP_RIGHT:
      corner_x_offset = child_width;
      corner_y_offset = 0;
      break;
    case Corners::BOTTOM_LEFT:
      corner_x_offset = 0;
      corner_y_offset = child_height;
      break;
    case Corners::BOTTOM_RIGHT:
      corner_x_offset = child_width;
      corner_y_offset = child_height;
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

bool CanvasGroupResizable::get_is_line() const
{
  auto line = std::dynamic_pointer_cast<CanvasLineMovable>(m_child);
  return (bool)line;
}

CanvasGroupResizable::type_signal_resized CanvasGroupResizable::signal_resized()
{
  return m_signal_resized;
}

void CanvasGroupResizable::set_grid(const Glib::RefPtr<const CanvasGroupGrid>& grid)
{
  //Call the base class:
  CanvasItemMovable::set_grid(grid);

  //Apply the grid to all the manipulators:
  if(!m_group_edge_manipulators || !m_group_corner_manipulators)
    return;

  int count = m_group_edge_manipulators->get_n_children();
  for(int i = 0; i < count; ++i)
  {
    auto child = m_group_edge_manipulators->get_child(i);
    auto movable = CanvasItemMovable::cast_to_movable(child);
    if(movable)
    {
      movable->set_grid(grid);
    }
  }

  count = m_group_corner_manipulators->get_n_children();
  for(int i = 0; i < count; ++i)
  {
    auto child = m_group_corner_manipulators->get_child(i);
    auto movable = CanvasItemMovable::cast_to_movable(child);
    if(movable)
    {
      movable->set_grid(grid);
    }
  }
}

void CanvasGroupResizable::show_selected()
{
  if(!m_group_edge_manipulators)
    return;

  Goocanvas::ItemVisibility edge_visibility = Goocanvas::ItemVisibility::INVISIBLE;
  if(get_selected())
    edge_visibility = Goocanvas::ItemVisibility::VISIBLE;

  //This is also set the same way if set_manipulators_visibility(),
  //in case that is called at some other time.
  m_group_edge_manipulators->property_visibility() = edge_visibility;
}


} //namespace Glom

