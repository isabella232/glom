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
#include "../appstate.h"
#include <vector>
#include <map>

class Document_Glom : public Bakery::Document_XML
{
public: 
  Document_Glom();
  virtual ~Document_Glom();

  virtual void set_modified(bool value = true);

  virtual void set_connection_server(const Glib::ustring& strVal);
  virtual void set_connection_user(const Glib::ustring& strVal);
  virtual void set_connection_database(const Glib::ustring& strVal);

  virtual Glib::ustring get_connection_server();
  virtual Glib::ustring get_connection_user();
  virtual Glib::ustring get_connection_database();

  typedef std::vector<Relationship> type_vecRelationships;
  virtual type_vecRelationships get_relationships(const Glib::ustring& table_name) const;
  virtual void set_relationships(const Glib::ustring& table_name, const type_vecRelationships& vecRelationships);

  virtual bool get_relationship(const Glib::ustring& table_name, const Glib::ustring& relationship_name, Relationship& relationship) const;
  
  typedef std::vector<Field> type_vecFields;
  virtual type_vecFields get_table_fields(const Glib::ustring& table_name) const;
  virtual void set_table_fields(const Glib::ustring& table_name, const type_vecFields& vecFields);

  virtual bool get_field(const Glib::ustring& table_name, const Glib::ustring& strFieldName, Field& fieldResult) const;


  typedef std::map<guint, LayoutGroup> type_mapLayoutGroupSequence;
  virtual type_mapLayoutGroupSequence get_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& table_name) const;
  virtual void set_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& table_name, const type_mapLayoutGroupSequence& groups);

  virtual type_mapLayoutGroupSequence get_data_layout_groups_plus_new_fields(const Glib::ustring& layout_name, const Glib::ustring& table_name) const;

        
  ///When a field name is changed, change it in the relationships and fields data:
  virtual void change_field_name(const Glib::ustring& table_name, const Glib::ustring& strFieldNameOld, const Glib::ustring& strFieldNameNew);

  ///When a table name is changed, change it in the relationships and tables data:
  virtual void change_table_name(const Glib::ustring& strTableNameOld, const Glib::ustring& strTableNameNew);

  typedef std::list<TableInfo> type_listTableInfo;
  virtual type_listTableInfo get_tables() const;
  virtual void set_tables(const type_listTableInfo& tables);

  virtual bool get_table_is_known(const Glib::ustring& table_name) const;
    
  virtual Glib::ustring get_table_title(const Glib::ustring& table_name) const;
  virtual void set_table_title(const Glib::ustring& table_name, const Glib::ustring& value);

  virtual Glib::ustring get_name() const; //override.
  
  virtual Glib::ustring get_default_table() const;

  virtual Glib::ustring get_database_title() const;
  virtual void set_database_title(const Glib::ustring& title);
  
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
    
protected:
 
  //Overrides:
  virtual bool load_after();
  virtual bool save_before();

  virtual void load_after_layout_group(const xmlpp::Element* node, LayoutGroup& group);
  virtual void save_before_layout_group(xmlpp::Element* node, const LayoutGroup& group);
  
  virtual void on_app_state_userlevel_changed(AppState::userlevels userleve);
  
  static bool get_node_attribute_value_as_bool(const xmlpp::Element* node, const Glib::ustring& strAttributeName);
  static void set_node_attribute_value_as_bool(xmlpp::Element* node, const Glib::ustring& strAttributeName, bool value = true);

  static void set_node_attribute_value_as_decimal(xmlpp::Element* node, const Glib::ustring& strAttributeName, int value);
  static guint get_node_attribute_value_as_decimal(const xmlpp::Element* node, const Glib::ustring& strAttributeName);

  AppState m_app_state;
  type_signal_userlevel_changed m_signal_userlevel_changed;

  Glib::ustring m_connection_server, m_connection_user, m_connection_database;
  
  class DocumentTableInfo
  {
  public:
    TableInfo m_info;
      
    type_vecFields m_fields;
    type_vecRelationships m_relationships;
      
    //Map layout names to layouts:
    typedef std::map<Glib::ustring, type_mapLayoutGroupSequence> type_layouts;
    type_layouts m_layouts;
  };
  
  DocumentTableInfo& get_table_info_with_add(const Glib::ustring& table_name);
  
  typedef std::map<Glib::ustring, DocumentTableInfo> type_tables;
  type_tables m_tables;

  Glib::ustring m_database_title;
};

//The base View for this document;
typedef Bakery::View<Document_Glom> View_Glom;
typedef Bakery::View_Composite<Document_Glom> View_Composite_Glom;


#endif //DOCUMENT_GLOM_H
