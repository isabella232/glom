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

#ifndef GLOM_BAKERY_APP_WITHDOC_GTK_H
#define GLOM_BAKERY_APP_WITHDOC_GTK_H

#include <glom/bakery/app_withdoc.h>
#include <config.h> // For GLOM_ENABLE_MAEMO
#include <glom/bakery/app.h>

#ifdef GLOM_ENABLE_MAEMO
#include <hildonmm/stackable-window.h>
#endif

#include <gtkmm/menubar.h>
#include <gtkmm/menu.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/handlebox.h>
#include <gtkmm/dialog.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/builder.h>


#include <libglom/document/bakery/document.h>
#include <gtkmm/toolbutton.h>
#include <gtkmm/recentmanager.h>
#include <gtkmm/recentchooser.h>

namespace GlomBakery
{

/** This class implements GlomBakery::App_WithDoc using gtkmm.
 *
 * Your application's installation should register your document's MIME-type in GNOME's (freedesktop's) MIME-type system,
 * and register your application as capable of opening documents of that MIME-type.
 * 
 *
 */
class App_WithDoc_Gtk
  : public App_WithDoc,
#ifdef GLOM_ENABLE_MAEMO
    public Hildon::StackableWindow //inherit virtually to share sigc::trackable.
#else
    public Gtk::Window //inherit virtually to share sigc::trackable.
#endif
{
public:
#ifdef GLOM_ENABLE_MAEMO
  typedef Hildon::StackableWindow ParentWindow;
#else
  typedef Gtk::Window ParentWindow;
#endif

  ///Don't forget to call init() too.
  App_WithDoc_Gtk(const Glib::ustring& appname);

  /// This constructor can be used to implement derived classes for use with Gtk::Builder::get_derived_widget().
  App_WithDoc_Gtk(BaseObjectType* cobject, const Glib::ustring& appname);

  virtual ~App_WithDoc_Gtk();

  virtual void init(); //Unique final overrider.

  /// Overidden to add a widget in the middle, under the menu, instead of replacing the whole contents.
  virtual void add(Gtk::Widget& child);

  /// For instance, to create bold primary text for a dialog box, without marking the markup for translation.
  static Glib::ustring util_bold_message(const Glib::ustring& message);

protected:
  virtual void init_layout(); //Arranges the menu, toolbar, etc.
  void init_menus_file_recentfiles(const Glib::ustring& path); // call this in init_menus_file()
  virtual void init_ui_manager(); //Override this to add more UI placeholders
  virtual void init_menus(); //Override this to add more or different menus.
  virtual void init_menus_file(); //Call this from init_menus() to add the standard file menu.
  virtual void init_menus_edit(); //Call this from init_menus() to add the standard edit menu
  virtual void init_menus_help(); //Call this from init_menus() to add the standard help menu.	
  virtual void init_toolbars();

  void add_ui_from_string(const Glib::ustring& ui_description); //Convenience function

  virtual void on_hide(); //override.

  //Overrides from App_WithDoc:
  virtual void document_history_add(const Glib::ustring& file_uri); //overridden.
  virtual void document_history_remove(const Glib::ustring& file_uri); //overridden.
  virtual void update_window_title();
  virtual void ui_warning(const Glib::ustring& text, const Glib::ustring& secondary_text);
  virtual Glib::ustring ui_file_select_open(const Glib::ustring& starting_folder_uri = Glib::ustring());
  virtual Glib::ustring ui_file_select_save(const Glib::ustring& old_file_uri);
  virtual void ui_show_modification_status();
  virtual enumSaveChanges ui_offer_to_save_changes();


  //Signal handlers:

  //Menus:

  //Overrides from App:
  virtual void on_menu_help_about();
  virtual void on_about_close();


  virtual void ui_hide();
  virtual void ui_bring_to_front();

  virtual bool on_delete_event(GdkEventAny* event); //override

  void on_recent_files_activate(Gtk::RecentChooser& recent_chooser);

  //UIManager and Actions
  Glib::RefPtr<Gtk::UIManager> m_refUIManager;
  Glib::RefPtr<Gtk::ActionGroup> m_refFileActionGroup;
  Glib::RefPtr<Gtk::ActionGroup> m_refEditActionGroup;
  Glib::RefPtr<Gtk::ActionGroup> m_refHelpActionGroup;

  //Member widgets:
  Gtk::VBox* m_pVBox;
  Gtk::VBox m_VBox_PlaceHolder;

  Gtk::Window* m_pAbout; //About box.

  //Menu stuff:
  Glib::RefPtr<Gtk::Action> m_action_save, m_action_saveas;
};

} //namespace

#endif //BAKERY_APP_WITHDOC_GTK_H
