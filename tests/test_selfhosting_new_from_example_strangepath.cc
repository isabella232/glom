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

#include "tests/test_selfhosting_utils.h"
#include <libglom/init.h>
#include <libglom/utils.h>
#include <libglom/db_utils.h>
#include <glib.h> //For g_assert()
#include <iostream>
#include <cstdlib> //For EXIT_SUCCESS and EXIT_FAILURE

static bool test(Glom::Document::HostingMode hosting_mode)
{
  //TODO: MySQL: See if we can get this to work.
  if(hosting_mode == Glom::Document::HOSTING_MODE_MYSQL_SELF)
  {
    test_selfhosting_cleanup(false /* do not delete the file. */);
    return true;
  }

  Glom::Document document;
  //Note: We avoid using a path that is longer than 107 characters to avoid a PostgreSQL error.
  //(107 == sizeof(struct sockaddr_un.sun_path) at least here). murrayc.
  //See http://lists.debian.org/debian-wb-team/2013/05/msg00015.html
  //TODO: Fail with long paths meaningfully in the libglom API too, and check for that? 
  const bool recreated = 
    test_create_and_selfhost_from_example("example_music_collection.glom", document, 
      hosting_mode, "path w space and \" and ' and $ and / and \\ ");
      //hosting_mode, "path with space and \" quote and single ' quote and $ dollar sign and / forward slash and \\ backwards slash ");
  if(!recreated)
  {
    std::cerr << G_STRFUNC << ": Recreation failed." << std::endl;
    return false;
  }
  
  if(!test_example_musiccollection_data(&document))
  {
    std::cerr << G_STRFUNC << ": test_example_musiccollection_data() failed." << std::endl;
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

  const int result = test_all_hosting_modes(sigc::ptr_fun(&test));

  Glom::libglom_deinit();

  return result;
}
