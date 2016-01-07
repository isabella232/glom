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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include <libglom/document/document.h>
#include <libglom/init.h>
#include <libglom/utils.h>
#include <giomm/file.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>
#include <iostream>

Glib::ustring file_uri;

void cleanup()
{
  try
  {
    auto file = Gio::File::create_for_uri(file_uri);

    //Ignore the return value because it will throw an exception instead.
    file->remove(); //This should be OK because it is a file, not a directory.
  }
  catch(const Gio::Error& ex)
  {
    //It's OK if it's not found - we just want to make sure it doesn't exist.
    if(ex.code() == Gio::Error::NOT_FOUND)
      return;

    std::cerr << G_STRFUNC << ": Exception from Gio::File::remove(): " << ex.what() << std::endl
      << "  file_uri= " << file_uri << std::endl;
    exit(EXIT_FAILURE);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ":Exception from Gio::File::remove(): " << ex.what() << std::endl
      << "  file_uri= " << file_uri << std::endl;
    exit(EXIT_FAILURE);
  }
}

int main()
{
  Glom::libglom_init();

  file_uri = Glom::Utils::get_temp_file_uri("testglom_document_autosave", ".glom");
  
  //Make sure that the file does not exist yet:
  cleanup();

  const Glib::ustring test_title = "test_title";

  //Test manual saving:
  {
    Glom::Document document;
    document.set_allow_autosave(false);
    document.set_file_uri(file_uri);
    document.set_hosting_mode(Glom::Document::HostingMode::POSTGRES_CENTRAL);
    document.set_database_title_original(test_title);
    const auto saved = document.save();
    g_assert(saved);
  }
  {
    Glom::Document document;
    document.set_file_uri(file_uri);
    int failure_code = 0;
    const auto test = document.load(failure_code);
    g_assert(test);

    g_assert( document.get_database_title_original() == test_title );
  }

  cleanup();

  //Test autosaving:
  {
    Glom::Document document;
    document.set_file_uri(file_uri);
    document.set_hosting_mode(Glom::Document::HostingMode::POSTGRES_CENTRAL);
    document.set_allow_autosave();
    document.set_database_title_original(test_title);
    g_assert( !document.get_modified() );
  }
  {
    Glom::Document document;
    document.set_file_uri(file_uri);
    int failure_code = 0;
    const auto test = document.load(failure_code);
    g_assert(test);

    g_assert( document.get_database_title_original() == test_title );
  }

  cleanup();

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
