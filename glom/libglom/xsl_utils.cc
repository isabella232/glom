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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include <libglom/libglom_config.h>

#include <libglom/xsl_utils.h>
#include <libglom/connectionpool.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/utils.h>
#include <libxml++/libxml++.h>
#include <libxslt/transform.h>
//#include <libexslt/exslt.h> //For exsltRegisterAll().
#include <giomm/file.h>
#include <glibmm/miscutils.h>
#include <glibmm/i18n.h>

#include <sstream> //For stringstream

#include <iostream>
#include <fstream>

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl
#include <iomanip>


namespace
{
  static std::string get_xslt_filepath(const std::string& xsl_file)
  {
    const std::string resource_path = "/org/gnome/glom/libglom/data/xslt/" + xsl_file;
    if(!Glom::Utils::get_resource_exists(resource_path))
    {
      std::cerr << G_STRFUNC << ": xslt resource not found: " << resource_path << std::endl;
    }

    return "resource://" + resource_path;
  }
}

namespace Glom
{


static Glib::ustring xslt_process(const xmlpp::Document& xml_document, const std::string& resource_path_xslt)
{
  //Debug output:
  //std::cout << "XML before XSLT processing: " << std::endl;
  //std::cout << "  ";
  //xmlpp::Document& nonconst = const_cast<xmlpp::Document&>(xml_document);
  //nonconst.write_to_stream_formatted(std::cout);
  //std::cout << std::endl;

  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(resource_path_xslt); //This must use a resource:// URI.
  char* xslt_data = 0;
  gsize xslt_length = 0;
  try
  {
    file->load_contents(xslt_data, xslt_length);
  }
  catch(const xmlpp::exception& ex)
  {
    std::cerr << G_STRFUNC << ": Could not load XSLT from resource path: " << resource_path_xslt << std::endl;
    return Glib::ustring();
  }

  Glib::ustring result;

  //Use libxslt to transform the XML:
  xmlDocPtr style = xmlReadDoc((xmlChar*)xslt_data, 0, 0, 0);
  if(style)
  {
    //We need this to be able to use the exsl: functions, even if we declare the namespace at the start of the xsl.
    //We don't need this anymore - we use xsl:copy-of instead of exsl:node-set (which didn't work as expected anyway):
    //exsltRegisterAll();

    //Parse the stylesheet:
    xsltStylesheetPtr cur = xsltParseStylesheetDoc(style);
    if(cur)
    {
      //Use the parsed stylesheet on the XML:
      xmlDocPtr pDocOutput = xsltApplyStylesheet(cur, const_cast<xmlDoc*>(xml_document.cobj()), 0);
      xsltFreeStylesheet(cur);

      //Get the output text:
      xmlChar* buffer = 0;
      int length = 0;
      xmlIndentTreeOutput = 1; //Format the output with extra white space. TODO: Is there a better way than this global variable?
      xmlDocDumpFormatMemoryEnc(pDocOutput, &buffer, &length, 0, 0);

      if(buffer)
      {
        // Create a Glib::ustring copy of the buffer

        // Here we force the use of Glib::ustring::ustring( InputIterator begin, InputIterator end )
        // instead of Glib::ustring::ustring( const char*, size_type ) because it
        // expects the length of the string in characters, not in bytes.
        result = Glib::ustring( reinterpret_cast<const char *>(buffer), reinterpret_cast<const char *>(buffer + length) );

        // Deletes the original buffer
        xmlFree(buffer);
      }

      xmlFreeDoc(pDocOutput);
    }
  }

  return result;
}

Glib::ustring GlomXslUtils::transform(const xmlpp::Document& xml_document, const std::string& xsl_file_path)
{
  //Use libxslt to convert the XML to HTML:
  return xslt_process(xml_document, get_xslt_filepath(xsl_file_path));
  //std::cout << "After xslt: " << result << std::endl;
}

} //namespace Glom
