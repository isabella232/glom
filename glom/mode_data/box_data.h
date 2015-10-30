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

#ifndef GLOM_MODE_DATA_BOX_DATA_H
#define GLOM_MODE_DATA_BOX_DATA_H

#include "config.h" // GLOM_ENABLE_CLIENT_ONLY

#include <glom/box_withbuttons.h>
#include <glom/base_db_table_data.h>
#include <glom/mode_design/layout/dialog_layout.h>

namespace Glom
{

/** A Box for viewing and editing the data in a database table.
 *
 * Call init_db_details() to create the layout and fill it with data from the database.
 * Call refresh_data_from_database() to fill the existing layout with up-to-date data from the database.
 *
 * Derived classes should implement create_layout() to create/arrange the widgets for the groups, fields, portals, etc.
 * Derived classes should implement fill_from_database() to get the data from the database and fill the widgets created by create_layout().
 */
class Box_Data
: public Box_WithButtons,
  public Base_DB_Table_Data
{
public:
  Box_Data();
  virtual ~Box_Data();

  //TODO: Put this in Base_DB_Table_Data?
  ///Create the layout for the database structure, and fill it with data from the database.
  bool init_db_details(const FoundSet& found_set, const Glib::ustring& layout_platform);

  //Fill the existing layout with data from the database:
  bool refresh_data_from_database_with_where_clause(const FoundSet& found_set);

  virtual void print_layout(); //A test, for now.

  ///Get the existing where clause, previously supplied to init_db_details().
  FoundSet get_found_set() const;

  Gnome::Gda::SqlExpr get_find_where_clause() const;

  void set_unstored_data(bool bVal);
  bool get_unstored_data() const;

  bool confirm_discard_unstored_data() const;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void show_layout_dialog();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  Glib::ustring get_layout_name() const;

  //Signals:

  /** Emitted when the user has entered a find critera that
   * should be used to find and display records.
   * Used by _Find sub-classes.
   * @param find_criteria The SQL where clause.
   */
  //Should be a MI class, derived by those sub-classes. TODO.
  //where_clause.
  sigc::signal<void, Gnome::Gda::SqlExpr> signal_find_criteria;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //g++ 3.4 needs this to be public when used from Box_Data_Details. I'm not sure why. murrayc.
  virtual void on_dialog_layout_hide();
#endif // !GLOM_ENABLE_CLIENT_ONLY

protected:

  /* Create the layout based on the database structure and saved layout,
   * so that fill_from_database() can fill it with data.
   */
  virtual void create_layout();

  ///Fill the existing layout with data from the database.
  bool fill_from_database() override;

  virtual type_vecConstLayoutFields get_fields_to_show() const;

  type_vecConstLayoutFields get_table_fields_to_show(const Glib::ustring& table_name) const;

  /** Get the layout groups, with the Field information filled in.
   */
  Document::type_list_layout_groups get_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& layout_platform);
  void fill_layout_group_field_info(const std::shared_ptr<LayoutGroup>& group, const Privileges& table_privs);

  void execute_button_script(const std::shared_ptr<const LayoutItem_Button>& layout_item, const Gnome::Gda::Value& primary_key_value);

private:

  //Signal handlers:
  void on_Button_Find(); //only used by _Find sub-classes. Should be MI.

  //Signal handlers for the PyGlomUI callbacks:
  void on_python_requested_show_table_details(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value);
  void on_python_requested_show_table_list(const Glib::ustring& table_name);
  void on_python_requested_print_report(const Glib::ustring& report_name);
  void on_python_requested_print_layout();
  void on_python_requested_start_new_record();

protected:

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual Dialog_Layout* create_layout_dialog() const = 0;
  virtual void prepare_layout_dialog(Dialog_Layout* dialog) = 0;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  void handle_error(const Glib::Exception& ex);
  void handle_error(const std::exception& ex); //TODO_port: This is probably useless now.

  Gtk::Button m_Button_Find; //only used by _Find sub-classes. Should be MI.
  Gtk::Label m_Label_FindStatus;

  bool m_bUnstoredData;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  Dialog_Layout* m_pDialogLayout;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  /// "details" or "list", as specified in the Document's XML.
  Glib::ustring m_layout_name;

  /// Empty string or "maemo" as specified in the Document's XML.
  Glib::ustring m_layout_platform;
};

} //namespace Glom

#endif // GLOM_MODE_DATA_BOX_DATA_H
