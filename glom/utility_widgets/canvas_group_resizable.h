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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_GROUP_RESIZABLE_H
#define GLOM_UTILITY_WIDGETS_CANVAS_GROUP_RESIZABLE_H

#include <libgoocanvasmm/group.h>
#include "canvas_rect_movable.h"


namespace Glom
{

class CanvasGroupResizable : public Goocanvas::Group
{
protected:
  CanvasGroupResizable();
  virtual ~CanvasGroupResizable();

public:
  static Glib::RefPtr<CanvasGroupResizable> create();

  /** This should only be called after this CanvasGroupResizable has already been added to a canvas.
   */
  void set_child(const Glib::RefPtr<Goocanvas::Item>& child);

protected:
  //virtual bool on_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);
  //virtual bool on_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);
  //virtual bool on_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event);

  enum Manipulators
  {
    MANIPULATOR_CORNER_TOP_LEFT,
    MANIPULATOR_CORNER_TOP_RIGHT,
    MANIPULATOR_CORNER_BOTTOM_LEFT,
    MANIPULATOR_CORNER_BOTTOM_RIGHT,
    MANIPULATOR_EDGE_TOP,
    MANIPULATOR_EDGE_BOTTOM,
    MANIPULATOR_EDGE_LEFT,
    MANIPULATOR_EDGE_RIGHT
  };

  void manipulator_connect_signals(const Glib::RefPtr<CanvasRectMovable> manipulator, Manipulators manipulator_id);

  void on_manipulator_moved(const Glib::RefPtr<CanvasRectMovable> manipulator, Manipulators manipulator_id);

  //bool on_manipulator_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event, Manipulators manipulator);
  //bool on_manipulator_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event, Manipulators manipulator);
  //bool on_manipulator_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event, Manipulators manipulator);

  Glib::RefPtr<Goocanvas::Item> m_child;

  Glib::RefPtr<CanvasRectMovable> m_manipulator_corner_top_left, m_manipulator_corner_top_right, m_manipulator_corner_bottom_left, m_manipulator_corner_bottom_right;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_GROUP_RESIZABLE_H

