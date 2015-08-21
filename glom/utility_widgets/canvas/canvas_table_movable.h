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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_TABLE_MOVABLE_H
#define GLOM_UTILITY_WIDGETS_CANVAS_TABLE_MOVABLE_H

#include "canvas_item_movable.h"
#include <goocanvasmm/table.h>

namespace Glom
{

class CanvasTableMovable
  : public Goocanvas::Table,
    public CanvasItemMovable
{
private:
  CanvasTableMovable();
  virtual ~CanvasTableMovable();

public:
  static Glib::RefPtr<CanvasTableMovable> create();

  virtual void get_xy(double& x, double& y) const;
  virtual void set_xy(double x, double y);
  virtual void get_width_height(double& width, double& height) const;
  virtual void set_width_height(double width, double height);
  virtual void set_grid(const Glib::RefPtr<const CanvasGroupGrid>& grid);

  void set_lines_details(double row_line_width, double column_line_width, const Glib::ustring& color);

  /** Show horizontal and vertical grid lines, if they were not shown already somehow.
   * @param show If this is false then the normal lines will be shown, or no lines. See set_line_details().
   */
  void set_lines_visibility(bool show = true);

private:
  virtual Goocanvas::Canvas* get_parent_canvas_widget();

  enum class Corners
  {
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    COUNT
  };

  void snap_position_one_corner(Corners corner, double& x, double& y) const;

  virtual void snap_position(double& x, double& y) const;
  
  double m_row_line_width, m_column_line_width;
  Glib::ustring m_line_color;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_TABLE_MOVABLE_H

