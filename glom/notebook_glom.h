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

#ifndef GLOM_NOTEBOOK_GLOM_H
#define GLOM_NOTEBOOK_GLOM_H

#include <glom/utility_widgets/notebook_noframe.h>
#include <glom/box_withbuttons.h>
#include <libglom/document/document.h>

namespace Glom
{

/** Notebook with document methods.
  */
class Notebook_Glom :
  public NotebookNoFrame,
  public Base_DB
{
public: 
  Notebook_Glom();

  //virtual void show_hint();

  //Signals:
  //Page number
  //typedef sigc::signal<void, guint> type_signal_leave_page;
  // type_signal_leave_page signal_leave_page();

  //Overridden by derived classes:
  virtual void do_menu_developer_layout();
  virtual void do_menu_file_print();

protected:

  void on_show() override;

  Gtk::Window* get_app_window();

  //Signal handlers:
  virtual void on_switch_page_handler(Gtk::Widget* pPage);  //The _handler suffix is to avoid overriding the base class's method.
  //void on_leave_page(guint uiPageNumber);

  //type_signal_leave_page m_signal_leave_page; //Signals when the user leaves a page.

  bool m_destructor_in_progress; //A hack to prevent calling wrap() on dead C instances.
  sigc::connection m_connection_switch_page; //This allows us to delay connecting, and to block the handler temporarily.
};

} //namespace Glom

#endif // GLOM_NOTEBOOK_GLOM_H
