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

void CanvasGroupResizable::set_child(const Glib::RefPtr<Goocanvas::Item>& child)
{
  if(!child)
    return;

  m_child = child;
  add_child(child);

  //Note that this only works after the child has been added to the canvas:
  Goocanvas::Bounds bounds;
  child->get_bounds(bounds);
  printf("%s: bounds=%f, %f, %f, %f\n", __FUNCTION__, bounds.get_x1(), bounds.get_y1(), bounds.get_x2(), bounds.get_y2());

  m_manipulator_corner_top_left->property_x() = bounds.get_x1();
  m_manipulator_corner_top_left->property_y() = bounds.get_y1();
  add_child(m_manipulator_corner_top_left);
  manipulator_connect_signals(m_manipulator_corner_top_left, MANIPULATOR_CORNER_TOP_LEFT);

  m_manipulator_corner_top_right->property_x() = bounds.get_x2() - corner_size;
  m_manipulator_corner_top_right->property_y() = bounds.get_y1();
  add_child(m_manipulator_corner_top_right);
  manipulator_connect_signals(m_manipulator_corner_top_right, MANIPULATOR_CORNER_TOP_RIGHT);

  m_manipulator_corner_bottom_left->property_x() = bounds.get_x1();
  m_manipulator_corner_bottom_left->property_y() = bounds.get_y2() - corner_size;
  add_child(m_manipulator_corner_bottom_left);
  manipulator_connect_signals(m_manipulator_corner_bottom_left, MANIPULATOR_CORNER_BOTTOM_LEFT);

  m_manipulator_corner_bottom_right->property_x() = bounds.get_x2() - corner_size;
  m_manipulator_corner_bottom_right->property_y() = bounds.get_y2() - corner_size;
  add_child(m_manipulator_corner_bottom_right);
  manipulator_connect_signals(m_manipulator_corner_bottom_right, MANIPULATOR_CORNER_BOTTOM_RIGHT);
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
  std::cout << "CanvasGroupResizable::on_manipulator_moved(): manipulator=" << manipulator << std::endl;

  Goocanvas::Bounds bounds_child;
  m_child->get_bounds(bounds_child);
  printf("%s: bounds=%f, %f, %f, %f\n", __FUNCTION__, bounds_child.get_x1(), bounds_child.get_x2(), bounds_child.get_y1(), bounds_child.get_y2());

  double x1 = bounds_child.get_x1();
  double x2 = bounds_child.get_x2();
  double y1 = bounds_child.get_y1();
  double y2 = bounds_child.get_y2();
  const double height = y2 - y1;
  const double width = x2 - x1;
  printf("%s: height=%f, width=%f\n", __FUNCTION__,  height, width);

  Goocanvas::Bounds bounds_manipulator;
  manipulator->get_bounds(bounds_manipulator);

  switch(manipulator_id)
  {
    case(MANIPULATOR_CORNER_TOP_LEFT):
    {
      x1 = bounds_manipulator.get_x1();
      y1 = bounds_manipulator.get_y1(); 
      std::cout << "  x1 =" << x1 << ", y1=" << y1 << std::endl;
      std::cout << "    translate: " << x1 - bounds_child.get_x1() << ", " << y1 - bounds_child.get_y1() << std::endl;
     
      double new_width = x2-x1;
      double new_height = y2-y1;
      printf("%s:   new_height=%f, new_width=%f\n", __FUNCTION__,  new_height, new_width);

      m_child->translate(x1 - bounds_child.get_x1(), y1 - bounds_child.get_y1());
      //m_child->scale(new_width / width, new_height / height);
      break;
    }
    /*
    case(MANIPULATOR_CORNER_TOP_RIGHT):
      x2 = bounds_manipulator.get_x2();
      y1 = bounds_manipulator.get_y1();
     
      m_child->translate(x1 - bounds_child.get_x1(), y1 - bounds_child.get_y1());
   
      break;
    */
    case(MANIPULATOR_CORNER_BOTTOM_LEFT):
    {
      double new_x1 = bounds_manipulator.get_x1();

      const double new_height = bounds_manipulator.get_y2() - y1;
      printf("%s: new_height=%f\n", __FUNCTION__, new_height);
      const double new_width = x2 - bounds_manipulator.get_x1();
     
      m_child->translate(new_x1 - bounds_child.get_x1(), 0);
      m_child->scale(new_width / width, new_height / height);
      break;
    }
    /*
    case(MANIPULATOR_CORNER_BOTTOM_RIGHT):
      x1 = m_manipulator_corner_top_left->get_x();
      y1 = m_manipulator_corner_top_left->get_y();
      break;
    */
    default:
      break;
  }
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

