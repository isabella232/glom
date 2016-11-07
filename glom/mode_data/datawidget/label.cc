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

#include "label.h"
#include <gtkmm/messagedialog.h>
#include <glom/appwindow.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_textobject.h>
#include <glom/glade_utils.h>

namespace Glom
{

namespace DataWidgetChildren
{

Label::Label()
{
  init();
}

Label::Label(const Glib::ustring& label, bool mnemonic)
: m_label(label, mnemonic)
{
  init();
}

Label::Label(const Glib::ustring& label, Gtk::Align xalign, Gtk::Align yalign, bool mnemonic)
: m_label(label, xalign, yalign, mnemonic)
{
  init();
}

void Label::init()
{
  setup_util_menu(this);

  add(m_label);
  m_label.show();
  set_events(Gdk::ALL_EVENTS_MASK);
  //This would be more efficient if we were only using the (base) EventBox to get events,
  //but we also want to allow changing of the background color, so we don't use it: set_visible_window(false);

  //Otherwise the label could demand a huge width if there is lots of text.
  m_label.set_line_wrap();
}

AppWindow* Label::get_appwindow() const
{
  auto pWindow = const_cast<Gtk::Container*>(get_toplevel());
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<AppWindow*>(pWindow);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Label::on_menu_properties_activate()
{
  auto textobject = std::dynamic_pointer_cast<LayoutItem_Text>(m_layout_item);
  if(!textobject)
    return;

  Dialog_TextObject* dialog = nullptr;
  Glom::Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;

  dialog->set_textobject(textobject, m_table_name);
  const auto response = dialog->run();
  dialog->hide();
  if(response == Gtk::RESPONSE_OK)
  {
    //Get the chosen relationship:
    dialog->get_textobject(textobject);
  }

  signal_layout_changed().emit();

  delete dialog;
}

bool Label::on_button_press_event(GdkEventButton *button_event)
{
  auto pApp = get_appwindow();
  if(pApp && pApp->get_userlevel() == AppState::userlevels::DEVELOPER)
  {
    GdkModifierType mods;
    gdk_window_get_device_position( gtk_widget_get_window (Gtk::Widget::gobj()), button_event->device, nullptr, nullptr, &mods );
    if(mods & GDK_BUTTON3_MASK)
    {
      //Give user choices of actions on this item:
      m_popup_menu_utils->popup(button_event->button, button_event->time);
      return true; //We handled this event.
    }
  }

  return Gtk::EventBox::on_button_press_event(button_event);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Gtk::Label* Label::get_label()
{
  return &m_label;
}

} //namespace DataWidetChildren
} //namespace Glom
