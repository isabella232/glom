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

#ifndef GLOM_FRAME_GLOM_H
#define GLOM_FRAME_GLOM_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include "window_boxholder.h"

#include <gtkmm/frame.h>
#include <libglom/document/bakery/view/view_composite.h>
#include <libglom/document/document.h>

#include "navigation/box_tables.h"

#include "mode_data/notebook_data.h"
#include "mode_find/notebook_find.h"

#ifndef GLOM_ENABLE_CLIENT_ONLY
#include "box_reports.h"
#include "mode_design/print_layouts/box_print_layouts.h"
#include "mode_design/dialog_fields.h"
#include "mode_design/dialog_relationships.h"
#endif // !GLOM_ENABLE_CLIENT_ONLY

#include "dialog_connection.h"
#include <gtkmm/applicationwindow.h>

#include "mode_data/box_data_list_related.h" //only for m_HackToFixLinkerError.

namespace Glom
{

#ifndef GLOM_ENABLE_CLIENT_ONLY
class Dialog_Layout_Report;
class Window_PrintLayout_Edit;
class Dialog_AddRelatedTable;
class Window_RelationshipsOverview;
#endif // !GLOM_ENABLE_CLIENT_ONLY

class Frame_Glom :
  public PlaceHolder,
  //public GlomBakery::View_Composite<Document>,
  public Base_DB //Inherits from View_Composite.
{
public:
  Frame_Glom(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Frame_Glom();

  void set_databases_selected(const Glib::ustring& strName);

  void do_print_layout(const Glib::ustring& print_layout_name, bool preview = false, Gtk::Window* transient_for = 0);

  void on_box_tables_selected(const Glib::ustring& strName);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_box_reports_selected(const Glib::ustring& strName);
  void on_box_print_layouts_selected(const Glib::ustring& strName);

  bool attempt_change_usermode_to_developer();
  bool attempt_change_usermode_to_operator();

  void on_menu_file_export();
  void on_menu_file_import();
  void on_menu_file_print_edit_layouts();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  bool attempt_toggle_shared(bool shared);
  void on_menu_file_print();

  /** Show the widgets for find mode.
   * It is up to the caller to indicate in the menu that find mode is active.
   */
  void set_mode_find();

  /** Show the widgets for data mode.
   * It is up to the caller to indicate in the menu that find mode is not active.
   */
  void set_mode_data();

  void on_menu_add_record();

  void on_menu_report_selected(const Glib::ustring& report_name);

  void do_menu_Navigate_Table(bool open_default = false);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_menu_print_layout_selected(const Glib::ustring& print_layout_name);
  void on_menu_Tables_EditTables();

/* Commented out because it is useful but confusing to new users:
  void on_menu_Tables_AddRelatedTable();
*/
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_menu_Reports_EditReports();
  void on_menu_developer_database_preferences();
  void on_menu_developer_fields();
  void do_menu_developer_fields(Gtk::Window& parent);
  void do_menu_developer_fields(Gtk::Window& parent, const Glib::ustring& table_name);
  void on_menu_developer_relationships_overview();
  void on_menu_developer_relationships();
  void do_menu_developer_relationships(Gtk::Window& parent, const Glib::ustring& table_name);
  void on_menu_developer_users();
  void on_menu_developer_layout();
  void on_menu_developer_reports();
  void on_menu_developer_print_layouts();
  void on_menu_developer_script_library();

  void on_developer_dialog_hide();

  void on_dialog_layout_report_hide();
  void on_dialog_layout_print_hide();

  void on_dialog_add_related_table_request_edit_fields();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  void on_dialog_tables_hide();

  void set_document(const std::shared_ptr<Document>& document) override; //View override
  void load_from_document() override; //View override

  enum class enumModes
  {
    NONE, //at the start.
    DATA,
    FIND
  };
  enumModes m_Mode;
  enumModes m_Mode_Previous; // see comments in set_mode_widget().

  static void show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message, Gtk::Window& parent, Gtk::MessageType message_type = Gtk::MessageType::INFO);

  /** Show the dialog to request the password, and check whether it works.
   *
   * @param database_not_found true if the connection failed only because the database was not found on the server.
   * @param known_username The username if known. Otherwise, the user will be asked via a dialog.
   * @param known_password The password if known. Otherwise, the user will be asked via a dialog.
   * @param confirm_existing_user If true then an alternative message text will be shown.
   * @result true if the connection succeeded and the database was found on the server.
   */
  bool connection_request_password_and_attempt(bool& database_not_found, const Glib::ustring& known_username = Glib::ustring(), const Glib::ustring& known_password = Glib::ustring(), bool confirm_existing_user = false);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Show the dialog to request the password, and choose an unused database name.
  bool connection_request_password_and_choose_new_database_name();

  ///Create the database for new documents, showing the Connection dialog
  bool create_database(const Glib::ustring& database_name, const Glib::ustring& title);

  void set_enable_layout_drag_and_drop(bool enable = true);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  /** Show the table again. For instance, if the document has changed, or we want to display it differently.
   */
  void show_table_refresh();

  Glib::ustring get_shown_table_name() const;

  /** Show the table, possibly selecting a particular record, possibly showing that in the details tab.
   *
   * @param table_name The database table to show.
   * @param primary_key_value_for_details If specified, switch to the details view, and show this record.
   */
  void show_table(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value_for_details = Gnome::Gda::Value());

private:

  //void set_document(const std::shared_ptr<Document>& document) override;

  /** Show the table, possibly selecting a particular record, possibly showing that in the details tab. This allows table_name to be empty in which case no
   * table will be shown.
   *
   * @param table_name The database table to show.
   * @param primary_key_value_for_details If specified, switch to the details view, and show this record.
   */
  void show_table_allow_empty(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value_for_details = Gnome::Gda::Value());

  /** Hide the currently shown table so that no table is shown.
   */
  void show_no_table();

  void show_table_title();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  bool connection_request_initial_password(Glib::ustring& user, Glib::ustring& password);

  void update_table_in_document_from_database();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  void set_mode_widget(Gtk::Widget& widget); //e.g. show the design mode notebook.
  bool set_mode(enumModes mode); //bool indicates that there was a change.

  Gtk::Window* get_app_window();
  const Gtk::Window* get_app_window() const;

  void add_window_to_app(Gtk::ApplicationWindow* window);

  /** Show the number of records in the table, and the number found, in the UI.
   * @result The number of records found, for convenience for the caller.
   */
  gulong update_records_count();

  void alert_no_table();

  void instantiate_dialog_connection();

  //Signal handlers:
  void on_notebook_find_criteria(const Gnome::Gda::SqlExpr& where_clause);
  void on_button_quickfind();
  void on_button_find_all();
  void on_notebook_data_switch_page(Gtk::Widget* page);
  void on_notebook_data_record_details_requested(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value);
  void on_userlevel_changed(AppState::userlevels userlevel) override;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_dialog_add_related_table_response(int response);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  void on_connection_initialize_progress();
  void on_connection_startup_progress();
  void on_connection_cleanup_progress();
  void cleanup_connection();
  bool handle_connection_initialize_errors(ConnectionPool::InitErrors error);

private:

  void on_notebook_data_record_selection_changed();

  /**
   * @result Whether to try again.
   */
  bool handle_request_password_connection_error(bool asked_for_password, const ExceptionConnection& ex, bool& database_not_found);

  //Member data:
  Glib::ustring m_table_name;

  //Child widgets:
  Gtk::Label* m_label_table_data_mode;
  Gtk::Label* m_label_table_find_mode;

  Gtk::Box m_box_records_count; //Only show this when in Data mode.
  Gtk::Label m_label_records_count;
  Gtk::Label m_label_found_count;
  Gtk::Button m_button_find_all;

  Gtk::Stack* m_stack_mode; //Contains e.g. data or find mode notebook.

  //Navigation:
  Box_Tables* m_box_tables;
  Window_BoxHolder* m_dialog_tables;

  Notebook_Data m_notebook_data;

  Gtk::Box* m_box_quick_find; //Only show this when in Find mode.
  Gtk::Entry* m_entry_quick_find;
  Gtk::Button* m_button_quick_find;
  Notebook_Find m_notebook_find;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Developer:
  std::unique_ptr<Window_BoxHolder> m_dialog_reports;
  Dialog_Layout_Report* m_dialog_layout_report;
  Box_Reports* m_box_reports;

  std::unique_ptr<Window_BoxHolder> m_dialog_print_layouts;
  Window_PrintLayout_Edit* m_dialog_layout_print;
  Box_Print_Layouts* m_box_print_layouts;

  Dialog_Fields* m_dialog_fields;
  Dialog_Relationships* m_dialog_relationships;
  Dialog_AddRelatedTable* m_dialog_addrelatedtable;
  Window_RelationshipsOverview* m_window_relationships_overview;

#endif //GLOM_ENABLE_CLIENT_ONLY

  Dialog_Connection* m_dialog_connection;
};

} //namespace Glom

#endif // GLOM_FRAME_GLOM_H
