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
//#include "config.h" //To get GLOM_DTD_INSTALL_DIR - dependent on configure prefix.
#include <algorithm> //For std::find_if().
#include <sstream> //For stringstream

Document_Glom::Document_Glom()
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

Glib::ustring Document_Glom::get_connection_user()
{
  return m_connection_user;
}

Glib::ustring Document_Glom::get_connection_server()
{
  return m_connection_server;
}

Glib::ustring Document_Glom::get_connection_database()
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

    bool something_changed = true;
    type_vecRelationships::iterator iterRel = info.m_relationships.begin();
    while(something_changed)
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
        if(iterRel == info.m_relationships.end())
          something_changed = false; //We've looked at them all, without changing things.
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
      g_warning("Document_Glom::get_table_fields: table not found in document: %s", table_name.c_str());
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
      for(type_mapLayoutGroupSequence::iterator iterGroup = iterLayouts->second.begin(); iterGroup != iterLayouts->second.end(); ++iterGroup)
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
  Glib::ustring strValue = (value ? "true" : "false");
  set_node_attribute_value(node, strAttributeName, strValue);
}

void Document_Glom::set_node_attribute_value_as_decimal(xmlpp::Element* node, const Glib::ustring& strAttributeName, int value)
{
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
  return get_data_layout_groups_plus_new_fields(layout_name, relationship.get_from_table() + "_related_" + relationship.get_name()); 
}

Document_Glom::type_mapLayoutGroupSequence Document_Glom::get_data_layout_groups_plus_new_fields(const Glib::ustring& layout_name, const Glib::ustring& table_name) const
{
  type_mapLayoutGroupSequence result = get_data_layout_groups(layout_name, table_name);


  //If there are no fields in the layout, then add a default:
  bool create_default = false;
  if(result.empty())
    create_default = true;
  //TODO: Also set create_default is all groups have no fields.

  if(create_default)
  {
    //Get the last top-level group. We will add new fields into this one:
    //TODO_Performance: There must be a better way to do this:
    LayoutGroup* pTopLevel = 0;
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
    }

    //Discover new fields, and add them:
    type_vecFields all_fields = get_table_fields(table_name);
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
          layout_item.set_name(field_name);
          //layout_item.set_table_name(table_name); //TODO: Allow viewing of fields through relationships.
          //layout_item.m_sequence = sequence;  add_item() will fill this.


          pTopLevel->add_item(layout_item);
        }
      }
    }
  }

  return result;  
}
  
Document_Glom::type_mapLayoutGroupSequence Document_Glom::get_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    const DocumentTableInfo& info = iterFind->second;

    //Look for the layout with this name:
    DocumentTableInfo::type_layouts::const_iterator iter = info.m_layouts.find(layout_name);
    if(iter != info.m_layouts.end())
    {
      return iter->second; //found   
    }
  }

  return type_mapLayoutGroupSequence(); //not found
}

void Document_Glom::set_relationship_data_layout_groups(const Glib::ustring& layout_name, const Relationship& relationship, const type_mapLayoutGroupSequence& groups)
{
  //TODO: Use an actual relationship_name instead of concatenating:
  set_data_layout_groups(layout_name, relationship.get_from_table() + "_related_" + relationship.get_name(), groups);
}

void Document_Glom::set_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& table_name, const type_mapLayoutGroupSequence& groups)
{
  if(!table_name.empty())
  {
    DocumentTableInfo& info = get_table_info_with_add(table_name);
    info.m_layouts[layout_name] = groups;
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

void Document_Glom::set_modified(bool value)
{
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

void Document_Glom::load_after_layout_group(const xmlpp::Element* node, const Glib::ustring table_name, LayoutGroup& group)
{
  if(!node)
    return;

  //Get the group details:
  group.set_name( get_node_attribute_value(node, "name") );
  group.m_title = get_node_attribute_value(node, "title");
  group.m_columns_count = get_node_attribute_value_as_decimal(node, "columns_count");

  group.m_sequence = get_node_attribute_value_as_decimal(node, "sequence");

  //Get the child items:
  xmlpp::Node::NodeList listNodes = node->get_children(); 
  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
  {
    const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(*iter);
    if(element)
    {
      const guint sequence = get_node_attribute_value_as_decimal(element, "sequence");

      if(element->get_name() == "data_layout_item")
      {
        LayoutItem_Field item;

        item.set_name( get_node_attribute_value(element, "name") );
        item.set_relationship_name( get_node_attribute_value(element, "relationship") );
        item.set_editable( get_node_attribute_value_as_bool(element, "editable") );
        //item.set_table_name(table_name);

        item.m_sequence = sequence;
        group.add_item(item, sequence);
      }
      else if(element->get_name() == "data_layout_group")
      {
        LayoutGroup child_group;
        //Recurse:
        load_after_layout_group(element, table_name, child_group);

        group.add_item(child_group);
      }
      else if(element->get_name() == "data_layout_portal")
      {
        LayoutItem_Portal item;
        item.set_relationship( get_node_attribute_value(element, "relationship") );

        item.m_sequence = sequence;
        group.add_item(item, sequence);
      }
    }
  }
}

bool Document_Glom::load_after()
{
  bool result = Bakery::Document_XML::load_after();  

  if(result)
  {
    const xmlpp::Element* nodeRoot = get_node_document();
    if(nodeRoot)
    {
      m_database_title = get_node_attribute_value(nodeRoot, "database_title");

      const xmlpp::Element* nodeConnection = get_node_child_named(nodeRoot, "connection");
      if(nodeConnection)
      {
        //Connection information:
        m_connection_server = get_node_attribute_value(nodeConnection, "server");
        m_connection_user = get_node_attribute_value(nodeConnection, "user");
        m_connection_database = get_node_attribute_value(nodeConnection, "database");
      }

      //Tables:
      m_tables.clear();

      //Look at each "table" node.
      xmlpp::Node::NodeList listNodes = nodeRoot->get_children("table");
      for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
      {
        xmlpp::Element* nodeTable = dynamic_cast<xmlpp::Element*>(*iter);
        if(nodeTable)
        {

          const Glib::ustring table_name = get_node_attribute_value(nodeTable, "name");

          m_tables[table_name] = DocumentTableInfo();
          DocumentTableInfo& doctableinfo = m_tables[table_name]; //Setting stuff directly in the reference is more efficient than copying it later:

          TableInfo table_info;
          table_info.m_name = table_name;
          table_info.m_hidden = get_node_attribute_value_as_bool(nodeTable, "hidden");
          table_info.m_title = get_node_attribute_value(nodeTable, "title");
          table_info.m_default = get_node_attribute_value_as_bool(nodeTable, "default");

          doctableinfo.m_info = table_info;

          //Fields:
          const xmlpp::Element* nodeFields = get_node_child_named(nodeTable, "fields");
          if(nodeFields)
          {
            const Field::type_map_type_names type_names = Field::get_type_names();

            //Loop through Field child nodes:
            xmlpp::Node::NodeList listNodes = nodeFields->get_children("field");
            for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
            {
              const xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
              if(nodeChild)
              {
                Field field;

                Gnome::Gda::FieldAttributes field_info = field.get_field_info();
                const Glib::ustring strName = get_node_attribute_value(nodeChild, "name");
                field_info.set_name( strName );

                const Glib::ustring strTitle = get_node_attribute_value(nodeChild, "title");
                field.set_title(strTitle);
 
                field_info.set_primary_key( get_node_attribute_value_as_bool(nodeChild, "primary_key") );
                field_info.set_unique_key( get_node_attribute_value_as_bool(nodeChild, "unique") );
                field_info.set_auto_increment( get_node_attribute_value_as_bool(nodeChild, "auto_increment") );

                //Get lookup information, if present.
                xmlpp::Element* nodeLookup = get_node_child_named(nodeChild, "field_lookup");
                if(nodeLookup)
                { 
                  field.set_lookup_relationship( get_node_attribute_value(nodeLookup, "relationship") );
                  field.set_lookup_field( get_node_attribute_value(nodeLookup, "field") );
                }

                field.set_calculation( get_node_attribute_value(nodeChild, "calculation") );

                //Field Type:
                const Glib::ustring field_type = get_node_attribute_value(nodeChild, "type");

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

                //Default value:
                const Glib::ustring default_value_text = get_node_attribute_value(nodeChild, "default_value");
                //Interpret the text as per the field type:
                bool success = false;
                field_info.set_default_value( GlomConversions::parse_value(field_type_enum, default_value_text, success, true /* iso_format */) );
                field.set_field_info(field_info);

                //We set this after set_field_info(), because that gets a glom type from the (not-specified) gdatype. Yes, that's strange, and should probably be more explicit.
                field.set_glom_type( field_type_enum );
 
                doctableinfo.m_fields.push_back(field);
              }
            }
          }

          //Relationships:
          const xmlpp::Element* nodeRelationships = get_node_child_named(nodeTable, "relationships");
          if(nodeRelationships)
          {
            const xmlpp::Node::NodeList listNodes = nodeRelationships->get_children("relationship");
            for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
            {
              const xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
              if(nodeChild)
              {
                Relationship relationship;
                const Glib::ustring relationship_name = get_node_attribute_value(nodeChild, "name");

                relationship.set_from_table( table_name );
                relationship.set_name( relationship_name );;

                const Glib::ustring relationship_title = get_node_attribute_value(nodeChild, "title");
                relationship.set_title( relationship_title );

                relationship.set_from_field( get_node_attribute_value(nodeChild, "key") );
                relationship.set_to_table( get_node_attribute_value(nodeChild, "other_table") );
                relationship.set_to_field( get_node_attribute_value(nodeChild, "other_key") );
                relationship.set_auto_create( get_node_attribute_value_as_bool(nodeChild, "auto_create") );

                doctableinfo.m_relationships.push_back(relationship);
              }
            }
          }

          //Layouts:
          const xmlpp::Element* nodeDataLayouts = get_node_child_named(nodeTable, "data_layouts");
          if(nodeDataLayouts)
          {
            xmlpp::Node::NodeList listNodes = nodeDataLayouts->get_children("data_layout");
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                const Glib::ustring layout_name = get_node_attribute_value(node, "name");

                type_mapLayoutGroupSequence layout_groups;

                const xmlpp::Element* nodeGroups = get_node_child_named(node, "data_layout_groups");
                if(nodeGroups)
                {
                  //Look at all its children:
                  xmlpp::Node::NodeList listNodes = nodeGroups->get_children("data_layout_group");
                  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
                  {
                    const xmlpp::Element* node = dynamic_cast<const xmlpp::Element*>(*iter);
                    if(node)
                    {
                      const Glib::ustring group_name = get_node_attribute_value(node, "name");
                      if(!group_name.empty())
                      {
                        LayoutGroup group;
                        load_after_layout_group(node, table_name, group);

                        layout_groups[group.m_sequence] = group;
                      }
                    }
                  }
                }

                doctableinfo.m_layouts[layout_name] = layout_groups;
              }
            }
          } //if(nodeDataLayouts)


          //Groups:
          m_groups.clear();

          const xmlpp::Element* nodeGroups = get_node_child_named(nodeRoot, "groups");
          if(nodeGroups)
          {
            xmlpp::Node::NodeList listNodes = nodeGroups->get_children("group");
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                GroupInfo group_info;

                group_info.m_name = get_node_attribute_value(node, "name");
                group_info.m_developer = get_node_attribute_value_as_bool(node, "developer");

                xmlpp::Node::NodeList listTablePrivs = nodeGroups->get_children("table_privs");
                for(xmlpp::Node::NodeList::iterator iter = listTablePrivs.begin(); iter != listTablePrivs.end(); ++iter)
                {
                  xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
                  if(node)
                  {
                    const Glib::ustring table_name = get_node_attribute_value(node, "table_name");

                    Privileges privs;
                    privs.m_view = get_node_attribute_value_as_bool(node, "priv_view");
                    privs.m_edit = get_node_attribute_value_as_bool(node, "priv_edit");
                    privs.m_create = get_node_attribute_value_as_bool(node, "priv_create");
                    privs.m_delete = get_node_attribute_value_as_bool(node, "priv_delete");

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
  
  return result;
}

void Document_Glom::save_before_layout_group(xmlpp::Element* node, const LayoutGroup& group)
{
  xmlpp::Element* child = node->add_child("data_layout_group");

  child->set_attribute("name", group.get_name());
  child->set_attribute("title", group.m_title);
  set_node_attribute_value_as_decimal(child, "columns_count", group.m_columns_count);

  set_node_attribute_value_as_decimal(child, "sequence", group.m_sequence);

  //Add the child items:
  LayoutGroup::type_map_const_items items = group.get_items();
  for(LayoutGroup::type_map_const_items::const_iterator iterItems = items.begin(); iterItems != items.end(); ++iterItems)
  {
    const LayoutItem* item = iterItems->second;
    const LayoutGroup* child_group = dynamic_cast<const LayoutGroup*>(item);
    if(child_group) //If it is a group
    {
      //recurse:
      save_before_layout_group(child, *child_group);
    }
    else
    {
      const LayoutItem_Field* field = dynamic_cast<const LayoutItem_Field*>(item);
      if(field) //If it is a field
      {
        xmlpp::Element* nodeItem = child->add_child("data_layout_item");
        nodeItem->set_attribute("name", item->get_name());
        nodeItem->set_attribute("relationship", field->get_relationship_name());
        set_node_attribute_value_as_bool(nodeItem, "editable", item->get_editable());

        set_node_attribute_value_as_decimal(nodeItem, "sequence", item->m_sequence);
      }
      else
      {
        const LayoutItem_Portal* portal = dynamic_cast<const LayoutItem_Portal*>(item);
        if(portal) //If it is a portal
        {
          xmlpp::Element* nodeItem = child->add_child("data_layout_portal");
          nodeItem->set_attribute("relationship", portal->get_relationship());

          set_node_attribute_value_as_decimal(nodeItem, "sequence", item->m_sequence);
        }
      }

    }
  } 
}

bool Document_Glom::save_before()
{
  xmlpp::Element* nodeRoot = get_node_document();
  if(nodeRoot)
  {
    set_node_attribute_value(nodeRoot, "database_title", m_database_title);

    xmlpp::Element* nodeConnection = get_node_child_named_with_add(nodeRoot, "connection");
    set_node_attribute_value(nodeConnection, "server", m_connection_server); 
    set_node_attribute_value(nodeConnection, "user", m_connection_user); 
    set_node_attribute_value(nodeConnection, "database", m_connection_database);

    //Remove existing tables:
    xmlpp::Node::NodeList listNodes = nodeRoot->get_children("table");
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
        xmlpp::Element* nodeTable = nodeRoot->add_child("table");
        set_node_attribute_value(nodeTable, "name", doctableinfo.m_info.m_name);
        set_node_attribute_value(nodeTable, "title", doctableinfo.m_info.m_title);
        set_node_attribute_value_as_bool(nodeTable, "hidden", doctableinfo.m_info.m_hidden);
        set_node_attribute_value_as_bool(nodeTable, "default", doctableinfo.m_info.m_default);

        //Fields:
        xmlpp::Element* elemFields = nodeTable->add_child("fields");

        const Field::type_map_type_names type_names = Field::get_type_names();

        for(type_vecFields::const_iterator iter = doctableinfo.m_fields.begin(); iter != doctableinfo.m_fields.end(); ++iter)
        {
          const Field& field = *iter;

          xmlpp::Element* elemField = elemFields->add_child("field");
          set_node_attribute_value(elemField, "name", field.get_field_info().get_name());
          set_node_attribute_value(elemField, "title", field.get_title());

          const Gnome::Gda::FieldAttributes field_info = field.get_field_info();
          set_node_attribute_value_as_bool(elemField, "primary_key", field_info.get_primary_key());
          set_node_attribute_value_as_bool(elemField, "unique", field_info.get_unique_key());
          set_node_attribute_value_as_bool(elemField, "auto_increment", field_info.get_auto_increment());
          set_node_attribute_value(elemField, "default_value", field_info.get_default_value().to_string());

          set_node_attribute_value(elemField, "calculation", field.get_calculation());

          Glib::ustring field_type;
          Field::type_map_type_names::const_iterator iterTypes = type_names.find( field.get_glom_type() );
          if(iterTypes != type_names.end())
            field_type = iterTypes->second;

          set_node_attribute_value(elemField, "type", field_type);

          //Add Lookup sub-node:
          if(field.get_is_lookup())
          {
            xmlpp::Element* elemFieldLookup = elemField->add_child("field_lookup");
            set_node_attribute_value(elemFieldLookup, "relationship", field.get_lookup_relationship());
            set_node_attribute_value(elemFieldLookup, "field", field.get_lookup_field());
          }
        }

        //Relationships:
        //Add new <relationships> node:
        xmlpp::Element* elemRelationships = nodeTable->add_child("relationships");

        //Add each <relationship> node:
        for(type_vecRelationships::const_iterator iter = doctableinfo.m_relationships.begin(); iter != doctableinfo.m_relationships.end(); iter++)
        {
          const Relationship& relationship = *iter;

          xmlpp::Element* elemRelationship = elemRelationships->add_child("relationship");
          set_node_attribute_value(elemRelationship, "name", relationship.get_name());
          set_node_attribute_value(elemRelationship, "title", relationship.get_title());
          set_node_attribute_value(elemRelationship, "key", relationship.get_from_field());
          set_node_attribute_value(elemRelationship, "other_table", relationship.get_to_table());
          set_node_attribute_value(elemRelationship, "other_key", relationship.get_to_field());
          set_node_attribute_value_as_bool(elemRelationship, "auto_create", relationship.get_auto_create());
        }


        //Layouts:
        xmlpp::Element* nodeDataLayouts = nodeTable->add_child("data_layouts");

        //Add the groups:
        for(DocumentTableInfo::type_layouts::const_iterator iter = doctableinfo.m_layouts.begin(); iter != doctableinfo.m_layouts.end(); ++iter)
        {
          xmlpp::Element* nodeLayout = nodeDataLayouts->add_child("data_layout");
          nodeLayout->set_attribute("name", iter->first);

          xmlpp::Element* nodeGroups = nodeLayout->add_child("data_layout_groups");

          const type_mapLayoutGroupSequence& group_sequence = iter->second;
          for(type_mapLayoutGroupSequence::const_iterator iterGroups = group_sequence.begin(); iterGroups != group_sequence.end(); ++iterGroups)
          {
            save_before_layout_group(nodeGroups, iterGroups->second);
          }
        }
      }

    } //for m_tables


    //Remove existing groups:
    listNodes = nodeRoot->get_children("groups");
    for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
      nodeRoot->remove_child(*iter);

    //Add groups:
    xmlpp::Element* nodeGroups = nodeRoot->add_child("groups");

    nodeGroups->add_child_comment("These are only used when recreating a database from an example file. The actualy access-control is on the server, of course.");

    for(type_map_groups::const_iterator iter = m_groups.begin(); iter != m_groups.end(); ++iter)
    {
      const GroupInfo& group_info = iter->second;

      xmlpp::Element* nodeGroup = nodeGroups->add_child("group");
      nodeGroup->set_attribute("name", group_info.m_name);
      set_node_attribute_value_as_bool(nodeGroup, "developer", group_info.m_developer);

      //The privilieges for each table, for this group:
      for(GroupInfo::type_map_table_privileges::const_iterator iter = group_info.m_map_privileges.begin(); iter != group_info.m_map_privileges.end(); ++iter)
      {
        xmlpp::Element* nodeTablePrivs = nodeGroups->add_child("table_privs");

        set_node_attribute_value(nodeTablePrivs, "table_name", iter->first);

        const Privileges& privs = iter->second;
        set_node_attribute_value_as_bool(nodeTablePrivs, "priv_view", privs.m_view);
        set_node_attribute_value_as_bool(nodeTablePrivs, "priv_edit", privs.m_edit);
        set_node_attribute_value_as_bool(nodeTablePrivs, "priv_create", privs.m_create);
        set_node_attribute_value_as_bool(nodeTablePrivs, "priv_delete", privs.m_delete);
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
