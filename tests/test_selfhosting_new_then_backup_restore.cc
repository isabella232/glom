/* Glom
 *
 * Copyright (C) 2011 Openismus GmbH
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

#include "tests/test_selfhosting_utils.h"
#include <libglom/init.h>
#include <libglom/file_utils.h>
#include <glib.h> //For g_assert()
#include <iostream>
#include <cstdlib> //For EXIT_SUCCESS and EXIT_FAILURE


static void on_backup_progress()
{
  std::cout << "Restore progress\n";
}


static bool test(Glom::Document::HostingMode hosting_mode)
{
  //Create a file from an example:

  Glib::ustring backup_uri_tarball;
  {
    auto document = std::make_shared<Glom::Document>();
    const bool recreated =
      test_create_and_selfhost_from_example("example_music_collection.glom", document, hosting_mode);
    if(!recreated)
    {
      std::cerr << G_STRFUNC << ": Recreation from the example failed.\n";
      return false;
    }

    const auto backup_uri = Glom::FileUtils::get_temp_directory_uri();
    backup_uri_tarball = document->save_backup_file(
      backup_uri,
      sigc::ptr_fun(&on_backup_progress));
    if(backup_uri_tarball.empty())
    {
      std::cerr << G_STRFUNC << ": Backup failed.\n";
      return false;
    }

    test_selfhosting_cleanup();
  }

  //Create a new document from the backup:
  {
    std::string backup_data_file_path;
    const Glib::ustring backup_glom_file_contents =
      Glom::Document::extract_backup_file(
        backup_uri_tarball,
        backup_data_file_path,
        sigc::ptr_fun(&on_backup_progress));
    if(backup_glom_file_contents.empty())
    {
      std::cerr << G_STRFUNC << ": Extraction from the backup file failed.\n";
      return false;
    }

    //Create a document from the backup:
    //std::cout << "debug: recreated_uri=" << recreated_uri << std::endl;
    auto document = std::make_shared<Glom::Document>();
    const bool recreated =
      test_create_and_selfhost_from_data(backup_glom_file_contents, document, hosting_mode);
    if(!recreated)
    {
      std::cerr << G_STRFUNC << ": Recreation from the backup failed.\n";
      return false;
    }


    //Check that the new file has the expected data:
    /* TODO: Find out why this test fails, though it seems to work fine in the UI:
    if(!test_example_musiccollection_data(document))
    {
      std::cerr << G_STRFUNC << ": test_example_musiccollection_data() failed.\n";
      return false;
    }
    */

    test_selfhosting_cleanup();
  }

  return true;
}

int main()
{
  Glom::libglom_init();

  if(!test(Glom::Document::HostingMode::POSTGRES_SELF))
  {
    std::cerr << G_STRFUNC << ": Failed with PostgreSQL\n";
    test_selfhosting_cleanup();
    return EXIT_FAILURE;
  }

  /* TODO: Make this work with sqlite too:
  if(!test(Glom::Document::HostingMode::SQLITE))
  {
    std::cerr << G_STRFUNC << ": Failed with SQLite\n";
    test_selfhosting_cleanup();
    return EXIT_FAILURE;
  }
  */

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
