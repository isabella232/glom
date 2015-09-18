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

#include <libglom/data_structure/field.h>
#include <libxml++/libxml++.h>
#include <libgdamm/value.h>

#ifndef GLOM_XML_UTILS_H
#define GLOM_XML_UTILS_H

namespace Glom
{

namespace XmlUtils
{

Glib::ustring get_node_attribute_value(const xmlpp::Element* node, const Glib::ustring& strAttributeName);
void set_node_attribute_value(xmlpp::Element* node, const Glib::ustring& strAttributeName, const Glib::ustring& strValue);

xmlpp::Element* get_node_child_named(xmlpp::Element* node, const Glib::ustring& strName);
const xmlpp::Element* get_node_child_named(const xmlpp::Element* node, const Glib::ustring& strName);

xmlpp::Element* get_node_child_named_with_add(xmlpp::Element* node, const Glib::ustring& strName);

/// If the attribute is not there, then the default will be returned.
bool get_node_attribute_value_as_bool(const xmlpp::Element* node, const Glib::ustring& strAttributeName, bool value_default = false);
guint get_node_attribute_value_as_decimal(const xmlpp::Element* node, const Glib::ustring& strAttributeName, guint value_default = 0);
double get_node_attribute_value_as_decimal_double(const xmlpp::Element* node, const Glib::ustring& strAttributeName);
float get_node_attribute_value_as_float(const xmlpp::Element* node, const Glib::ustring& strAttributeName);

void set_node_attribute_value_as_bool(xmlpp::Element* node, const Glib::ustring& strAttributeName, bool value = true, bool value_default = false);
void set_node_attribute_value_as_float( xmlpp::Element* node, const Glib::ustring& strAttributeName, float value );
void set_node_attribute_value_as_value(xmlpp::Element* node, const Glib::ustring& strAttributeName, const Gnome::Gda::Value& value, Field::glom_field_type field_type);
void set_node_text_child_as_value(xmlpp::Element* node, const Gnome::Gda::Value& value, Field::glom_field_type field_type);

Gnome::Gda::Value get_node_attribute_value_as_value(const xmlpp::Element* node, const Glib::ustring& strAttributeName, Field::glom_field_type field_type);

///If value is equal to the default then no attribute will be set, to save text space in the XML file.
void set_node_attribute_value_as_decimal(xmlpp::Element* node, const Glib::ustring& strAttributeName, guint value, guint value_default = 0);

// This is required by the report builder, so it cannot be disabled
// in client only mode
void set_node_attribute_value_as_decimal_double(xmlpp::Element* node, const Glib::ustring& strAttributeName, double value);

Gnome::Gda::Value get_node_text_child_as_value(const xmlpp::Element* node, Field::glom_field_type field_type);

Glib::ustring get_child_text_node(const xmlpp::Element* node, const Glib::ustring& child_node_name);

void set_child_text_node(xmlpp::Element* node, const Glib::ustring& child_node_name, const Glib::ustring& text);

} //namespace XmlUtils

} //namespace Glom

#endif //GLOM_XML_UTILS_H
