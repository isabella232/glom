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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_GROUP_GRID_H
#define GLOM_UTILITY_WIDGETS_CANVAS_GROUP_GRID_H

#include <goocanvasmm/grid.h>
#include <goocanvasmm/group.h>
#include <goocanvasmm/polyline.h>
#include <vector>


namespace Glom
{

class CanvasGroupGrid : public Goocanvas::Group
{
private:
  CanvasGroupGrid();
  virtual ~CanvasGroupGrid();

public:
  static Glib::RefPtr<CanvasGroupGrid> create();
 
  /** Snap a coordinate position to any nearby grid or rule line, if the coordinate is close enough to one.
   */
  void snap_position(double& x, double& y) const;

  /// 0.0 means no grid.
  double m_grid_gap;

  /// How close we have to be to a grid line to snap to it:
  double m_grid_sensitivity;

  typedef std::vector<double> type_vec_double;

  /// The x coordinates of any vertical rules:
  type_vec_double m_rules_x;

  /// The y coordinates of any horizontal rules:
  type_vec_double m_rules_y;

  /** Set the distance between grid lines, 
   * used to snap to the grid lines when moving or resizing items.
   */
  void set_grid_gap(double gap);

  /** Remove grid lines.
   * See also remove_rules().
   */
  void remove_grid();

  void add_vertical_rule(double x);
  void add_horizontal_rule(double x);

private:
  void create_lines();
  Glib::RefPtr<Goocanvas::Polyline> create_grid_or_rule_line(double x1, double y1, double x2, double y2, bool is_rule = false);

  double snap_position_grid(double a) const;
  double snap_position_rules(const type_vec_double& rules, double a) const;
  double snap_position_rules_x(double x) const;
  double snap_position_rules_y(double y) const;

  bool is_close(double a, double b) const;

  Glib::RefPtr<Goocanvas::Grid> m_grid_lines;
  Glib::RefPtr<Goocanvas::Group> m_grid_rules_group;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_GROUP_GRID_H

