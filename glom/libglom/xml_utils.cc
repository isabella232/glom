/* Glom
 *
 * Copyright (C) 2001-2012 Murray Cumming
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

#include <libglom/xml_utils.h>
#include <libglom/utils.h>
#include <limits> // for numeric_limits

static const char GLOM_ATTRIBUTE_IMAGE_DATA_FORMAT[] = "format";
static const char GLOM_ATTRIBUTE_IMAGE_DATA_FORMAT_BASE64[] = "base64"; //No attribute here means the old GDA format.

namespace Glom
{

namespace XmlUtils
{

Glib::ustring get_node_attribute_value(const xmlpp::Element* node, const Glib::ustring& strAttributeName)
{
  if(node)
  {
    const auto attribute = node->get_attribute(strAttributeName);
    if(attribute)
    {
      Glib::ustring value = attribute->get_value(); //Success.
      return value;
    }
  }

  return ""; //Failed.
}

void set_node_attribute_value(xmlpp::Element* node, const Glib::ustring& strAttributeName, const Glib::ustring& strValue)
{
  if(node)
  {
    xmlpp::Attribute* attribute = node->get_attribute(strAttributeName);
    if(attribute)
      attribute->set_value(strValue);
    else
    {
      if(!strValue.empty()) //Don't add an attribute if the value is empty, to keep the document smaller.
        node->set_attribute(strAttributeName, strValue);
    }
  }
}

xmlpp::Element* get_node_child_named(const xmlpp::Element* node, const Glib::ustring& strName)
{
  xmlpp::Element* nodeResult = nullptr;

  if(node)
  { 
    xmlpp::Node::NodeList list = node->get_children(strName);

    //We check all of them, instead of just the first, until we find one,
    //because get_children() returns, for instance, TextNodes (which are not Elements) for "text", 
    //as well as Elements with the name "text".
    for(const auto& item : list)
    {
      nodeResult = dynamic_cast<xmlpp::Element*>(item);  
      if(nodeResult)
        return nodeResult;
    }                       
  }

  return nodeResult;
}

xmlpp::Element* get_node_child_named_with_add(xmlpp::Element* node, const Glib::ustring& strName)
{
  xmlpp::Element* nodeResult = get_node_child_named(node, strName);

  if(!nodeResult)
    nodeResult = node->add_child(strName);

  return nodeResult;
}

bool get_node_attribute_value_as_bool(const xmlpp::Element* node, const Glib::ustring& strAttributeName, bool value_default)
{
  bool result = value_default;
  const auto value_string = get_node_attribute_value(node, strAttributeName);

  //Get number for string:
  if(!value_string.empty())
  {
    result = (value_string == "true");
  }

  return result;
}

void set_node_attribute_value_as_bool(xmlpp::Element* node, const Glib::ustring& strAttributeName, bool value, bool value_default)
{
  if((value == value_default) && !node->get_attribute(strAttributeName))
    return; //Use the non-existance of an attribute to mean zero, to save space.

  Glib::ustring strValue = (value ? "true" : "false");
  set_node_attribute_value(node, strAttributeName, strValue);
}

void set_node_attribute_value_as_decimal(xmlpp::Element* node, const Glib::ustring& strAttributeName, guint value, guint value_default)
{
  if((value == value_default) && !node->get_attribute(strAttributeName))
    return; //Use the non-existance of an attribute to mean zero, to save space.

  //Get text representation of int:
  std::stringstream thestream;
  thestream.imbue( std::locale::classic() ); //The C locale.
  thestream << value;
  const auto value_string = thestream.str();

  set_node_attribute_value(node, strAttributeName, value_string);
}

void set_node_attribute_value_as_decimal_double(xmlpp::Element* node, const Glib::ustring& strAttributeName, double value)
{
  if(!value && !node->get_attribute(strAttributeName))
    return; //Use the non-existance of an attribute to mean zero, to save space.

  //Get text representation of int:
  std::stringstream thestream;
  thestream.imbue( std::locale::classic() ); //The C locale.
  thestream << value;
  const auto value_string = thestream.str();

  set_node_attribute_value(node, strAttributeName, value_string);
}

guint get_node_attribute_value_as_decimal(const xmlpp::Element* node, const Glib::ustring& strAttributeName, guint value_default)
{
  guint result = value_default;
  const auto value_string = get_node_attribute_value(node, strAttributeName);

  //Get number for string:
  if(!value_string.empty())
  {
    std::stringstream thestream;
    thestream.imbue( std::locale::classic() ); //The C locale.
    thestream.str(value_string);
    thestream >> result;
  }

  return result;
}

double get_node_attribute_value_as_decimal_double(const xmlpp::Element* node, const Glib::ustring& strAttributeName)
{
  double result = 0;
  const auto value_string = get_node_attribute_value(node, strAttributeName);

  //Get number for string:
  if(!value_string.empty())
  {
    std::stringstream thestream;
    thestream.imbue( std::locale::classic() ); //The C locale.
    thestream.str(value_string);
    thestream >> result;
  }

  return result;
}

void set_node_attribute_value_as_float(xmlpp::Element* node, const Glib::ustring& strAttributeName, float value)
{
    if(value == std::numeric_limits<float>::infinity() && !node->get_attribute(strAttributeName))
    return; //Use the non-existance of an attribute to mean "invalid"/infinity, to save space.

  //Get text representation of float:
  std::stringstream thestream;
  thestream.imbue( std::locale::classic() ); //The C locale.
  thestream << value;
  const auto value_string = thestream.str();

  set_node_attribute_value(node, strAttributeName, value_string);
}

float get_node_attribute_value_as_float(const xmlpp::Element* node, const Glib::ustring& strAttributeName)
{
  float result = std::numeric_limits<float>::infinity();
  const auto value_string = get_node_attribute_value(node, strAttributeName);

  //Get number for string:
  if(!value_string.empty())
  {
    std::stringstream thestream;
    thestream.imbue( std::locale::classic() ); //The C locale.
    thestream.str(value_string);
    thestream >> result;
  }

  return result;
}

//TODO: Stop using this. It's a bad idea to put values in attributes, which cannot escape all characters.
void set_node_attribute_value_as_value(xmlpp::Element* node, const Glib::ustring& strAttributeName, const Gnome::Gda::Value& value,  Field::glom_field_type field_type)
{
  NumericFormat format_ignored; //Because we use ISO format.
  const auto value_as_text = Field::to_file_format(value, field_type);
  set_node_attribute_value(node, strAttributeName, value_as_text);
}

void set_node_text_child_as_value(xmlpp::Element* node, const Gnome::Gda::Value& value, Field::glom_field_type field_type)
{
  if(!node)
    return;

  const auto value_as_text = Field::to_file_format(value, field_type);
  node->set_child_text( Utils::string_clean_for_xml(value_as_text) );

  if(field_type == Field::TYPE_IMAGE)
  {
    set_node_attribute_value(node, GLOM_ATTRIBUTE_IMAGE_DATA_FORMAT, GLOM_ATTRIBUTE_IMAGE_DATA_FORMAT_BASE64);
  }
}

Gnome::Gda::Value get_node_attribute_value_as_value(const xmlpp::Element* node, const Glib::ustring& strAttributeName, Field::glom_field_type field_type)
{
  const auto value_string = get_node_attribute_value(node, strAttributeName);

  bool success = false;
  const auto result = Field::from_file_format(value_string, field_type, success);
  if(success)
    return result;
  else
    return Gnome::Gda::Value();
}

Gnome::Gda::Value get_node_text_child_as_value(const xmlpp::Element* node, Field::glom_field_type field_type)
{
  const auto text_child = node->get_child_text();
  if(!text_child)
    return Gnome::Gda::Value();

  const auto value_string = text_child->get_content();

  bool old_image_format = false;
  const auto format = get_node_attribute_value(node, GLOM_ATTRIBUTE_IMAGE_DATA_FORMAT);
  if(format.empty()) //We previously used the GDA format, before we even specified it.
    old_image_format = true;
    
  bool success = false;
  const auto result = Field::from_file_format(value_string, field_type, success, old_image_format);
  if(success)
    return result;

  return Gnome::Gda::Value();
}


Glib::ustring get_child_text_node(const xmlpp::Element* node, const Glib::ustring& child_node_name)
{
  const auto child = get_node_child_named(node, child_node_name);
  if(child)
  {
     const auto text_child = child->get_child_text();
     if(text_child)
       return text_child->get_content();
  }

  return Glib::ustring();
}

void set_child_text_node(xmlpp::Element* node, const Glib::ustring& child_node_name, const Glib::ustring& text)
{
  xmlpp::Element* child = get_node_child_named(node, child_node_name);
  if(!child)
  {
    if(text.empty())
      return; //Keep the document smaller by avoiding empty nodes.

    child = node->add_child(child_node_name);
  }

  const auto text_used = Utils::string_clean_for_xml(text);

  xmlpp::TextNode* text_child = child->get_child_text();
  if(!text_child)
    child->add_child_text(text_used);
  else
    text_child->set_content(text_used);
}

} //namespace XmlUtils

} //namespace Glom

