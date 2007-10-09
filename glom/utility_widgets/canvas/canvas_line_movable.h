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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_LINE_MOVABLE_H
#define GLOM_UTILITY_WIDGETS_CANVAS_LINE_MOVABLE_H


#include "canvas_item_movable.h"
#include <libgoocanvasmm/polyline.h>

namespace Glom
{

class CanvasLineMovable
  : public Goocanvas::Polyline,
    public CanvasItemMovable
{
protected:
  CanvasLineMovable();
  virtual ~CanvasLineMovable();

  virtual void get_xy(double& x, double& y);
  virtual void move(double x, double y);

public:
  static Glib::RefPtr<CanvasLineMovable> create();

protected:
  virtual Goocanvas::Canvas* get_parent_canvas_widget();

};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_LINE_MOVABLE_H

