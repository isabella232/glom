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
 
#ifndef HEADER_APP_GLOM
#define HEADER_APP_GLOM

#include "bakery/bakery.h"
#include "frame_glom.h"

class App_Glom : public Bakery::App_WithDoc_Gtk
{
public:
  App_Glom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~App_Glom();

  virtual bool init(const Glib::ustring& document_uri = Glib::ustring()); //override

  virtual void statusbar_set_text(const Glib::ustring& strText);
  virtual void statusbar_clear();

  /// Get the UIManager so we can merge new menus in.
  Glib::RefPtr<Gtk::UIManager> get_ui_manager();

  /** Changes the mode to Data mode, as if the user had selected the Data Mode menu item.
   */
  virtual void set_mode_data();

  virtual AppState::userlevels get_userlevel() const;
    
protected:
  virtual void init_layout(); //override.
  virtual void init_menus(); //override.
  virtual void init_menus_help(); //override
  virtual void init_create_document(); //override
  virtual void on_document_load(); //override.

  virtual bool offer_new_or_existing();

  virtual void on_menu_help_contents();
  virtual void on_menu_userlevel_developer();
  virtual void on_menu_userlevel_operator();

  virtual void on_userlevel_changed(AppState::userlevels userlevel);

  virtual Bakery::App* new_instance(); //Override

  typedef Bakery::App_WithDoc_Gtk type_base;

  //Widgets:

  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup_Others;

  typedef std::list< Glib::RefPtr<Gtk::Action> > type_listActions; 
  type_listActions m_listDeveloperActions; //Only enabled when in developer mode.
  Glib::RefPtr<Gtk::Action> m_action_mode_data;
  Glib::RefPtr<Gtk::RadioAction> m_action_menu_userlevel_developer, m_action_menu_userlevel_operator;
  
  Gtk::VBox* m_pBoxTop;
  Frame_Glom* m_pFrame;
  Gtk::Label* m_pStatus;
};

#endif //HEADER_APP_GLOM
