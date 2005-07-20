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

#include "document_glom.h"
#include "../data_structure/glomconversions.h"
#include "../data_structure/layout/report_parts/layoutitem_summary.h"
#include "../data_structure/layout/report_parts/layoutitem_fieldsummary.h"
//#include "config.h" //To get GLOM_DTD_INSTALL_DIR - dependent on configure prefix.
#include <algorithm> //For std::find_if().
#include <sstream> //For stringstream

#define GLOM_NODE_CONNECTION "connection"
#define GLOM_ATTRIBUTE_SERVER "server"
#define GLOM_ATTRIBUTE_USER "user"
#define GLOM_ATTRIBUTE_DATABASE "database"

#define GLOM_NODE_DATA_LAYOUT_GROUPS "data_layout_groups"
#define GLOM_NODE_DATA_LAYOUT_GROUP "data_layout_group"
#define GLOM_ATTRIBUTE_COLUMNS_COUNT "columns_count"

#define GLOM_NODE_DATA_LAYOUTS "data_layouts"
#define GLOM_NODE_DATA_LAYOUT "data_layout"
#define GLOM_ATTRIBUTE_PARENT_TABLE_NAME "parent_table"

#define GLOM_NODE_DATA_LAYOUT_PORTAL "data_layout_portal"
#define GLOM_NODE_DATA_LAYOUT_ITEM "data_layout_item"
#define GLOM_ATTRIBUTE_DATA_LAYOUT_ITEM_FIELD_USE_DEFAULT_FORMATTING "use_default_formatting"
#define GLOM_NODE_DATA_LAYOUT_ITEM_GROUPBY "data_layout_item_groupby"
#define GLOM_NODE_DATA_LAYOUT_GROUP_SECONDARYFIELDS "secondary_fields"
#define GLOM_NODE_DATA_LAYOUT_ITEM_SUMMARY "data_layout_item_summary"
#define GLOM_NODE_DATA_LAYOUT_ITEM_FIELDSUMMARY "data_layout_item_fieldsummary"
#define GLOM_NODE_TABLE "table"
#define GLOM_NODE_FIELDS "fields"
#define GLOM_NODE_FIELD "field"
#define GLOM_ATTRIBUTE_PRIMARY_KEY "primary_key"
#define GLOM_ATTRIBUTE_DEFAULT_VALUE "default_value"
#define GLOM_ATTRIBUTE_UNIQUE "unique"
#define GLOM_ATTRIBUTE_AUTOINCREMENT "auto_increment"
#define GLOM_ATTRIBUTE_CALCULATION "calculation"
#define GLOM_ATTRIBUTE_TYPE "type"

#define GLOM_NODE_FIELD_LOOKUP "field_lookup"
#define GLOM_NODE_RELATIONSHIPS "relationships"
#define GLOM_NODE_RELATIONSHIP "relationship"
#define GLOM_ATTRIBUTE_KEY "key"
#define GLOM_ATTRIBUTE_OTHER_TABLE "other_table"
#define GLOM_ATTRIBUTE_OTHER_KEY "other_key"
#define GLOM_ATTRIBUTE_AUTO_CREATE "auto_create"
#define GLOM_ATTRIBUTE_ALLOW_EDIT "allow_edit"

#define GLOM_NODE_GROUPS "groups"
#define GLOM_NODE_GROUP "group"
#define GLOM_ATTRIBUTE_DEVELOPER "developer"
#define GLOM_NODE_TABLE_PRIVS "table_privs"
#define GLOM_ATTRIBUTE_TABLE_NAME "table_name"
#define GLOM_ATTRIBUTE_PRIV_VIEW "priv_view"
#define GLOM_ATTRIBUTE_PRIV_EDIT "priv_edit"
#define GLOM_ATTRIBUTE_PRIV_CREATE "priv_create"
#define GLOM_ATTRIBUTE_PRIV_DELETE "priv_delete"

#define GLOM_ATTRIBUTE_DATABASE_TITLE "database_title"
#define GLOM_ATTRIBUTE_NAME "name"
#define GLOM_ATTRIBUTE_TITLE "title"
#define GLOM_ATTRIBUTE_SEQUENCE "sequence"
#define GLOM_ATTRIBUTE_HIDDEN "hidden"
#define GLOM_ATTRIBUTE_DEFAULT "default"
#define GLOM_ATTRIBUTE_FIELD "field"
#define GLOM_ATTRIBUTE_EDITABLE "editable"



#define GLOM_ATTRIBUTE_RELATIONSHIP_NAME "relationship"

#define GLOM_NODE_REPORTS "reports"
#define GLOM_NODE_REPORT "report"
#define GLOM_ATTRIBUTE_REPORT_ITEM_GROUPBY_GROUPBY "groupby"
#define GLOM_ATTRIBUTE_REPORT_ITEM_GROUPBY_SORTBY "sortby"
#define GLOM_ATTRIBUTE_LAYOUT_ITEM_FIELDSUMMARY_SUMMARYTYPE "summarytype"

#define GLOM_NODE_FORMAT "formatting"
#define GLOM_ATTRIBUTE_FORMAT_THOUSANDS_SEPARATOR "format_thousands_separator"
#define GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES_RESTRICTED "format_decimal_places_restricted"
#define GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES "format_decimal_places"
#define GLOM_ATTRIBUTE_FORMAT_CURRENCY_SYMBOL "format_currency_symbol"

#define GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE "format_text_multiline"

#define GLOM_ATTRIBUTE_FORMAT_CHOICES_RESTRICTED "choices_restricted"
#define GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM "choices_custom"
#define GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM_LIST "custom_choice_list"
#define GLOM_NODE_FORMAT_CUSTOM_CHOICE "custom_choice"
#define GLOM_ATTRIBUTE_VALUE "value"
#define GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED "choices_related"
#define GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_RELATIONSHIP "choices_related_relationship"
#define GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_FIELD "choices_related_field"
#define GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SECOND "choices_related_second"

Document_Glom::Document_Glom()
: m_block_cache_update(false),
  m_block_modified_set(false)
{
  //Conscious use of virtual methods in a constructor:
  set_file_extension("glom");

  set_dtd_name("glom_document.dtd");
  //set_DTD_Location(GLOM_DTD_INSTALL_DIR); //Determined at configure time. It still looks in the working directory first.

  set_dtd_root_node_name("glom_document");

  set_write_formatted(); //Make the output more human-readable, just in case.

  //Set default database name:
  //This is also the XML attribute default value,
  //but that isn't available for new documents.
  if(get_connection_server().empty())
    set_connection_server("localhost");

  m_app_state.signal_userlevel_changed().connect( sigc::mem_fun(*this, &Document_Glom::on_app_state_userlevel_changed) );
}

Document_Glom::~Document_Glom()
{
}

Glib::ustring Document_Glom::get_connection_user() const
{
  return m_connection_user;
}

Glib::ustring Document_Glom::get_connection_server() const
{
  return m_connection_server;
}

Glib::ustring Document_Glom::get_connection_database() const
{
  return m_connection_database;
}

void Document_Glom::set_connection_user(const Glib::ustring& strVal)
{
  if(strVal != m_connection_user)
  {
    m_connection_user = strVal;
    set_modified();
  }
}

void Document_Glom::set_connection_server(const Glib::ustring& strVal)
{
  if(strVal != m_connection_server)
  {
    m_connection_server = strVal;
    set_modified();
  }
}

void Document_Glom::set_connection_database(const Glib::ustring& strVal)
{
  if(strVal != m_connection_database)
  {
    m_connection_database = strVal;
    set_modified();
  }
}

void Document_Glom::set_relationship(const Glib::ustring& table_name, const Relationship& relationship)
{
  //Find the existing relationship:
  type_tables::iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    DocumentTableInfo& info = iterFind->second;

    //Look for the relationship with this name:
    bool existing = false;
    const Glib::ustring relationship_name = relationship.get_name();

    for(type_vecRelationships::iterator iter = info.m_relationships.begin(); iter != info.m_relationships.end(); ++iter)
    {
      if(iter->get_name() == relationship_name)
      {
        *iter = relationship;
        existing = true;
      } 
    }

    if(!existing)
    {
      //Add a new one if it's not there.
      info.m_relationships.push_back(relationship);
    }
  }

  update_cached_relationships();
}

bool Document_Glom::get_relationship(const Glib::ustring& table_name, const Glib::ustring& relationship_name, Relationship& relationship) const
{
  //Initialize output parameter:
  relationship = Relationship();

  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    const DocumentTableInfo& info = iterFind->second;

    //Look for the relationship with this name:
    for(type_vecRelationships::const_iterator iter = info.m_relationships.begin(); iter != info.m_relationships.end(); ++iter)
    {
      if(iter->get_name() == relationship_name)
      {
        relationship = *iter;
        return true; //found
      }
    }
  }

  return false; //Not found.
}

  
Document_Glom::type_vecRelationships Document_Glom::get_relationships(const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
    return iterFind->second.m_relationships;
  else
    return type_vecRelationships(); 
}

void Document_Glom::set_relationships(const Glib::ustring& table_name, const type_vecRelationships& vecRelationships)
{
  if(!table_name.empty())
  {
    DocumentTableInfo& info = get_table_info_with_add(table_name);
    info.m_relationships = vecRelationships;

    set_modified();
  }
}

void Document_Glom::remove_relationship(const Relationship& relationship)
{
  //Get the table that this relationship is part of:
  type_tables::iterator iter = m_tables.find(relationship.get_from_table());
  if(iter != m_tables.end())
  {
    DocumentTableInfo& info = iter->second;

    //Find the relationship and remove it:
    for(type_vecRelationships::iterator iter = info.m_relationships.begin(); iter != info.m_relationships.end(); ++iter)
    {
      if(iter->get_name() == relationship.get_name())
      {
        iter = info.m_relationships.erase(iter);

        //TODO: Remove any lookups, view fields, or related records portals, that use this relationship.

        set_modified(true);
      }
    }
  }
}


void Document_Glom::remove_table(const Glib::ustring& table_name)
{
  type_tables::iterator iter = m_tables.find(table_name);
  if(iter != m_tables.end())
  {
    m_tables.erase(iter);
    set_modified(true);
  }

  //Remove any relationships that use this table:
  for(type_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
  {
    DocumentTableInfo& info = iter->second;
    
    if(!(info.m_relationships.empty()))
    {
      type_vecRelationships::iterator iterRel = info.m_relationships.begin();
      bool something_changed = true;
      while(something_changed && !info.m_relationships.empty())
      {
        if(iterRel->get_to_table() == table_name)
        {
          //Loop again, because we have changed the structure:
          remove_relationship(*iterRel); //Also removes anything that uses the relationship.
  
          something_changed = true;
          iterRel = info.m_relationships.begin();
        }
        else
        {
          ++iterRel;
          
          if(iterRel == info.m_relationships.end())
            something_changed = false; //We've looked at them all, without changing things.
        }
      }
    }

  }
}


Document_Glom::type_vecFields Document_Glom::get_table_fields(const Glib::ustring& table_name) const
{
  if(!table_name.empty())
  {
    type_tables::const_iterator iterFind = m_tables.find(table_name);
    if(iterFind != m_tables.end())
    {
      if(iterFind->second.m_fields.empty())
      {
         g_warning("Document_Glom::get_table_fields: table found, but m_fields is empty");
      }

      return iterFind->second.m_fields;
    }
    else
    {
      //g_warning("Document_Glom::get_table_fields: table not found in document: %s", table_name.c_str());
      return type_vecFields();
    }
  }
  else
  {
    //g_warning("Document_Glom::get_table_fields: table name is empty.");
    return type_vecFields();
  }
}

void Document_Glom::set_table_fields(const Glib::ustring& table_name, const type_vecFields& vecFields)
{
  if(!table_name.empty())
  {
    if(vecFields.empty())
    {
      g_warning("Document_Glom::set_table_fields(): vecFields is empty: table_name=%s", table_name.c_str());
    }

    DocumentTableInfo& info = get_table_info_with_add(table_name);
    info.m_fields = vecFields;

    set_modified();
  }
}

bool Document_Glom::get_field(const Glib::ustring& table_name, const Glib::ustring& strFieldName, Field& fieldResult) const
{
  fieldResult = Field(); //Initialize output arg.

  type_vecFields vecFields = get_table_fields(table_name);
  type_vecFields::iterator iterFind = std::find_if( vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(strFieldName) );
  if(iterFind != vecFields.end()) //If it was found:
  {
    fieldResult = *iterFind;
    return true;
  }
  else
  {
    return false; //not found.
  }
}


void Document_Glom::change_field_name(const Glib::ustring& table_name, const Glib::ustring& strFieldNameOld, const Glib::ustring& strFieldNameNew)
{      
  type_tables::iterator iterFindTable = m_tables.find(table_name);
  if(iterFindTable != m_tables.end())
  {
    //Fields:
    type_vecFields& vecFields = iterFindTable->second.m_fields;
    type_vecFields::iterator iterFind = std::find_if( vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(strFieldNameOld) );
    if(iterFind != vecFields.end()) //If it was found:
    {
      //Change it:
      iterFind->set_name(strFieldNameNew);
    }

    //Find any relationships or layouts that use this field
    //Look at each table:
    for(type_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
    {
      //Look at each relationship in the table:
      for(type_vecRelationships::iterator iterRels = iter->second.m_relationships.begin(); iterRels != iter->second.m_relationships.end(); ++iterRels)
      {
        if(iterRels->get_from_table() == table_name)
        {
          if(iterRels->get_from_field() == strFieldNameOld)
          {
            //Change it:
            iterRels->set_from_field(strFieldNameNew);
          }
        }

        if(iterRels->get_to_table() == table_name)
        {
          if(iterRels->get_to_field() == strFieldNameOld)
          {
            //Change it:
            iterRels->set_to_field(strFieldNameNew);
          }
        }
      }
    }

    //Look at each layout:
    //TODO: Remember to change it in other layouts when we add the ability to show fields from other tables.
    for(DocumentTableInfo::type_layouts::iterator iterLayouts = iterFindTable->second.m_layouts.begin(); iterLayouts != iterFindTable->second.m_layouts.end(); ++iterLayouts)
    {
      //Look at each group:
      for(type_mapLayoutGroupSequence::iterator iterGroup = iterLayouts->m_layout_groups.begin(); iterGroup != iterLayouts->m_layout_groups.end(); ++iterGroup)
      {
        //Change the field if it is in this group:
        iterGroup->second.change_field_item_name(strFieldNameOld, strFieldNameNew);
      }
    }
   
    set_modified();
  }
}

void Document_Glom::change_table_name(const Glib::ustring& strTableNameOld, const Glib::ustring& strTableNameNew)
{
  type_tables::iterator iterFindTable = m_tables.find(strTableNameOld);
  if(iterFindTable != m_tables.end())
  {
    //Change it:
    //We can't just change the key of the iterator (I think),
    //so we copy the whole thing and put it back in the map under a different key:
    
    //iterFindTable->first = strTableNameNew;
    DocumentTableInfo doctableinfo = iterFindTable->second;
    m_tables.erase(iterFindTable);
  
    doctableinfo.m_info.m_name = strTableNameNew; 
    m_tables[strTableNameNew] = doctableinfo; 
    
    //Find any relationships or layouts that use this table
    //Look at each table:
    for(type_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
    {
      //Look at each relationship in the table:
      for(type_vecRelationships::iterator iterRels = iter->second.m_relationships.begin(); iterRels != iter->second.m_relationships.end(); ++iterRels)
      {
        if(iterRels->get_from_table() == strTableNameOld)
        {
          //Change it:
           iterRels->set_from_table(strTableNameNew);
        }

        if(iterRels->get_to_table() == strTableNameOld)
        {
          //Change it:
           iterRels->set_to_table(strTableNameNew);
        }           
      }
    }
    
    //TODO: Remember to change it in layouts when we add the ability to show fields from other tables.
 
    set_modified();
  }
}



bool Document_Glom::get_node_attribute_value_as_bool(const xmlpp::Element* node, const Glib::ustring& strAttributeName)
{
  Glib::ustring strValue = get_node_attribute_value(node, strAttributeName);
  return strValue == "true";
}
  
void Document_Glom::set_node_attribute_value_as_bool(xmlpp::Element* node, const Glib::ustring& strAttributeName, bool value)
{
  if(!value && !node->get_attribute(strAttributeName))
    return; //Use the non-existance of an attribute to mean false, to save space.

  Glib::ustring strValue = (value ? "true" : "false");
  set_node_attribute_value(node, strAttributeName, strValue);
}

void Document_Glom::set_node_attribute_value_as_decimal(xmlpp::Element* node, const Glib::ustring& strAttributeName, int value)
{
  if(!value && !node->get_attribute(strAttributeName))
    return; //Use the non-existance of an attribute to mean zero, to save space.

  //Get text representation of int:
  std::stringstream thestream;
  thestream.imbue( std::locale::classic() ); //The C locale.
  thestream << value;
  const Glib::ustring sequence_string = thestream.str();

  set_node_attribute_value(node, strAttributeName, sequence_string);
}

guint Document_Glom::get_node_attribute_value_as_decimal(const xmlpp::Element* node, const Glib::ustring& strAttributeName)
{
  guint result = 0;
  const Glib::ustring value_string = get_node_attribute_value(node, strAttributeName);

  //Get number for string:
  if(!value_string.empty())
  {
    //Visible fields, with sequence:
    std::stringstream thestream;
    thestream.imbue( std::locale::classic() ); //The C locale.
    thestream.str(value_string);
    thestream >> result;
  }

  return result;
}

void Document_Glom::set_node_attribute_value_as_value(xmlpp::Element* node, const Glib::ustring& strAttributeName, const Gnome::Gda::Value& value,  Field::glom_field_type field_type)
{
  NumericFormat format_ignored; //Because we use ISO format.
  const Glib::ustring value_as_text = GlomConversions::get_text_for_gda_value(field_type, value, std::locale() /* Use the C locale */, format_ignored, true /* ISO standard */);

  set_node_attribute_value(node, strAttributeName, value_as_text);
}

Gnome::Gda::Value Document_Glom::get_node_attribute_value_as_value(const xmlpp::Element* node, const Glib::ustring& strAttributeName, Field::glom_field_type field_type)
{
  const Glib::ustring value_string = get_node_attribute_value(node, strAttributeName);

  bool success = false;
  Gnome::Gda::Value  result = GlomConversions::parse_value(field_type, value_string, success, true /* iso_format */);
  if(success)
    return result;
  else 
    return Gnome::Gda::Value();
}



Document_Glom::type_listTableInfo Document_Glom::get_tables() const
{
  type_listTableInfo result;

  for(type_tables::const_iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
  {
    result.push_back(iter->second.m_info);
  }

  return result;
}

void Document_Glom::set_tables(const type_listTableInfo& tables)
{
  //TODO: Avoid adding information about tables that we don't know about - that should be done explicitly.
  //Look at each "table".

  bool something_changed = false;
  for(type_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); iter++)
  {
    const DocumentTableInfo& doctableinfo = iter->second;

    const Glib::ustring table_name = doctableinfo.m_info.m_name;

    type_listTableInfo::const_iterator iterfind = std::find_if(tables.begin(), tables.end(), predicate_FieldHasName<TableInfo>(table_name));
    if(iterfind != tables.end())
    {
      TableInfo& info = iter->second.m_info;
      info.m_hidden = iterfind->m_hidden;
      info.m_title = iterfind->m_title;
      info.m_default = iterfind->m_default;

      something_changed = true;
    }
  }

  if(something_changed)
    set_modified();

}

Document_Glom::type_mapLayoutGroupSequence Document_Glom::get_relationship_data_layout_groups_plus_new_fields(const Glib::ustring& layout_name, const Relationship& relationship) const
{
  //TODO: Use an actual relationship_name instead of concatenating:
  return get_data_layout_groups_plus_new_fields(layout_name + "_related_" + relationship.get_name(), relationship.get_from_table(), relationship.get_to_table());
}

void Document_Glom::fill_layout_field_details(const Glib::ustring& parent_table_name, LayoutGroup& layout_group) const
{
  //Get the full field information for the LayoutItem_Fields in this group:

  for(LayoutGroup::type_map_items::iterator iter = layout_group.m_map_items.begin(); iter != layout_group.m_map_items.end(); ++iter)
  {
    LayoutItem* layout_item = iter->second;

    LayoutItem_Field* layout_field = dynamic_cast<LayoutItem_Field*>(layout_item);
    if(layout_field)
    {
      if(layout_field->get_has_relationship_name()) //If it is a related field, instead of a field in parent_table_name
        get_field(layout_field->m_relationship.get_to_table(), layout_field->get_name(), layout_field->m_field);
      else  
        get_field(parent_table_name, layout_field->get_name(), layout_field->m_field);
    }
    else
    {
      LayoutGroup* layout_group_child = dynamic_cast<LayoutGroup*>(layout_item);
      if(layout_group_child)
        fill_layout_field_details(parent_table_name, *layout_group_child); //recurse
    }
  }
}

void Document_Glom::fill_layout_field_details(const Glib::ustring& parent_table_name, type_mapLayoutGroupSequence& sequence) const
{
  for(type_mapLayoutGroupSequence::iterator iterGroups = sequence.begin(); iterGroups != sequence.end(); ++iterGroups)
  {
    fill_layout_field_details(parent_table_name, iterGroups->second);
  }
}

Document_Glom::type_mapLayoutGroupSequence Document_Glom::get_data_layout_groups_plus_new_fields(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name, const Glib::ustring& table_name) const
{
  Glib::ustring child_table_name = table_name;
  if(child_table_name.empty())
    child_table_name = parent_table_name;

  type_mapLayoutGroupSequence result = get_data_layout_groups(layout_name, parent_table_name, child_table_name);


  //If there are no fields in the layout, then add a default:
  bool create_default = false;
  if(result.empty())
    create_default = true;
  //TODO: Also set create_default is all groups have no fields.

  if(create_default)
  {
    //g_warning("Document_Glom::get_data_layout_groups_plus_new_fields(): Creating default layout for table %s (child_table=%s), for layout %s", parent_table_name.c_str(), child_table_name.c_str(), layout_name.c_str());

    //Get the last top-level group. We will add new fields into this one:
    //TODO_Performance: There must be a better way to do this:
    LayoutGroup* pTopLevel = 0;
    LayoutGroup* pOverview = 0; //The default layout has a main group with an overview and details group inside.
    LayoutGroup* pDetails = 0;
    for(type_mapLayoutGroupSequence::iterator iterGroups = result.begin(); iterGroups != result.end(); ++iterGroups)
    {
      pTopLevel = &(iterGroups->second);
    }

    //Add one if necessary:
    if(!pTopLevel)
    {
      LayoutGroup group;
      group.set_name("main");
      group.m_sequence = 1;
      group.m_columns_count = 1;
      result[1] = group;
      pTopLevel = &(result[1]);
      
      if(layout_name == "details") //The Details default layut is a bit more complicated.
      {
        LayoutGroup overview;
        overview.set_name("overview");
        overview.m_title = "Overview"; //Don't translate this, but TODO: add standard translations.
        overview.m_columns_count = 2;
        pOverview = dynamic_cast<LayoutGroup*>(pTopLevel->add_item(overview));
        
        LayoutGroup details;
        details.set_name("details");
        details.m_title = "Details"; //Don't translate this, but TODO: add standard translations.
        details.m_columns_count = 2;
        pDetails = dynamic_cast<LayoutGroup*>(pTopLevel->add_item(details));
      }
    }

    //If, for some reason, we didn't create the-subgroups, add everything to the top level group:
    if(!pOverview)
      pOverview = pTopLevel;
      
    if(!pDetails)
      pDetails = pTopLevel;
      
    
    //Discover new fields, and add them:
    type_vecFields all_fields = get_table_fields(child_table_name);
    for(type_vecFields::const_iterator iter = all_fields.begin(); iter != all_fields.end(); ++iter)
    {
      const Glib::ustring field_name = iter->get_name();
      if(!field_name.empty())
      {
        //See whether it's already in the result:
        //TODO_Performance: There is a lot of iterating and comparison here:
        bool found = false; //TODO: This is horrible.
        for(type_mapLayoutGroupSequence::const_iterator iterFind = result.begin(); iterFind != result.end(); ++iterFind)
        {
          if(iterFind->second.has_field(field_name))
          {
            found = true;
            break;
          }
        }

        if(!found)
        {
          LayoutItem_Field layout_item;
          layout_item.m_field = *iter;
          //layout_item.set_table_name(child_table_name); //TODO: Allow viewing of fields through relationships.
          //layout_item.m_sequence = sequence;  add_item() will fill this.

          if(layout_item.m_field.get_primary_key())
            pOverview->add_item(layout_item);
          else
            pDetails->add_item(layout_item);
        }
      }
    }
  }

  return result;  
}
  
Document_Glom::type_mapLayoutGroupSequence Document_Glom::get_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name, const Glib::ustring& table_name) const
{
  Glib::ustring child_table = table_name;
  if(child_table.empty())
    child_table = parent_table_name;

  type_tables::const_iterator iterFind = m_tables.find(parent_table_name);
  if(iterFind != m_tables.end())
  {
    const DocumentTableInfo& info = iterFind->second;

    //Look for the layout with this name:
    DocumentTableInfo::type_layouts::const_iterator iter = std::find_if(info.m_layouts.begin(), info.m_layouts.end(), predicate_Layout<LayoutInfo>(child_table, layout_name));
    if(iter != info.m_layouts.end())
    {
      return iter->m_layout_groups; //found
    }
  }

  return type_mapLayoutGroupSequence(); //not found
}

void Document_Glom::set_relationship_data_layout_groups(const Glib::ustring& layout_name, const Relationship& relationship, const type_mapLayoutGroupSequence& groups)
{
  //TODO: Use an actual relationship_name instead of concatenating:
  set_data_layout_groups(layout_name + "_related_" + relationship.get_name(), relationship.get_from_table(), groups, relationship.get_to_table());
}

void Document_Glom::set_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name, const type_mapLayoutGroupSequence& groups, const Glib::ustring& table_name)
{
  Glib::ustring child_table_name = table_name;
  if(child_table_name.empty())
    child_table_name = parent_table_name;

  //g_warning("Document_Glom::set_data_layout_groups(): ADDING layout for table %s (child_table=%s), for layout %s", parent_table_name.c_str(), child_table_name.c_str(), layout_name.c_str());


  if(!parent_table_name.empty())
  {
    DocumentTableInfo& info = get_table_info_with_add(parent_table_name);

    LayoutInfo layout_info;
    layout_info.m_parent_table = child_table_name;
    layout_info.m_layout_name = layout_name;
    layout_info.m_layout_groups = groups;

    DocumentTableInfo::type_layouts::iterator iter = std::find_if(info.m_layouts.begin(), info.m_layouts.end(), predicate_Layout<LayoutInfo>(child_table_name, layout_name));
    if(iter == info.m_layouts.end())
      info.m_layouts.push_back(layout_info);
    else
      *iter = layout_info;

    set_modified();
  }
}

Document_Glom::DocumentTableInfo& Document_Glom::get_table_info_with_add(const Glib::ustring& table_name)
{
  type_tables::iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    return iterFind->second;
  }
  else
  {
    m_tables[table_name] = DocumentTableInfo();
    m_tables[table_name].m_info.m_name = table_name;
    return get_table_info_with_add(table_name);
  }
}

Glib::ustring Document_Glom::get_table_title(const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
    return iterFind->second.m_info.m_title;
  else
    return Glib::ustring();
}

void Document_Glom::set_table_title(const Glib::ustring& table_name, const Glib::ustring& value)
{
  if(!table_name.empty())
  {
    DocumentTableInfo& info = get_table_info_with_add(table_name);
    if(info.m_info.m_title != value)
    {
      info.m_info.m_title = value;
      set_modified();
    }
  }
}

bool Document_Glom::get_table_is_known(const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  return (iterFind != m_tables.end());
}

AppState::userlevels Document_Glom::get_userlevel() const
{
  userLevelReason reason;
  return get_userlevel(reason);
}

AppState::userlevels Document_Glom::get_userlevel(userLevelReason& reason) const
{
  //Initialize output parameter:
  reason = USER_LEVEL_REASON_UNKNOWN;

  if(get_read_only())
  {
    reason = USER_LEVEL_REASON_FILE_READ_ONLY;
    return AppState::USERLEVEL_OPERATOR; //A read-only document can not be changed, so there's no point in being in developer mode. This is one way to control the user level on purpose.
  }
  else if(m_file_uri.empty()) //If it has never been saved then this is a new default document, so the user created it, so the user can be a developer.
  {
    return AppState::USERLEVEL_DEVELOPER;
  }
  else
  {
    return m_app_state.get_userlevel();
  }
}

Document_Glom::type_signal_userlevel_changed Document_Glom::signal_userlevel_changed()
{
  return m_signal_userlevel_changed;
}

void Document_Glom::on_app_state_userlevel_changed(AppState::userlevels userlevel)
{
  m_signal_userlevel_changed.emit(userlevel);
}

bool Document_Glom::set_userlevel(AppState::userlevels userlevel)
{
  //Prevent incorrect user level:
  if((userlevel == AppState::USERLEVEL_DEVELOPER) && get_read_only())
  {
    m_app_state.set_userlevel(AppState::USERLEVEL_OPERATOR);
    return false;
  }
  else
  { 
    m_app_state.set_userlevel(userlevel);
    return true;
  }
}

void Document_Glom::emit_userlevel_changed()
{
  m_signal_userlevel_changed.emit(m_app_state.get_userlevel());
}

Glib::ustring Document_Glom::get_default_table() const
{
  for(type_tables::const_iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
  {
    if(iter->second.m_info.m_default)
      return iter->second.m_info.m_name;
  }

  //If there is only one table then pretend that is the default:
  if(m_tables.size() == 1)
  {
    type_tables::const_iterator iter = m_tables.begin();
    return iter->second.m_info.m_name;
  }

  return Glib::ustring();
}

Glib::ustring Document_Glom::get_first_table() const
{
  if(m_tables.empty())
    return Glib::ustring();
    
  type_tables::const_iterator iter = m_tables.begin();
  return iter->second.m_info.m_name;
}

void Document_Glom::set_modified(bool value)
{
  if(value && m_block_modified_set)
    return;

  if(value != get_modified()) //Prevent endless loops
  {
    Bakery::Document_XML::set_modified(value);

    if(value)
    {
      //Save changes automatically
      //(when in developer mode - no changes should even be possible when not in developer mode)
      if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
      {
        //This rebuilds the whole XML DOM and saves the whole document,
        //so we need to be careful not to call set_modified() too often.

         bool test = save_before();
         if(test)
         {
           test = write_to_disk();
           if(test)
           {
             set_modified(false);
           }
         }
      }
    }
  }
}

void Document_Glom::load_after_layout_item_field_formatting(const xmlpp::Element* element, FieldFormatting& format, const Field& field)
{
  //Numeric formatting:
  format.m_numeric_format.m_use_thousands_separator = get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_THOUSANDS_SEPARATOR);
  format.m_numeric_format.m_decimal_places_restricted = get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES_RESTRICTED);
  format.m_numeric_format.m_decimal_places = get_node_attribute_value_as_decimal(element, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES);
  format.m_numeric_format.m_currency_symbol = get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CURRENCY_SYMBOL);

  //Text formatting:
  format.set_text_format_multiline( get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE) );

  //Choices:
  format.set_choices_restricted( get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RESTRICTED) );
  format.set_has_custom_choices( get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM) );

  if(format.get_has_custom_choices())
  {
    const xmlpp::Element* nodeChoiceList = get_node_child_named(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM_LIST);
    if(nodeChoiceList)
    {
      FieldFormatting::type_list_values list_values;

      xmlpp::Node::NodeList listNodesCustomChoices = nodeChoiceList->get_children(GLOM_NODE_FORMAT_CUSTOM_CHOICE);
      for(xmlpp::Node::NodeList::iterator iter = listNodesCustomChoices.begin(); iter != listNodesCustomChoices.end(); ++iter)
      {
        const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(*iter);
        if(element)
        {
          Gnome::Gda::Value value = get_node_attribute_value_as_value(element, GLOM_ATTRIBUTE_VALUE, field.get_glom_type());
          list_values.push_back(value);
        }
      }

      format.set_choices_custom(list_values);
    }
  }

  format.set_has_related_choices( get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED) );

  format.set_choices(get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_RELATIONSHIP),
    get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_FIELD),
    get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SECOND) );
  //Full details are updated in filled-in ().

}

void Document_Glom::load_after_layout_item_field(const xmlpp::Element* element, LayoutItem_Field& item)
{
  item.set_name( get_node_attribute_value(element, GLOM_ATTRIBUTE_NAME) );

  item.m_relationship.set_name( get_node_attribute_value(element, GLOM_ATTRIBUTE_RELATIONSHIP_NAME) ); //Full details are updated in filled-in ().

  item.set_editable( get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_EDITABLE) );

  const xmlpp::Element* elementFormatting = get_node_child_named(element, GLOM_NODE_FORMAT);
  if(elementFormatting)
    load_after_layout_item_field_formatting(elementFormatting, item.m_formatting, item.m_field);

  item.set_formatting_use_default( get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_DATA_LAYOUT_ITEM_FIELD_USE_DEFAULT_FORMATTING) );
}

void Document_Glom::load_after_layout_group(const xmlpp::Element* node, const Glib::ustring table_name, LayoutGroup& group)
{
  if(!node)
  {
    //g_warning("Document_Glom::load_after_layout_group(): node is NULL");
    return;
  }

  //Get the group details:
  group.set_name( get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME) );
  group.m_title = get_node_attribute_value(node, GLOM_ATTRIBUTE_TITLE);
  group.m_columns_count = get_node_attribute_value_as_decimal(node, GLOM_ATTRIBUTE_COLUMNS_COUNT);

  group.m_sequence = get_node_attribute_value_as_decimal(node, GLOM_ATTRIBUTE_SEQUENCE);

  //Get the child items:
  xmlpp::Node::NodeList listNodes = node->get_children();
  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
  {
    const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(*iter);
    if(element)
    {
      const guint sequence = get_node_attribute_value_as_decimal(element, GLOM_ATTRIBUTE_SEQUENCE);
      if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM)
      {
        LayoutItem_Field item;
        load_after_layout_item_field(element, item);

        item.m_sequence = sequence;
        group.add_item(item, sequence);
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_FIELDSUMMARY)
      {
        LayoutItem_FieldSummary item;
        load_after_layout_item_field(element, item);
        item.set_summary_type_from_sql( get_node_attribute_value(element, GLOM_ATTRIBUTE_LAYOUT_ITEM_FIELDSUMMARY_SUMMARYTYPE) );

        item.m_sequence = sequence;
        group.add_item(item, sequence);
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_GROUP)
      {
        LayoutGroup child_group;
        //Recurse:
        load_after_layout_group(element, table_name, child_group);

        group.add_item(child_group);
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_PORTAL)
      {
        LayoutItem_Portal item;
        item.set_relationship( get_node_attribute_value(element, GLOM_ATTRIBUTE_RELATIONSHIP_NAME) );

        item.m_sequence = sequence;
        group.add_item(item, sequence);
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_GROUPBY)
      {
        LayoutItem_GroupBy child_group;
        //Recurse:
        load_after_layout_group(element, table_name, child_group);

        LayoutItem_Field field_groupby; //TODO: Handle related fields too.
        field_groupby.set_name( get_node_attribute_value(element, GLOM_ATTRIBUTE_REPORT_ITEM_GROUPBY_GROUPBY) );
        child_group.set_field_group_by(field_groupby);

        LayoutItem_Field field_sortby;
        field_sortby.set_name( get_node_attribute_value(element, GLOM_ATTRIBUTE_REPORT_ITEM_GROUPBY_SORTBY) );
        child_group.set_field_sort_by(field_sortby);

        //Secondary fields:
        xmlpp::Element* elementSecondary = get_node_child_named(element, GLOM_NODE_DATA_LAYOUT_GROUP_SECONDARYFIELDS);
        if(elementSecondary)
        {
          xmlpp::Element* elementGroup = get_node_child_named(elementSecondary, GLOM_NODE_DATA_LAYOUT_GROUP);
          if(elementGroup)
          {
            load_after_layout_group(elementGroup, table_name, child_group.m_group_secondary_fields);
          }
        }

        group.add_item(child_group);
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_SUMMARY)
      {
        LayoutItem_Summary child_group;
        //Recurse:
        load_after_layout_group(element, table_name, child_group);

        group.add_item(child_group);
      }
    }
  }
}

bool Document_Glom::load_after()
{
  bool result = Bakery::Document_XML::load_after();  

  m_block_cache_update = true; //Don't waste time repeatedly updating this until we have finished.

  if(result)
  {
    const xmlpp::Element* nodeRoot = get_node_document();
    if(nodeRoot)
    {
      m_database_title = get_node_attribute_value(nodeRoot, GLOM_ATTRIBUTE_DATABASE_TITLE);

      const xmlpp::Element* nodeConnection = get_node_child_named(nodeRoot, GLOM_NODE_CONNECTION);
      if(nodeConnection)
      {
        //Connection information:
        m_connection_server = get_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_SERVER);
        m_connection_user = get_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_USER);
        m_connection_database = get_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_DATABASE);
      }

      //Tables:
      m_tables.clear();

      //Look at each "table" node.
      xmlpp::Node::NodeList listNodes = nodeRoot->get_children(GLOM_NODE_TABLE);
      for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
      {
        xmlpp::Element* nodeTable = dynamic_cast<xmlpp::Element*>(*iter);
        if(nodeTable)
        {

          const Glib::ustring table_name = get_node_attribute_value(nodeTable, GLOM_ATTRIBUTE_NAME);

          m_tables[table_name] = DocumentTableInfo();
          DocumentTableInfo& doctableinfo = m_tables[table_name]; //Setting stuff directly in the reference is more efficient than copying it later:

          TableInfo table_info;
          table_info.m_name = table_name;
          table_info.m_hidden = get_node_attribute_value_as_bool(nodeTable, GLOM_ATTRIBUTE_HIDDEN);
          table_info.m_title = get_node_attribute_value(nodeTable, GLOM_ATTRIBUTE_TITLE);
          table_info.m_default = get_node_attribute_value_as_bool(nodeTable, GLOM_ATTRIBUTE_DEFAULT);

          doctableinfo.m_info = table_info;

          //Fields:
          const xmlpp::Element* nodeFields = get_node_child_named(nodeTable, GLOM_NODE_FIELDS);
          if(nodeFields)
          {
            const Field::type_map_type_names type_names = Field::get_type_names();

            //Loop through Field child nodes:
            xmlpp::Node::NodeList listNodes = nodeFields->get_children(GLOM_NODE_FIELD);
            for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
            {
              const xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
              if(nodeChild)
              {
                Field field;

                const Glib::ustring strName = get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_NAME);
                field.set_name( strName );

                const Glib::ustring strTitle = get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_TITLE);
                field.set_title(strTitle);
 
                field.set_primary_key( get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_PRIMARY_KEY) );
                field.set_unique_key( get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_UNIQUE) );
                field.set_auto_increment( get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_AUTOINCREMENT) );

                //Get lookup information, if present.
                xmlpp::Element* nodeLookup = get_node_child_named(nodeChild, GLOM_NODE_FIELD_LOOKUP);
                if(nodeLookup)
                { 
                  field.set_lookup_relationship( get_node_attribute_value(nodeLookup, GLOM_ATTRIBUTE_RELATIONSHIP_NAME) );
                  field.set_lookup_field( get_node_attribute_value(nodeLookup, GLOM_ATTRIBUTE_FIELD) );
                }

                field.set_calculation( get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_CALCULATION) );

                //Field Type:
                const Glib::ustring field_type = get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_TYPE);

                //Get the type enum for this string representation of the type:
                Field::glom_field_type field_type_enum = Field::TYPE_INVALID;
                for(Field::type_map_type_names::const_iterator iter = type_names.begin(); iter !=type_names.end(); ++iter)
                {
                  if(iter->second == field_type)
                  {
                    field_type_enum = iter->first;
                    break;
                  }
                }

                field.set_default_value( get_node_attribute_value_as_value(nodeChild, GLOM_ATTRIBUTE_DEFAULT_VALUE, field_type_enum) );

                //We set this after set_field_info(), because that gets a glom type from the (not-specified) gdatype. Yes, that's strange, and should probably be more explicit.
                field.set_glom_type( field_type_enum );

                //Default Formatting:
                const xmlpp::Element* elementFormatting = get_node_child_named(nodeChild, GLOM_NODE_FORMAT);
                if(elementFormatting)
                  load_after_layout_item_field_formatting(elementFormatting, field.m_default_formatting, field);

                doctableinfo.m_fields.push_back(field);
              }
            }
          }

          //Relationships:
          const xmlpp::Element* nodeRelationships = get_node_child_named(nodeTable, GLOM_NODE_RELATIONSHIPS);
          if(nodeRelationships)
          {
            const xmlpp::Node::NodeList listNodes = nodeRelationships->get_children(GLOM_NODE_RELATIONSHIP);
            for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
            {
              const xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
              if(nodeChild)
              {
                Relationship relationship;
                const Glib::ustring relationship_name = get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_NAME);

                relationship.set_from_table( table_name );
                relationship.set_name( relationship_name );;

                const Glib::ustring relationship_title = get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_TITLE);
                relationship.set_title( relationship_title );

                relationship.set_from_field( get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_KEY) );
                relationship.set_to_table( get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_OTHER_TABLE) );
                relationship.set_to_field( get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_OTHER_KEY) );
                relationship.set_auto_create( get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_AUTO_CREATE) );
                 relationship.set_allow_edit( get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_ALLOW_EDIT) );

                doctableinfo.m_relationships.push_back(relationship);
              }
            }
          }

          //Layouts:
          const xmlpp::Element* nodeDataLayouts = get_node_child_named(nodeTable, GLOM_NODE_DATA_LAYOUTS);
          if(nodeDataLayouts)
          {
            xmlpp::Node::NodeList listNodes = nodeDataLayouts->get_children(GLOM_NODE_DATA_LAYOUT);
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                const Glib::ustring layout_name = get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME);
                Glib::ustring parent_table = get_node_attribute_value(node, GLOM_ATTRIBUTE_PARENT_TABLE_NAME);
                if(parent_table.empty())
                  parent_table = table_name; //Deal with the earlier file format that did not include this.

                type_mapLayoutGroupSequence layout_groups;

                const xmlpp::Element* nodeGroups = get_node_child_named(node, GLOM_NODE_DATA_LAYOUT_GROUPS);
                if(nodeGroups)
                {
                  //Look at all its children:
                  xmlpp::Node::NodeList listNodes = nodeGroups->get_children(GLOM_NODE_DATA_LAYOUT_GROUP);
                  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
                  {
                    const xmlpp::Element* node = dynamic_cast<const xmlpp::Element*>(*iter);
                    if(node)
                    {
                      const Glib::ustring group_name = get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME);
                      if(!group_name.empty())
                      {
                        LayoutGroup group;
                        load_after_layout_group(node, table_name, group);

                        layout_groups[group.m_sequence] = group;
                      }
                    }
                  }
                }

                LayoutInfo layout_info;
                layout_info.m_parent_table = parent_table;
                layout_info.m_layout_name = layout_name;
                layout_info.m_layout_groups = layout_groups;
                doctableinfo.m_layouts.push_back(layout_info);
              }
            }
          } //if(nodeDataLayouts)


          //Reports:
          const xmlpp::Element* nodeReports = get_node_child_named(nodeTable, GLOM_NODE_REPORTS);
          if(nodeReports)
          {
            xmlpp::Node::NodeList listNodes = nodeReports->get_children(GLOM_NODE_REPORT);
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                const Glib::ustring report_name = get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME);
                const Glib::ustring report_title = get_node_attribute_value(node, GLOM_ATTRIBUTE_TITLE);

                //type_mapLayoutGroupSequence layout_groups;

                Report report;
                report.m_name = report_name;
                report.m_title = report_title;

                const xmlpp::Element* nodeGroups = get_node_child_named(node, GLOM_NODE_DATA_LAYOUT_GROUPS);
                if(nodeGroups)
                {
                  //Look at all its children:
                  xmlpp::Node::NodeList listNodes = nodeGroups->get_children(GLOM_NODE_DATA_LAYOUT_GROUP);
                  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
                  {
                    const xmlpp::Element* node = dynamic_cast<const xmlpp::Element*>(*iter);
                    if(node)
                    {
                      LayoutGroup group;
                      load_after_layout_group(node, table_name, group);

                      //layout_groups[group.m_sequence] = group;
                      report.m_layout_group = group; //TODO: Get rid of the for loop here.
                    }
                  }
                }

                doctableinfo.m_reports[report.m_name] = report;
              }
            }
          } //if(nodeReports)


          //Groups:
          m_groups.clear();

          const xmlpp::Element* nodeGroups = get_node_child_named(nodeRoot, GLOM_NODE_GROUPS);
          if(nodeGroups)
          {
            xmlpp::Node::NodeList listNodes = nodeGroups->get_children(GLOM_NODE_GROUP);
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                GroupInfo group_info;

                group_info.m_name = get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME);
                group_info.m_developer = get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_DEVELOPER);

                xmlpp::Node::NodeList listTablePrivs = nodeGroups->get_children(GLOM_NODE_TABLE_PRIVS);
                for(xmlpp::Node::NodeList::iterator iter = listTablePrivs.begin(); iter != listTablePrivs.end(); ++iter)
                {
                  xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
                  if(node)
                  {
                    const Glib::ustring table_name = get_node_attribute_value(node, GLOM_ATTRIBUTE_TABLE_NAME);

                    Privileges privs;
                    privs.m_view = get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRIV_VIEW);
                    privs.m_edit = get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRIV_EDIT);
                    privs.m_create = get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRIV_CREATE);
                    privs.m_delete = get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRIV_DELETE);

                    group_info.m_map_privileges[table_name] = privs;
                  }
                }

                m_groups[group_info.m_name] = group_info;
              }
            }
          }

        } //root
      }
    }
  }

  m_block_cache_update = false;
  update_cached_relationships();

  return result;
}

void Document_Glom::save_before_layout_item_field_formatting(xmlpp::Element* nodeItem, const FieldFormatting& format, const Field& field)
{
  //Numeric format:
  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_THOUSANDS_SEPARATOR,  format.m_numeric_format.m_use_thousands_separator);
  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES_RESTRICTED, format.m_numeric_format.m_decimal_places_restricted);
  set_node_attribute_value_as_decimal(nodeItem, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES, format.m_numeric_format.m_decimal_places);
  set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_CURRENCY_SYMBOL, format.m_numeric_format.m_currency_symbol);

  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RESTRICTED, format.get_choices_restricted());
  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM, format.get_has_custom_choices());

  //Text formatting:
  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE, format.get_text_format_multiline());

  //Choices:
  if(format.get_has_custom_choices())
  {
    xmlpp::Element* child = nodeItem->add_child(GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM_LIST);

    const FieldFormatting::type_list_values list_values = format.get_choices_custom();
    for(FieldFormatting::type_list_values::const_iterator iter = list_values.begin(); iter != list_values.end(); ++iter)
    {
      xmlpp::Element* childChoice = child->add_child(GLOM_NODE_FORMAT_CUSTOM_CHOICE);
      set_node_attribute_value_as_value(childChoice, GLOM_ATTRIBUTE_VALUE, *iter, field.get_glom_type());
    }
  }

  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED, format.get_has_related_choices() );

  Glib::ustring choice_relationship, choice_field, choice_second;
  format.get_choices(choice_relationship, choice_field, choice_second);
  set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_RELATIONSHIP, choice_relationship);
  set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_FIELD, choice_field);
  set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SECOND, choice_second);
}

void Document_Glom::save_before_layout_item_field(xmlpp::Element* nodeItem, const LayoutItem_Field& field)
{
  nodeItem->set_attribute(GLOM_ATTRIBUTE_NAME, field.get_name());
  nodeItem->set_attribute(GLOM_ATTRIBUTE_RELATIONSHIP_NAME, field.get_relationship_name());
  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_EDITABLE, field.get_editable());

  xmlpp::Element* elementFormat = nodeItem->add_child(GLOM_NODE_FORMAT);
  save_before_layout_item_field_formatting(elementFormat, field.m_formatting, field.m_field);

  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_DATA_LAYOUT_ITEM_FIELD_USE_DEFAULT_FORMATTING, field.get_formatting_use_default());

  set_node_attribute_value_as_decimal(nodeItem, GLOM_ATTRIBUTE_SEQUENCE, field.m_sequence);
}

void Document_Glom::save_before_layout_group(xmlpp::Element* node, const LayoutGroup& group)
{
  //g_warning("save_before_layout_group");

  xmlpp::Element* child = 0;

  const LayoutItem_GroupBy* group_by = dynamic_cast<const LayoutItem_GroupBy*>(&group);
  if(group_by) //If it is a GroupBy report part.
  {
    child = node->add_child(GLOM_NODE_DATA_LAYOUT_ITEM_GROUPBY);

    set_node_attribute_value(child, GLOM_ATTRIBUTE_REPORT_ITEM_GROUPBY_GROUPBY, group_by->get_field_group_by()->get_name());
    set_node_attribute_value(child, GLOM_ATTRIBUTE_REPORT_ITEM_GROUPBY_SORTBY, group_by->get_field_sort_by()->get_name());

    if(!group_by->m_group_secondary_fields.m_map_items.empty())
    {
      xmlpp::Element* secondary_fields = child->add_child(GLOM_NODE_DATA_LAYOUT_GROUP_SECONDARYFIELDS);
      save_before_layout_group(secondary_fields, group_by->m_group_secondary_fields);
    }
  }
  else
  {
    const LayoutItem_Summary* summary = dynamic_cast<const LayoutItem_Summary*>(&group);
    if(summary) //If it is a GroupBy report part.
    {
      child = node->add_child(GLOM_NODE_DATA_LAYOUT_ITEM_SUMMARY);
      //TODO: summary_type.
    }
    else
    {
      child = node->add_child(GLOM_NODE_DATA_LAYOUT_GROUP);
    }
  }

  child->set_attribute(GLOM_ATTRIBUTE_NAME, group.get_name());
  child->set_attribute(GLOM_ATTRIBUTE_TITLE, group.m_title);
  set_node_attribute_value_as_decimal(child, GLOM_ATTRIBUTE_COLUMNS_COUNT, group.m_columns_count);

  set_node_attribute_value_as_decimal(child, GLOM_ATTRIBUTE_SEQUENCE, group.m_sequence);

  //Add the child items:
  LayoutGroup::type_map_const_items items = group.get_items();
  for(LayoutGroup::type_map_const_items::const_iterator iterItems = items.begin(); iterItems != items.end(); ++iterItems)
  {
    const LayoutItem* item = iterItems->second;
    //g_warning("save_before_layout_group: child part type=%s", item->get_part_type_name().c_str());

    const LayoutGroup* child_group = dynamic_cast<const LayoutGroup*>(item);
    if(child_group) //If it is a group
    {
      //recurse:
      save_before_layout_group(child, *child_group);
    }
    else
    {
      const LayoutItem_FieldSummary* fieldsummary = dynamic_cast<const LayoutItem_FieldSummary*>(item);
      if(fieldsummary) //If it is a summaryfield
      {
        xmlpp::Element* nodeItem = child->add_child(GLOM_NODE_DATA_LAYOUT_ITEM_FIELDSUMMARY);
        save_before_layout_item_field(nodeItem, *fieldsummary);
        set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_LAYOUT_ITEM_FIELDSUMMARY_SUMMARYTYPE, fieldsummary->get_summary_type_sql()); //The SQL name is as good as anything as an identifier for the summary function.
      }
      else
      {
        const LayoutItem_Field* field = dynamic_cast<const LayoutItem_Field*>(item);
        if(field) //If it is a field
        {
          xmlpp::Element* nodeItem = child->add_child(GLOM_NODE_DATA_LAYOUT_ITEM);
          save_before_layout_item_field(nodeItem, *field);
        }
        else
        {
          const LayoutItem_Portal* portal = dynamic_cast<const LayoutItem_Portal*>(item);
          if(portal) //If it is a portal
          {
            xmlpp::Element* nodeItem = child->add_child(GLOM_NODE_DATA_LAYOUT_PORTAL);
            nodeItem->set_attribute(GLOM_ATTRIBUTE_RELATIONSHIP_NAME, portal->get_relationship());

            set_node_attribute_value_as_decimal(nodeItem, GLOM_ATTRIBUTE_SEQUENCE, item->m_sequence);
          }
        }
      }

    }

    //g_warning("save_before_layout_group: after child part type=%s", item->get_part_type_name().c_str());
  } 
}

bool Document_Glom::save_before()
{
  update_cached_relationships(); //It's helpful to update this whenever something has changed.

  xmlpp::Element* nodeRoot = get_node_document();
  if(nodeRoot)
  {
    set_node_attribute_value(nodeRoot, GLOM_ATTRIBUTE_DATABASE_TITLE, m_database_title);

    xmlpp::Element* nodeConnection = get_node_child_named_with_add(nodeRoot, GLOM_NODE_CONNECTION);
    set_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_SERVER, m_connection_server);
    set_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_USER, m_connection_user);
    set_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_DATABASE, m_connection_database);

    //Remove existing tables:
    xmlpp::Node::NodeList listNodes = nodeRoot->get_children(GLOM_NODE_TABLE);
    for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
      nodeRoot->remove_child(*iter);

    //Add tables:
    for(type_tables::const_iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
    {
      const DocumentTableInfo& doctableinfo = iter->second;

      if(doctableinfo.m_info.m_name.empty())
        g_warning("Document_Glom::save_before(): table name is empty.");

      if(!doctableinfo.m_info.m_name.empty())
      {
        xmlpp::Element* nodeTable = nodeRoot->add_child(GLOM_NODE_TABLE);
        set_node_attribute_value(nodeTable, GLOM_ATTRIBUTE_NAME, doctableinfo.m_info.m_name);
        set_node_attribute_value(nodeTable, GLOM_ATTRIBUTE_TITLE, doctableinfo.m_info.m_title);
        set_node_attribute_value_as_bool(nodeTable, GLOM_ATTRIBUTE_HIDDEN, doctableinfo.m_info.m_hidden);
        set_node_attribute_value_as_bool(nodeTable, GLOM_ATTRIBUTE_DEFAULT, doctableinfo.m_info.m_default);

        //Fields:
        xmlpp::Element* elemFields = nodeTable->add_child(GLOM_NODE_FIELDS);

        const Field::type_map_type_names type_names = Field::get_type_names();

        for(type_vecFields::const_iterator iter = doctableinfo.m_fields.begin(); iter != doctableinfo.m_fields.end(); ++iter)
        {
          const Field& field = *iter;

          xmlpp::Element* elemField = elemFields->add_child(GLOM_NODE_FIELD);
          set_node_attribute_value(elemField, GLOM_ATTRIBUTE_NAME, field.get_name());
          set_node_attribute_value(elemField, GLOM_ATTRIBUTE_TITLE, field.get_title());

          set_node_attribute_value_as_bool(elemField, GLOM_ATTRIBUTE_PRIMARY_KEY, field.get_primary_key());
          set_node_attribute_value_as_bool(elemField, GLOM_ATTRIBUTE_UNIQUE, field.get_unique_key());
          set_node_attribute_value_as_bool(elemField, GLOM_ATTRIBUTE_AUTOINCREMENT, field.get_auto_increment());
          set_node_attribute_value_as_value(elemField, GLOM_ATTRIBUTE_DEFAULT_VALUE, field.get_default_value(), field.get_glom_type());

          set_node_attribute_value(elemField, GLOM_ATTRIBUTE_CALCULATION, field.get_calculation());

          Glib::ustring field_type;
          Field::type_map_type_names::const_iterator iterTypes = type_names.find( field.get_glom_type() );
          if(iterTypes != type_names.end())
            field_type = iterTypes->second;

          set_node_attribute_value(elemField, GLOM_ATTRIBUTE_TYPE, field_type);

          //Add Lookup sub-node:
          if(field.get_is_lookup())
          {
            xmlpp::Element* elemFieldLookup = elemField->add_child(GLOM_NODE_FIELD_LOOKUP);
            set_node_attribute_value(elemFieldLookup, GLOM_ATTRIBUTE_RELATIONSHIP_NAME, field.get_lookup_relationship());
            set_node_attribute_value(elemFieldLookup, GLOM_ATTRIBUTE_FIELD, field.get_lookup_field());
          }

          //Default Formatting:
          xmlpp::Element* elementFormat = elemField->add_child(GLOM_NODE_FORMAT);
          save_before_layout_item_field_formatting(elementFormat, field.m_default_formatting, field);
        }

        //Relationships:
        //Add new <relationships> node:
        xmlpp::Element* elemRelationships = nodeTable->add_child(GLOM_NODE_RELATIONSHIPS);

        //Add each <relationship> node:
        for(type_vecRelationships::const_iterator iter = doctableinfo.m_relationships.begin(); iter != doctableinfo.m_relationships.end(); iter++)
        {
          const Relationship& relationship = *iter;

          xmlpp::Element* elemRelationship = elemRelationships->add_child(GLOM_NODE_RELATIONSHIP);
          set_node_attribute_value(elemRelationship, GLOM_ATTRIBUTE_NAME, relationship.get_name());
          set_node_attribute_value(elemRelationship, GLOM_ATTRIBUTE_TITLE, relationship.get_title());
          set_node_attribute_value(elemRelationship, GLOM_ATTRIBUTE_KEY, relationship.get_from_field());
          set_node_attribute_value(elemRelationship, GLOM_ATTRIBUTE_OTHER_TABLE, relationship.get_to_table());
          set_node_attribute_value(elemRelationship, GLOM_ATTRIBUTE_OTHER_KEY, relationship.get_to_field());
          set_node_attribute_value_as_bool(elemRelationship, GLOM_ATTRIBUTE_AUTO_CREATE, relationship.get_auto_create());
          set_node_attribute_value_as_bool(elemRelationship, GLOM_ATTRIBUTE_ALLOW_EDIT, relationship.get_allow_edit());
        }


        //Layouts:
        xmlpp::Element* nodeDataLayouts = nodeTable->add_child(GLOM_NODE_DATA_LAYOUTS);

        //Add the groups:
        //Make sure that we always get these _after_ the relationships.
        for(DocumentTableInfo::type_layouts::const_iterator iter = doctableinfo.m_layouts.begin(); iter != doctableinfo.m_layouts.end(); ++iter)
        {
          xmlpp::Element* nodeLayout = nodeDataLayouts->add_child(GLOM_NODE_DATA_LAYOUT);
          nodeLayout->set_attribute(GLOM_ATTRIBUTE_NAME, iter->m_layout_name);
          nodeLayout->set_attribute(GLOM_ATTRIBUTE_PARENT_TABLE_NAME, iter->m_parent_table);

          xmlpp::Element* nodeGroups = nodeLayout->add_child(GLOM_NODE_DATA_LAYOUT_GROUPS);

          const type_mapLayoutGroupSequence& group_sequence = iter->m_layout_groups;
          for(type_mapLayoutGroupSequence::const_iterator iterGroups = group_sequence.begin(); iterGroups != group_sequence.end(); ++iterGroups)
          {
            save_before_layout_group(nodeGroups, iterGroups->second);
          }
        }

        //Reports:
        xmlpp::Element* nodeReports = nodeTable->add_child(GLOM_NODE_REPORTS);

        //Add the groups:
        for(DocumentTableInfo::type_reports::const_iterator iter = doctableinfo.m_reports.begin(); iter != doctableinfo.m_reports.end(); ++iter)
        {
          xmlpp::Element* nodeReport = nodeReports->add_child(GLOM_NODE_REPORT);
          nodeReport->set_attribute(GLOM_ATTRIBUTE_NAME, iter->second.get_name());
          nodeReport->set_attribute(GLOM_ATTRIBUTE_TITLE, iter->second.m_title);

          xmlpp::Element* nodeGroups = nodeReport->add_child(GLOM_NODE_DATA_LAYOUT_GROUPS);
          save_before_layout_group(nodeGroups, iter->second.m_layout_group);
        }

      }

    } //for m_tables


    //Remove existing groups:
    listNodes = nodeRoot->get_children(GLOM_NODE_GROUPS);
    for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
      nodeRoot->remove_child(*iter);

    //Add groups:
    xmlpp::Element* nodeGroups = nodeRoot->add_child(GLOM_NODE_GROUPS);

    nodeGroups->add_child_comment("These are only used when recreating a database from an example file. The actual access-control is on the server, of course.");

    for(type_map_groups::const_iterator iter = m_groups.begin(); iter != m_groups.end(); ++iter)
    {
      const GroupInfo& group_info = iter->second;

      xmlpp::Element* nodeGroup = nodeGroups->add_child(GLOM_NODE_GROUP);
      nodeGroup->set_attribute(GLOM_ATTRIBUTE_NAME, group_info.m_name);
      set_node_attribute_value_as_bool(nodeGroup, GLOM_ATTRIBUTE_DEVELOPER, group_info.m_developer);

      //The privilieges for each table, for this group:
      for(GroupInfo::type_map_table_privileges::const_iterator iter = group_info.m_map_privileges.begin(); iter != group_info.m_map_privileges.end(); ++iter)
      {
        xmlpp::Element* nodeTablePrivs = nodeGroups->add_child(GLOM_NODE_TABLE_PRIVS);

        set_node_attribute_value(nodeTablePrivs, GLOM_ATTRIBUTE_TABLE_NAME, iter->first);

        const Privileges& privs = iter->second;
        set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_VIEW, privs.m_view);
        set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_EDIT, privs.m_edit);
        set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_CREATE, privs.m_create);
        set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_DELETE, privs.m_delete);
      }
    }

  }

  return Bakery::Document_XML::save_before();  
}

Glib::ustring Document_Glom::get_database_title() const
{
  return m_database_title;
}

void Document_Glom::set_database_title(const Glib::ustring& title)
{
  if(m_database_title != title)
  {
    m_database_title = title;
    set_modified();
  }
}

Glib::ustring Document_Glom::get_name() const
{
  //Show the database title in the window title bar:
  if(m_database_title.empty())
    return Bakery::Document_XML::get_name();
  else
    return m_database_title;
}

Document_Glom::type_list_groups Document_Glom::get_groups() const
{
  type_list_groups result;
  for(type_map_groups::const_iterator iter = m_groups.begin(); iter != m_groups.end(); ++iter)
  {
    result.push_back(iter->second);
  }

  return result;
}

///This adds the group if necessary.
void Document_Glom::set_group(GroupInfo& group)
{
  type_map_groups::iterator iter = m_groups.find(group.m_name);
  if(iter == m_groups.end())
  {
    //Add it if necesary:
    m_groups[group.m_name] = group;
    set_modified();
  }
  else
  {
    const GroupInfo this_group = iter->second;
    if(this_group != group)
    {
      iter->second = group;
      set_modified();
    }
  }
}

void Document_Glom::remove_group(const Glib::ustring& group_name)
{
  type_map_groups::iterator iter = m_groups.find(group_name);
  if(iter != m_groups.end())
  {
    m_groups.erase(iter);
    set_modified();
  }
}

void Document_Glom::update_cached_relationships(FieldFormatting& formatting, const Glib::ustring& table_name)
{
  if(formatting.m_choices_related_relationship.get_name_not_empty())
  {
    get_relationship(table_name, formatting.m_choices_related_relationship.get_name(), formatting.m_choices_related_relationship);
  }
}

void Document_Glom::update_cached_relationships(LayoutGroup& group, const Glib::ustring& table_name)
{
  const bool old_block_modified_set = m_block_modified_set;
  m_block_modified_set = true; //Don't let any of this cause a document save. It's just a cache.

  //Find any LayoutItem_Fields, and fill in their full relationship details:
  for(LayoutGroup::type_map_items::iterator iter = group.m_map_items.begin(); iter != group.m_map_items.end(); ++iter)
  {
    LayoutItem* pItem = iter->second;

    LayoutItem_Field* pField = dynamic_cast<LayoutItem_Field*>(pItem);
    if(pField)
    {
       if(pField->m_relationship.get_name_not_empty())
       {
         get_relationship(table_name, pField->m_relationship.get_name(), pField->m_relationship);
       }

       if(pField->m_formatting.m_choices_related_relationship.get_name_not_empty())
       {
         get_relationship(table_name, pField->m_formatting.m_choices_related_relationship.get_name(), pField->m_formatting.m_choices_related_relationship);
       }
    }
    else
    {
      LayoutItem_Portal* pPortal = dynamic_cast<LayoutItem_Portal*>(pItem);
      if(pPortal)
      {

        get_relationship(table_name, pPortal->get_relationship(), pPortal->m_relationship);

        //Update the Portal's group: TODO: Do this in place, instead of using set/get.
        const Glib::ustring layout_name = "list_related"; //TODO: This is silly.
        type_mapLayoutGroupSequence portalGroups = get_relationship_data_layout_groups_plus_new_fields(layout_name, pPortal->m_relationship);

        for(type_mapLayoutGroupSequence::iterator iterGroup = portalGroups.begin(); iterGroup != portalGroups.end(); ++iterGroup)
        {
          update_cached_relationships(iterGroup->second, pPortal->m_relationship.get_to_table());
        }

        set_relationship_data_layout_groups(layout_name, pPortal->m_relationship, portalGroups);
      }
      else
      {
        LayoutItem_GroupBy* pGroupBy = dynamic_cast<LayoutItem_GroupBy*>(pItem);
        if(pGroupBy)
        {
          update_cached_relationships(*pGroupBy, table_name); //recurse:

          update_cached_relationships(pGroupBy->m_group_secondary_fields, table_name); //recuse.
        }
        else
        {
          LayoutGroup* pGroup = dynamic_cast<LayoutGroup*>(pItem);
          if(pGroup)
            update_cached_relationships(*pGroup, table_name); //recurse:
        }
      }
    }
  }

  m_block_modified_set = old_block_modified_set;
}

void Document_Glom::update_cached_relationships()
{
  if(m_block_cache_update)
    return;


  //Update all the formatting and groups (groups are in layouts, in tables)
  for(type_tables::iterator iterTable = m_tables.begin(); iterTable != m_tables.end(); ++iterTable)
  {
    DocumentTableInfo& tableInfo = iterTable->second;

    //Fields (formatting):
    for(type_vecFields::iterator iterField = tableInfo.m_fields.begin(); iterField != tableInfo.m_fields.end(); ++iterField)
    {
       update_cached_relationships(iterField->m_default_formatting, tableInfo.m_info.get_name());
    }

    //Layouts:
    for(DocumentTableInfo::type_layouts::iterator iterLayout = tableInfo.m_layouts.begin(); iterLayout != tableInfo.m_layouts.end(); ++iterLayout)
    {
      if(tableInfo.m_info.get_name() == iterLayout->m_parent_table) //If it is not a related layout:
      {
        type_mapLayoutGroupSequence& groups = iterLayout->m_layout_groups;
        for(type_mapLayoutGroupSequence::iterator iterGroup = groups.begin(); iterGroup != groups.end(); ++iterGroup)
        {
          update_cached_relationships(iterGroup->second, tableInfo.m_info.get_name());
        }
      }
    }

    //Reports:
    for(DocumentTableInfo::type_reports::iterator iterReport = tableInfo.m_reports.begin(); iterReport != tableInfo.m_reports.end(); ++iterReport)
    {
      update_cached_relationships(iterReport->second.m_layout_group, tableInfo.m_info.get_name());
    }
  }
}

Document_Glom::type_listReports Document_Glom::get_report_names(const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    type_listReports result;
    for(DocumentTableInfo::type_reports::const_iterator iter = iterFind->second.m_reports.begin(); iter != iterFind->second.m_reports.end(); ++iter)
    {
      result.push_back(iter->second.get_name());
    }

    return result;
  }
  else
    return type_listReports(); 
}

void Document_Glom::remove_all_reports(const Glib::ustring& table_name)
{
  type_tables::iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    iterFind->second.m_reports.clear();
    set_modified();
  }
}

void Document_Glom::set_report(const Glib::ustring& table_name, const Report& report)
{
  type_tables::iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    iterFind->second.m_reports[report.m_name] = report;
    set_modified();
  }
}

bool Document_Glom::get_report(const Glib::ustring& table_name, const Glib::ustring& report_name, Report& report) const
{
  type_tables::const_iterator iterFindTable = m_tables.find(table_name);
  if(iterFindTable != m_tables.end())
  {
    DocumentTableInfo::type_reports::const_iterator iterFindReport = iterFindTable->second.m_reports.find(report_name);
    if(iterFindReport != iterFindTable->second.m_reports.end())
    {
      report = iterFindReport->second;
      return true; //success.
    }
  }

  return false;
}

void Document_Glom::remove_report(const Glib::ustring& table_name, const Glib::ustring& report_name)
{
  type_tables::iterator iterFindTable = m_tables.find(table_name);
  if(iterFindTable != m_tables.end())
  {
    DocumentTableInfo::type_reports::iterator iterFindReport = iterFindTable->second.m_reports.find(report_name);
    if(iterFindReport != iterFindTable->second.m_reports.end())
    {
      iterFindTable->second.m_reports.erase(iterFindReport);

      set_modified();
    }
  }
}


