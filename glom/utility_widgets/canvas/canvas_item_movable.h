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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_ITEM_MOVABLE_H
#define GLOM_UTILITY_WIDGETS_CANVAS_ITEM_MOVABLE_H

#include "canvas_group_grid.h"
#include <goocanvasmm/item.h>
#include <gdkmm/cursor.h>

namespace Glom
{

class CanvasItemMovable : virtual public Glib::ObjectBase
{
protected:
  CanvasItemMovable();

public:

  /* Get the position of the item.
   * For some items, this is an arbitrary part of the item,
   * such as the top-left of a rectangle,
   * or the first point in a line.
   */
  virtual void get_xy(double& x, double& y) const = 0;

  /** Move the item.
   * This should be the same arbitrary part of the item that is used by get_xy().
   * All other parts of the item will move by the same offset.
   */
  virtual void set_xy(double x, double y) = 0;

  /*
   */
  virtual void get_width_height(double& width, double& height) const = 0;

  /**
   */
  virtual void set_width_height(double width, double height) = 0;

  void set_drag_cursor(Gdk::Cursor::Type cursor_type);

  /** For instance,
   *
   *   @param item The item (this item) that was moved, for convenience.
   *   @param x_offset How much the item has moved in the x dimension.
   *   @param y_offset How much the item has moved in the y dimension.
   *
   *   void on_moved(bool group_select, double x_offset, double y_offset);
   */
  typedef sigc::signal<void(const Glib::RefPtr<CanvasItemMovable>&, double, double)> type_signal_moved;

  /// This signal is emitted when the canvas item is moved by the user.
  type_signal_moved signal_moved();

  /** void on_show_context(guint button, guint32 activate_time);
   */
  typedef sigc::signal<void(guint, guint32)> type_signal_show_context;
  type_signal_show_context signal_show_context();

  /** For instance,
   *
   *   @param item The item (this item) that was selected/deseleted, for convenience.
   *   @param group_select Whether the user selected this while pressing Shift to select multiple items.
   *
   *   void on_selected(bool group_select);
   */
  typedef sigc::signal<void(const Glib::RefPtr<CanvasItemMovable>&, bool)> type_signal_selected;

  /** This signal is emitted if the user causes the item
   * to be selected or deselected. See get_selected().
   */
  type_signal_selected signal_selected();


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
  static Glib::RefPtr<const CanvasItemMovable> cast_const_to_movable(const Glib::RefPtr<const Goocanvas::Item>& item);

  static Glib::RefPtr<Goocanvas::Item> cast_to_item(const Glib::RefPtr<CanvasItemMovable>& item);

  virtual void snap_position(double& x, double& y) const;

  /** Mark the item as selected,
   * meaning that its boundaries will be visible,
   * and this can be queried later, for instance to move several items together.
   */
  void set_selected(bool selected = true);

  bool get_selected() const;

private:

  /** Show some visual cue that the item is selected,
   * depending on the value of get_selected(),
   * hiding that visual cue if it is not selected.
   */
  virtual void show_selected();

  virtual Goocanvas::Canvas* get_parent_canvas_widget() = 0;

  void set_cursor(const Glib::RefPtr<Gdk::Cursor>& cursor);
  void unset_cursor();

public:
  //These should really be protected, but the compiler doesn't allow it:
  void on_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);
  void on_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event);
  void on_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);
  void on_enter_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event);
  void on_leave_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event);

private:
  bool m_dragging;
  bool m_dragging_vertical_only, m_dragging_horizontal_only; //Set by using Ctrl while dragging.
  double m_drag_start_cursor_x, m_drag_start_cursor_y;
  double m_drag_start_position_x, m_drag_start_position_y;
  double m_drag_latest_position_x, m_drag_latest_position_y; //To discover how much the latest motion_event has moved the item.
  Gdk::Cursor::Type m_drag_cursor_type;

protected:
  Glib::RefPtr<const CanvasGroupGrid> m_grid;

private:
  bool m_allow_vertical_movement, m_allow_horizontal_movement;
  bool m_selected;
  bool m_shift_click;

  type_signal_moved m_signal_moved;
  type_signal_show_context m_signal_show_context;
  type_signal_selected m_signal_selected;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_ITEM_MOVABLE_H
