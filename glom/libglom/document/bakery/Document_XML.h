/*
 * Copyright 2000-2002 Murray Cumming
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef GLOM_BAKERY_DOCUMENT_XML_H
#define GLOM_BAKERY_DOCUMENT_XML_H

#include <libglom/document/bakery/Document.h>
#include <libxml++/libxml++.h>

//Features:
//- Read/Write to the Document in terms of XML DOM.

//Future features:
//- Parse the document from disk instad of memory - This *might* allow for very large documents.

namespace GlomBakery
{

class Document_XML : public GlomBakery::Document
{
public: 
  Document_XML();
  virtual ~Document_XML();

  //overrides:
  virtual bool load_after();
  virtual bool save_before();

  virtual void set_dtd_name(const std::string& strVal); //e.g. "glom.dtd"
  virtual std::string get_dtd_name() const;

  virtual void set_dtd_root_node_name(const Glib::ustring& strVal);
  virtual Glib::ustring get_dtd_root_node_name() const;

  /** Whether to add extra whitespace when writing the XML to disk.
   * Do not use this if whitespace is significant in your XML format.
   * See also add_indenting_white_space().
   */
  void set_write_formatted(bool formatted = true);

  /** Put each node on its own line and add white space for indenting,
   * even if there are child text nodes.
   * set_write_formatted() does not cause nodes to be indented if there are child text nodes,
   * because it assumes that the white space is then significant.
   */
  void add_indenting_white_space();
  
  virtual bool set_xml(const Glib::ustring& strXML); //Parse the XML from the text.
  virtual Glib::ustring get_xml() const; //Get the text for the XML.

protected:
  static Glib::ustring get_node_attribute_value(const xmlpp::Element* node, const Glib::ustring& strAttributeName);
  static void set_node_attribute_value(xmlpp::Element* node, const Glib::ustring& strAttributeName, const Glib::ustring& strValue);

  static xmlpp::Element* get_node_child_named(const xmlpp::Element* node, const Glib::ustring& strName);
  static xmlpp::Element* get_node_child_named_with_add(xmlpp::Element* node, const Glib::ustring& strName);

  virtual const xmlpp::Element* get_node_document() const; //e.g. <glom_document> (root name)
  virtual xmlpp::Element* get_node_document(); //e.g. <glom_document> (root name)

  virtual void Util_DOM_Write(Glib::ustring& refstrXML) const;

  void add_indenting_white_space_to_node(xmlpp::Node* node = 0, const Glib::ustring& start_indent = Glib::ustring());

  typedef GlomBakery::Document type_base;

  //XML Parsing bits:
  xmlpp::DomParser m_DOM_Parser; //Could be mutable to allow us to guarantee a root node.
  xmlpp::Document* m_pDOM_Document; //1-to-1 with the m_DOM_Parser.
  
  std::string m_strDTD_Name;
  Glib::ustring m_strRootNodeName;
  bool m_write_formatted;
};

} //namespace GlomBakery.

#endif // GLOM_BAKERY_DOCUMENT_XML_H
