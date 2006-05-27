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

#include "xsl_utils.h"
#include <glom/libglom/connectionpool.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <glom/libglom/data_structure/glomconversions.h>
#include "frame_glom.h"
#include <libxml++/libxml++.h>
#include <libxslt/transform.h>
//#include <libexslt/exslt.h> //For exsltRegisterAll().
#include <libgnomevfsmm.h>
#include <glibmm/i18n.h>
#include <libgnome/gnome-url.h>
#include <sstream> //For stringstream

#include <iostream>
#include <fstream>

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl
#include <iomanip>


namespace Glom
{

void GlomXslUtils::transform_and_open(const xmlpp::Document& xml_document, const Glib::ustring& xsl_file_path, Gtk::Window* parent_window)
{
  //Use libxslt to convert the XML to HTML:
  Glib::ustring result = xslt_process(xml_document, GLOM_XSLTDIR + xsl_file_path);
  std::cout << "After xslt: " << result << std::endl;

  //Save it to a temporary file and show it in a browser:
  //TODO: This actually shows it in gedit.
  const Glib::ustring temp_uri = "file:///tmp/glom_printout.html";
  std::cout << "temp_uri=" << temp_uri << std::endl;

  Gnome::Vfs::Handle write_handle;

  try
  {
    //0660 means "this user and his group can read and write this non-executable file".
    //The 0 prefix means that this is octal.
    write_handle.create(temp_uri, Gnome::Vfs::OPEN_WRITE, false, 0660 /* leading zero means octal */);
  }
  catch(const Gnome::Vfs::exception& ex)
  {
    // If the operation was not successful, print the error and abort
    return; // print_error(ex, output_uri_string);
  }

  try
  {
    //Write the data to the output uri
    /* GnomeVFSFileSize bytes_written = */ write_handle.write(result.data(), result.bytes());
  }
  catch(const Gnome::Vfs::exception& ex)
  {
    // If the operation was not successful, print the error and abort
    return; //print_error(ex, output_uri_string);
  }

  //Give the user a clue, in case the web browser opens in the background, for instance in a new tab:
  if(parent_window)
    Frame_Glom::show_ok_dialog(_("Report Finished"), _("The report will now be opened in your web browser."), *parent_window, Gtk::MESSAGE_INFO);

  //Use the GNOME browser:
  GError* error = 0;
  gnome_url_show(temp_uri.c_str(), &error); //This is in libgnome.
}

Glib::ustring GlomXslUtils::xslt_process(const xmlpp::Document& xml_document, const std::string& filepath_xslt)
{
  //Debug output:
  std::cout << "XML before XSLT processing: " << std::endl;
  std::cout << "  ";
  xmlpp::Document& nonconst = const_cast<xmlpp::Document&>(xml_document);
  nonconst.write_to_stream_formatted(std::cout);
  std::cout << std::endl;

  Glib::ustring  result;

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

} //namespace Glom
