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

#include <libglom/document/bakery/document.h>
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
  virtual bool load_after(int& failure_code) override;
  virtual bool save_before() override;

  void set_dtd_name(const std::string& strVal); //e.g. "glom.dtd"
  std::string get_dtd_name() const;

  /** Set the root node name and (optionally) the xmlns ID to be written 
   * when writing the document.
   * The root node name is also used when reading documents.
   */
  void set_dtd_root_node_name(const Glib::ustring& strVal, const Glib::ustring& xmlns = Glib::ustring());
  
  Glib::ustring get_dtd_root_node_name() const;
  
  Glib::ustring get_xml() const; //Get the text for the XML.

protected:

  const xmlpp::Element* get_node_document() const; //e.g. <glom_document> (root name)
  xmlpp::Element* get_node_document(); //e.g. <glom_document> (root name)

  void Util_DOM_Write(Glib::ustring& refstrXML) const;

  /** Put each node on its own line and add white space for indenting,
   * even if there are child text nodes.
   * set_write_formatted() does not cause nodes to be indented if there are child text nodes,
   * because it assumes that the white space is then significant.
   */
  void add_indenting_white_space_to_node(xmlpp::Node* node = 0, const Glib::ustring& start_indent = Glib::ustring());

  typedef GlomBakery::Document type_base;

  //XML Parsing bits:
  xmlpp::DomParser m_DOM_Parser; //Could be mutable to allow us to guarantee a root node.
  xmlpp::Document* m_pDOM_Document; //1-to-1 with the m_DOM_Parser.
  
  std::string m_strDTD_Name;
  Glib::ustring m_strRootNodeName, m_root_xmlns;
  bool m_write_formatted;
};

} //namespace GlomBakery.

#endif // GLOM_BAKERY_DOCUMENT_XML_H
