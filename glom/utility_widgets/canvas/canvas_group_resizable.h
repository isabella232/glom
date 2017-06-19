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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_GROUP_RESIZABLE_H
#define GLOM_UTILITY_WIDGETS_CANVAS_GROUP_RESIZABLE_H

#include "canvas_item_movable.h"
#include <goocanvasmm/group.h>
#include <goocanvasmm/rect.h>


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

public:
  static Glib::RefPtr<CanvasGroupResizable> create();

  /** This should only be called after this CanvasGroupResizable has already been added to a canvas.
   * The position (x, y, width, height) of the child will match the position of the CanvasGroupResizable,
   * overriding any previous position of the child.
   */
  void set_child(const Glib::RefPtr<CanvasItemMovable>& child);

  /// Get the only child:
  Glib::RefPtr<CanvasItemMovable> get_child();

  /// Get the only child:
  Glib::RefPtr<const CanvasItemMovable> get_child() const;

  void get_xy(double& x, double& y) const override;
  void set_xy(double x_offet, double y_offset) override;
  void get_width_height(double& width, double& height) const override;
  void set_width_height(double width, double height) override;
  void set_grid(const Glib::RefPtr<const CanvasGroupGrid>& grid) override;

  void snap_position(double& x, double& y) const override;

  void set_outline_visible(bool visible = true);

  typedef sigc::signal<void()> type_signal_resized;

  /// This signal is emitted when the canvas item is resized by the user.
  type_signal_resized signal_resized();

  //Get the outline details so they can be used elsewhere, for consistency.
  static void get_outline_stroke(Glib::ustring& color, double& width);

private:
  void show_selected() override;
  Goocanvas::Canvas* get_parent_canvas_widget() override;

  enum class Corners
  {
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    COUNT
  };

  void snap_position(Corners corner, double& x, double& y) const;


  bool on_rect_enter_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, Gdk::EventCrossing& event);
  bool on_rect_leave_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, Gdk::EventCrossing& event);

  bool on_resizer_enter_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, Gdk::EventCrossing& event);
  bool on_resizer_leave_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, Gdk::EventCrossing& event);


  void on_child_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, Gdk::EventButton& event);
  void on_child_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, Gdk::EventButton& event);
  void on_child_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, Gdk::EventMotion& event);

  enum class Manipulators
  {
    NONE,

    //For rectangles:
    CORNER_TOP_LEFT,
    CORNER_TOP_RIGHT,
    CORNER_BOTTOM_LEFT,
    CORNER_BOTTOM_RIGHT,
    EDGE_TOP,
    EDGE_BOTTOM,
    EDGE_LEFT,
    EDGE_RIGHT,

    //For straight lines:
    START,
    END
  };

  Glib::RefPtr<CanvasItemMovable> get_manipulator(Manipulators manipulator_id);

  void create_manipulators();
  void create_rect_manipulators();
  void create_line_manipulators();

  void create_outline_group();
  Glib::RefPtr<CanvasLineMovable> create_outline_line(double x1, double y1, double x2, double y2);


  void manipulator_connect_signals(const Glib::RefPtr<Goocanvas::Item>& manipulator, Manipulators manipulator_id);
  void position_manipulators();
  void position_rect_manipulators();
  void position_line_manipulators();
  void set_manipulators_visibility(Goocanvas::ItemVisibility visibility);

  void position_outline();

  /** Make sure that the outline and manipulators are correctly placed and sized.
   */
  void position_extras();

  void on_manipulator_corner_moved(const Glib::RefPtr<CanvasItemMovable>& item, double x_offset, double y_offset, Manipulators manipulator_id);
  void on_manipulator_edge_moved(const Glib::RefPtr<CanvasItemMovable>& item, double x_offset, double y_offset, Manipulators manipulator_id);
  void on_manipulator_line_end_moved(const Glib::RefPtr<CanvasItemMovable>& item, double x_offset, double y_offset, Manipulators manipulator_id);
  bool on_manipulator_enter_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, Gdk::EventCrossing& event);
  bool on_manipulator_leave_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, Gdk::EventCrossing& event);


  bool get_is_line() const;

  //bool on_manipulator_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, Gdk::EventButton& event, Manipulators manipulator);
  //bool on_manipulator_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, Gdk::EventButton& event, Manipulators manipulator);
  //bool on_manipulator_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, Gdk::EventMotion& event, Manipulators manipulator);

  static Glib::RefPtr<CanvasRectMovable> create_corner_manipulator();
  static Glib::RefPtr<CanvasLineMovable> create_edge_manipulator();
  static void set_edge_points(const Glib::RefPtr<Glom::CanvasLineMovable>& line, double x1, double y1, double x2, double y2);

  Glib::RefPtr<CanvasItemMovable> m_child;

  Glib::RefPtr<Goocanvas::Group> m_group_edge_manipulators; //not including the rect.
  Glib::RefPtr<Goocanvas::Group> m_group_corner_manipulators; //not including the rect.

  //Manipulators for a rectangle:
  Glib::RefPtr<Goocanvas::Rect> m_rect; //Something to get events on, because m_child might actually be smaller than indicated by our manipulators.
  Glib::RefPtr<CanvasRectMovable> m_manipulator_corner_top_left, m_manipulator_corner_top_right, m_manipulator_corner_bottom_left, m_manipulator_corner_bottom_right;
  Glib::RefPtr<CanvasLineMovable> m_manipulator_edge_top, m_manipulator_edge_bottom, m_manipulator_edge_left, m_manipulator_edge_right;

  //Manipulators for a line:
  Glib::RefPtr<CanvasRectMovable> m_manipulator_start, m_manipulator_end;

  //These are visible to indicate the item's dimensions:
  Glib::RefPtr<Goocanvas::Group> m_group_outline;
  Glib::RefPtr<CanvasLineMovable> m_outline_top, m_outline_bottom, m_outline_left, m_outline_right;


  bool m_in_manipulator; //Whether the cursor is in a manipulator.

  //These are used only before there is a child.
  //When there is a child, we delegate to it instead.
  double m_x, m_y, m_width, m_height;

  type_signal_resized m_signal_resized;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_GROUP_RESIZABLE_H

