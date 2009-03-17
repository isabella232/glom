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

#ifndef BAKERY_APP_WITHDOC_GTK_H
#define BAKERY_APP_WITHDOC_GTK_H

#include <glom/bakery/App_WithDoc.h>
#include <glom/bakery/App_Gtk.h>
#include <libglom/document/bakery/Document.h>
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
    //These are virtual base classes, with shared shared App and sigc::trackable base classes:
  : public App_WithDoc, 
    public App_Gtk
{
public:
  ///Don't forget to call init() too.
  App_WithDoc_Gtk(const Glib::ustring& appname);

  /// This constructor can be used to implement derived classes for use with Gnome::Glade::Xml::get_derived_widget().
  App_WithDoc_Gtk(BaseObjectType* cobject, const Glib::ustring& appname);

  virtual ~App_WithDoc_Gtk();

  virtual void init(); //Unique final overrider.

protected:
  virtual void init_menus_file(); //overridden to add open/save/save as.
  virtual void init_menus_file_recentfiles(const Glib::ustring& path); // call this in init_menus_file()
  virtual void init_toolbars(); //overridden to add open/save

  virtual void document_history_add(const Glib::ustring& file_uri); //overridden.
  virtual void document_history_remove(const Glib::ustring& file_uri); //overridden.

  virtual void update_window_title();

  virtual void ui_warning(const Glib::ustring& text, const Glib::ustring& secondary_text);
  virtual Glib::ustring ui_file_select_open(const Glib::ustring& starting_folder_uri = Glib::ustring());
  virtual Glib::ustring ui_file_select_save(const Glib::ustring& old_file_uri);
  virtual void ui_show_modification_status();
  virtual enumSaveChanges ui_offer_to_save_changes();

  void on_recent_files_activate(Gtk::RecentChooser& recent_chooser);

  //Menu stuff:
  Glib::RefPtr<Gtk::Action> m_action_save, m_action_saveas;
};

} //namespace

#endif //BAKERY_APP_WITHDOC_GTK_H
