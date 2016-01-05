/* Glom
 *
 * Copyright (C) 2011 Murray Cumming
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

#ifndef GLOM_UTILITY_WIDGETS_NOTEBOOK_NOFRAME_H
#define GLOM_UTILITY_WIDGETS_NOTEBOOK_NOFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/stack.h>
#include <gtkmm/stackswitcher.h>

namespace Glom
{

class AppWindow;
class NotebookLabel;

class NotebookNoFrame
: public Gtk::Box
{
public:
  explicit NotebookNoFrame();

  void append_page(Widget& child, const Glib::ustring& name, const Glib::ustring& tab_label);

  /** This allows a workaround for GTK_IS_WIDGET(parent) failing sometimes
   * during destruction. See the comment in the code.
   */
  void remove_all_pages_without_signalling();

  Gtk::Widget* get_visible_child();
  Glib::ustring get_visible_child_name() const;
  void set_visible_child(const Glib::ustring& name);

  std::vector<Gtk::Widget*> get_page_children();

  void set_action_widget(Gtk::Widget* widget, Gtk::PackType pack_type);

  typedef sigc::signal<void, Gtk::Widget*> type_signal_switch_page;

  type_signal_switch_page signal_switch_page();

private:
  void on_visible_child_changed();

  Gtk::Box m_box_top;
  Gtk::StackSwitcher m_box_tabs;
  Gtk::Stack m_box_pages;
  Gtk::Box m_box_action_left, m_box_action_right;

  type_signal_switch_page m_signal_switch_page;
  sigc::connection m_connection_visible_child_changed;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_NOTEBOOK_NOFRAME_H
