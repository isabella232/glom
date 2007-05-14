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

#ifndef FRAME_GLOM_H
#define FRAME_GLOM_H

#include "config.h" // For ENABLE_CLIENT_ONLY

#include <gtkmm/frame.h>
#include "bakery/View/View_Composite.h"
#include <glom/libglom/document/document_glom.h>

#include "dialog_glom.h"

#include "navigation/box_tables.h"
#include "box_reports.h"

#include "mode_data/notebook_data.h"
#include "mode_find/notebook_find.h"

#ifndef ENABLE_CLIENT_ONLY
#include "mode_design/dialog_fields.h"
#include "mode_design/dialog_relationships.h"
#endif // !ENABLE_CLIENT_ONLY

#include "dialog_connection.h"
#include <glom/libglom/utils.h>

#include "mode_data/box_data_list_related.h" //only for m_HackToFixLinkerError.

namespace Glom
{

#ifndef ENABLE_CLIENT_ONLY
class Dialog_Layout_Report;
class Dialog_AddRelatedTable;
#endif // !ENABLE_CLIENT_ONLY

class Frame_Glom :
  public PlaceHolder,
  //public Bakery::View_Composite<Document_Glom>,
  public Base_DB //Inherits from View_Composite.
{
public: 
  Frame_Glom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Frame_Glom();

  void set_databases_selected(const Glib::ustring& strName);

  void on_box_tables_selected(const Glib::ustring& strName);
#ifndef ENABLE_CLIENT_ONLY
  void on_box_reports_selected(const Glib::ustring& strName);
#endif // !ENABLE_CLIENT_ONLY

#ifndef ENABLE_CLIENT_ONLY
  void on_menu_userlevel_Developer(const Glib::RefPtr<Gtk::RadioAction>& action, const Glib::RefPtr<Gtk::RadioAction>& operator_action);
  void on_menu_userlevel_Operator(const Glib::RefPtr<Gtk::RadioAction>& action);
#endif // !ENABLE_CLIENT_ONLY

  void on_menu_file_export();
  void on_menu_file_print();

  void on_menu_Mode_Data();
  void on_menu_Mode_Find();

  void on_menu_report_selected(const Glib::ustring& report_name);

  //virtual void on_menu_Navigate_Database();
  //virtual void do_menu_Navigate_Database(bool bUseList = true);
#ifndef ENABLE_CLIENT_ONLY
  void on_menu_Tables_EditTables();
  void on_menu_Tables_AddRelatedTable();
#endif // !ENABLE_CLIENT_ONLY

  void do_menu_Navigate_Table(bool open_default = false);

#ifndef ENABLE_CLIENT_ONLY
  void on_menu_Tables_EditReports();

  void on_menu_developer_database_preferences();

  void on_menu_developer_fields();
  void do_menu_developer_fields(Gtk::Window& parent);

  void on_menu_developer_relationships_overview();
  void on_menu_developer_relationships();
  void on_menu_developer_users();
  void on_menu_developer_layout();
  void on_menu_developer_reports();
  void on_menu_developer_script_library();

  void on_developer_dialog_hide();
#endif // !ENABLE_CLIENT_ONLY

#ifndef ENABLE_CLIENT_ONLY
  void on_dialog_layout_report_hide();

  void on_dialog_reports_hide();
  void on_dialog_tables_hide();

  void on_dialog_add_related_table_request_edit_fields();
#endif // !ENABLE_CLIENT_ONLY

  virtual void set_document(Document_Glom* pDocument); //View override
  virtual void load_from_document(); //View override

  void show_system_name();

  enum enumModes
  {
    MODE_None, //at the start.
    MODE_Data,
    MODE_Find
  };
  enumModes m_Mode;
  enumModes m_Mode_Previous; // see comments in set_mode_widget().

  static void show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message, Gtk::Window& parent, Gtk::MessageType message_type = Gtk::MESSAGE_INFO);

  //Show the dialog to request the password, and check whether it works.
  bool connection_request_password_and_attempt();

  //Show the dialog to request the password, and choose an unused database name.
  bool connection_request_password_and_choose_new_database_name();

  ///Create the database for new documents, showing the Connection dialog
  bool create_database(const Glib::ustring& database_name, const Glib::ustring& title, bool request_password = true);

  void export_data_to_string(Glib::ustring& the_string, const FoundSet& found_set, const Document_Glom::type_mapLayoutGroupSequence& sequence);
  void export_data_to_stream(std::ostream& the_stream, const FoundSet& found_set, const Document_Glom::type_mapLayoutGroupSequence& sequence);

  /** Show the table again. For instance, if the document has changed, or we want to display it differently.
   */
  void show_table_refresh();

protected:

  //virtual void set_document(Document_Glom* pDocument); //override

  void show_table(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value_for_details = Gnome::Gda::Value());
  void show_table_title();
  void update_table_in_document_from_database();

  virtual void set_mode_widget(Gtk::Widget& widget); //e.g. show the design mode notebook.
  virtual bool set_mode(enumModes mode); //bool indicates that there was a change.

  virtual Gtk::Window* get_app_window();
  virtual const Gtk::Window* get_app_window() const;

  void update_records_count();

  void alert_no_table();

  //Signal handlers:
  void on_notebook_find_criteria(const Glib::ustring& where_clause);
  void on_button_quickfind();
  void on_button_find_all();
  void on_notebook_data_record_details_requested(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value);
  void on_userlevel_changed(AppState::userlevels userlevel);

#ifndef ENABLE_CLIENT_ONLY
  void on_dialog_add_related_table_response(int response);
#endif // !ENABLE_CLIENT_ONLY

  //Member data:
  Glib::ustring m_table_name;

  //Child widgets:
  Gtk::Label* m_pLabel_Name;
  Gtk::Label* m_pLabel_Table;
  Gtk::Label* m_pLabel_Mode;
  Gtk::Label* m_pLabel_userlevel;

  Gtk::HBox* m_pBox_QuickFind; //Only show this when in Find mode.
  Gtk::Entry* m_pEntry_QuickFind;
  Gtk::Button* m_pButton_QuickFind;

  Gtk::HBox* m_pBox_RecordsCount; //Only show this when in Data mode.
  Gtk::Label* m_pLabel_RecordsCount;
  Gtk::Label* m_pLabel_FoundCount;
  Gtk::Button* m_pButton_FindAll;

  PlaceHolder* m_pBox_Mode; //Contains e.g. design mode notebook.

#ifndef ENABLE_CLIENT_ONLY
  Box_Tables* m_pBox_Tables;
  Box_Reports* m_pBox_Reports;
#endif // !ENABLE_CLIENT_ONLY

  Notebook_Data m_Notebook_Data;
  Notebook_Find m_Notebook_Find;

#ifndef ENABLE_CLIENT_ONLY
  //Navigation:
  Dialog_Glom* m_pDialog_Tables;
  Dialog_Glom* m_pDialog_Reports;
#endif // !ENABLE_CLIENT_ONLY

#ifndef ENABLE_CLIENT_ONLY
  //Developer:
  Dialog_Fields* m_pDialog_Fields;
  Dialog_Relationships* m_pDialog_Relationships;
  Dialog_AddRelatedTable* m_dialog_addrelatedtable;
#endif // !ENABLE_CLIENT_ONLY

  Dialog_Connection* m_pDialogConnection;
  Gtk::Dialog* m_pDialogConnectionFailed;

#ifndef ENABLE_CLIENT_ONLY
  Dialog_Layout_Report* m_pDialogLayoutReport;
#endif // !ENABLE_CLIENT_ONLY

  Box_Data_List_Related m_HackToFixLinkerError; //The implementation of this class does not seem to be in the library unless I do this. murrayc.
};

} //namespace Glom

#endif //FRAME_GLOM_H
