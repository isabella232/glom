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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_GRID_H
#define GLOM_UTILITY_WIDGETS_CANVAS_GRID_H

#include <vector>


namespace Glom
{

class CanvasGrid
{
public:
  CanvasGrid();
  virtual ~CanvasGrid();

  /** Snap a coordinate position to any nearby grid or rule line, if the coordinate is close enough to one.
   */
  void snap_position(double& x, double& y) const;

  /// 0.0 means no grid.
  double m_grid_gap;

  typedef std::vector<double> type_vec_double;

  /// The x coordinates of any vertical rules:
  type_vec_double m_rules_x;

  /// The y coordinates of any horizontal rules:
  type_vec_double m_rules_y;

protected:
  double snap_position_grid(double a) const;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_GRID_H

