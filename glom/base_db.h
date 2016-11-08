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

#ifndef BASE_DB_H
#define BASE_DB_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <libglom/document/view.h>
#include <libglom/connectionpool.h>
#include <libglom/appstate.h>
#include <libglom/data_structure/foundset.h>
#include <libglom/data_structure/privileges.h>
#include <libglom/data_structure/system_prefs.h>
#include <libglom/calcinprogress.h>
#include <libglom/document/bakery/view/view.h>
#include <glom/bakery/busy_cursor.h>
#include <gtkmm/treemodel.h>

#include <libgdamm/set.h>
#include <libgdamm/sqlbuilder.h>

namespace Glom
{

class LayoutItem_GroupBy;
class LayoutItem_Summary;
class LayoutItem_VerticalGroup;


/** A base class that is a Bakery View with some database functionality.
 */
class Base_DB :
  public View_Composite_Glom
{
public:
  Base_DB();

  /// Specify the structure of what will be shown, and fill it.
  bool init_db_details();

  /** Returns whether we are in developer mode.
   * Some functionality will be deactivated when not in developer mode.
   */
  AppState::userlevels get_userlevel() const;
  void set_userlevel(AppState::userlevels value);

  static std::shared_ptr<SharedConnection> connect_to_server(Gtk::Window* parent_window = 0);

  void set_document(const std::shared_ptr<Document>& document) override; //View override
  void load_from_document() override; //View override

  std::shared_ptr<Field> change_column(const Glib::ustring& table_name, const std::shared_ptr<const Field>& field_old, const std::shared_ptr<const Field>& field, Gtk::Window* parent_window) const;

  typedef std::vector< std::shared_ptr<Field> > type_vec_fields;
  typedef std::vector< std::shared_ptr<const Field> > type_vec_const_fields;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  bool change_columns(const Glib::ustring& table_name, const type_vec_const_fields& old_fields, type_vec_fields& fields, Gtk::Window* parent_window) const;
#endif //GLOM_ENABLE_CLIENT_ONLY

  //TODO: This is not a very good place for this function.
  /// Get the active layout platform for the document, or get a suitable default.
  static Glib::ustring get_active_layout_platform(const std::shared_ptr<Document>& document);

  typedef std::vector< std::shared_ptr<LayoutItem_Field> > type_vecLayoutFields;
  typedef std::vector< std::shared_ptr<const LayoutItem_Field> > type_vecConstLayoutFields;

protected:

  void clear_fields_calculation_in_progress();

  typedef std::vector< std::shared_ptr<LayoutItem_Field> > type_list_field_items;
  typedef std::vector< std::shared_ptr<const LayoutItem_Field> > type_list_const_field_items;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  /** Allow the user to select a field from the list of fields for the table.
   */
  std::shared_ptr<LayoutItem_Field> offer_field_list_select_one_field(const Glib::ustring& table_name, Gtk::Window* transient_for = 0);

  /** Allow the user to select a field from the list of fields for the table,
   * with @a start_field selected by default.
   */
  std::shared_ptr<LayoutItem_Field> offer_field_list_select_one_field(const std::shared_ptr<const LayoutItem_Field>& start_field, const Glib::ustring& table_name, Gtk::Window* transient_for = 0);


  /** Allow the user to select fields from the list of fields for the table.
   */
  type_list_field_items offer_field_list(const Glib::ustring& table_name, Gtk::Window* transient_for = 0);


  std::shared_ptr<LayoutItem_Field> offer_field_formatting(const std::shared_ptr<const LayoutItem_Field>& start_field, const Glib::ustring& table_name, Gtk::Window* transient_for, bool show_editable_options = true);

  /** Offer generic formatting for a @a layout_item, starting with its current options.
   * @result true if the user changed some formatting for the items.
   */
  bool offer_non_field_item_formatting(const std::shared_ptr<LayoutItem_WithFormatting>& layout_item, Gtk::Window* transient_for = 0);

  std::shared_ptr<LayoutItem_Text> offer_textobject(const std::shared_ptr<LayoutItem_Text>& start_textobject, Gtk::Window* transient_for = 0, bool show_title = true);
  std::shared_ptr<LayoutItem_Image> offer_imageobject(const std::shared_ptr<LayoutItem_Image>& start_imageobject, Gtk::Window* transient_for = 0, bool show_title = true);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  bool get_relationship_exists(const Glib::ustring& table_name, const Glib::ustring& relationship_name);

  std::shared_ptr<Field> get_field_primary_key_for_table(const Glib::ustring& table_name) const;

  //Methods to be overridden by derived classes:
  virtual void set_entered_field_data(const LayoutItem_Field& field, const Gnome::Gda::Value&  value);
  virtual void set_entered_field_data(const Gtk::TreeModel::iterator& row, const LayoutItem_Field& field, const Gnome::Gda::Value& value);


  class FieldInRecord
  {
  public:
    FieldInRecord()
    {}

    FieldInRecord(const Glib::ustring& table_name, const std::shared_ptr<const Field>& field, const std::shared_ptr<const Field>& key, const Gnome::Gda::Value& key_value)
    : m_table_name(table_name), m_field(field), m_key(key), m_key_value(key_value)
    {
    }

    FieldInRecord(const std::shared_ptr<const LayoutItem_Field>& layout_item, const Glib::ustring& parent_table_name, const std::shared_ptr<const Field>& parent_key, const Gnome::Gda::Value& key_value, const std::shared_ptr<const Document>& document)
    : m_field(layout_item->get_full_field_details()),
      m_key_value(key_value)
    {
      m_table_name = layout_item->get_table_used(parent_table_name);

      //The key:
      if(layout_item->get_has_relationship_name())
      {
        //The field is in a related table.
        std::shared_ptr<const Relationship> rel = layout_item->get_relationship();
        if(rel)
        {
          if(layout_item->get_has_related_relationship_name()) //For doubly-related fields
          {
            std::shared_ptr<const Relationship> related_rel = layout_item->get_related_relationship();
            if(related_rel)
            {
              //Actually a foreign key in a doubly-related table:
              m_key = document->get_field(m_table_name, related_rel->get_to_field());
            }
          }
          else
          {
            //Actually a foreign key:
            m_key = document->get_field(m_table_name, rel->get_to_field());
          }
        }
      }
      else
      {
        m_key = parent_key;
      }
    }

    //Identify the field:
    Glib::ustring m_table_name;
    std::shared_ptr<const Field> m_field;

    //Identify the record:
    std::shared_ptr<const Field> m_key;
    Gnome::Gda::Value m_key_value;
  };


  class LayoutFieldInRecord
  {
  public:
    LayoutFieldInRecord()
    {}

    LayoutFieldInRecord(const std::shared_ptr<const LayoutItem_Field>& layout_item, const Glib::ustring& parent_table_name, const std::shared_ptr<const Field>& parent_key, const Gnome::Gda::Value& key_value)
    : m_table_name(parent_table_name),
      m_field(layout_item),
      m_key(parent_key),
      m_key_value(key_value)
    {
    }

    LayoutFieldInRecord(const LayoutFieldInRecord& src) = delete;
    LayoutFieldInRecord& operator=(const LayoutFieldInRecord& src) = delete;

    LayoutFieldInRecord(LayoutFieldInRecord&& src) = default;
    LayoutFieldInRecord& operator=(LayoutFieldInRecord&& src) = default;

    FieldInRecord get_fieldinrecord(const std::shared_ptr<const Document>& document) const
    {
      return FieldInRecord(m_field, m_table_name, m_key, m_key_value, document);
    }

    //Identify the field:
    Glib::ustring m_table_name;
    std::shared_ptr<const LayoutItem_Field> m_field;

    //Identify the record:
    std::shared_ptr<const Field> m_key;
    Gnome::Gda::Value m_key_value;
  };

  /** Calculate values for fields, set them in the database, and show them in the layout.
   * @param field_changed The field that has changed, causing other fields to be recalculated because they use its value.
   * @param primary_key The primary key field for this table.
   * @param priamry_key_value: The primary key value for this record.
   * @param first_calc_field: false if this is called recursively.
   */
  void do_calculations(const LayoutFieldInRecord& field_changed, bool first_calc_field);

  typedef std::unordered_map<Glib::ustring, CalcInProgress, std::hash<std::string>> type_field_calcs;


  /** Get the fields whose values should be recalculated when @a field_name changes.
   */
  type_list_const_field_items get_calculated_fields(const Glib::ustring& table_name, const LayoutItem_Field& field);

  /** Get the fields used, if any, in the calculation of this field.
   */
  type_list_const_field_items get_calculation_fields(const Glib::ustring& table_name, const std::shared_ptr<const LayoutItem_Field>& field);

  void calculate_field(const LayoutFieldInRecord& field_in_record);

  void calculate_field_in_all_records(const Glib::ustring& table_name, const std::shared_ptr<const Field>& field);
  void calculate_field_in_all_records(const Glib::ustring& table_name, const std::shared_ptr<const Field>& field, const std::shared_ptr<const Field>& primary_key);

  typedef std::unordered_map<Glib::ustring, Gnome::Gda::Value, std::hash<std::string>> type_map_fields;
  //TODO: Performance: This is massively inefficient:
  type_map_fields get_record_field_values_for_calculation(const Glib::ustring& table_name, const std::shared_ptr<const Field>& primary_key, const Gnome::Gda::Value& primary_key_value);


  void do_lookups(const LayoutFieldInRecord& field_in_record, const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& field_value);


  virtual void refresh_related_fields(const LayoutFieldInRecord& field_in_record_changed, const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& field_value);



  bool set_field_value_in_database(const LayoutFieldInRecord& field_in_record, const Gnome::Gda::Value& field_value, bool use_current_calculations = false, Gtk::Window* parent_window = 0);
  bool set_field_value_in_database(const LayoutFieldInRecord& field_in_record, const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& field_value, bool use_current_calculations = false, Gtk::Window* parent_window = 0);

  ///Get a single field value from the database.
  Gnome::Gda::Value get_field_value_in_database(const LayoutFieldInRecord& field_in_record, Gtk::Window* parent_window);

  ///Get a single field value from the database.
  Gnome::Gda::Value get_field_value_in_database(const std::shared_ptr<Field>& field, const FoundSet& found_set, Gtk::Window* parent_window);

  bool get_field_value_is_unique(const Glib::ustring& table_name, const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value);

  bool check_entered_value_for_uniqueness(const Glib::ustring& table_name, const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value, Gtk::Window* parent_window);
  bool check_entered_value_for_uniqueness(const Glib::ustring& table_name, const Gtk::TreeModel::iterator& /* row */,  const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value, Gtk::Window* parent_window);

  //TODO: Make this private?
  /** Fill the UI with information (data or structure, depending on the widget).
   * Overridden by derived widgets to provide implementation.
   */
  virtual bool fill_from_database();

  virtual void on_userlevel_changed(AppState::userlevels userlevel);

  bool get_primary_key_is_in_foundset(const FoundSet& found_set, const Gnome::Gda::Value& primary_key_value);


  /** A utility function to set the where_clause, even for doubly-related records.
   *
   * @param found_set The FoundSet to change.
   * @param portal The related records portal whose records should be selected by the SQL query.
   * @param foreign_key_value The value of the from field in the parent table.
   */
  void set_found_set_where_clause_for_portal(FoundSet& found_set, const std::shared_ptr<LayoutItem_Portal>& portal, const Gnome::Gda::Value& foreign_key_value);

  static Glib::RefPtr<Gnome::Gda::Connection> get_connection();

  typedef std::vector<Glib::ustring> type_vec_strings;
  static type_vec_strings util_vecStrings_from_Fields(const type_vec_fields& fields);

  bool set_database_owner_user(const Glib::ustring& user);

  /** Revoke any login rights from the user and remove it from any groups.
   * This is a workaround for these problems:
   * 1. We can only specify a superuser _user_, not a role, to initdb (because it needs a password, but groups have no password),
   *    so that user is then the owner of various objects.
   * 2. Even when changing the owner of these objects we still get this error "cannot drop role glom_default_developer_user because it is required by the database system"
   */
  bool disable_user(const Glib::ustring& user);


  static void handle_error(const Glib::Exception& ex, Gtk::Window* parent);
  static void handle_error(const std::exception& ex, Gtk::Window* parent); //TODO_port: This is probably useless now.
  static bool handle_error();

private:
  type_field_calcs m_FieldsCalculationInProgress; //Prevent circular calculations and recalculations.
};

} //namespace Glom

#endif //BASE_DB_H
