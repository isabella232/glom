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
#include "bakery/View/View.h"
#include <bakery/Utilities/BusyCursor.h>
#include <libglademm.h>

/**
  *@author Murray Cumming
  */

class Box_DB :
  public Gtk::VBox,
  public View_Composite_Glom
{
public: 
  Box_DB();

  ///For use with libglademm's get_widget_derived():
  Box_DB(BaseObjectType* cobject);
  
  virtual ~Box_DB();
  
  virtual void initialize(const Glib::ustring& strDatabaseName);

  /** Returns whether we are in developer mode.
   * Some functionality will be deactivated when not in developer mode.
   */
  virtual AppState::userlevels get_userlevel() const;
  virtual void set_userlevel(AppState::userlevels value);
   
  static sharedptr<SharedConnection> connect_to_server();
  
  virtual Glib::ustring get_databaseName();

  virtual void load_from_document(); //View override

  Gtk::Window* get_app_window();
  const Gtk::Window* get_app_window() const;
  
  void show_hint(); //Public so that it can be called *after* this widget is added to its container.

  void set_button_cancel(Gtk::Button& button);
  
  //Signals:
  sigc::signal<void, Glib::ustring> signal_selected; //When an item is selected.
  sigc::signal<void> signal_cancelled; //When the cancel button is clicked.

  //Signal handlers:
  virtual void on_Button_Cancel();

  typedef std::vector< Field > type_vecFields;
    
protected:
  typedef std::vector<Glib::ustring> type_vecStrings;
  type_vecStrings get_table_names();

  virtual void fill_from_database();
  virtual void fill_end(); //Call this from the end of fill_from_database() overrides.

  virtual void on_userlevel_changed(AppState::userlevels userlevel);

  Glib::ustring util_string_from_decimal(guint decimal);
  guint util_decimal_from_string(const Glib::ustring& str);
  
  virtual void handle_error(const std::exception& ex); //TODO_port: This is probably useless now.
  virtual void handle_error();

  virtual Glib::RefPtr<Gnome::Gda::DataModel> Query_execute(const Glib::ustring& strQuery);

  virtual void hint_set(const Glib::ustring& strText);

  //Member data:
  Glib::ustring m_strDatabaseName;

  Glib::ustring m_strHint; //Help text.

  
  Gtk::HBox m_Box_Buttons;
  Gtk::Button m_Button_Cancel; //Derived classes can use it if it's necessary.
};

#endif //BOX_DB_H
