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

#include "buttonglom.h"
#include <gtkmm/messagedialog.h>
#include <glom/appwindow.h>
#include <glom/glade_utils.h>
#include <glom/utils_ui.h>
#ifndef GLOM_ENABLE_CLIENT_ONLY
#include <glom/mode_design/layout/layout_item_dialogs/dialog_buttonscript.h>
#endif
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

namespace Glom
{

ButtonGlom::ButtonGlom(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& /* builder */)
: Gtk::Button(cobject)
{
  init();
}

ButtonGlom::ButtonGlom()
{
  init();
}

ButtonGlom::~ButtonGlom()
{
}

void ButtonGlom::init()
{
  setup_util_menu(this);
}

AppWindow* ButtonGlom::get_appwindow() const
{
  Gtk::Container* pWindow = const_cast<Gtk::Container*>(get_toplevel());
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<AppWindow*>(pWindow);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void ButtonGlom::on_menu_properties_activate()
{
  Dialog_ButtonScript* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;

  std::shared_ptr<LayoutItem_Button> layout_item = 
    std::dynamic_pointer_cast<LayoutItem_Button>(get_layout_item());
  dialog->set_script(layout_item, m_table_name);
  const auto response = Glom::UiUtils::dialog_run_with_help(dialog);
  dialog->hide();
  if(response == Gtk::RESPONSE_OK)
  {
    dialog->get_script(layout_item);
    signal_layout_changed().emit();
  }

  delete dialog;
}

bool ButtonGlom::on_button_press_event(GdkEventButton *button_event)
{
  AppWindow* pApp = get_appwindow();
  if(pApp && pApp->get_userlevel() == AppState::userlevels::DEVELOPER)
  {
    GdkModifierType mods;
    gdk_window_get_device_position( gtk_widget_get_window (Gtk::Widget::gobj()), button_event->device, 0, 0, &mods );
    if(mods & GDK_BUTTON3_MASK)
    {
      //Give user choices of actions on this item:
      m_pPopupMenuUtils->popup(button_event->button, button_event->time);
      return true; //We handled this event.
    }
  }
  return Gtk::Button::on_button_press_event(button_event);
}
#endif

} //namespace Glom
