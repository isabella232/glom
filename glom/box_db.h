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

#ifndef BOX_DB_H
#define BOX_DB_H

#include "gtkmm.h"
#include "utility_widgets/adddel/adddel_withbuttons.h"

#include "document/document_glom.h"
#include "connectionpool.h"
#include "appstate.h"
#include "base_db.h"
#include <bakery/Utilities/BusyCursor.h>
#include <libglademm.h>

/**
  *@author Murray Cumming
  */

class Box_DB :
  public Gtk::VBox,
  public Base_DB
{
public: 
  Box_DB();
  Box_DB(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

  ///For use with libglademm's get_widget_derived():
  Box_DB(BaseObjectType* cobject);
  
  virtual ~Box_DB();
  
  Gtk::Window* get_app_window();
  const Gtk::Window* get_app_window() const;
  
  void show_hint(); //Public so that it can be called *after* this widget is added to its container.

  void set_button_cancel(Gtk::Button& button);
  
  //Signals:
  sigc::signal<void, Glib::ustring> signal_selected; //When an item is selected.
  sigc::signal<void> signal_cancelled; //When the cancel button is clicked.

  //Signal handlers:
  virtual void on_Button_Cancel();

  virtual Gtk::Widget* get_default_button();


protected:

  virtual void hint_set(const Glib::ustring& strText);

  //Member data:
  Glib::ustring m_strHint; //Help text.

  
  Gtk::HBox m_Box_Buttons;
  Gtk::Button m_Button_Cancel; //Derived classes can use it if it's necessary.
};

#endif //BOX_DB_H
