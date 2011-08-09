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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_EDITABLE_H
#define GLOM_UTILITY_WIDGETS_CANVAS_EDITABLE_H

#include <goocanvasmm.h>
#include <glom/utility_widgets/canvas/canvas_group_grid.h>
#include <glom/utility_widgets/canvas/canvas_item_movable.h>
#include <map>
#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

namespace Glom
{

class CanvasEditable : public Goocanvas::Canvas
{
public:
  CanvasEditable();
  virtual ~CanvasEditable();

  void add_item(const Glib::RefPtr<Goocanvas::Item>& item, bool resizable = false);
  void add_item(const Glib::RefPtr<Goocanvas::Item>& item, const Glib::RefPtr<Goocanvas::Group>& group, bool resizable = false);

  void remove_item(const Glib::RefPtr<Goocanvas::Item>& item, const Glib::RefPtr<Goocanvas::Group>& group);

  void remove_all_items();
  void remove_all_items(const Glib::RefPtr<Goocanvas::Group>& group);

  /** Set the distance between grid lines, 
   * used to snap to the grid lines when moving or resizing items.
   */
  virtual void set_grid_gap(double gap = 20.0);

  /** Remove grid lines.
   * See also remove_rules().
   */
  void remove_grid();

  void set_rules_visibility(bool visible = true);

  void add_vertical_rule(double x);
  void add_horizontal_rule(double x);

  /** For items not added directly via add_item(),
   * but which need to snap to the grid.
   */
  void associate_with_grid(const Glib::RefPtr<Goocanvas::Item>& item);


  typedef std::vector< Glib::RefPtr<CanvasItemMovable> > type_vec_items;

  /** Get any items that have get_selected()==true.
   * Derived classes may override this to only examine items that they consider interesting.
   */
  virtual type_vec_items get_selected_items();

  //TODO: Actually emit this, so we actually show the context menu when clicking on blank space:
  /** void on_show_context(guint button, guint32 activate_time);
   */
  typedef sigc::signal<void, guint, guint32> type_signal_show_context;
  type_signal_show_context signal_show_context();


  /** For instance,
   *   void on_selection_changed();
   */
  typedef sigc::signal<void> type_signal_selection_changed;

  /** This signal is emitted if the user causes items 
   * to be selected or deselected. See get_selected_items().
   */
  type_signal_selection_changed signal_selection_changed();

private:
  
  void on_item_selected(const Glib::RefPtr<CanvasItemMovable>& item, bool group_select);

  static Glib::RefPtr<Goocanvas::Item> get_parent_container_or_self(const Glib::RefPtr<Goocanvas::Item>& item);

  bool m_dragging;
  double m_drag_x, m_drag_y;

  class ItemInfo
  {
  public:
    //ItemInfo()
    //ItemInfo(const ItemInfo& src);
    //ItemInfo& operator=(const ItemInfo& src);

    bool m_resizable;
  };

protected:
  Glib::RefPtr<CanvasGroupGrid> m_grid;

private:
  type_signal_show_context m_signal_show_context;
  type_signal_selection_changed m_signal_selection_changed;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_EDITABLE_H

