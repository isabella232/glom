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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef NOTEBOOK_GLOM_H
#define NOTEBOOK_GLOM_H

#include <glom/box_withbuttons.h>
#include <gtkmm/notebook.h>
#include <gtkmm/window.h>
#include <libglom/document/document.h>

namespace Glom
{

/** Notebook with document methods.
  */
class Notebook_Glom :
  public Gtk::Notebook,
  public Base_DB
{
public: 
  Notebook_Glom();
  virtual ~Notebook_Glom();

  //virtual void show_hint();

  //Signals:
  //Page number
  //typedef sigc::signal<void, guint> type_signal_leave_page;
  // type_signal_leave_page signal_leave_page();

  //Overridden by derived classes:
  virtual void do_menu_developer_layout();
  virtual void do_menu_file_print();

protected:

  void on_show();

  Gtk::Window* get_app_window();

  //Signal handlers:
  virtual void on_switch_page_handler(GtkNotebookPage* pPage, guint uiPageNumber);  //The _handler suffix is to avoid overriding the base class's method.
  void on_leave_page(guint uiPageNumber);

  //type_signal_leave_page m_signal_leave_page; //Signals when the user leaves a page.

  guint m_uiPreviousPage;
  bool m_destructor_in_progress; //A hack to prevent calling wrap() on dead C instances.
  sigc::connection m_connection_switch_page; //This allows us to delay connecting, and to block the handler temporarily.
};

} //namespace Glom

#endif
