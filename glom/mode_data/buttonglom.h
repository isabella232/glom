/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
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

#ifndef GLOM_UTILITY_WIDGETS_BUTTON_GLOM_H
#define GLOM_UTILITY_WIDGETS_BUTTON_GLOM_H

#include <gtkmm/button.h>
#include <glom/utility_widgets/layoutwidgetutils.h>
#include <libglom/data_structure/layout/layoutitem_button.h>
#include <gtkmm/builder.h>

namespace Glom
{

class AppWindow;

class ButtonGlom
: public Gtk::Button,
  public LayoutWidgetUtils
{
public:
  explicit ButtonGlom(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  ButtonGlom();
  virtual ~ButtonGlom();

private:
  void init();

  AppWindow* get_appwindow() const override;

#ifndef GLOM_ENABLE_CLIENT_ONLY    
  void on_menu_properties_activate() override;
  bool on_button_press_event(GdkEventButton *event) override;
#endif // !GLOM_ENABLE_CLIENT_ONLY
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_BUTTON_GLOM_H

