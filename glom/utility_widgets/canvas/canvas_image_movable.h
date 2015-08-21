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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_IMAGE_MOVABLE_H
#define GLOM_UTILITY_WIDGETS_CANVAS_IMAGE_MOVABLE_H

#include "canvas_item_movable.h"
#include <goocanvasmm/image.h>

namespace Glom
{

class CanvasImageMovable
  : public Goocanvas::Image,
    public CanvasItemMovable
{
private:
  explicit CanvasImageMovable(double x = 0.0, double y = 0.0);
  explicit CanvasImageMovable(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, double x = 0.0, double y = 0.0);
  virtual ~CanvasImageMovable();

  void init();

public:
  static Glib::RefPtr<CanvasImageMovable> create(double x = 0.0, double y = 0.0);
  static Glib::RefPtr<CanvasImageMovable> create(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, double x = 0.0, double y = 0.0);

  /** Use this instead of property_pixbuf(), 
   * to make sure that m_image_empty is set to false.
   *
   * This also scales the image (maintaining the aspect ratio) to fit the current width and 
   * height if they are not 0.
   */
  void set_image(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, bool scale = true);

  /// Show the no-image picture. 
  void set_image_empty();
  bool get_image_empty() const;

  /** Scale the pixbuf to the current height and width, keeping the aspect ratio.
   * This uses the original pixbuf provided to set_image(), so this should not 
   * result in a loss of quality if the original was large enough.
   */
  void scale_to_size();

  enum class Corners
  {
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT
  };

  /** Specify the corner to be considered when snapping to a grid while moving.
   */
  void set_snap_corner(Corners corner);

  virtual void get_xy(double& x, double& y) const;
  virtual void set_xy(double x, double y);
  virtual void get_width_height(double& width, double& height) const;
  virtual void set_width_height(double width, double height);

private:
  virtual Goocanvas::Canvas* get_parent_canvas_widget();

  virtual void snap_position(double& x, double& y) const;

  //What corner is considered when snapping to a grid while moving:
  Corners m_snap_corner;

  //Whether we are showing the no-image picture:
  bool m_image_empty;

  //We keep a copy of this here because
  //- GooCanvasImage doesn't let use read the pixbuf property (just write it),
  //- This allows us to rescale (if wanted) when resizing, without losing quality.
  Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_IMAGE_MOVABLE_H

