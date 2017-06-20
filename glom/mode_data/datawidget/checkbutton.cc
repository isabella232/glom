/* Glom
 *
 * Copyright (C) 2008 Openismus GmbH
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

#include "checkbutton.h"
#include <gtkmm/messagedialog.h>
#include <glom/dialog_invalid_data.h>
#include <glom/appwindow.h>
#include <glom/utils_ui.h>

namespace Glom
{

namespace DataWidgetChildren
{

CheckButton::CheckButton(const Glib::ustring& title)
  : Gtk::CheckButton(title)
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_menu(this);
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool CheckButton::on_button_press_event(Gdk::EventButton& button_event)
{
  //Enable/Disable items.
  //We did this earlier, but get_appwindow is more likely to work now:
  auto pApp = get_appwindow();
  if(pApp)
  {
    pApp->add_developer_action(m_context_layout); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_context_add_field);
    pApp->add_developer_action(m_context_add_related_records);
    pApp->add_developer_action(m_context_add_group);

    pApp->update_userlevel_ui(); //Update our action's sensitivity.

    //Only show this popup in developer mode, so operators still see the default GtkCheckButton context menu.
    //TODO: It would be better to add it somehow to the standard context menu.
    if(pApp->get_userlevel() == AppState::userlevels::DEVELOPER)
    {
      if(UiUtils::popup_menu_if_button3_click(*this, *m_menu_popup, button_event))
        return true;
    }
  }

  return Gtk::CheckButton::on_button_press_event(button_event);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

AppWindow* CheckButton::get_appwindow() const
{
  auto pWindow = const_cast<Gtk::Container*>(get_toplevel());
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<AppWindow*>(pWindow);
}

void CheckButton::set_value(const Gnome::Gda::Value& value)
{
  set_active (value.get_boolean());
}

Gnome::Gda::Value CheckButton::get_value() const
{
  return Gnome::Gda::Value(get_active());
}

} //namespace DataWidetChildren
} //namespace Glom
