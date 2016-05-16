/* Glom
 *
 * Copyright (C) 2012 Openismus GmbH
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

#include <config.h> //For GLOM_MSGFMT

#include <libglom/document/document.h>
#include <libglom/translations_po.h>
#include <libglom/init.h>
#include <libglom/file_utils.h>
#include <giomm/file.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <glibmm/shell.h>
#include <glibmm/spawn.h>

#include <iostream>

static bool check_po_file(const std::string& filepath)
{
  if(filepath.empty())
    return false;

  //We could use the gettext-po po_file_check_all() function to check
  //the file, but the gettext-po error handling is very awkward,
  //so let's keep it simple:
  int return_status = EXIT_FAILURE;
  std::string stdout_output;
  const auto command = Glib::ustring::compose(GLOM_MSGFMT " %1",
    Glib::shell_quote(filepath));
  try
  {
    Glib::spawn_command_line_sync(command, &stdout_output, 0, &return_status);
    //std::cout << " debug: output=" << stdout_output << std::endl;
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception caught: " << ex.what() << std::endl;
  }

  if(return_status != EXIT_SUCCESS)
  {
    std::cout << stdout_output << std::endl;
    return false;
  }

  return true;
}

int main()
{
  Glom::libglom_init();

  // Get a URI for a test file:
  Glib::ustring uri;

  try
  {
    const auto path =
       Glib::build_filename(GLOM_DOCDIR_EXAMPLES_NOTINSTALLED,
         "example_film_manager.glom");
    uri = Glib::filename_to_uri(path);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what();
    return EXIT_FAILURE;
  }

  //std::cout << "URI=" << uri << std::endl;


  // Load the document:
  auto document = std::make_shared<Glom::Document>();
  document->set_file_uri(uri);
  int failure_code = 0;
  const auto test = document->load(failure_code);
  //std::cout << "Document load result=" << test << std::endl;

  if(!test)
  {
    std::cerr << G_STRFUNC << ": Document::load() failed with failure_code=" << failure_code << std::endl;
    return EXIT_FAILURE;
  }

  const auto po_file_uri = Glom::FileUtils::get_temp_file_uri("glom_export.po");
  if(po_file_uri.empty())
  {
    std::cerr << G_STRFUNC << ": Could not generate a temporary file URI=\n";
    return EXIT_FAILURE;
  }

  //std::cout << "po file URI: " << po_file_uri << std::endl;

  const Glib::ustring locale = "de";
  const bool success =
    Glom::write_translations_to_po_file(document, po_file_uri, locale);
  if(!success)
  {
    std::cerr << G_STRFUNC << ": Glom::write_translations_to_po_file() failed.\n";
    return EXIT_FAILURE;
  }

  //Get a filepath for the URI:
  std::string po_file_path;
  try
  {
    po_file_path = Glib::filename_from_uri(po_file_uri);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what();
    return EXIT_FAILURE;
  }

  //Check that the exported po file contains an expected string:
  std::string data;
  try
  {
    data = Glib::file_get_contents(po_file_path);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Failed: file_get_contents() failed: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  bool text_found =
    (data.find("Stabliste") != std::string::npos);
  g_assert(text_found);

  text_found =
    (data.find("\u00DCbersicht") != std::string::npos);
  g_assert(text_found);


  //Check that the .po file is valid:
  check_po_file(po_file_path);

  Glom::FileUtils::delete_file(po_file_uri);

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
