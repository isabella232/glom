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


namespace Glom
{

class Window_Translations;

class App_Glom : public Bakery::App_WithDoc_Gtk
{
public:
  App_Glom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~App_Glom();

  virtual bool init(const Glib::ustring& document_uri = Glib::ustring()); //override

  //virtual void statusbar_set_text(const Glib::ustring& strText);
  //virtual void statusbar_clear();

  /// Get the UIManager so we can merge new menus in.
  Glib::RefPtr<Gtk::UIManager> get_ui_manager();

  /** Changes the mode to Data mode, as if the user had selected the Data Mode menu item.
   */
  void set_mode_data();
  void set_mode_find();

  void add_developer_action(const Glib::RefPtr<Gtk::Action>& refAction);
  void remove_developer_action(const Glib::RefPtr<Gtk::Action>& refAction);

  AppState::userlevels get_userlevel() const;

  void update_userlevel_ui();
  void fill_menu_tables();
  void fill_menu_reports(const Glib::ustring& table_name);

  ///Whether to show the generated SQL queries on stdout, for debugging.
  bool get_show_sql_debug() const;

  ///Whether to show the generated SQL queries on stdout, for debugging.
  void set_show_sql_debug(bool val = true);

  static App_Glom* get_application();

protected:
  virtual void init_layout(); //override.
  virtual void init_menus_file(); //override.
  virtual void init_menus(); //override.
  virtual void init_menus_help(); //override
  virtual void init_toolbars(); //override
  virtual void init_create_document(); //override
  virtual bool on_document_load(); //override.

  virtual bool offer_new_or_existing();

  virtual void on_menu_help_contents();
  virtual void on_menu_userlevel_developer();
  virtual void on_menu_userlevel_operator();
  virtual void on_menu_file_save_as_example();
  virtual void on_menu_developer_changelanguage();
  virtual void on_menu_developer_translations();
  virtual void on_window_translations_hide();

  virtual void on_userlevel_changed(AppState::userlevels userlevel);

  virtual Bakery::App* new_instance(); //Override

  virtual bool recreate_database(bool& user_cancelled); //return indicates success.

  typedef Bakery::App_WithDoc_Gtk type_base;

  //Widgets:

  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup_Others;

  typedef std::list< Glib::RefPtr<Gtk::Action> > type_listActions; 
  type_listActions m_listDeveloperActions; //Only enabled when in developer mode.
  Glib::RefPtr<Gtk::Action> m_action_mode_data, m_action_mode_find;
  Glib::RefPtr<Gtk::RadioAction> m_action_menu_userlevel_developer, m_action_menu_userlevel_operator;

  Gtk::VBox* m_pBoxTop;
  Frame_Glom* m_pFrame;

  Window_Translations* m_window_translations;

  Glib::RefPtr<Gtk::ActionGroup> m_refNavTablesActionGroup, m_refNavReportsActionGroup;
  type_listActions m_listNavTableActions, m_listNavReportActions;
  Gtk::UIManager::ui_merge_id m_menu_tables_ui_merge_id, m_menu_reports_ui_merge_id;

  bool m_show_sql_debug;
};

} //namespace Glom

#endif //HEADER_APP_GLOM
