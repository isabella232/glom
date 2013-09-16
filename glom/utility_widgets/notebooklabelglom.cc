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
#include <gtkmm/menu.h>
#include <giomm/menu.h>
#include <glibmm/i18n.h>

#include <iostream>

namespace Glom
{

NotebookLabel::NotebookLabel(NotebookGlom* notebook) 
: m_notebook(notebook),
  m_pPopupMenu(0)
{
  init();
}

NotebookLabel::NotebookLabel(const Glib::ustring& label, NotebookGlom* notebook)
: m_label(label),
  m_notebook (notebook),
  m_pPopupMenu(0)
{
  init();
}

NotebookLabel::~NotebookLabel()
{

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
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<AppWindow*>(pWindow);
}

void NotebookLabel::on_menu_new_group_activate()
{
  sharedptr<LayoutGroup> group(new LayoutGroup());
  group->set_title_original(_("New Group"));
  group->set_name(_("Group"));
  
  sharedptr<LayoutGroup> notebook_group = sharedptr<LayoutGroup>::cast_dynamic (m_notebook->get_layout_item());
  notebook_group->add_item(group);
  
  m_notebook->signal_layout_changed().emit();
}

void NotebookLabel::on_menu_delete_activate()
{
  Glib::ustring message;
  const Glib::ustring notebook_title = item_get_title(m_notebook->get_layout_item());
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
  m_refActionGroup = Gio::SimpleActionGroup::create();

  m_refNewGroup = m_refActionGroup->add_action("new-group",
    sigc::mem_fun(*this, &NotebookLabel::on_menu_new_group_activate) );
  m_refDelete = m_refActionGroup->add_action("delete",
    sigc::mem_fun(*this, &NotebookLabel::on_menu_delete_activate) );
 
  insert_action_group("context", m_refActionGroup);


  Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create();

  const Glib::ustring ui_info =
    "<interface>"
    "  <menu id='ContextMenu'>"
    "    <section>"
    "      <item>"
    "        <attribute name='label' translatable='yes'>New Group</attribute>"
    "        <attribute name='action'>context.new-group</attribute>"
    "      </item>"
    "      <item>"
    "        <attribute name='label' translatable='yes'>_Delete</attribute>"
    "        <attribute name='action'>context.delete</attribute>"
    "      </item>"
    "    </section>"
    "  </menu>"
    "</interface>";

  try
  {
    builder->add_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": building menus failed: " <<  ex.what() << std::endl;
  }

  //Get the menu:
  Glib::RefPtr<Glib::Object> object =
    builder->get_object("ContextMenu");
  Glib::RefPtr<Gio::Menu> gmenu =
    Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
  if(!gmenu)
    g_warning("GMenu not found");

  m_pPopupMenu = new Gtk::Menu(gmenu);
}

bool NotebookLabel::on_button_press_event(GdkEventButton *event)
{
  AppWindow* pApp = get_appwindow();
  if(pApp && pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  {
    GdkModifierType mods;
    gdk_window_get_device_position( gtk_widget_get_window (Gtk::Widget::gobj()), event->device, 0, 0, &mods );
    if(mods & GDK_BUTTON3_MASK)
    {
      //Give user choices of actions on this item:
      m_pPopupMenu->popup(event->button, event->time);
      return true; //We handled this event.
    }
  }
  return Gtk::EventBox::on_button_press_event(event);
}

} //namespace Glom
