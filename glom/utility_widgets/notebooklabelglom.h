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

#ifndef GLOM_UTILITY_WIDGETS_NOTEBOOK_LABEL_GLOM_H
#define GLOM_UTILITY_WIDGETS_NOTEBOOK_LABEL_GLOM_H

#include "notebookglom.h"
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>
#include <gtkmm/menu.h>
#include <giomm/simpleactiongroup.h>

namespace Glom
{

class AppWindow;

class NotebookLabel
: public Gtk::EventBox
{
public:
  explicit NotebookLabel(NotebookGlom* notebook);
  explicit NotebookLabel(const Glib::ustring& label, NotebookGlom* notebook);

  void set_label(const Glib::ustring& title);

private:
  void init();

  virtual AppWindow* get_appwindow();

  Gtk::Label m_label;
  NotebookGlom* m_notebook;

  void setup_menu(Gtk::Widget* widget);
  std::unique_ptr<Gtk::Menu> m_popup_menu;

  void on_menu_new_group_activate();
  void on_menu_delete_activate();

  bool on_button_press_event(Gdk::EventButton& event) override;

  Glib::RefPtr<Gio::SimpleAction> m_new_group;
  Glib::RefPtr<Gio::SimpleAction> m_deelete;
  Glib::RefPtr<Gio::SimpleActionGroup> m_action_group;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_NOTEBOOK_LABEL_GLOM_H

