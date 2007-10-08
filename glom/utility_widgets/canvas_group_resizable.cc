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
#include <goocanvasrect.h>
#include <goocanvasgroup.h>
#include <iostream>

namespace Glom
{

const double corner_size = 10;

static Glib::RefPtr<CanvasRectMovable> create_corner()
{
  Glib::RefPtr<CanvasRectMovable> result = CanvasRectMovable::create();
  result->property_fill_color() = "green"; //This makes the whole area clickable, not just the outline stroke:
  result->property_line_width() = 2.0f;
  result->property_stroke_color() = "black";

  result->property_height() = corner_size;
  result->property_width() = corner_size;
  

  return result;
}

CanvasGroupResizable::CanvasGroupResizable()
: Goocanvas::Group((GooCanvasGroup*)goo_canvas_group_new(NULL, NULL)) //TODO: Remove this when goocanvas has been fixed.
{
  m_manipulator_corner_top_left = create_corner();
  m_manipulator_corner_top_right = create_corner();
  m_manipulator_corner_bottom_left = create_corner();
  m_manipulator_corner_bottom_right = create_corner();
}

CanvasGroupResizable::~CanvasGroupResizable()
{
}

Glib::RefPtr<CanvasGroupResizable> CanvasGroupResizable::create()
{
  return Glib::RefPtr<CanvasGroupResizable>(new CanvasGroupResizable());
}

void CanvasGroupResizable::position_corners()
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

  m_manipulator_corner_top_right->property_x() = x2 - corner_size;
  m_manipulator_corner_top_right->property_y() = y1;

  m_manipulator_corner_bottom_left->property_x() = x1;
  m_manipulator_corner_bottom_left->property_y() = y2 - corner_size;

  m_manipulator_corner_bottom_right->property_x() = x2 - corner_size;
  m_manipulator_corner_bottom_right->property_y() = y2 - corner_size;
}

void CanvasGroupResizable::set_child(const Glib::RefPtr<Goocanvas::Rect>& child)
{
  if(!child)
    return;

  if(m_child)
    return;

  m_child = child;
  add_child(child);

  add_child(m_manipulator_corner_top_left);
  add_child(m_manipulator_corner_top_right);
  add_child(m_manipulator_corner_bottom_left);
  add_child(m_manipulator_corner_bottom_right);

  manipulator_connect_signals(m_manipulator_corner_top_left, MANIPULATOR_CORNER_TOP_LEFT);
  manipulator_connect_signals(m_manipulator_corner_top_right, MANIPULATOR_CORNER_TOP_RIGHT);
  manipulator_connect_signals(m_manipulator_corner_bottom_left, MANIPULATOR_CORNER_BOTTOM_LEFT);
  manipulator_connect_signals(m_manipulator_corner_bottom_right, MANIPULATOR_CORNER_BOTTOM_RIGHT);

  position_corners();
}

void CanvasGroupResizable::manipulator_connect_signals(const Glib::RefPtr<CanvasRectMovable> manipulator, Manipulators manipulator_id)
{
  //Respond when the corner rectangles move (they implement their own dragging):

  //TODO: x and y property notification doesn't seem to work. Investigate that in goocanvas.
  //For now, I added a signal_moved property.
  manipulator->signal_moved().connect(
    sigc::bind( sigc::mem_fun(*this, &CanvasGroupResizable::on_manipulator_moved), manipulator, manipulator_id) );

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


void CanvasGroupResizable::on_manipulator_moved(const Glib::RefPtr<CanvasRectMovable> manipulator, Manipulators manipulator_id)
{
  //std::cout << "CanvasGroupResizable::on_manipulator_moved(): manipulator=" << manipulator_id << std::endl;

  switch(manipulator_id)
  {
    case(MANIPULATOR_CORNER_TOP_LEFT):
    {
      const double new_x = manipulator->property_x();
      const double new_y = manipulator->property_y();
      const double new_height = m_child->property_y() + m_child->property_height() - manipulator->property_y();
      const double new_width = m_child->property_x() + m_child->property_width() - manipulator->property_x();

      m_child->property_x() = new_x;
      m_child->property_y() = new_y;
      m_child->property_width() = new_width;
      m_child->property_height() = new_height;

      break;
    }
    case(MANIPULATOR_CORNER_TOP_RIGHT):
    {
      const double new_y = manipulator->property_y();
      const double new_height = m_child->property_y() + m_child->property_height() - manipulator->property_y();
      const double new_width = manipulator->property_x() + corner_size - m_child->property_x();

      m_child->property_y() = new_y;
      m_child->property_width() = new_width;
      m_child->property_height() = new_height;

      break;
    }
    case(MANIPULATOR_CORNER_BOTTOM_LEFT):
    {
      const double new_x = manipulator->property_x();
      const double new_height = manipulator->property_y() + corner_size - m_child->property_y();
      const double new_width = m_child->property_x() + m_child->property_width() - manipulator->property_x();

      m_child->property_x() = new_x;
      m_child->property_width() = new_width;
      m_child->property_height() = new_height;

      break;
    }
    case(MANIPULATOR_CORNER_BOTTOM_RIGHT):
    {
      const double new_height = manipulator->property_y() + corner_size - m_child->property_y();
      const double new_width = manipulator->property_x() + corner_size - m_child->property_x();

      m_child->property_width() = new_width;
      m_child->property_height() = new_height;

      break;
    }
    default:
      break;
  }

  position_corners();
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



} //namespace Glom

