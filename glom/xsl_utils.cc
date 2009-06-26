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

#include <libglom/libglom_config.h> // For GLOM_ENABLE_MAEMO

#include "xsl_utils.h"
#include <libglom/connectionpool.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/glomconversions.h>
#include "frame_glom.h"
#include <libxml++/libxml++.h>
#include <libxslt/transform.h>
//#include <libexslt/exslt.h> //For exsltRegisterAll().
#include <giomm.h>
#include <glibmm/i18n.h>
#include <gtk/gtk.h>

#include <sstream> //For stringstream

#include <iostream>
#include <fstream>

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl
#include <iomanip>

// For ShellExecute:
#ifdef G_OS_WIN32
# define GLOM_SAVE_DATADIR DATADIR
# undef DATADIR
# include <windows.h>
# define DATADIR GLOM_SAVE_DATADIR
#endif

namespace
{
	Glib::ustring get_xslt_file(const Glib::ustring& xsl_file)
	{
#ifdef G_OS_WIN32
		gchar* directory;
		directory = g_win32_get_package_installation_directory_of_module(NULL);
		Glib::ustring xsltdir = Glib::build_filename(directory, "share/glom/xslt/" + xsl_file);
		g_free(directory);
		return xsltdir;
#else
		return GLOM_XSLTDIR + xsl_file;
#endif
	}
}

namespace Glom
{

void GlomXslUtils::transform_and_open(const xmlpp::Document& xml_document, const Glib::ustring& xsl_file_path, Gtk::Window* parent_window)
{
  //Use libxslt to convert the XML to HTML:
  Glib::ustring result = xslt_process(xml_document, get_xslt_file(xsl_file_path));
  std::cout << "After xslt: " << result << std::endl;

  //Save it to a temporary file and show it in a browser:
  //TODO: This actually shows it in gedit.
  const Glib::ustring temp_path = Glib::get_tmp_dir() + "/glom_printout.html";
  std::cout << "temp_path=" << temp_path << std::endl;

  Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(temp_path);
  Glib::RefPtr<Gio::FileOutputStream> stream;

  //Create the file if it does not already exist:
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    if(file->query_exists())
    {
      stream = file->replace(); //Instead of append_to().
    }
    else
    {
      //By default files created are generally readable by everyone, but if we pass FILE_CREATE_PRIVATE in flags the file will be made readable only to the current user, to the level that is supported on the target filesystem.
      //TODO: Do we want to specify 0660 exactly? (means "this user and his group can read and write this non-executable file".)
      stream = file->create_file();
    }
  }
  catch(const Gio::Error& ex)
  {
#else
  std::auto_ptr<Gio::Error> error;
  stream.create(error);
  if(error.get() != NULL)
  {
    const Gio::Error& ex = *error.get();
#endif
    // If the operation was not successful, print the error and abort
    return; // false; // print_error(ex, output_uri_string);
  }

  //Write the data to the output uri
  gsize bytes_written = 0;
  const Glib::ustring::size_type result_bytes = result.bytes();
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    bytes_written = stream->write(result.data(), result_bytes);
  }
  catch(const Gio::Error& ex)
  {
#else
  bytes_written = stream->write(result.data(), result_bytes, error);
  if(error.get() != NULL)
  {
    Gio::Error& ex = *error.get();
#endif
    // If the operation was not successful, print the error and abort
    return; // false; //print_error(ex, output_uri_string);
  }

  if(bytes_written != result_bytes)
    return; //false


  //Give the user a clue, in case the web browser opens in the background, for instance in a new tab:
  if(parent_window)
    Frame_Glom::show_ok_dialog(_("Report Finished"), _("The report will now be opened in your web browser."), *parent_window, Gtk::MESSAGE_INFO);

#ifdef G_OS_WIN32
  // gtk_show_uri doesn't seem to work on Win32, at least not for local files
  // We use Windows API instead.
  // TODO: Check it again and file a bug if necessary.
  ShellExecute(NULL, "open", file->get_path().c_str(), NULL, NULL, SW_SHOW);
#else
  //Use the GNOME browser:
  GError* gerror = 0;
  if(!gtk_show_uri(0 /* screen */, file->get_uri().c_str(), GDK_CURRENT_TIME, &gerror))
  {
    std::cerr << "Error while calling gtk_show_uri(): " << gerror->message << std::endl;
    g_error_free(gerror);
  }
#endif
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
