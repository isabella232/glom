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

#include <libgoocanvasmm.h>
#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

namespace Glom
{

class CanvasEditable : public Goocanvas::Canvas
{
public:
  CanvasEditable();
  virtual ~CanvasEditable();

  void add_item(const Glib::RefPtr<Goocanvas::Item>& item);
  void remove_all_items();

protected:
  virtual void on_show_context_menu(guint button, guint32 activate_time);

  virtual bool on_item_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);
  virtual bool on_item_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);
  virtual bool on_item_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event);

  static Glib::RefPtr<Goocanvas::Item> get_parent_container_or_self(const Glib::RefPtr<Goocanvas::Item>& item);

  bool m_dragging;
  double m_drag_x, m_drag_y;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_EDITABLE_H

