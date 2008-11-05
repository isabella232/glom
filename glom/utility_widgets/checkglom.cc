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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "checkglom.h"
#include <gtkmm/messagedialog.h>
#include "../dialog_invalid_data.h"
#include "../application.h"
#include <glibmm/i18n.h>

#include <iostream>   // for cout, endl

namespace Glom
{

CheckGlom::CheckGlom(const Glib::ustring& title)
  : Gtk::CheckButton(title)
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_menu();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  init();
}

CheckGlom::~CheckGlom()
{
}

void CheckGlom::init()
{

}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool CheckGlom::on_button_press_event(GdkEventButton *event)
{
  //Enable/Disable items.
  //We did this earlier, but get_application is more likely to work now:
  App_Glom* pApp = get_application();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_refContextAddField);
    pApp->add_developer_action(m_refContextAddRelatedRecords);
    pApp->add_developer_action(m_refContextAddGroup);

    pApp->update_userlevel_ui(); //Update our action's sensitivity. 

    //Only show this popup in developer mode, so operators still see the default GtkCheckButton context menu.
    //TODO: It would be better to add it somehow to the standard context menu.
    if(pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
    {
      GdkModifierType mods;
      gdk_window_get_pointer( Gtk::Widget::gobj()->window, 0, 0, &mods );
      if(mods & GDK_BUTTON3_MASK)
      {
        //Give user choices of actions on this item:
        m_pMenuPopup->popup(event->button, event->time);
        return true; //We handled this event.
      }
    }
  }
  return Gtk::CheckButton::on_button_press_event(event);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

App_Glom* CheckGlom::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<App_Glom*>(pWindow);
}

void CheckGlom::set_value(const Gnome::Gda::Value& value)
{
  set_active (value.get_boolean());
}

Gnome::Gda::Value CheckGlom::get_value() const
{
  return Gnome::Gda::Value(get_active());
}

} //namespace Glom
