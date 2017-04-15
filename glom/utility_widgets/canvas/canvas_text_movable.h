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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_TEXT_MOVABLE_H
#define GLOM_UTILITY_WIDGETS_CANVAS_TEXT_MOVABLE_H

#include "canvas_item_movable.h"
#include <goocanvasmm/text.h>

namespace Glom
{

class CanvasTextMovable
  : public Goocanvas::Text,
    public CanvasItemMovable
{
private:
  explicit CanvasTextMovable(const Glib::ustring& string = Glib::ustring(), double x = 0.0, double y = 0.0, double width = 0.0, Goocanvas::AnchorType anchor = Goocanvas::AnchorType::NORTH_WEST);

  void init();

public:
  static Glib::RefPtr<CanvasTextMovable> create(const Glib::ustring& string = Glib::ustring(), double x = 0.0, double y = 0.0, double width = 0.0, Goocanvas::AnchorType anchor = Goocanvas::AnchorType::NORTH_WEST);

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

  void get_xy(double& x, double& y) const override;
  void set_xy(double x, double y) override;
  void get_width_height(double& width, double& height) const override;
  void set_width_height(double width, double height) override;

  /** Use this instead of property_text() (from the base class),
   * so that the desired points size will be used.
   */
  void set_text(const Glib::ustring& text);

  /** The font name, as returned from Gtk::FontButton::get_font_name(),
   * which may include the size and style.
   * This assumes that the font size is specified in points.
   * Note that property_font() assumes that the size is in canavs units (usually mm).
   */
  void set_font_points(const Glib::ustring& font);

private:
  Goocanvas::Canvas* get_parent_canvas_widget() override;

  void snap_position(double& x, double& y) const override;

  void reconstruct_markup();

  //What corner is considered when snapping to a grid while moving:
  Corners m_snap_corner;

  //We remember this so we can reconstruct the pango markup when the text size changes:
  Glib::ustring m_text;
  Glib::ustring m_font;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_TEXT_MOVABLE_H

