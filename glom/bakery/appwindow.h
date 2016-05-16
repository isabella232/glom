/*
 * Copyright 2000 Murray Cumming
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef GLOM_BAKERY_APP_H
#define GLOM_BAKERY_APP_H

#include <glibmm/object.h>

#include <vector>

namespace GlomBakery
{

/** Bakery's Main Window.
 * This is an abstract class. You must use a class such as AppWindow_Gtk, which implements
 * the ui_* methods for a particular GUI toolkit.
 *
 * Features:
 * - Override methods to add/change menus/toolbars/statusbar.
 *   - Default is basic File, Edit, Help menus and toolbar icons.
 *

 *
 * TODO:
 * - Command-line args - wrap popt?
 * - Session Management - need Command-line args.
 *
 */

class AppWindow : virtual public Glib::ObjectBase
{
public:

  //The constructor has a default argument so that there is a default constructor,
  //so that derived classes do not need to call a specific constructor. This is
  //a virtual base class so they would otherwise need to do that.

  ///Don't forget to call init() too.
  explicit AppWindow(const Glib::ustring& appname = Glib::ustring());

  virtual void init(); //Sets it up and shows it.

  static void set_command_line_args(int argc, char** &argv); //Needed for session management.

  typedef sigc::signal<void()> type_signal_hide;
  type_signal_hide ui_signal_hide();

protected:
  static void init_app_name(const Glib::ustring& appname);

  /** Override this to add more menus or different menus.
   */
  virtual void init_menus();

  /** Call this from init_menus() to add the standard file menu
   */
  virtual void init_menus_file() = 0;

  /** Call this from init_menus() to add the standard edit menu
   */
  virtual void init_menus_edit() = 0;

  virtual void new_instance(const Glib::ustring& uri = Glib::ustring()) = 0; //Must override in order to new() the derived document class.

//  virtual void close_window() = 0;
//  virtual void bring_to_front() = 0;
  //Signal handlers:

public: // We can not take function pointers of these methods in a
        // derived class, if they are protected - for instance, with sigc::mem_fun()
  //Menus:
  virtual void on_menu_file_new();
  virtual void on_menu_file_close();

  //Edit menu handlers overriden in AppWindow_WithDoc:
  virtual void on_menu_edit_cut();
  virtual void on_menu_edit_copy();
  virtual void on_menu_edit_paste();
  virtual void on_menu_edit_clear();

  virtual void on_menu_help_about() = 0;

protected:
  //GUI abstractions:
  virtual void ui_hide() = 0;
  virtual void ui_bring_to_front() = 0;

  //operation_cancelled:
  //e.g. A File|Open tries to save existing data,
  //but this needs to be cancelled if the save failed.
  static void set_operation_cancelled(bool value = true);
  static bool get_operation_cancelled();

  //Member data:

  //'About Box'/WM Class information:
  static Glib::ustring m_strAppName;

  //Instances

  static bool m_operation_cancelled; //see set/get_operation_cancelled().

  //Command line args:
  static Glib::ustring m_strCommandLine_0;

  type_signal_hide m_signal_hide;
};

} //namespace

#endif //BAKERY_APP_H
