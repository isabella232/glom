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

#include "notebooklabelglom.h"
#include <glom/appwindow.h>
#include <glibmm/i18n.h>

#include <iostream>

namespace Glom
{

NotebookLabel::NotebookLabel(NotebookGlom* notebook)
: m_notebook(notebook),
  m_popup_menu(nullptr)
{
  init();
}

NotebookLabel::NotebookLabel(const Glib::ustring& label, NotebookGlom* notebook)
: m_label(label),
  m_notebook (notebook),
  m_popup_menu(nullptr)
{
  init();
}

void NotebookLabel::init()
{
  add(m_label);
  m_label.show();
  set_events (Gdk::ALL_EVENTS_MASK);
  set_visible_window (false);
  setup_menu(this);
}

void NotebookLabel::set_label (const Glib::ustring& title)
{
  m_label.set_label (title);
}

AppWindow* NotebookLabel::get_appwindow()
{
  auto pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<AppWindow*>(pWindow);
}

void NotebookLabel::on_menu_new_group_activate()
{
  auto group = std::make_shared<LayoutGroup>();
  group->set_title_original(_("New Group"));
  group->set_name(_("Group"));

  auto notebook_group = std::dynamic_pointer_cast<LayoutGroup>(m_notebook->get_layout_item());
  notebook_group->add_item(group);

  m_notebook->signal_layout_changed().emit();
}

void NotebookLabel::on_menu_delete_activate()
{
  Glib::ustring message;
  const auto notebook_title = item_get_title(m_notebook->get_layout_item());
  if(!notebook_title.empty())
  {
    message = Glib::ustring::compose (_("Delete whole notebook \"%1\"?"),
                                      notebook_title);
  }
  else
  {
    message = _("Delete whole notebook?");
  }

  Gtk::MessageDialog dlg (message, false, Gtk::MESSAGE_QUESTION,
                          Gtk::BUTTONS_YES_NO, true);
  switch(dlg.run())
  {
    case Gtk::RESPONSE_YES:
      m_notebook->delete_from_layout();
      break;
    default:
      return;
  }
}

void NotebookLabel::setup_menu(Gtk::Widget* /* widget */)
{
  m_action_group = Gio::SimpleActionGroup::create();

  m_new_group = m_action_group->add_action("new-group",
    sigc::mem_fun(*this, &NotebookLabel::on_menu_new_group_activate) );
  m_deelete = m_action_group->add_action("delete",
    sigc::mem_fun(*this, &NotebookLabel::on_menu_delete_activate) );

  insert_action_group("context", m_action_group);

  auto menu = Gio::Menu::create();
  menu->append(_("New Group"), "context.new-group");
  menu->append(_("_Delete"), "context.delete");

  m_popup_menu = std::make_unique<Gtk::Menu>(menu);
  m_popup_menu->attach_to_widget(*this);
}

bool NotebookLabel::on_button_press_event(GdkEventButton *button_event)
{
  auto pApp = get_appwindow();
  if(pApp && pApp->get_userlevel() == AppState::userlevels::DEVELOPER)
  {
    GdkModifierType mods;
    gdk_window_get_device_position( gtk_widget_get_window (Gtk::Widget::gobj()), button_event->device, 0, 0, &mods );
    if(mods & GDK_BUTTON3_MASK)
    {
      //Give user choices of actions on this item:
      m_popup_menu->popup(button_event->button, button_event->time);
      return true; //We handled this event.
    }
  }

  return Gtk::EventBox::on_button_press_event(button_event);
}

} //namespace Glom
