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

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY


//Avoid including the header here:
extern "C"
{
typedef struct AvahiStringList AvahiStringList;

typedef struct _EpcServiceInfo EpcServiceInfo;
}

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

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void add_developer_action(const Glib::RefPtr<Gtk::Action>& refAction);
  void remove_developer_action(const Glib::RefPtr<Gtk::Action>& refAction);

  void update_userlevel_ui();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  AppState::userlevels get_userlevel() const;

  void fill_menu_tables();
  void fill_menu_reports(const Glib::ustring& table_name);
  void fill_menu_print_layouts(const Glib::ustring& table_name);

  void do_menu_developer_fields(Gtk::Window& parent, const Glib::ustring table_name);
  void do_menu_developer_relationships(Gtk::Window& parent, const Glib::ustring table_name);

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
  
  virtual void add_ui_from_string(const Glib::ustring& ui_description);

  bool offer_new_or_existing();

  void on_menu_help_contents();
#ifndef GLOM_ENABLE_CLIENT_ONLY
  void existing_or_new_new();
	
  void on_menu_userlevel_developer();
  void on_menu_userlevel_operator();
  void on_menu_file_save_as_example();
  void on_menu_developer_changelanguage();
  void on_menu_developer_translations();
  void on_menu_developer_active_platform_normal();
  void on_menu_developer_active_platform_maemo();
  void on_menu_developer_show_layout_toolbar();

  void on_window_translations_hide();

  virtual Glib::ustring ui_file_select_save(const Glib::ustring& old_file_uri); //overridden.
  void on_userlevel_changed(AppState::userlevels userlevel);

  Document_Glom* on_connection_pool_get_document();

  bool recreate_database(bool& user_cancelled); //return indicates success.
  void stop_self_hosting_of_document_database();
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifndef G_OS_WIN32
  /** Offer a file chooser dialog, with a Browse Network button.
   * @param browsed This will be set to true if the user chose a networked glom instance to open.
   * @browsed_server This will be filled with the server details if browsed was set to true.
   */
  Glib::ustring ui_file_select_open_with_browse(bool& browsed, EpcServiceInfo*& browsed_server, Glib::ustring& browsed_service_name, const Glib::ustring& starting_folder_uri = Glib::ustring());
#endif // !G_OS_WIN32

  virtual void on_menu_file_open(); //overridden.
  virtual void on_menu_file_close(); //override.
  virtual void document_history_add(const Glib::ustring& file_uri); //overridden.

  virtual Bakery::App* new_instance(); //Override

#ifndef G_OS_WIN32
  void open_browsed_document(const EpcServiceInfo* server, const Glib::ustring& service_name);
#endif // !G_OS_WIN32

  static Glib::ustring get_file_uri_without_extension(const Glib::ustring& uri);

  typedef Bakery::App_WithDoc_Gtk type_base;

  //Widgets:

  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup_Others;

  typedef std::list< Glib::RefPtr<Gtk::Action> > type_listActions; 
  type_listActions m_listDeveloperActions; //Only enabled when in developer mode.
  Glib::RefPtr<Gtk::Action> m_action_mode_data, m_action_mode_find;
#ifndef GLOM_ENABLE_CLIENT_ONLY
  Glib::RefPtr<Gtk::RadioAction> m_action_menu_userlevel_developer, m_action_menu_userlevel_operator;
  Glib::RefPtr<Gtk::ToggleAction> m_action_show_layout_toolbar;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  Gtk::VBox* m_pBoxTop;
  Gtk::VBox* m_pBoxSidebar;
  Frame_Glom* m_pFrame;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  Window_Translations* m_window_translations;

#endif // !GLOM_ENABLE_CLIENT_ONLY

  Glib::RefPtr<Gtk::ActionGroup> m_refNavTablesActionGroup, m_refNavReportsActionGroup, m_refNavPrintLayoutsActionGroup;
  type_listActions m_listNavTableActions, m_listNavReportActions, m_listNavPrintLayoutActions;
  Gtk::UIManager::ui_merge_id m_menu_tables_ui_merge_id, m_menu_reports_ui_merge_id, m_menu_print_layouts_ui_merge_id;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Set these before calling offer_saveas() (which uses ui_file_select_save()), and clear it afterwards.
  bool m_ui_save_extra_showextras;
  Glib::ustring m_ui_save_extra_title;
  Glib::ustring m_ui_save_extra_message;

  Glib::ustring m_ui_save_extra_newdb_title;

  bool m_ui_save_extra_newdb_selfhosted;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  // This is set to the URI of an example file that is loaded to be able to
  // prevent adding it into the recently used resources in
  // document_history_add().
  Glib::ustring m_example_uri;

  //A temporary store for the username/password if 
  //we already asked for them when getting the document over the network,
  //so we can use them again when connecting directly to the database:
  Glib::ustring m_temp_username, m_temp_password;

  bool m_show_sql_debug;
};

} //namespace Glom

#endif //HEADER_APP_GLOM
