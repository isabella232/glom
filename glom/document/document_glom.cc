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

  get_node_connection();

  //Set default database name:
  //This is also the XML attribute default value,
  //but that isn't available for new documents.
  if(get_connection_server().size() == 0)
    set_connection_server("localhost");

  m_app_state.signal_userlevel_changed().connect( sigc::mem_fun(*this, &Document_Glom::on_app_state_userlevel_changed) );
}

Document_Glom::~Document_Glom()
{
}

Glib::ustring Document_Glom::get_connection_user()
{
  return get_node_attribute_value(get_node_connection(), "user");
}

Glib::ustring Document_Glom::get_connection_server()
{
  return get_node_attribute_value(get_node_connection(), "server");
}

Glib::ustring Document_Glom::get_connection_database()
{
  return get_node_attribute_value(get_node_connection(), "database");
}

void Document_Glom::set_connection_user(const Glib::ustring& strVal)
{
  set_node_attribute_value(get_node_connection(), "user", strVal);
  set_modified();
}

void Document_Glom::set_connection_server(const Glib::ustring& strVal)
{
  set_node_attribute_value(get_node_connection(), "server", strVal);
  set_modified();
}

void Document_Glom::set_connection_database(const Glib::ustring& strVal)
{
  set_node_attribute_value(get_node_connection(), "database", strVal);
  set_modified();
}


Document_Glom::type_vecRelationships Document_Glom::get_relationships(const Glib::ustring& strTableName)
{
  type_vecRelationships vecResult;

  xmlpp::Element* nodeTable = get_node_table(strTableName);
  if(nodeTable)
  {
    xmlpp::Element* nodeRelationships = get_node_child_named_with_add(nodeTable, "relationships");
    if(nodeRelationships)
    {
      xmlpp::Node::NodeList listNodes = nodeRelationships->get_children();
      for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
      {
        const xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
        if(nodeChild)
        {
          Glib::ustring strChildName = nodeChild->get_name();
          if(strChildName == "relationship")
          {
            Relationship relationship;
            relationship.set_from_table( strTableName );
            relationship.set_name( get_node_attribute_value(nodeChild, "name") );
            relationship.set_from_field( get_node_attribute_value(nodeChild, "key") );
            relationship.set_to_table( get_node_attribute_value(nodeChild, "other_table") );
            relationship.set_to_field( get_node_attribute_value(nodeChild, "other_key") );

            vecResult.push_back(relationship);
          }
        }
      }
    }
  }

  return vecResult;
}

void Document_Glom::set_relationships(const Glib::ustring& strTableName, type_vecRelationships vecRelationships)
{
  xmlpp::Element* nodeTable = get_node_table_with_add(strTableName);
  xmlpp::Element* nodeRelationships = get_node_child_named(nodeTable, "relationships");
  if(nodeRelationships)
  {
    //Remove previous node:
    nodeTable->remove_child(nodeRelationships);
  }

  //Add new <relationships> node:
  xmlpp::Element* elemRelationships = nodeTable->add_child("relationships");

  //Add each <relationship> node:
  for(type_vecRelationships::iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); iter++)
  {
    const Relationship& relationship = *iter;
    if(relationship.get_from_table() == strTableName)
    {
      xmlpp::Element* elemRelationship = elemRelationships->add_child("relationship");
      set_node_attribute_value(elemRelationship, "name", relationship.get_name());
      set_node_attribute_value(elemRelationship, "key", relationship.get_from_field());
      set_node_attribute_value(elemRelationship, "other_table", relationship.get_to_table());
      set_node_attribute_value(elemRelationship, "other_key", relationship.get_to_field());
    }
  }

  set_modified();
}

Document_Glom::type_vecFields Document_Glom::get_table_fields(const Glib::ustring& strTableName) const
{
  type_vecFields vecResult;

  FieldType::type_map_type_names type_names = FieldType::get_usable_type_names();

  const xmlpp::Element* nodeTable = get_node_table(strTableName);
  if(nodeTable)
  {
    const xmlpp::Element* nodeFields = get_node_child_named(nodeTable, "fields");
    if(nodeFields)
    {
     //Loop through Field child nodes:
     xmlpp::Node::NodeList listNodes = nodeFields->get_children();
     for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
     {
       const xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
       if(nodeChild)
       {
         //Get field information:
         Glib::ustring strChildName = nodeChild->get_name();
         if(strChildName == "field")
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
           field_info.set_default_value( Gnome::Gda::Value (get_node_attribute_value(nodeChild, "default_value")) );
           
           field.set_field_info(field_info);

           //Get lookup information, if present.
           xmlpp::Element* nodeLookup = get_node_child_named(nodeChild, "field_lookup");
           if(nodeLookup)
           {
             field.set_lookup_relationship( get_node_attribute_value(nodeLookup, "relationship") );
             field.set_lookup_field( get_node_attribute_value(nodeLookup, "field") );
           }

           //Field Type:
           const Glib::ustring field_type = get_node_attribute_value(nodeChild, "type");

           //Get the type enum for this string representation of the type:
           FieldType::enumTypes field_type_enum = FieldType::TYPE_INVALID;
           for(FieldType::type_map_type_names::const_iterator iter = type_names.begin(); iter !=type_names.end(); ++iter)
           {
             if(iter->second == field_type)
             {
               field_type_enum = iter->first;
               break;
             }
           }

           field.set_field_type( FieldType(field_type_enum) );
             

           vecResult.push_back(field);
          }
        }
      }
    }
  }

  return vecResult;
}

void Document_Glom::set_table_fields(const Glib::ustring& strTableName, type_vecFields vecFields)
{
  xmlpp::Element* nodeTable = get_node_table_with_add(strTableName);
  xmlpp::Element* nodeFields = get_node_child_named(nodeTable, "fields");
  if(nodeFields)
  {
    //Remove previous node:
    nodeTable->remove_child(nodeFields);
  }

  //Add new <relationships> node:
  xmlpp::Element* elemFields = nodeTable->add_child("fields");

  FieldType::type_map_type_names type_names = FieldType::get_usable_type_names();

  //Add each <field> node:
  for(type_vecFields::iterator iter = vecFields.begin(); iter != vecFields.end(); iter++)
  {
    const Field& field = *iter;

    xmlpp::Element* elemField = elemFields->add_child("field");
    set_node_attribute_value(elemField, "name", field.get_field_info().get_name());
    set_node_attribute_value(elemField, "title", field.get_title());

    Gnome::Gda::FieldAttributes field_info = field.get_field_info();
    set_node_attribute_value_as_bool(elemField, "primary_key", field_info.get_primary_key());
    set_node_attribute_value_as_bool(elemField, "unique", field_info.get_unique_key());
    set_node_attribute_value_as_bool(elemField, "auto_increment", field_info.get_auto_increment());
    set_node_attribute_value(elemField, "default_value", field_info.get_default_value().to_string());

    Glib::ustring field_type;
    FieldType::type_map_type_names::iterator iterTypes = type_names.find( field.get_field_type().get_glom_type() );
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

  set_modified();
}

bool Document_Glom::get_field(const Glib::ustring& strTableName, const Glib::ustring& strFieldName, Field& fieldResult) const
{
  fieldResult = Field(); //Initialize output arg.

  type_vecFields vecFields = get_table_fields(strTableName);
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

xmlpp::Element* Document_Glom::get_node_connection()
{
  xmlpp::Element* nodeRoot = get_node_document();
  return get_node_child_named_with_add(nodeRoot, "connection");
}

xmlpp::Element* Document_Glom::get_node_table(const Glib::ustring& strTableName)
{
  xmlpp::Element* nodeRoot = get_node_document();
  if(nodeRoot)
  {
    //Look at each "table" node.
    xmlpp::Node::NodeList listNodes = nodeRoot->get_children("table");
    for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
    {
      xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
      if(nodeChild)
      {
        const Glib::ustring& strTableNameNode = get_node_attribute_value(nodeChild, "name");
        if(strTableName == strTableNameNode)
          return nodeChild; //Found.
      }
    }
  }
  else
  {
    std::cout << "Document_Glom::get_node_table() not found." << std::endl;
    return 0;
  }

  return 0;
}

const xmlpp::Element* Document_Glom::get_node_table(const Glib::ustring& strTableName) const
{
  return const_cast<Document_Glom*>(this)->get_node_table(strTableName);
}

xmlpp::Element* Document_Glom::get_node_table_with_add(const Glib::ustring& strTableName)
{
  xmlpp::Element* nodeResult = get_node_table(strTableName);

  //Create it if it's not there:
  if(!nodeResult)
  {
    xmlpp::Element* nodeRoot = get_node_document();
    if(nodeRoot)
    {
      xmlpp::Element* elemNode = nodeRoot->add_child("table");
      set_node_attribute_value(elemNode, "name", strTableName);

      nodeResult = elemNode;

      set_modified();
    }
  }

  return nodeResult;
}

void Document_Glom::change_field_name(const Glib::ustring& strTableName, const Glib::ustring& strFieldNameOld, const Glib::ustring& strFieldNameNew)
{
  //Fields:
  type_vecFields vecFields = get_table_fields(strTableName);
  type_vecFields::iterator iterFind = std::find_if( vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(strFieldNameOld) );
  if(iterFind != vecFields.end()) //If it was found:
  {
    Field field = *iterFind;

    //Change field name:
    Gnome::Gda::FieldAttributes fieldInfo = field.get_field_info();
    fieldInfo.set_name(strFieldNameNew);
    field.set_field_info(fieldInfo);

    //Replace fields:
    set_table_fields(strTableName, vecFields);
  }

  //Relationships:

  //Change relationships in which the field name is a From field.
  //These are grouped together by the table:


  //Change relationships in which the field name is a To field.
  xmlpp::Element* elemRoot = get_node_document();
  if(elemRoot)
  {

    //Get the <tables> nodes:
    xmlpp::Element::NodeList listNodes = elemRoot->get_children("table");
    for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
    {
      xmlpp::Element* elemTable = dynamic_cast<xmlpp::Element*>(*iter);
      if(elemTable)
      {
        //If this is the field's table then change the from field of it's relationships:
        //Always change the To field.
        bool bChangeFromField = false;
        if(get_node_attribute_value(elemTable, "name") == strTableName)
          bChangeFromField = true;

        //Get the <relationships> node, if any:
        xmlpp::Element* elemRelParent = get_node_child_named(elemTable, "relationships");
        if(elemRelParent)
        {
          xmlpp::Node::NodeList listRelParent = elemRelParent->get_children("relationship");
          for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
          {
            xmlpp::Element* nodeRelationship = dynamic_cast<xmlpp::Element*>(*iter);
            if(nodeRelationship)
            {
              //Change the key attribute if necessary:
              {
                const Glib::ustring& strAttName = "key";
                if(get_node_attribute_value(nodeRelationship, strAttName) == strFieldNameOld)
                  set_node_attribute_value(nodeRelationship, strAttName, strFieldNameNew);
              }

              //Change the other_key attribute if necessary:
              {
                //If the other_table is the table:
                const Glib::ustring& strAttName = "other_table";
                if(get_node_attribute_value(nodeRelationship, strAttName) == strTableName)
                {
                   const Glib::ustring& strAttName = "other_key";
                  if(get_node_attribute_value(nodeRelationship, strAttName) == strFieldNameOld)
                    set_node_attribute_value(nodeRelationship, strAttName, strFieldNameNew);
                }
              }
            }
          }
        }
      }

    } //for tables

  } //if(root != null)

}

void Document_Glom::change_table_name(const Glib::ustring& strTableNameOld, const Glib::ustring& strTableNameNew)
{
  //Change <table> element name:
  xmlpp::Element* nodeRoot = get_node_document();
  if(nodeRoot)
  {

    //Get the <tables> nodes:
    xmlpp::Node::NodeList listNodes = nodeRoot->get_children("table");
    for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
    {
      xmlpp::Element* elemTable = dynamic_cast<xmlpp::Element*>(*iter);
      if(elemTable)
      {
        if(get_node_attribute_value(elemTable, "name") == strTableNameOld) //If its the correct <table>
        {
          //Change the table name:
          set_node_attribute_value(elemTable, "name", strTableNameNew);
        }
        else
        {
          //It's not the table, but we need to check its relationships in case they refer to the table:

          //Get the <relationships> node, if any:
          xmlpp::Element* elemRelParent = get_node_child_named(elemTable, "relationships");
          if(elemRelParent)
          {
            //Get the <relationship> nodes:
            xmlpp::Node::NodeList listNodes = elemRelParent->get_children("relationship");
            for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
            {
              xmlpp::Element* nodeRelationship = dynamic_cast<xmlpp::Element*>(*iter);
              if(nodeRelationship)
              {
                const Glib::ustring& strAttName = "other_table";
                if(get_node_attribute_value(nodeRelationship, strAttName) == strTableNameOld)
                  set_node_attribute_value(nodeRelationship, strAttName, strTableNameNew);
              }
            }
          }
        }
      }

    } //for tables

  } //if(root != null)

}

Document_Glom::type_mapFieldSequence Document_Glom::get_data_layout_list(const Glib::ustring& strTableName) const
{
  return get_data_layout("list", strTableName);
}

Document_Glom::type_mapFieldSequence Document_Glom::get_data_layout_details(const Glib::ustring& strTableName) const
{
  return get_data_layout("details", strTableName);
}

xmlpp::Element* Document_Glom::get_node_data_layout(const Glib::ustring& layout_name, const Glib::ustring& strTableName)
{
  const Document_Glom* constThis = const_cast<const Document_Glom*>(this);
  return const_cast<xmlpp::Element*>(constThis->get_node_data_layout(layout_name, strTableName));
}
  
const xmlpp::Element* Document_Glom::get_node_data_layout(const Glib::ustring& layout_name, const Glib::ustring& strTableName) const
{
  const xmlpp::Element* nodeTable = get_node_table(strTableName);
  if(nodeTable)
  {
    const xmlpp::Element* nodeDataLayouts = get_node_child_named(nodeTable, "data_layouts");
    if(nodeDataLayouts)
    {
       return get_node_child_named(nodeDataLayouts, "data_layout_" + layout_name);
    }
  }
  
  return 0;
}

xmlpp::Element* Document_Glom::get_node_data_layout_with_add(const Glib::ustring& layout_name, const Glib::ustring& strTableName)
{
  xmlpp::Element* nodeTable = get_node_table_with_add(strTableName);
  if(nodeTable)
  {
    xmlpp::Element* nodeDataLayouts = get_node_child_named_with_add(nodeTable, "data_layouts");
    if(nodeDataLayouts)
    {
       return get_node_child_named_with_add(nodeDataLayouts, "data_layout_" + layout_name);
    }
  }

  return 0;
}

  
Document_Glom::type_mapFieldSequence Document_Glom::get_data_layout(const Glib::ustring& layout_name, const Glib::ustring& strTableName) const
{
  type_mapFieldSequence result;

  const xmlpp::Element* nodeDataLayoutList = get_node_data_layout(layout_name, strTableName);
  if(nodeDataLayoutList)
  {
    //Get its children:
    xmlpp::Node::NodeList listNodes = nodeDataLayoutList->get_children("data_layout_item");
    for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
    {
       xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
       if(node)
       {
         const Glib::ustring name = get_node_attribute_value(node, "name");
       
         if(!name.empty())
         {
           const guint sequence = get_node_attribute_value_as_decimal(node, "sequence");
           const bool hidden = get_node_attribute_value_as_bool(node, "hidden");
           const Glib::ustring group = get_node_attribute_value(node, "group");
         
           LayoutItem layout_item;
           layout_item.m_field_name = name;
           layout_item.m_sequence = sequence;
           layout_item.m_hidden = hidden;
           layout_item.m_group = group;

           result[sequence] = layout_item;
         }
       }
    }
  }

  return result;
}

void Document_Glom::set_data_layout_list(const Glib::ustring& strTableName, const type_mapFieldSequence& sequence)
{
  set_data_layout("list", strTableName, sequence);
}

void Document_Glom::set_data_layout_details(const Glib::ustring& strTableName, const type_mapFieldSequence& sequence)
{
  set_data_layout("details", strTableName, sequence);
}

void Document_Glom::set_data_layout(const Glib::ustring& layout_name, const Glib::ustring& strTableName, const type_mapFieldSequence& sequence)
{
  xmlpp::Element* nodeDataLayoutList = get_node_data_layout_with_add(layout_name, strTableName);
  if(nodeDataLayoutList)
  {
    //Remove all its children:
    xmlpp::Node::NodeList listNodes = nodeDataLayoutList->get_children("data_layout_item");
    for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
    {
       xmlpp::Node* node = *iter;
       nodeDataLayoutList->remove_child(node);
    }

    //Add the new children:

    //Fields, with sequence:
    for(type_mapFieldSequence::const_iterator iter = sequence.begin(); iter != sequence.end(); ++iter)
    {
      const guint sequence = iter->first;
      const Glib::ustring field_name = (iter->second).m_field_name;

      xmlpp::Element* child = nodeDataLayoutList->add_child("data_layout_item");

      //Get text representation of int:
      std::stringstream thestream;
      thestream << sequence;
      Glib::ustring sequence_string = thestream.str();

      child->set_attribute("sequence", sequence_string);
      child->set_attribute("name", field_name);
      set_node_attribute_value_as_bool(child, "hidden", (iter->second).m_hidden);
      child->set_attribute("group", iter->second.m_group);
    }

  }

  set_modified();
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

guint Document_Glom::get_node_attribute_value_as_decimal(const xmlpp::Element* node, const Glib::ustring& strAttributeName)
{
  guint result = 0;
  const Glib::ustring value_string = get_node_attribute_value(node, strAttributeName);

  //Get number for string:
  if(!value_string.empty())
  {
    //Visible fields, with sequence:
    std::stringstream thestream;
    thestream << value_string;
    thestream >> result;
  }

  return result;
}

Document_Glom::type_listTableInfo Document_Glom::get_tables() const
{
  type_listTableInfo result;

  const xmlpp::Element* nodeRoot = get_node_document();
  if(nodeRoot)
  {
    //Look at each "table" node.
    xmlpp::Node::NodeList listNodes = nodeRoot->get_children("table");
    for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
    {
      xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
      if(nodeChild)
      {
        TableInfo table_info;
        
        table_info.m_name = get_node_attribute_value(nodeChild, "name");
        table_info.m_hidden = get_node_attribute_value_as_bool(nodeChild, "hidden");
        table_info.m_title = get_node_attribute_value(nodeChild, "title");
        table_info.m_default = get_node_attribute_value_as_bool(nodeChild, "default");
        
        result.push_back(table_info);
      }
    }
  }
        
  return result;
}

void Document_Glom::set_tables(const type_listTableInfo& tables)
{
  //Unset any existing ones, so that we are guaranteed to unset any that have been changed to non-hidden:
  const xmlpp::Element* nodeRoot = get_node_document();
  if(nodeRoot)
  {
    //TODO: Store the titles.
    //TODO: Avoid adding information about tables that we don't know about - that should be done explicitly.
    //Look at each "table" node.
    xmlpp::Node::NodeList listNodes = nodeRoot->get_children("table");

    for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
    {
      xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
      if(nodeChild)
      {
        const Glib::ustring table_name = get_node_attribute_value(nodeChild, "name");
      
        type_listTableInfo::const_iterator iterFind = std::find_if(tables.begin(), tables.end(), predicate_FieldHasName<TableInfo>(table_name));
        if(iterFind != tables.end())
        {
          set_node_attribute_value_as_bool(nodeChild, "hidden", iterFind->m_hidden);
          set_node_attribute_value(nodeChild, "title", iterFind->m_title);
          set_node_attribute_value_as_bool(nodeChild, "default", iterFind->m_default);
        }
      }
    }
  }
  
}

Document_Glom::type_mapLayoutGroupSequence Document_Glom::get_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& strTableName)
{
  type_mapLayoutGroupSequence result;

  xmlpp::Element* nodeDataLayoutList = get_node_data_layout(layout_name, strTableName);
  if(nodeDataLayoutList)
  {
    xmlpp::Element* nodeGroups = get_node_child_named(nodeDataLayoutList, "data_layout_groups");
    if(nodeGroups)
    {
      //Look at all its children:
      xmlpp::Node::NodeList listNodes = nodeGroups->get_children("data_layout_group");
      for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
      {
         xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
         if(node)
         {
          Glib::ustring name = get_node_attribute_value(node, "name");
          if(!name.empty())
          {
            LayoutGroup item;
            item.m_group_name = get_node_attribute_value(node, "name");;

            guint sequence = get_node_attribute_value_as_decimal(node, "sequence");
            item.m_sequence = sequence;
            
            result[sequence] = item;
          }
        }
      }
    }
  }
      
  return result;
}

void Document_Glom::set_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& strTableName, const type_mapLayoutGroupSequence& groups)
{
  xmlpp::Element* nodeDataLayoutList = get_node_data_layout_with_add(layout_name, strTableName);
  if(nodeDataLayoutList)
  {
    xmlpp::Element* nodeGroups = get_node_child_named_with_add(nodeDataLayoutList, "data_layout_groups");
    if(nodeGroups)
    {
     //Remove all its children:
      xmlpp::Node::NodeList listNodes = nodeGroups->get_children("data_layout_group");
      for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
      {
         xmlpp::Node* node = *iter;
         nodeDataLayoutList->remove_child(node);
      }

      //Add the groups:
      for(type_mapLayoutGroupSequence::const_iterator iter = groups.begin(); iter != groups.end(); ++iter)
      {
        xmlpp::Element* child = nodeGroups->add_child("data_layout_group");

        child->set_attribute("name", iter->second.m_group_name);


        //Get text representation of int:
        std::stringstream thestream;
        thestream <<  iter->second.m_sequence;
        Glib::ustring sequence_string = thestream.str();
      
        child->set_attribute("sequence", sequence_string);
      }
    }
  }
}

Glib::ustring Document_Glom::get_table_title(const Glib::ustring& table_name) const
{
  Glib::ustring result;
  
  const xmlpp::Element* nodeTable = get_node_table(table_name);
  if(nodeTable)
  {
    result = get_node_attribute_value(nodeTable, "title");
  }
  return result;
}

void Document_Glom::set_table_title(const Glib::ustring& table_name, const Glib::ustring value)
{
  xmlpp::Element* nodeTable = get_node_table_with_add(table_name);
  if(nodeTable)
    set_node_attribute_value(nodeTable, "title", value);
}

bool Document_Glom::get_table_is_known(const Glib::ustring& table_name) const
{
  return ( get_node_table(table_name) != 0);
}

Document_Glom::type_mapFieldSequence Document_Glom::get_data_layout_plus_new_fields(const Glib::ustring& layout_name, const Glib::ustring& strTableName) const
{
  type_mapFieldSequence result = get_data_layout(layout_name, strTableName);

  //Get next available sequence:
  guint sequence = 0;
   if(!result.empty())
     sequence = result.rbegin()->first;
  ++sequence;

  //Add extra fields that are not mentioned in the layout. (They should at least be in the layout as hidden)
  type_vecFields all_fields = get_table_fields(strTableName);
  for(type_vecFields::const_iterator iter = all_fields.begin(); iter != all_fields.end(); ++iter)
  {
    const Glib::ustring name = iter->get_name();

    //See whether it's already in the result:
    bool found = false; //TODO: This is horrible.
    for(type_mapFieldSequence::const_iterator iterFind = result.begin(); iterFind != result.end(); ++iterFind)
    {
      if(iterFind->second.m_field_name == name)
      {
        found = true;
        break;
      }
    }
    
    if(!found)
    {
      LayoutItem layout_item;
      layout_item.m_field_name = name;
      layout_item.m_sequence = sequence;
      layout_item.m_hidden = false;

      result[sequence] = layout_item;

      ++sequence;
    }
  }
  
  return result; 
}

AppState::userlevels Document_Glom::get_userlevel() const
{
  if(get_read_only())
  {
    return AppState::USERLEVEL_OPERATOR; //A read-only document can not be changed, so there's no point in being in developer mode. This is one way to control the user level on purpose.
  }
  else if(m_file_uri.empty()) //If it has never been saved then this is a new default document, so the user created it, so the user can be a developer.
  {
    return AppState::USERLEVEL_DEVELOPER;
  }
  else
    return m_app_state.get_userlevel();
}

Document_Glom::type_signal_userlevel_changed Document_Glom::signal_userlevel_changed()
{
  return m_signal_userlevel_changed;
}

void Document_Glom::on_app_state_userlevel_changed(AppState::userlevels userlevel)
{
  m_signal_userlevel_changed.emit(userlevel);
}

void Document_Glom::set_userlevel(AppState::userlevels userlevel)
{
  m_app_state.set_userlevel(userlevel);
}

void Document_Glom::emit_userlevel_changed()
{
  m_signal_userlevel_changed.emit(m_app_state.get_userlevel());
}

 

  
  

