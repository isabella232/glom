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

#ifndef BAKERY_APP_GTK_H
#define BAKERY_APP_GTK_H

#include <config.h> // For BAKERY_MAEMO_ENABLED
#include <glom/bakery/App.h>

#ifdef BAKERY_MAEMO_ENABLED
#include <hildonmm/window.h>
#endif

#include <gtkmm/menubar.h>
#include <gtkmm/menu.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/handlebox.h>
#include <gtkmm/dialog.h>
#include <gtkmm/uimanager.h>
#include <libglademm.h>

namespace GlomBakery
{

/** This class implements GlomBakery::App using gtkmm.
 *
 * Features:
 * - Override methods to add/change menus/toolbars/statusbar.
 *   - Default is basic File, Edit, Help menus and toolbar icons.
 */
class App_Gtk
  : virtual public App, //virtual because App_WithDoc_Gtk will inherit it via App_Gtk and via App_With_Doc
#ifdef BAKERY_MAEMO_ENABLED
    virtual public Hildon::Window //inherit virtually to share sigc::trackable.
#else
    virtual public Gtk::Window //inherit virtually to share sigc::trackable.
#endif
{
public:
#ifdef BAKERY_MAEMO_ENABLED
  typedef Hildon::Window ParentWindow;
#else
  typedef Gtk::Window ParentWindow;
#endif
  friend class AppInstanceManager;

  ///Don't forget to call init() too.
  App_Gtk(const Glib::ustring& appname);

  /// This constructor can be used to implement derived classes for use with Gnome::Glade::Xml::get_derived_widget().
  App_Gtk(BaseObjectType* cobject, const Glib::ustring& appname);

  virtual ~App_Gtk();

  /// Overidden to add a widget in the middle, under the menu, instead of replacing the whole contents.
  virtual void add(Gtk::Widget& child);

  /// For instance, to create bold primary text for a dialog box, without marking the markup for translation.
  static Glib::ustring util_bold_message(const Glib::ustring& message);

protected:

  virtual void init(); //Override to show().
  virtual void init_ui_manager(); //Override this to add more UI placeholders
  virtual void init_menus(); //Override this to add more or different menus.
  virtual void init_menus_file(); //Call this from init_menus() to add the standard file menu.
  virtual void init_menus_edit(); //Call this from init_menus() to add the standard edit menu
  virtual void init_menus_help(); //Call this from init_menus() to add the standard help menu.	
  virtual void init_toolbars();

  virtual void init_layout(); //Arranges the menu, toolbar, etc.

  virtual void add_ui_from_string(const Glib::ustring& ui_description); //Convenience function

  virtual void on_hide(); //override.

  //Signal handlers:

  //Menus:
  virtual void on_menu_help_about();

  virtual void on_about_close();


  virtual void ui_hide();
  virtual void ui_bring_to_front();

  virtual bool on_delete_event(GdkEventAny* event); //override

  //virtual void destroy_and_remove_from_list();

  //Member data:

  //UIManager and Actions
  Glib::RefPtr<Gtk::UIManager> m_refUIManager;
  Glib::RefPtr<Gtk::ActionGroup> m_refFileActionGroup;
  Glib::RefPtr<Gtk::ActionGroup> m_refEditActionGroup;
  Glib::RefPtr<Gtk::ActionGroup> m_refHelpActionGroup;

  //Member widgets:
  Gtk::VBox* m_pVBox;
  Gtk::VBox m_VBox_PlaceHolder;

  //Gtk::MenuBar m_MenuBar;
  //Gtk::Menu m_Menu_File, m_Menu_Edit, m_Menu_Help;

  Gtk::HandleBox m_HandleBox_Toolbar;
  //Gtk::Toolbar m_Toolbar;

  //All instances share 1 About box:
  static Gtk::Window* m_pAbout; //About box.


  //typedef std::vector<poptOption> type_vecPoptOptions;
  //type_vecPoptOptions m_vecPoptOptions;
};

} //namespace

#endif //BAKERY_APP_GTK_H
