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

#ifndef BASE_DB_H
#define BASE_DB_H

#include "gtkmm.h"

#include "document/document_glom.h"
#include "connectionpool.h"
#include "appstate.h"
#include "data_structure/privileges.h"
#include "data_structure/system_prefs.h"
#include "utils.h"
#include "bakery/View/View.h"
#include <bakery/Utilities/BusyCursor.h>

class LayoutItem_GroupBy;
class LayoutItem_Summary;

/** A base class that is a Bakery View with some database functionality.
*/
class Base_DB :
  public View_Composite_Glom
{
public:
  Base_DB();
  virtual ~Base_DB();

  /// Specify the structure of what will be shown, and fill it.
  virtual bool init_db_details();

  /// Specify what actual data will be shown:
  virtual bool refresh_data_from_database();

  /** Returns whether we are in developer mode.
   * Some functionality will be deactivated when not in developer mode.
   */
  virtual AppState::userlevels get_userlevel() const;
  virtual void set_userlevel(AppState::userlevels value);

  static sharedptr<SharedConnection> connect_to_server();

  virtual void set_document(Document_Glom* pDocument); //View override
  virtual void load_from_document(); //View override

  typedef std::vector< Field > type_vecFields;

  static type_vecFields get_fields_for_table_from_database(const Glib::ustring& table_name);

  /** Create an appropriate title for an ID string.
   * For instance, date_of_birth would become Date Of Birth.
   */
  static Glib::ustring util_title_from_string(const Glib::ustring& text);

  //This is const because const means not changing this instance, not whether we change the database.
  virtual Glib::RefPtr<Gnome::Gda::DataModel> Query_execute(const Glib::ustring& strQuery) const;

  void add_standard_groups();
  void add_standard_tables() const;

  bool create_table(const TableInfo& table_info, const Document_Glom::type_vecFields& fields) const;

  typedef std::vector< sharedptr<LayoutItem_Field> > type_vecLayoutFields;

protected:
  bool offer_field_list(LayoutItem_Field& field, const Glib::ustring& table_name);
  
  ///@result Whether the user would like to find again.
  static bool show_warning_no_records_found(Gtk::Window& transient_for);

  void fill_full_field_details(const Glib::ustring& parent_table_name, LayoutItem_Field& layout_item);

  ///Get the table name. It's either the current table or the relationship's to_table:
  Glib::ustring get_layout_item_table_name(const LayoutItem_Field& layout_item, const Glib::ustring& table_name);

  typedef std::vector<Glib::ustring> type_vecStrings;
  type_vecStrings get_table_names(bool ignore_system_tables = false) const;

  bool get_table_exists_in_database(const Glib::ustring& table_name) const;

  type_vecFields get_fields_for_table(const Glib::ustring& table_name) const;

  type_vecStrings get_database_groups() const;
  type_vecStrings get_database_users(const Glib::ustring& group_name = Glib::ustring()) const;
  Privileges get_table_privileges(const Glib::ustring& group_name, const Glib::ustring& table_name) const;
  void set_table_privileges(const Glib::ustring& group_name, const Glib::ustring& table_name, const Privileges& privs, bool developer_privs = false);
  Glib::ustring get_user_visible_group_name(const Glib::ustring& group_name) const;

  type_vecStrings get_groups_of_user(const Glib::ustring& user) const;
  bool get_user_is_in_group(const Glib::ustring& user, const Glib::ustring& group) const;
  Privileges get_current_privs(const Glib::ustring& table_name) const;

  SystemPrefs get_database_preferences() const;
  void set_database_preferences(const SystemPrefs& prefs);

  void report_build(const Glib::ustring& table_name, const Report& report, const Glib::ustring& where_clause);
  void report_build_groupby(const Glib::ustring& table_name, xmlpp::Element& parent_node, LayoutItem_GroupBy& group_by, const Glib::ustring& where_clause_parent);
  void report_build_summary(const Glib::ustring& table_name, xmlpp::Element& parent_node, LayoutItem_Summary& summary, const Glib::ustring& where_clause_parent);

  ///@show_null_records means show a summary of zero values when there are no records to actually summarise.
  void report_build_records(const Glib::ustring& table_name, xmlpp::Element& parent_node, const type_vecLayoutFields& fieldsToGet, const Glib::ustring& where_clause, const Glib::ustring& sort_clause = Glib::ustring(), bool one_record_only = false);

  Gnome::Gda::Value auto_increment_insert_first_if_necessary(const Glib::ustring& table_name, const Glib::ustring& field_name);

  /** Get the next auto-increment value for this primary key, from the glom system table.
   * Add a row for this field in the system table if it does not exist already.
   */
  Gnome::Gda::Value get_next_auto_increment_value(const Glib::ustring& table_name, const Glib::ustring& field_name);

  virtual bool fill_from_database();
  virtual void fill_end(); //Call this from the end of fill_from_database() overrides.

  virtual void on_userlevel_changed(AppState::userlevels userlevel);

  static Glib::ustring util_string_from_decimal(guint decimal);
  static guint util_decimal_from_string(const Glib::ustring& str);

  static bool util_string_has_whitespace(const Glib::ustring& text);

  static type_vecStrings util_vecStrings_from_Fields(const type_vecFields& fields);

  //Utlility functions to help with the odd formats of postgres internal catalog fields:
  static Glib::ustring string_trim(const Glib::ustring& str, const Glib::ustring& to_remove);
  static type_vecStrings string_separate(const Glib::ustring& str, const Glib::ustring& separator);
  static type_vecStrings pg_list_separate(const Glib::ustring& str);


  virtual void handle_error(const std::exception& ex) const; //TODO_port: This is probably useless now.
  virtual bool handle_error() const;

};

#endif //BASE_DB_H
