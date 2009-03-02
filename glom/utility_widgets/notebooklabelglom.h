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

#ifndef GLOM_UTILITY_WIDGETS_NOTEBOOK_LABEL_GLOM_H
#define GLOM_UTILITY_WIDGETS_NOTEBOOK_LABEL_GLOM_H

#include <gtkmm.h>
#include "notebookglom.h"
#include <libglademm.h>

namespace Glom
{

class App_Glom;

class NotebookLabelGlom
: public Gtk::EventBox
{
public:
  explicit NotebookLabelGlom(NotebookGlom* notebook);
  explicit NotebookLabelGlom(const Glib::ustring& label, NotebookGlom* notebook);
  virtual ~NotebookLabelGlom();

  void set_label (const Glib::ustring& title);  
    
private:
  void init();

  virtual App_Glom* get_application();
    
  Gtk::Label m_label;
  NotebookGlom* m_notebook;
  
  void setup_menu();
  Gtk::Menu* m_pPopupMenu;
  
  void on_menu_new_group_activate();
  void on_menu_delete_activate();
    
  virtual bool on_button_press_event(GdkEventButton *event);
  
  Glib::RefPtr<Gtk::Action> m_refNewGroup;
  Glib::RefPtr<Gtk::Action> m_refDelete;  
  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
  Glib::RefPtr<Gtk::UIManager> m_refUIManager;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_NOTEBOOK_LABEL_GLOM_H

