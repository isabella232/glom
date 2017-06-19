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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_APP_WINDOW_H
#define GLOM_APP_WINDOW_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <glom/bakery/appwindow_withdoc.h>
#include <glom/frame_glom.h>
#include <glom/show_progress_message.h>
#include <glom/infobar_progress_creating.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/messagedialog.h>

#include <gtkmm/dialog.h>
#include <gtkmm/menubar.h>
#include <gtkmm/menu.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/builder.h>
#include <gtkmm/applicationwindow.h>


#include <libglom/document/bakery/document.h>
#include <gtkmm/toolbutton.h>
#include <gtkmm/recentmanager.h>
#include <gtkmm/recentchooser.h>

//Avoid including the header here:
extern "C"
{
typedef struct AvahiStringList AvahiStringList;

typedef struct _EpcServiceInfo EpcServiceInfo;
}

namespace Glom
{

class Window_Translations;

class AppWindow
  : public GlomBakery::AppWindow_WithDoc,
    public Gtk::ApplicationWindow //inherit virtually to share sigc::trackable.
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  AppWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~AppWindow();

  void init() override; //Unique final overrider.

  /**
   * @param restore Whether @a document_uri is a .tar.gz backup file to restore.
   */
  bool init_with_document(const Glib::ustring& document_uri = Glib::ustring(), bool restore = false);

  /** Changes the mode to Data mode, as if the user had selected the Data Mode menu item.
   */
  void set_mode_data();

private:
  /** Changes the mode to Find mode, as if the user had selected the Data Mode menu item.
   */
  void set_mode_find();

public:

  /** Show in the UI whether the database is shared on the network.
   */
  void update_network_shared_ui();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void add_developer_action(const Glib::RefPtr<Gio::SimpleAction>& refAction);
  void remove_developer_action(const Glib::RefPtr<Gio::SimpleAction>& refAction);

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
  void enable_menu_print_layouts_details(bool enable = true);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void do_menu_developer_fields(Gtk::Window& parent, const Glib::ustring table_name);
  void do_menu_developer_relationships(Gtk::Window& parent, const Glib::ustring table_name);
  void do_print_layout(const Glib::ustring& print_layout_name, bool preview = false, Gtk::Window* transient_for = 0);
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

  void set_progress_message(const Glib::ustring& message);
  void pulse_progress_message();
  void clear_progress_message();

  /** Set the locale used for original text of titles. This
   * must usually be stored in the document.
   * Ideally, it would be English.
   */
  static void set_original_locale(const Glib::ustring& locale);

  static Glib::ustring get_original_locale();

  static bool get_current_locale_not_original();

  /** Set the locale used for titles, to test translations.
   * Usually the current locale is just the locale at startup.
   */
  static void set_current_locale(const Glib::ustring& locale);

  /** Get the locale used by this program when it was started,
   * or the locale set by set_current_locale().
   */
  static Glib::ustring get_current_locale();

  static AppWindow* get_appwindow();

  /// Overidden to add a widget in the middle, under the menu, instead of replacing the whole contents.
  void add(Gtk::Widget& child);

  /// For instance, to create bold primary text for a dialog box, without marking the markup for translation.
  static Glib::ustring util_bold_message(const Glib::ustring& message);

protected:
  void init_layout(); //Arranges the menu, toolbar, etc.
  void init_menus() override; //Override this to add more or different menus.
  void init_menus_file() override; //Call this from init_menus() to add the standard file menu.
  void init_menus_edit() override; //Call this from init_menus() to add the standard edit menu

  void on_hide() override;

  //Overrides from AppWindow_WithDoc:
  void document_history_add(const Glib::ustring& file_uri) override;
  void document_history_remove(const Glib::ustring& file_uri) override;
  void update_window_title() override;
  void ui_warning(const Glib::ustring& text, const Glib::ustring& secondary_text) override;
  Glib::ustring ui_file_select_open(const Glib::ustring& starting_folder_uri = Glib::ustring()) override;
  Glib::ustring ui_file_select_save(const Glib::ustring& old_file_uri) override;
  void ui_show_modification_status() override;
  enumSaveChanges ui_offer_to_save_changes() override;


  //Signal handlers:

  //Menus:


  void ui_hide() override;
  void ui_bring_to_front() override;

  bool on_delete_event(Gdk::Event& event) override;

  void on_menu_edit_copy_activate();
  void on_menu_edit_cut_activate();
  void on_menu_edit_paste_activate();
  void on_menu_edit_find();

  //Menu Builder and Actions
  Glib::RefPtr<Gtk::Builder> m_builder;
  std::unique_ptr<Gtk::MenuBar> m_menubar;
  Glib::RefPtr<Gio::SimpleActionGroup> m_action_group_file,
     m_action_group_edit, m_action_group_tables,
     m_action_group_developer, m_action_group_reports;


  //Member widgets:
  Gtk::Box* m_vbox;
  Gtk::Box m_vbox_placeHolder;

  //Menu stuff:
  Glib::RefPtr<Gio::SimpleAction> m_action_save, m_action_saveas;

protected:
  void ui_warning_load_failed(int failure_code = 0) override;

private:
  void init_create_document() override;
  bool on_document_load() override;
  void on_document_close() override;

  bool offer_new_or_existing();

  void on_menu_help_contents();

  /** Check that the file's hosting mode is supported by this build and
   * tell the user if necessary.
   */
  bool check_document_hosting_mode_is_supported(const std::shared_ptr<Document>& document);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void existing_or_new_new();

  void on_menu_file_toggle_share();
  void on_menu_developer_usermode(int parameter);
  void on_menu_file_save_as_example();
  void on_menu_developer_changelanguage();
  void on_menu_developer_translations();
  void on_menu_developer_active_platform(const Glib::ustring& parameter);
  void on_menu_developer_export_backup();
  void on_menu_developer_restore_backup();
  void on_menu_developer_enable_layout_drag_and_drop();

  void on_window_translations_hide();

  void on_userlevel_changed(AppState::userlevels userlevel);

  std::shared_ptr<Document> on_connection_pool_get_document();

  bool recreate_database_from_example(bool& user_cancelled); //return indicates success.
  bool recreate_database_from_backup(const std::string& backup_data_file_path, bool& user_cancelled); //return indicates success.
  void on_recreate_database_progress();

  //void stop_self_hosting_of_document_database();

  void on_connection_avahi_begin();
  void on_connection_avahi_progress();
  void on_connection_avahi_done();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  void on_menu_help_about() override;
  void on_about_close();

#ifndef G_OS_WIN32
  /** Offer a file chooser dialog, with a Browse Network button.
   * @param browsed This will be set to true if the user chose a networked glom instance to open.
   * @browsed_server This will be filled with the server details if browsed was set to true.
   */
  Glib::ustring ui_file_select_open_with_browse(bool& browsed, EpcServiceInfo*& browsed_server, Glib::ustring& browsed_service_name, const Glib::ustring& starting_folder_uri = Glib::ustring());
#endif // !G_OS_WIN32

  void new_instance(const Glib::ustring& uri = Glib::ustring()) override;

  void on_connection_create_database_progress();
  void on_connection_close_progress();
  void on_connection_save_backup_progress();
  void on_connection_convert_backup_progress();

#ifndef G_OS_WIN32
  void open_browsed_document(const EpcServiceInfo* server, const Glib::ustring& service_name);
#endif // !G_OS_WIN32

  //Widgets:
  typedef std::vector< Glib::RefPtr<Gio::SimpleAction> > type_listActions;
  type_listActions m_listDeveloperActions; //Only enabled when in developer mode.
  type_listActions m_listTableSensitiveActions; // Only enabled when a table is loaded.
  Glib::RefPtr<Gio::SimpleAction> m_action_mode_find;
#ifndef GLOM_ENABLE_CLIENT_ONLY
  Glib::RefPtr<Gio::SimpleAction> m_action_developer_users;
  Glib::RefPtr<Gio::SimpleAction> m_action_menu_developer_usermode;
  Glib::RefPtr<Gio::SimpleAction> m_action_menu_developer_active_platform;
  Glib::RefPtr<Gio::SimpleAction> m_action_enable_layout_drag_and_drop ;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  Glib::RefPtr<Gio::SimpleAction> m_toggleaction_network_shared;
  sigc::connection m_connection_toggleaction_network_shared;

  Gtk::Box* m_box_top;
  Frame_Glom* m_frame;

  bool m_about_shown;
  Gtk::AboutDialog* m_about; //About box.

  Infobar_ProgressCreating* m_infobar_progress;
  std::string m_progress_collate_key;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  Window_Translations* m_window_translations;

#endif // !GLOM_ENABLE_CLIENT_ONLY

  Glib::RefPtr<Gio::SimpleActionGroup> m_help_action_group;
  Glib::RefPtr<Gio::SimpleActionGroup> m_nav_tables_action_group, m_nav_reports_action_group, m_nav_print_layouts_action_group;
  type_listActions m_nav_table_actions, m_nav_report_actions, m_nav_print_layout_actions;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Set these before calling offer_saveas() (which uses ui_file_select_save()), and clear it afterwards.
  bool m_ui_save_extra_showextras;
  Glib::ustring m_ui_save_extra_title;
  Glib::ustring m_ui_save_extra_message;

  Glib::ustring m_ui_save_extra_newdb_title;

  Document::HostingMode m_ui_save_extra_newdb_hosting_mode;

  Gtk::MessageDialog* m_avahi_progress_dialog;

#endif // !GLOM_ENABLE_CLIENT_ONLY

  // This is set to the URI of an example file that is loaded to be able to
  // prevent adding it into the recently used resources in
  // document_history_add().
  Glib::ustring m_example_uri;

  //A temporary store for the username/password if
  //we already asked for them when getting the document over the network,
  //so we can use them again when connecting directly to the database:
  Glib::ustring m_temp_username, m_temp_password;

  //This is set temporarily while restoring a backup.
  std::string m_backup_data_filepath;

  bool m_show_sql_debug;

  static Glib::ustring m_current_locale, m_original_locale;
};

Glib::ustring item_get_title(const std::shared_ptr<const TranslatableItem>& item);

Glib::ustring item_get_title_or_name(const std::shared_ptr<const TranslatableItem>& item);

} //namespace Glom

#endif // GLOM_APP_WINDOW_H
