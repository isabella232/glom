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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_LINE_MOVABLE_H
#define GLOM_UTILITY_WIDGETS_CANVAS_LINE_MOVABLE_H


#include "canvas_item_movable.h"
#include <goocanvasmm/polyline.h>

namespace Glom
{

class CanvasLineMovable
  : public Goocanvas::Polyline,
    public CanvasItemMovable
{
protected:
  CanvasLineMovable();

public:
  static Glib::RefPtr<CanvasLineMovable> create();

  void set_hover_color(const Glib::ustring& color);

  void get_xy(double& x, double& y) const override;
  void set_xy(double x, double y) override;
  void get_width_height(double& width, double& height) const override;
  void set_width_height(double width, double height) override;

private:
  Goocanvas::Canvas* get_parent_canvas_widget() override;

  bool on_enter_notify_event(const Glib::RefPtr<Item>& target, Gdk::EventCrossing& event) override;
  bool on_leave_notify_event(const Glib::RefPtr<Item>& target, Gdk::EventCrossing& event) override;

  Gdk::RGBA m_stroke_color;
  Glib::ustring m_hover_color;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_LINE_MOVABLE_H

