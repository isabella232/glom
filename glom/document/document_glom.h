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

#include <bakery/Document/Document_XML.h>
#include <bakery/View/View_Composite.h>
#include "../data_structure/relationship.h"
#include "../data_structure/field.h"
#include "../data_structure/layout/layoutgroup.h"
#include "../data_structure/layout/layoutitem_portal.h"
#include "../data_structure/tableinfo.h"
#include "../data_structure/groupinfo.h"
#include "../data_structure/report.h"
#include "../appstate.h"
#include <vector>
#include <map>

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

  virtual void set_modified(bool value = true);

  virtual void set_connection_server(const Glib::ustring& strVal);
  virtual void set_connection_user(const Glib::ustring& strVal);
  virtual void set_connection_database(const Glib::ustring& strVal);

  virtual Glib::ustring get_connection_server() const;
  virtual Glib::ustring get_connection_user() const;
  virtual Glib::ustring get_connection_database() const;

  typedef std::vector<Relationship> type_vecRelationships;
  virtual type_vecRelationships get_relationships(const Glib::ustring& table_name) const;
  virtual void set_relationships(const Glib::ustring& table_name, const type_vecRelationships& vecRelationships);

  virtual bool get_relationship(const Glib::ustring& table_name, const Glib::ustring& relationship_name, Relationship& relationship) const;
  virtual void set_relationship(const Glib::ustring& table_name, const Relationship& relationship);

  virtual void remove_relationship(const Relationship& relationship);

  typedef std::vector<Field> type_vecFields;
  virtual type_vecFields get_table_fields(const Glib::ustring& table_name) const;
  virtual void set_table_fields(const Glib::ustring& table_name, const type_vecFields& vecFields);

  virtual bool get_field(const Glib::ustring& table_name, const Glib::ustring& strFieldName, Field& fieldResult) const;


  typedef std::map<guint, LayoutGroup> type_mapLayoutGroupSequence;
  virtual type_mapLayoutGroupSequence get_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name) const;

  virtual void set_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name, const type_mapLayoutGroupSequence& groups, const Glib::ustring& table_name = Glib::ustring());

  virtual void set_relationship_data_layout_groups(const Glib::ustring& layout_name, const Relationship& relationship, const type_mapLayoutGroupSequence& groups);

  /**
   * @para The layout_name, such as "details", "list".
   * @para parent_table_name The name of the table on whose layout the layout appears.
   */
  virtual type_mapLayoutGroupSequence get_data_layout_groups_plus_new_fields(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name) const;

  virtual void fill_layout_field_details(const Glib::ustring& parent_table_name, LayoutGroup& layout_group) const;
  virtual void fill_layout_field_details(const Glib::ustring& parent_table_name, type_mapLayoutGroupSequence& sequence) const;


  ///When a field name is changed, change it in the relationships, layouts, reports, and fields data:
  virtual void change_field_name(const Glib::ustring& table_name, const Glib::ustring& strFieldNameOld, const Glib::ustring& strFieldNameNew);

  ///When a table name is changed, change it in the relationships and tables data:
  virtual void change_table_name(const Glib::ustring& strTableNameOld, const Glib::ustring& strTableNameNew);
  
  ///When a relationship name is changed, change it in layouts and reports:
  virtual void change_relationship_name(const Glib::ustring& table_name, const Glib::ustring& name, const Glib::ustring& name_new);

  typedef std::list<TableInfo> type_listTableInfo;
  virtual type_listTableInfo get_tables() const;
  virtual void set_tables(const type_listTableInfo& tables);

  /** Use this after DROPing the table.
   * It removes information about the table, including fields and layouts,
   * and any place that parts of the table are used.
   */
  virtual void remove_table(const Glib::ustring& table_name);

  virtual bool get_table_is_known(const Glib::ustring& table_name) const;

  virtual Glib::ustring get_table_title(const Glib::ustring& table_name) const;
  virtual void set_table_title(const Glib::ustring& table_name, const Glib::ustring& value);

  virtual Glib::ustring get_name() const; //override.

  virtual Glib::ustring get_default_table() const;
  virtual Glib::ustring get_first_table() const;

  virtual Glib::ustring get_database_title() const;
  virtual void set_database_title(const Glib::ustring& title);

  /// These are only used when recreating a database from an example file. The actualy access-control is on the server, of course.
  typedef std::list<GroupInfo> type_list_groups;
  type_list_groups get_groups() const;

  /// This adds the group if necessary.
  void set_group(GroupInfo& group);

  void remove_group(const Glib::ustring& group_name);


  typedef std::list<Glib::ustring> type_listReports;
  type_listReports get_report_names(const Glib::ustring& table_name) const;
  void remove_all_reports(const Glib::ustring& table_name);

  void set_report(const Glib::ustring& table_name, const Report& report);
  bool get_report(const Glib::ustring& table_name, const Glib::ustring& report_name, Report& report) const;
  void remove_report(const Glib::ustring& table_name, const Glib::ustring& report_name);

  enum userLevelReason
  {
    USER_LEVEL_REASON_UNKNOWN,
    USER_LEVEL_REASON_FILE_READ_ONLY,
    USER_LEVEL_REASON_DATABASE_ACCESS_LEVEL
  };

  /**
   * @param reason The reason that the user is not a developer, if he is not.
   * @result Whether the user is a developer.
   */
  virtual AppState::userlevels get_userlevel(userLevelReason& reason) const;

  virtual AppState::userlevels get_userlevel() const;

  /** This is transitory information, not saved to disk.
   */
  virtual bool set_userlevel(AppState::userlevels userlevel);

  typedef sigc::signal<void, AppState::userlevels> type_signal_userlevel_changed;
  type_signal_userlevel_changed signal_userlevel_changed();

  virtual void emit_userlevel_changed();

  /** The LayouItem_Field contains cached full relationship information,
   *  so we don't have to look up the details so often. This method
   *  updates that cache. It should be called when the relationship details change.
   */
  void update_cached_relationships();
  void update_cached_relationships(LayoutGroup& group, const Glib::ustring& table_name);
  void update_cached_relationships(FieldFormatting& formatting, const Glib::ustring& table_name);

protected:

  //Overrides:
  virtual bool load_after();
  virtual bool save_before();

  void load_after_layout_group(const xmlpp::Element* node, const Glib::ustring table_name, LayoutGroup& group);
  void save_before_layout_group(xmlpp::Element* node, const LayoutGroup& group);

  void load_after_layout_item_field(const xmlpp::Element* element, LayoutItem_Field& item);
  void load_after_layout_item_field_formatting(const xmlpp::Element* element, FieldFormatting& format, const Field& layout_item);
  void save_before_layout_item_field(xmlpp::Element* nodeItem, const LayoutItem_Field& item);
  void save_before_layout_item_field_formatting(xmlpp::Element* nodeItem, const FieldFormatting& format, const Field& layout_item);

  virtual void on_app_state_userlevel_changed(AppState::userlevels userleve);

  static bool get_node_attribute_value_as_bool(const xmlpp::Element* node, const Glib::ustring& strAttributeName);
  static void set_node_attribute_value_as_bool(xmlpp::Element* node, const Glib::ustring& strAttributeName, bool value = true);

  static void set_node_attribute_value_as_decimal(xmlpp::Element* node, const Glib::ustring& strAttributeName, int value);
  static guint get_node_attribute_value_as_decimal(const xmlpp::Element* node, const Glib::ustring& strAttributeName);

  static void set_node_attribute_value_as_value(xmlpp::Element* node, const Glib::ustring& strAttributeName, const Gnome::Gda::Value& value, Field::glom_field_type field_type);
  static Gnome::Gda::Value get_node_attribute_value_as_value(const xmlpp::Element* node, const Glib::ustring& strAttributeName, Field::glom_field_type field_type);

  AppState m_app_state;
  type_signal_userlevel_changed m_signal_userlevel_changed;

  Glib::ustring m_connection_server, m_connection_user, m_connection_database;

  class LayoutInfo
  {
  public:
    Glib::ustring m_layout_name;
    Glib::ustring m_parent_table;

    type_mapLayoutGroupSequence m_layout_groups;
  };

  class DocumentTableInfo
  {
  public:
    TableInfo m_info;

    type_vecFields m_fields;
    type_vecRelationships m_relationships;

    typedef std::list< LayoutInfo > type_layouts;
    type_layouts m_layouts;

    typedef std::map<Glib::ustring, Report> type_reports; //map of report names to reports
    type_reports m_reports;
  };

  DocumentTableInfo& get_table_info_with_add(const Glib::ustring& table_name);

  typedef std::map<Glib::ustring, DocumentTableInfo> type_tables;
  type_tables m_tables;


  //User groups:
  typedef std::map<Glib::ustring, GroupInfo> type_map_groups;
  type_map_groups m_groups;

  Glib::ustring m_database_title;

  bool m_block_cache_update; //For efficiency.
  bool m_block_modified_set;
};

//The base View for this document;
typedef Bakery::View<Document_Glom> View_Glom;
typedef Bakery::View_Composite<Document_Glom> View_Composite_Glom;


#endif //DOCUMENT_GLOM_H
