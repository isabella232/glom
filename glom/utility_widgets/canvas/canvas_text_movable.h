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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_TEXT_MOVABLE_H
#define GLOM_UTILITY_WIDGETS_CANVAS_TEXT_MOVABLE_H

#include "canvas_item_movable.h"
#include <libgoocanvasmm/text.h>

namespace Glom
{

class CanvasTextMovable
  : public Goocanvas::Text,
    public CanvasItemMovable
{
protected:
  CanvasTextMovable(const Glib::ustring& string = Glib::ustring(), double x = 0.0, double y = 0.0, double width = 0.0, Gtk::AnchorType anchor = Gtk::ANCHOR_NORTH_WEST);
  virtual ~CanvasTextMovable();

  void init();

public:
  static Glib::RefPtr<CanvasTextMovable> create(const Glib::ustring& string = Glib::ustring(), double x = 0.0, double y = 0.0, double width = 0.0, Gtk::AnchorType anchor = Gtk::ANCHOR_NORTH_WEST);

  enum Corners
  {
    CORNER_TOP_LEFT,
    CORNER_TOP_RIGHT,
    CORNER_BOTTOM_LEFT,
    CORNER_BOTTOM_RIGHT
  };

  /** Specify the corner to be considered when snapping to a grid while moving.
   */
  void set_snap_corner(Corners corner);

  virtual void get_xy(double& x, double& y);
  virtual void set_xy(double x, double y);
  virtual void get_width_height(double& width, double& height);
  virtual void set_width_height(double width, double height);

protected:
  virtual Goocanvas::Canvas* get_parent_canvas_widget();

  virtual void snap_position(double& x, double& y) const;

  //What corner is considered when snapping to a grid while moving:
  Corners m_snap_corner;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_TEXT_MOVABLE_H

