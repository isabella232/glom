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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLOM_UTILITY_WIDGETS_NOTEBOOK_NOFRAME_H
#define GLOM_UTILITY_WIDGETS_NOTEBOOK_NOFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/togglebutton.h>

namespace Glom
{

class AppWindow;
class NotebookLabel;

class NotebookNoFrame
: public Gtk::Box
{
public:
  explicit NotebookNoFrame();
  virtual ~NotebookNoFrame();

  int append_page(Gtk::Widget& child, Gtk::Widget& tab_label);
  /*
  int append_page(Widget& child);
  */
  int append_page(Widget& child, const Glib::ustring& tab_label, bool use_mnemonic = false);

  Widget* get_nth_page(int page_num);
  const Widget* get_nth_page(int page_num) const;

  int get_current_page() const;
  void set_current_page(int page_num);

  void set_action_widget(Gtk::Widget* widget, Gtk::PackType pack_type);

  typedef sigc::signal<void, Gtk::Widget*, guint> type_signal_switch_page;

  type_signal_switch_page signal_switch_page();

protected:
  void on_tab_toggled(int index);

  Gtk::Box m_box_tabs;
  Gtk::Box m_box_pages;
  Gtk::Box m_box_action_left, m_box_action_right;

  type_signal_switch_page m_signal_switch_page;

  //Caching the widget pointers is nicer than repeatedly calling Gtk::Container::get_children().
  typedef std::vector<Gtk::ToggleButton*> type_vec_togglebuttons;
  type_vec_togglebuttons m_vec_tab_widgets;

  typedef std::vector<Gtk::Box*> type_vec_widgets;
  type_vec_widgets m_vec_page_widgets;

  int m_current_page;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_NOTEBOOK_NOFRAME_H
