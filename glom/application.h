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

#ifndef GLOM_APPLICATION_H
#define GLOM_APPLICATION_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <glom/bakery/app_withdoc_gtk.h>

//TODO: Remove this when maemomm's gtkmm has been updated. 7th September 2009.
#ifdef GLOM_ENABLE_MAEMO
//Because earlier versions of gtkmm/enums.h did not include this, so
//GTKMM_MAEMO_EXTENSIONS_ENABLED was not defined,
//leading to a compiler error in hildonmm/button.h
#include <gtkmmconfig.h>
#include <gtkmm/enums.h>

#include <hildonmm/app-menu.h>
#include <glom/navigation/maemo/pickerbutton_table.h>
#endif //GLOM_ENABLE_MAEMO

#include <glom/frame_glom.h>



//Avoid including the header here:
extern "C"
{
typedef struct AvahiStringList AvahiStringList;

typedef struct _EpcServiceInfo EpcServiceInfo;
}

namespace Glom
{

class Window_Translations;
class Dialog_ProgressCreating; //TODO: Rename this because it's not just about creating databases.

class Application : public GlomBakery::App_WithDoc_Gtk
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Application(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Application();

  virtual bool init(const Glib::ustring& document_uri = Glib::ustring()); //override

  /**
   * @param restore Whether @a document_uri is a .tar.gz backup file to restore.
   */
  bool init(const Glib::ustring& document_uri, bool restore);

  //virtual void statusbar_set_text(const Glib::ustring& strText);
  //virtual void statusbar_clear();

  /// Get the UIManager so we can merge new menus in.
  #ifndef GLOM_ENABLE_MAEMO
  Glib::RefPtr<Gtk::UIManager> get_ui_manager();
  #else
  Hildon::AppMenu* get_maemo_appmenu();
  #endif //GLOM_ENABLE_MAEMO

  /** Changes the mode to Data mode, as if the user had selected the Data Mode menu item.
   */
  void set_mode_data();
  void set_mode_find();

  /** Show in the UI whether the database is shared on the network.
   */
  void update_network_shared_ui();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void add_developer_action(const Glib::RefPtr<Gtk::Action>& refAction);
  void remove_developer_action(const Glib::RefPtr<Gtk::Action>& refAction);

  /** Show in the UI whether the document is in developer or operator mode.
   */
  void update_userlevel_ui();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  /** Enable/disable UI elements depending on whether a table is loaded.
   */
  void update_table_sensitive_ui();

  AppState::userlevels get_userlevel() const;

  void fill_menu_tables();
  void fill_menu_reports(const Glib::ustring& table_name);
  void fill_menu_print_layouts(const Glib::ustring& table_name);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void do_menu_developer_fields(Gtk::Window& parent, const Glib::ustring table_name);
  void do_menu_developer_relationships(Gtk::Window& parent, const Glib::ustring table_name);
  bool do_restore_backup(const Glib::ustring& backup_uri);
#endif //GLOM_ENABLE_CLIENT_ONLY

  ///Whether to show the generated SQL queries on stdout, for debugging.
  bool get_show_sql_debug() const;

  ///Whether to show the generated SQL queries on stdout, for debugging.
  void set_show_sql_debug(bool val = true);

  ///Whether to automatically shutdown the database server when Glom crashes.
  void set_stop_auto_server_shutdown(bool val = true);

  void show_table_details(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value);
  void show_table_list(const Glib::ustring& table_name);

  /** Print the named report for the current table.
   */
  void print_report(const Glib::ustring& report_name);

  /** Print the current layout for the current table.
   */
  void print_layout();

  /** Offer the user the UI to add a new record,
   */
  void start_new_record();

  static Application* get_application();

protected:
  virtual void ui_warning_load_failed(int failure_code = 0); //Override.

private:
  virtual void init_layout(); //override.
  virtual void init_menus(); //override.
  virtual void init_toolbars(); //override
  virtual void init_create_document(); //override
  virtual bool on_document_load(); //override.
  virtual void on_document_close(); //override.
  virtual void update_window_title(); //override.

#ifndef GLOM_ENABLE_MAEMO
  virtual void init_menus_file(); //override.
  virtual void init_menus_help(); //override
#endif //GLOM_ENABLE_MAEMO

  bool offer_new_or_existing();

#ifndef GLOM_ENABLE_MAEMO
  void on_menu_help_contents();
#endif //GLOM_ENABLE_MAEMO

  /** Check that the file's hosting mode is supported by this build and
   * tell the user if necessary.
   */
  bool check_document_hosting_mode_is_supported(Document* document);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void existing_or_new_new();

  void on_menu_file_toggle_share();
  void on_menu_userlevel_developer();
  void on_menu_userlevel_operator();
  void on_menu_file_save_as_example();
  void on_menu_developer_changelanguage();
  void on_menu_developer_translations();
  void on_menu_developer_active_platform_normal();
  void on_menu_developer_active_platform_maemo();
  void on_menu_developer_export_backup();
  void on_menu_developer_restore_backup();
  void on_menu_developer_show_layout_toolbar();

  void on_window_translations_hide();

  virtual Glib::ustring ui_file_select_save(const Glib::ustring& old_file_uri); //overridden.
  void on_userlevel_changed(AppState::userlevels userlevel);

  Document* on_connection_pool_get_document();

  bool recreate_database_from_example(bool& user_cancelled); //return indicates success.
  bool recreate_database_from_backup(const Glib::ustring& backup_uri, bool& user_cancelled); //return indicates success.
  void on_recreate_database_progress();

  void stop_self_hosting_of_document_database();

  void on_connection_avahi_begin();
  void on_connection_avahi_progress();
  void on_connection_avahi_done();
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifndef G_OS_WIN32
  /** Offer a file chooser dialog, with a Browse Network button.
   * @param browsed This will be set to true if the user chose a networked glom instance to open.
   * @browsed_server This will be filled with the server details if browsed was set to true.
   */
  Glib::ustring ui_file_select_open_with_browse(bool& browsed, EpcServiceInfo*& browsed_server, Glib::ustring& browsed_service_name, const Glib::ustring& starting_folder_uri = Glib::ustring());
#endif // !G_OS_WIN32

  virtual void document_history_add(const Glib::ustring& file_uri); //overridden.

  virtual void new_instance(const Glib::ustring& uri = Glib::ustring()); //Override

  void on_connection_close_progress();
  void on_connection_save_backup_progress();
  void on_connection_convert_backup_progress();

#ifndef G_OS_WIN32
  void open_browsed_document(const EpcServiceInfo* server, const Glib::ustring& service_name);
#endif // !G_OS_WIN32

  static Glib::ustring get_file_uri_without_extension(const Glib::ustring& uri);

  typedef GlomBakery::App_WithDoc_Gtk type_base;

  //Widgets:

  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup_Others;

  typedef std::list< Glib::RefPtr<Gtk::Action> > type_listActions;
  type_listActions m_listDeveloperActions; //Only enabled when in developer mode.
  type_listActions m_listTableSensitiveActions; // Only enabled when a table is loaded.
  Glib::RefPtr<Gtk::Action> m_action_mode_find;
#ifndef GLOM_ENABLE_CLIENT_ONLY
  Glib::RefPtr<Gtk::Action> m_action_developer_users;
  Glib::RefPtr<Gtk::RadioAction> m_action_menu_userlevel_developer, m_action_menu_userlevel_operator;
  Glib::RefPtr<Gtk::ToggleAction> m_action_show_layout_toolbar;
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifdef GLOM_ENABLE_MAEMO
  Hildon::AppMenu m_maemo_appmenu;
  PickerButton_Table m_appmenu_button_table;

  void on_appmenu_button_table_value_changed();
#endif //GLOM_ENABLE_MAEMO

  Glib::RefPtr<Gtk::ToggleAction> m_toggleaction_network_shared;
  sigc::connection m_connection_toggleaction_network_shared;

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

  Document::HostingMode m_ui_save_extra_newdb_hosting_mode;

  Gtk::MessageDialog* m_avahi_progress_dialog;

  Dialog_ProgressCreating* m_dialog_progress_creating;
  Dialog_ProgressCreating* m_dialog_progess_save_backup;
  Dialog_ProgressCreating* m_dialog_progess_convert_backup;

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

#endif // GLOM_APPLICATION_H
