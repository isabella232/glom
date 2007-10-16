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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_ITEM_MOVABLE_H
#define GLOM_UTILITY_WIDGETS_CANVAS_ITEM_MOVABLE_H

#include "canvas_item_movable.h"
#include "canvas_group_grid.h"
#include <libgoocanvasmm/item.h>
#include <gdkmm/cursor.h>

namespace Glom
{

class CanvasItemMovable : virtual public Glib::ObjectBase
{
protected:
  CanvasItemMovable();
  virtual ~CanvasItemMovable();

public:

  /* Get the position of the item.
   * For some items, this is an arbitrary part of the item, 
   * such as the top-left of a rectangle,
   * or the first point in a line.
   */
  virtual void get_xy(double& x, double& y) = 0;

  /** Move the item.
   * This should be the same arbitrary part of the item that is used by get_xy().
   * All other parts of the item will move by the same offset.
   */
  virtual void set_xy(double x, double y) = 0;

  /* 
   */
  virtual void get_width_height(double& width, double& height) = 0;

  /** 
   */
  virtual void set_width_height(double width, double height) = 0;

  void set_drag_cursor(Gdk::CursorType cursor);
  void set_drag_cursor(const Gdk::Cursor& cursor);

  typedef sigc::signal<void> type_signal_moved;
  type_signal_moved signal_moved();

  /** void on_show_context(guint button, guint32 activate_time);
   */
  typedef sigc::signal<void, guint, guint32> type_signal_show_context;
  type_signal_show_context signal_show_context();

  /** Provide information about a grid or rules, 
   * to which the item should snap when moving:
   *
   * @param grid: This must exist for as long as the canvas item.
   */
  virtual void set_grid(const Glib::RefPtr<const CanvasGroupGrid>& grid);

  /** Restrict drag movement (via dragging) to the x axis or the y axis,
   * or prevent all drag movement.
   */
  void set_movement_allowed(bool vertical = true, bool horizontal = true);

  ///A convenience function, to avoid repeating a large if/else block.
  static Glib::RefPtr<CanvasItemMovable> cast_to_movable(const Glib::RefPtr<Goocanvas::Item>& item);
  static Glib::RefPtr<Goocanvas::Item> cast_to_item(const Glib::RefPtr<CanvasItemMovable>& item);

protected:

  virtual Goocanvas::Canvas* get_parent_canvas_widget() = 0;

  virtual void snap_position(double& x, double& y) const;

  void set_cursor(const Gdk::Cursor& cursor);
  void unset_cursor();
  
public:
  //These should really be protected, but the compiler doesn't allow it:
  bool on_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);
  bool on_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event);
  bool on_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);
  bool on_enter_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event);
  bool on_leave_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event);

protected:
  bool m_dragging;
  bool m_dragging_vertical_only, m_dragging_horizontal_only; //Set by using Ctrl while dragging.
  double m_drag_start_cursor_x, m_drag_start_cursor_y;
  double m_drag_start_position_x, m_drag_start_position_y;
  Gdk::Cursor m_drag_cursor;

  Glib::RefPtr<const CanvasGroupGrid> m_grid;

  bool m_allow_vertical_movement, m_allow_horizontal_movement;

  type_signal_moved m_signal_moved;
  type_signal_show_context m_signal_show_context;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_ITEM_MOVABLE_H

