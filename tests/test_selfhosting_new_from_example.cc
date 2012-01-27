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
71 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "tests/test_selfhosting_utils.h"
#include <libglom/init.h>
#include <libglom/utils.h>
#include <libglom/db_utils.h>
#include <glib.h> //For g_assert()
#include <iostream>
#include <cstdlib> //For EXIT_SUCCESS and EXIT_FAILURE

static bool test(Glom::Document::HostingMode hosting_mode)
{
  Glom::Document document;
  const bool recreated = 
    test_create_and_selfhost_from_example("example_music_collection.glom", document, hosting_mode);
  if(!recreated)
  {
    std::cerr << "Recreation failed." << std::endl;
    return false;
  }
  
  if(!test_example_musiccollection_data(&document))
  {
    std::cerr << "test_example_musiccollection_data() failed." << std::endl;
    return false;
  }

  if(!test_table_exists("songs", document))
  {
    return false;
  }

  if(!test_table_exists("publishers", document))
  {
    return false;
  }

  test_selfhosting_cleanup();
 
  return true; 
}

int main()
{
  Glom::libglom_init();

  //We run this test in several locales via 
  //test_selfhosting_new_from_example_in_locales.sh,
  //so we do this so the locale will really be used:
  setlocale(LC_ALL, "");
  
  if(!test(Glom::Document::HOSTING_MODE_POSTGRES_SELF))
  {
    std::cerr << "Failed with PostgreSQL" << std::endl;
    test_selfhosting_cleanup();
    return EXIT_FAILURE;
  }
  
  if(!test(Glom::Document::HOSTING_MODE_SQLITE))
  {
    std::cerr << "Failed with SQLite" << std::endl;
    test_selfhosting_cleanup();
    return EXIT_FAILURE;
  }

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
