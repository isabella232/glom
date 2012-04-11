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
  static std::string get_xsl_file_dir()
  {
#ifdef G_OS_WIN32
    gchar* directory = g_win32_get_package_installation_directory_of_module(0);
    const std::string xsltdir = Glib::build_filename(directory,
        "share" G_DIR_SEPARATOR_S "glom" G_DIR_SEPARATOR_S "xslt");
    g_free(directory);
    return xsltdir;
#else
    return GLOM_PKGDATADIR_XSLT;
#endif
  }

  static std::string get_xslt_file(const std::string& xsl_file)
  {
    const std::string result = Glib::build_filename(get_xsl_file_dir(), xsl_file);

    // Check that it exists:
    const Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(result);
    if(file && file->query_exists())
      return result;

    return Glib::build_filename(GLOM_PKGDATADIR_XSLT_NOTINSTALLED, xsl_file);
  }
}

namespace Glom
{


static Glib::ustring xslt_process(const xmlpp::Document& xml_document, const std::string& filepath_xslt)
{
  //Debug output:
  //std::cout << "XML before XSLT processing: " << std::endl;
  //std::cout << "  ";
  //xmlpp::Document& nonconst = const_cast<xmlpp::Document&>(xml_document);
  //nonconst.write_to_stream_formatted(std::cout);
  //std::cout << std::endl;

  Glib::ustring result;

  //Use libxslt to transform the XML:
  xmlDocPtr style = xmlReadFile(filepath_xslt.c_str(), 0, 0);
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
  return xslt_process(xml_document, get_xslt_file(xsl_file_path));
  //std::cout << "After xslt: " << result << std::endl;
}

} //namespace Glom
