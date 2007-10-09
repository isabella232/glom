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

#include "canvas_item_movable.h"
#include <libgoocanvasmm/group.h>
#include <libgoocanvasmm/rect.h>


namespace Glom
{

class CanvasRectMovable;
class CanvasLineMovable;

class CanvasGroupResizable
  : public Goocanvas::Group,
    public CanvasItemMovable
{
protected:
  CanvasGroupResizable();
  virtual ~CanvasGroupResizable();

public:
  static Glib::RefPtr<CanvasGroupResizable> create();

  /** This should only be called after this CanvasGroupResizable has already been added to a canvas.
   */
  void set_child(const Glib::RefPtr<Goocanvas::Rect>& child);

  virtual void get_xy(double& x, double& y);
  virtual void move(double x_offet, double y_offset);

protected:
  virtual Goocanvas::Canvas* get_parent_canvas_widget();

  virtual void snap_position(double& x, double& y) const;

  enum Corners
  {
    CORNER_TOP_LEFT,
    CORNER_TOP_RIGHT,
    CORNER_BOTTOM_LEFT,
    CORNER_BOTTOM_RIGHT,
    CORNER_COUNT
  };

  void snap_position(Corners corner, double& x, double& y) const;


  bool on_enter_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event);
  bool on_leave_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event);

  virtual bool on_child_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);
  virtual bool on_child_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);
  virtual bool on_child_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event);

  enum Manipulators
  {
    MANIPULATOR_NONE,
    MANIPULATOR_CORNER_TOP_LEFT,
    MANIPULATOR_CORNER_TOP_RIGHT,
    MANIPULATOR_CORNER_BOTTOM_LEFT,
    MANIPULATOR_CORNER_BOTTOM_RIGHT,
    MANIPULATOR_EDGE_TOP,
    MANIPULATOR_EDGE_BOTTOM,
    MANIPULATOR_EDGE_LEFT,
    MANIPULATOR_EDGE_RIGHT
  };

  void manipulator_connect_signals(const Glib::RefPtr<Goocanvas::Item> manipulator, Manipulators manipulator_id);
  void position_manipulators();
  void set_manipulators_visibility(Goocanvas::ItemVisibility visibility);

  void on_manipulator_corner_moved(const Glib::RefPtr<CanvasRectMovable>& manipulator, Manipulators manipulator_id);
  void on_manipulator_edge_moved(const Glib::RefPtr<CanvasLineMovable>& manipulator, Manipulators manipulator_id);

  //bool on_manipulator_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event, Manipulators manipulator);
  //bool on_manipulator_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event, Manipulators manipulator);
  //bool on_manipulator_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event, Manipulators manipulator);

  static Glib::RefPtr<CanvasRectMovable> create_corner();
  static Glib::RefPtr<CanvasLineMovable> create_edge();
  static void set_edge_points(const Glib::RefPtr<Glom::CanvasLineMovable>& line, double x1, double y1, double x2, double y2);

  Glib::RefPtr<Goocanvas::Rect> m_child;
  Glib::RefPtr<CanvasRectMovable> m_manipulator_corner_top_left, m_manipulator_corner_top_right, m_manipulator_corner_bottom_left, m_manipulator_corner_bottom_right;
  Glib::RefPtr<CanvasLineMovable> m_manipulator_edge_top, m_manipulator_edge_bottom, m_manipulator_edge_left, m_manipulator_edge_right;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_GROUP_RESIZABLE_H

