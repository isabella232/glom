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

#ifndef DOCUMENT_GLOM_H
#define DOCUMENT_GLOM_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <bakery/Document/Document_XML.h>
#include <bakery/View/View_Composite.h>
#include <glom/libglom/data_structure/relationship.h>
#include <glom/libglom/data_structure/field.h>
#include <glom/libglom/data_structure/layout/layoutgroup.h>
#include <glom/libglom/data_structure/layout/layoutitem_notebook.h>
#include <glom/libglom/data_structure/layout/layoutitem_portal.h>
#include <glom/libglom/data_structure/layout/layoutitem_button.h>
#include <glom/libglom/data_structure/layout/layoutitem_text.h>
#include <glom/libglom/data_structure/layout/layoutitem_image.h>
#include <glom/libglom/data_structure/tableinfo.h>
#include <glom/libglom/data_structure/groupinfo.h>
#include <glom/libglom/data_structure/report.h>
#include <glom/libglom/data_structure/print_layout.h>
#include "../appstate.h"
#include <gtkmm/window.h>
#include <vector>
#include <map>

namespace Glom
{

/// Can be used with std::find_if() to find a layout with the same parent_table and layout_name.
template<class T_Element>
class predicate_Layout
{
public:
  predicate_Layout(const Glib::ustring& parent_table, const Glib::ustring& layout_name)
  : m_parent_table(parent_table),
    m_layout_name(layout_name)
  {
  }

  virtual ~predicate_Layout()
  {
  }

  bool operator() (const T_Element& element)
  {
    return (element.m_parent_table == m_parent_table) &&
           (element.m_layout_name == m_layout_name);
  }

protected:
  Glib::ustring m_parent_table, m_layout_name;
};

class Document_Glom : public Bakery::Document_XML
{
public: 
  Document_Glom();
  virtual ~Document_Glom();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual void set_modified(bool value = true);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  virtual void set_file_uri(const Glib::ustring& file_uri, bool bEnforceFileExtension = false);

  /** Whether the document was opened from another networked glom instance, 
   * instead of via a URI.
   */
  void set_opened_from_browse(bool val = true);
  bool get_opened_from_browse() const;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  /** The document usually saves itself when you call set_modified().
   * Pass false to this function to prevent that temporarily.
   * The document will be saved, if necessary, after you call this function with true.
   */
  void set_allow_autosave(bool value = true);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  bool get_is_example_file() const;
  void set_is_example_file(bool value = true);

  /* Get version of the document format used for this document.
   *  This can increase when the file has been re-saved.
   *  See get_latest_known_document_format_version().
   *  Old versions of the application cannot open documents with a newer document format,
   *  so saving with a version of the application that has a newer document format will 
   *  make it impossible to open the document in a version of the application with an older document format.
   */  
  guint get_document_format_version();

  static guint get_latest_known_document_format_version();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  /** When this is set, the postgres database is hosted by the local client,
   *  instead of connecting to an external database server.
   */
  void set_connection_is_self_hosted(bool self_hosted = true);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  void set_connection_server(const Glib::ustring& strVal);
  void set_connection_user(const Glib::ustring& strVal);
  void set_connection_database(const Glib::ustring& strVal);
  void set_connection_port(int port_number);
  void set_connection_try_other_ports(bool val);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  /** When this returns true, the postgres database should be hosted by the local client,
   *  instead of connecting to an external database server.
   */
  bool get_connection_is_self_hosted() const;

  /** If the database should be hosted, this provides the 
    * path to the directory that contains all the files needed to do that.
    * This is usually a specifically-named directory at the same level as the .glom file. 
    */   
  std::string get_connection_self_hosted_directory_uri() const;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  Glib::ustring get_connection_server() const;
  Glib::ustring get_connection_user() const;
  Glib::ustring get_connection_database() const;
  int get_connection_port() const;
  bool get_connection_try_other_ports() const;

  /** Set the language/locale used by original titles.
   * Title translations are translations of the text in this language.
   * @param locale: For instance, "en_US.UTF-8".
   */
  void set_translation_original_locale(const Glib::ustring& locale);

  /** Get the language/locale used by original titles.
   * Title translations are translations of the text in this language.
   */
  Glib::ustring get_translation_original_locale() const;

  typedef std::vector< sharedptr<Relationship> > type_vecRelationships;
  type_vecRelationships get_relationships(const Glib::ustring& table_name, bool plus_system_prefs = false) const;
  void set_relationships(const Glib::ustring& table_name, const type_vecRelationships& vecRelationships);

  sharedptr<Relationship> get_relationship(const Glib::ustring& table_name, const Glib::ustring& relationship_name) const;
  void set_relationship(const Glib::ustring& table_name, const sharedptr<Relationship>& relationship);

  void remove_relationship(const sharedptr<const Relationship>& relationship);

  /** Returns whether the relationship's to-field is a primary key  or unique field, meaning
   * that there can be only one related record for each value of the from-field.
   */
  bool get_relationship_is_to_one(const Glib::ustring& table_name, const Glib::ustring& relationship_name) const;

  /** Returns whether the field is the from-field in a to-one relationship.
   * @see get_relationship_is_to_one(). Ignores hidden tables.
   */
  sharedptr<Relationship> get_field_used_in_relationship_to_one(const Glib::ustring& table_name, const Glib::ustring& field_name) const;


  typedef std::vector< sharedptr<Field> > type_vecFields;
  type_vecFields get_table_fields(const Glib::ustring& table_name) const;
  void set_table_fields(const Glib::ustring& table_name, const type_vecFields& vecFields);

  sharedptr<Field> get_field(const Glib::ustring& table_name, const Glib::ustring& strFieldName) const;

  /** Use this after removing a field from a table,
   * so that it is not used anymore in relationships, layouts, reports, etc.
   */
  void remove_field(const Glib::ustring& table_name, const Glib::ustring& field_name);


  typedef std::vector< sharedptr<LayoutGroup> > type_list_layout_groups;
  type_list_layout_groups get_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name) const;

  void set_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name, const type_list_layout_groups& groups);

  /**
   * @para The layout_name, such as "details", "list".
   * @para parent_table_name The name of the table on whose layout the layout appears.
   */
  type_list_layout_groups get_data_layout_groups_plus_new_fields(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name) const;

  type_list_layout_groups get_data_layout_groups_default(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name) const;

  typedef std::list< sharedptr<TranslatableItem> > type_list_translatables;
  type_list_translatables get_translatable_layout_items(const Glib::ustring& table_name);
  type_list_translatables get_translatable_report_items(const Glib::ustring& table_name, const Glib::ustring& report_title);

  void fill_layout_field_details(const Glib::ustring& parent_table_name, const sharedptr<LayoutGroup>& layout_group) const;
  void fill_layout_field_details(const Glib::ustring& parent_table_name, type_list_layout_groups& groups) const;



  ///When a field name is changed, change it in the relationships, layouts, reports, and fields data:
  void change_field_name(const Glib::ustring& table_name, const Glib::ustring& strFieldNameOld, const Glib::ustring& strFieldNameNew);

  ///When a table name is changed, change it in the relationships and tables data:
  void change_table_name(const Glib::ustring& table_name_old, const Glib::ustring& table_name_new);

  ///When a relationship name is changed, change it in layouts and reports:
  void change_relationship_name(const Glib::ustring& table_name, const Glib::ustring& name, const Glib::ustring& name_new);

  typedef std::list< sharedptr<TableInfo> > type_listTableInfo;
  type_listTableInfo get_tables(bool plus_system_prefs = false) const;
  std::vector<Glib::ustring> get_table_names(bool plus_system_prefs = false) const;

  void set_tables(const type_listTableInfo& tables);

  sharedptr<TableInfo> get_table(const Glib::ustring& table_name) const;
  void add_table(const  sharedptr<TableInfo>& table_name);

  /** Use this after DROPing the table.
   * It removes information about the table, including fields and layouts,
   * and any place that parts of the table are used.
   */
  void remove_table(const Glib::ustring& table_name);

  bool get_table_is_known(const Glib::ustring& table_name) const;
  bool get_table_is_hidden(const Glib::ustring& table_name) const;

  Glib::ustring get_table_title(const Glib::ustring& table_name) const;
  void set_table_title(const Glib::ustring& table_name, const Glib::ustring& value);

  /** Save example data into the document, for use when creating the example database on the server.
   * Don't use this for large amounts of data.
   * @param table_name The table that should contain this example data.
   * @param rows Each row is separated by a newline. Each line has comma-separated field values, in SQL format.
   */
  void set_table_example_data(const Glib::ustring& table_name, const Glib::ustring& rows);

  Glib::ustring get_table_example_data(const Glib::ustring& table_name) const;

  virtual Glib::ustring get_name() const; //override.

  Glib::ustring get_default_table() const;
  Glib::ustring get_first_table() const;

  Glib::ustring get_database_title() const;
  void set_database_title(const Glib::ustring& title);

  std::vector<Glib::ustring> get_library_module_names() const;
  void set_library_module(const Glib::ustring& name, const Glib::ustring& script);
  Glib::ustring get_library_module(const Glib::ustring& name) const;
  void remove_library_module(const Glib::ustring& name);

  /// These are only used when recreating a database from an example file. The actualy access-control is on the server, of course.
  typedef std::list<GroupInfo> type_list_groups;
  type_list_groups get_groups() const;

  /// This adds the group if necessary.
  void set_group(GroupInfo& group);

  void remove_group(const Glib::ustring& group_name);


  typedef std::list<Glib::ustring> type_listReports;
  type_listReports get_report_names(const Glib::ustring& table_name) const;
  void remove_all_reports(const Glib::ustring& table_name);

  void set_report(const Glib::ustring& table_name, const sharedptr<Report>& report);
  sharedptr<Report> get_report(const Glib::ustring& table_name, const Glib::ustring& report_name) const;
  void remove_report(const Glib::ustring& table_name, const Glib::ustring& report_name);

  //Print Layouts are precisely positioned layouts for printing to a printer:
  typedef std::list<Glib::ustring> type_listPrintLayouts;
  type_listPrintLayouts get_print_layout_names(const Glib::ustring& table_name) const;
  void remove_all_print_layouts(const Glib::ustring& table_name);
  void set_print_layout(const Glib::ustring& table_name, const sharedptr<PrintLayout>& print_layout);
  sharedptr<PrintLayout> get_print_layout(const Glib::ustring& table_name, const Glib::ustring& print_layout_name) const;
  void remove_print_layout(const Glib::ustring& table_name, const Glib::ustring& print_layout_name);

  void set_layout_record_viewed(const Glib::ustring& table_name, const Glib::ustring& layout_name, const Gnome::Gda::Value& primary_key_value);
  void forget_layout_record_viewed(const Glib::ustring& table_name);
  Gnome::Gda::Value get_layout_record_viewed(const Glib::ustring& table_name, const Glib::ustring& layout_name) const;

  void set_layout_current(const Glib::ustring& table_name, const Glib::ustring& layout_name);
  Glib::ustring get_layout_current(const Glib::ustring& table_name) const;

  // Used by Relationship Overview dialog to preserve table locations accross instantiations:
    
    /**
     * Retrieve the x and y coordinates for the given table position.
     * 
     * @param table_name The name of the table to query.
     * @param x The x coordinate of the table position.
     * @param y The y coordinate of the table position.
     * @return false if the table does not have any 
     */
    bool get_table_overview_position ( const Glib::ustring &table_name, float &x, float &y ) const;
    
    /**
     * Set the position of a table in the relationship overview dialog.
     * 
     * @param table_name The name of the table to modify.
     * @param x The x coordinate of the table position.
     * @param y The y coordinate of the table position.
     */
    void set_table_overview_position ( const Glib::ustring &table_name, float x, float y );
    
  enum userLevelReason
  {
    USER_LEVEL_REASON_UNKNOWN,
    USER_LEVEL_REASON_FILE_READ_ONLY,
    USER_LEVEL_REASON_DATABASE_ACCESS_LEVEL,
    USER_LEVEL_REASON_OPENED_FROM_BROWSE
  };

  /**
   * @param reason The reason that the user is not a developer, if he is not.
   * @result Whether the user is a developer.
   */
  AppState::userlevels get_userlevel(userLevelReason& reason) const;

  AppState::userlevels get_userlevel() const;

  /** This is transitory information, not saved to disk.
   */
  bool set_userlevel(AppState::userlevels userlevel);

  typedef sigc::signal<void, AppState::userlevels> type_signal_userlevel_changed;
  type_signal_userlevel_changed signal_userlevel_changed();

  void emit_userlevel_changed();

  void set_parent_window(Gtk::Window* window);

  Glib::ustring build_and_get_contents() const;

  static sharedptr<TableInfo> create_table_system_preferences();
  static sharedptr<TableInfo> create_table_system_preferences(type_vecFields& fields);
  static sharedptr<Relationship>  create_relationship_system_preferences(const Glib::ustring& table_name);
  static bool get_relationship_is_system_properties(const sharedptr<const Relationship>& relationship);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  ///If value is equal to the default then no attribute will be set, to save text space in the XML file.
  static void set_node_attribute_value_as_decimal(xmlpp::Element* node, const Glib::ustring& strAttributeName, guint value, guint value_default = 0);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  // This is required by the report builder, so it cannot be disabled
  // in client only mode
  static void set_node_attribute_value_as_decimal_double(xmlpp::Element* node, const Glib::ustring& strAttributeName, double value);

protected:

  //Overrides:


#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual bool save_before();
  void save_before_layout_group(xmlpp::Element* node, const sharedptr<const LayoutGroup>& group, bool with_print_layout_positions = false);
  void save_before_sort_by(xmlpp::Element* node, const LayoutItem_GroupBy::type_list_sort_fields& list_fields);
  void save_before_layout_item_usesrelationship(xmlpp::Element* nodeItem, const sharedptr<const UsesRelationship>& item);
  void save_before_layout_item_field(xmlpp::Element* nodeItem, const sharedptr<const LayoutItem_Field>& item);
  void save_before_layout_item_formatting(xmlpp::Element* nodeItem, const FieldFormatting& format, Field::glom_field_type field_type = Field::TYPE_INVALID);

  void save_before_translations(xmlpp::Element* nodeItem, const TranslatableItem& item);
  void save_before_print_layout_position(xmlpp::Element* nodeItem, const sharedptr<const LayoutItem>& item);

  void save_changes();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  virtual bool load_after();
  void load_after_layout_group(const xmlpp::Element* node, const Glib::ustring& table_name, const sharedptr<LayoutGroup>& group, bool with_print_layout_positions = false);
  void load_after_sort_by(const xmlpp::Element* node, const Glib::ustring& table_name, LayoutItem_GroupBy::type_list_sort_fields& list_fields);
  void load_after_layout_item_usesrelationship(const xmlpp::Element* element, const Glib::ustring& table_name, const sharedptr<UsesRelationship>& item);
  void load_after_layout_item_field(const xmlpp::Element* element, const Glib::ustring& table_name, const sharedptr<LayoutItem_Field>& item);
  void load_after_layout_item_formatting(const xmlpp::Element* element, FieldFormatting& format, Field::glom_field_type field_type = Field::TYPE_INVALID, const Glib::ustring& table_name = Glib::ustring(), const Glib::ustring& field_name = Glib::ustring());

  void load_after_translations(const xmlpp::Element* element, TranslatableItem& item);
  void load_after_print_layout_position(const xmlpp::Element* nodeItem, const sharedptr<LayoutItem>& item);

  void on_app_state_userlevel_changed(AppState::userlevels userlevel);

  void fill_translatable_layout_items(const sharedptr<LayoutGroup>& group, type_list_translatables& the_list);


  /// If the attribute is not there, then the default will be returned.
  static bool get_node_attribute_value_as_bool(const xmlpp::Element* node, const Glib::ustring& strAttributeName, bool value_default = false);
  static guint get_node_attribute_value_as_decimal(const xmlpp::Element* node, const Glib::ustring& strAttributeName, guint value_default = 0);
  static double get_node_attribute_value_as_decimal_double(const xmlpp::Element* node, const Glib::ustring& strAttributeName);
  static float get_node_attribute_value_as_float(const xmlpp::Element* node, const Glib::ustring& strAttributeName);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  static void set_node_attribute_value_as_bool(xmlpp::Element* node, const Glib::ustring& strAttributeName, bool value = true, bool value_default = false);
  static void set_node_attribute_value_as_float( xmlpp::Element* node, const Glib::ustring& strAttributeName, float value );
  static void set_node_attribute_value_as_value(xmlpp::Element* node, const Glib::ustring& strAttributeName, const Gnome::Gda::Value& value, Field::glom_field_type field_type);
#endif // !GLOM_ENABLE_CLIENT_ONLY
  static Gnome::Gda::Value get_node_attribute_value_as_value(const xmlpp::Element* node, const Glib::ustring& strAttributeName, Field::glom_field_type field_type);

  Glib::ustring get_child_text_node(const xmlpp::Element* node, const Glib::ustring& child_node_name) const;
  void set_child_text_node(xmlpp::Element* node, const Glib::ustring& child_node_name, const Glib::ustring& text);

  void append_newline(xmlpp::Element* parent_node);

  AppState m_app_state;
  type_signal_userlevel_changed m_signal_userlevel_changed;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  bool m_connection_is_self_hosted;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  Glib::ustring m_connection_server, m_connection_user, m_connection_database;
  int m_connection_port; //0 means any port. Ignored when self-hosting (which may use a different port each time).
  bool m_connection_try_other_ports; //Set to false for self-hosted or browsed-from-network documents.

  class LayoutInfo
  {
  public:
    Glib::ustring m_layout_name;
    Glib::ustring m_parent_table;

    type_list_layout_groups m_layout_groups;
  };

  class DocumentTableInfo
  {
  public:
    DocumentTableInfo()
        : m_overviewx ( std::numeric_limits<float>::infinity () ),
          m_overviewy ( std::numeric_limits<float>::infinity () )
    {
      m_info = sharedptr<TableInfo>(new TableInfo()); //Avoid a null sharedptr.
    }

    sharedptr<TableInfo> m_info;

    type_vecFields m_fields;
    type_vecRelationships m_relationships;

    typedef std::list< LayoutInfo > type_layouts;
    type_layouts m_layouts;

    typedef std::map< Glib::ustring, sharedptr<Report> > type_reports; //map of report names to reports
    type_reports m_reports;

    typedef std::map< Glib::ustring, sharedptr<PrintLayout> > type_print_layouts; //map of print layout names to print layouts
    type_print_layouts m_print_layouts;

    Glib::ustring m_example_rows;

    //Per-session, not saved in document:
    typedef std::map<Glib::ustring, Gnome::Gda::Value> type_map_layout_primarykeys;
    type_map_layout_primarykeys m_map_current_record; //The record last viewed in each layout.

    Glib::ustring m_layout_current;
    
    float m_overviewx, m_overviewy;
  };

  DocumentTableInfo& get_table_info_with_add(const Glib::ustring& table_name);

  typedef std::map<Glib::ustring, DocumentTableInfo> type_tables;
  type_tables m_tables;


  //User groups:
  typedef std::map<Glib::ustring, GroupInfo> type_map_groups;
  type_map_groups m_groups;

  Glib::ustring m_database_title;
  Glib::ustring m_translation_original_locale;

  typedef std::map<Glib::ustring, Glib::ustring> type_map_library_scripts;
  type_map_library_scripts m_map_library_scripts;

  bool m_block_cache_update; //For efficiency.
  bool m_block_modified_set;
#ifndef GLOM_ENABLE_CLIENT_ONLY
  bool m_allow_auto_save;
#endif // !GLOM_ENABLE_CLIENT_ONLY
  bool m_is_example;
  guint m_document_format_version;

  bool m_opened_from_browse;

  Gtk::Window* m_parent_window; //Needed by BusyCursor.
};

//The base View for this document;
typedef Bakery::View<Document_Glom> View_Glom;
typedef Bakery::View_Composite<Document_Glom> View_Composite_Glom;

} //namespace Glom

#endif //DOCUMENT_GLOM_H
