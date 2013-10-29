/* Glom
 *
 * Copyright (C) 2010 Openismus GmbH
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
71 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include <libglom/document/document.h>
#include <libglom/init.h>
#include <libglom/utils.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>

#include <iostream>


static bool validate_glom_file(const Glib::ustring& uri, const std::string& dtd_filepath)
{
  std::string filepath;

  try
  {
    filepath = Glib::filename_from_uri(uri);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": Could not convert uri to filepath: " << ex.what();
    return false;
  }

  try
  {
    xmlpp::DomParser parser;
    //parser.set_validate();
    parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
    parser.parse_file(filepath);
    if(!parser)
      return false;

    xmlpp::DtdValidator validator(dtd_filepath);
    validator.validate(parser.get_document());
  }
  catch(const xmlpp::validity_error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception caught while validating file: " << ex.what() << std::endl;
    std::cerr << G_STRFUNC << ":   uri: " << uri << std::endl;
    return false;
  }
  catch(const xmlpp::parse_error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception caught while validating file: " << ex.what() << std::endl;
    std::cerr << G_STRFUNC << ":   uri: " << uri << std::endl;
    return false;
  }

  return true;
}

int main(int argc, char* argv[])
{
  Glom::libglom_init();

  if(argc <= 2 )
  {
    std::cerr << G_STRFUNC << ": Usage: test_document_load_and_save filepath dtd_filepath" << std::endl;
    return EXIT_FAILURE;
  }

  //Get a URI (file://something) from the filepath:
  Glib::RefPtr<Gio::File> file = Gio::File::create_for_commandline_arg(argv[1]);
  const Glib::ustring uri = file->get_uri();

  const std::string dtd_filepath = argv[2];

  //Validate the original document, though the test_dtd_file_validation.sh test does this anyway:
  if(!validate_glom_file(uri, dtd_filepath))
    return EXIT_FAILURE;
  

  // Load the document:
  Glom::Document document;
  document.set_file_uri(uri);
  int failure_code = 0;
  const bool loaded = document.load(failure_code);
  //std::cout << "Document load result=" << test << std::endl;

  if(!loaded)
  {
    std::cerr << G_STRFUNC << ": Document::load() failed with failure_code=" << failure_code << std::endl;
    return EXIT_FAILURE;
  }

  // Save the document:
  document.set_allow_autosave(false);
  const Glib::ustring temp_uri = 
    Glom::Utils::get_temp_file_uri("testglom_document", ".glom");
  document.set_file_uri(temp_uri);
  document.set_modified(); //TODO: Let save() succeed without this.
  const bool saved = document.save();
  if(!saved)
  {
    std::cerr << G_STRFUNC << ": Document::save() failed." << std::endl;
    return EXIT_FAILURE;
  }

  //Validate the saved document:
  if(!validate_glom_file(temp_uri, dtd_filepath))
    return EXIT_FAILURE;

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
