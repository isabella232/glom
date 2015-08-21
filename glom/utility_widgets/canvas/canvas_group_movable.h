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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_GROUP_MOVABLE_H
#define GLOM_UTILITY_WIDGETS_CANVAS_GROUP_MOVABLE_H

#include "canvas_item_movable.h"
#include <goocanvasmm/group.h>

namespace Glom
{

class CanvasGroupMovable
  : public Goocanvas::Group,
    public CanvasItemMovable
{
protected:
  CanvasGroupMovable();
  virtual ~CanvasGroupMovable();

public:
  static Glib::RefPtr<CanvasGroupMovable> create();

  virtual void get_xy(double& x, double& y) const;
  virtual void set_xy(double x, double y);
  virtual void get_width_height(double& width, double& height) const;
  virtual void set_width_height(double width, double height);
  virtual void set_grid(const Glib::RefPtr<const CanvasGroupGrid>& grid);

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

  //We store the position so that we have something before any children wer added:
  double m_x, m_y, m_width, m_height;

};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_GROUP_MOVABLE_H

