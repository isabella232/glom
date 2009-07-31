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

#include "canvas_image_movable.h"
#include <goocanvasmm/canvas.h>
#include <gtkmm/stock.h>
#include <glom/application.h> // For get_application().
#include <glom/utility_widgets/imageglom.h> //For ImageGlom::scale_keeping_ratio().
#include <iostream>

namespace Glom
{

CanvasImageMovable::CanvasImageMovable(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, double x, double y)
: Goocanvas::Image(pixbuf, x, y),
  m_snap_corner(CORNER_TOP_LEFT), //arbitrary default.
  m_image_empty(false)
{
  init();
}

CanvasImageMovable::CanvasImageMovable(double x, double y)
: Goocanvas::Image(x, y), 
  CanvasItemMovable(),
  m_snap_corner(CORNER_TOP_LEFT) //arbitrary default.
{
  init();
}

CanvasImageMovable::~CanvasImageMovable()
{
}

void CanvasImageMovable::init()
{
  signal_motion_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_motion_notify_event));
  signal_button_press_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_button_press_event));
  signal_button_release_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_button_release_event));

  signal_enter_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_enter_notify_event));
  signal_leave_notify_event().connect(sigc::mem_fun(*this, &CanvasItemMovable::on_leave_notify_event));
}

Glib::RefPtr<CanvasImageMovable> CanvasImageMovable::create(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, double x, double y)
{
  return Glib::RefPtr<CanvasImageMovable>(new CanvasImageMovable(pixbuf, x, y));
}

Glib::RefPtr<CanvasImageMovable> CanvasImageMovable::create(double x, double y)
{
  return Glib::RefPtr<CanvasImageMovable>(new CanvasImageMovable(x, y));
}

void CanvasImageMovable::get_xy(double& x, double& y) const
{
#ifdef GLIBMM_PROPERTIES_ENABLED
  x = property_x();
  y = property_y();
#else
  get_property("x", x);
  get_property("y", y);
#endif
}

void CanvasImageMovable::set_xy(double x, double y)
{
#ifdef GLIBMM_PROPERTIES_ENABLED
  property_x() = x;
  property_y() = y;
#else
  set_property("x", x);
  set_property("y", y);
#endif
}

void CanvasImageMovable::get_width_height(double& width, double& height) const
{
  //TODO: This only works when it is on a canvas already,
  //and this is apparently incorrect when the "coordinate space" of the item changes, whatever that means. murrayc.
  
#ifdef GLIBMM_PROPERTIES_ENABLED
  width = property_width();
  height = property_height();
#else
  get_property("width", width);
  get_property("height", height);
#endif 
}

void CanvasImageMovable::set_width_height(double width, double height)
{
#ifdef GLIBMM_PROPERTIES_ENABLED
  property_width() = width;
  property_height() = height;
#else
  set_property("width", width);
  set_property("height", height);
#endif 
}

void CanvasImageMovable::snap_position(double& x, double& y) const
{
  double width = 0;
  double height = 0;
  get_width_height(width, height);

  //Choose the offset of the part to snap to the grid:
  double corner_x_offset = 0;
  double corner_y_offset = 0;
  switch(m_snap_corner)
  {
    case CORNER_TOP_LEFT:
      corner_x_offset = 0;
      corner_y_offset = 0;
      break;
    case CORNER_TOP_RIGHT:
#ifdef GLIBMM_PROPERTIES_ENABLED    
      corner_x_offset = property_width();
#else
      get_property("width", corner_x_offset);
#endif      
      corner_y_offset = 0;
      break;
    case CORNER_BOTTOM_LEFT:
      corner_x_offset = 0;
      corner_y_offset = height;
      break;
    case CORNER_BOTTOM_RIGHT:
      corner_x_offset = width;
      corner_y_offset = height;
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

Goocanvas::Canvas* CanvasImageMovable::get_parent_canvas_widget()
{
  return get_canvas();
}

void CanvasImageMovable::set_snap_corner(Corners corner)
{
  m_snap_corner = corner;
}

void CanvasImageMovable::set_image(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, bool scale)
{
#ifdef GLIBMM_PROPERTIES_ENABLED
  property_pixbuf() = pixbuf;
#else
  set_property("pixbuf", pixbuf);
#endif    
  m_pixbuf = pixbuf;

  if(scale)
    scale_to_size();
 
  m_image_empty = false;
}

void CanvasImageMovable::scale_to_size()
{
  if(!m_pixbuf)
    return;

  double width = 0;
  double height = 0;
  get_width_height(width, height);
  //std::cout << "CanvasImageMovable::set_image(): width=" << width << ", height=" << height << std::endl;
  
  if(width && height)
  {
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = ImageGlom::scale_keeping_ratio(m_pixbuf, (int)height, (int)width);
#ifdef GLIBMM_PROPERTIES_ENABLED
    property_pixbuf() = pixbuf;
#else
    set_property("pixbuf", pixbuf);
#endif 
  }

  //Make sure that the size stays the same even if the scaling wasn't exact:
  set_width_height(width, height);
}

void CanvasImageMovable::set_image_empty()
{
  m_image_empty = true;

  //We need some widget to use either render_icon() or get_style()+IconSet.
  Gtk::Widget *widget = get_canvas();

  if(!widget)
    widget = App_Glom::get_application();

  Glib::RefPtr<Gdk::Pixbuf> pixbuf;
  if(widget)
    pixbuf = widget->render_icon(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_DIALOG);
#ifdef GLIBMM_PROPERTIES_ENABLED
    property_pixbuf() = pixbuf;
#else
    set_property("pixbuf", pixbuf);
#endif
}

bool CanvasImageMovable::get_image_empty() const
{
  return m_image_empty;
}

} //namespace Glom

