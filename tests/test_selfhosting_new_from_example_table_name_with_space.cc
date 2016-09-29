/* Glom
 *
 * Copyright (C) 2010 Openismus GmbH
 * Copyright (C) 2016 Murray Cumming
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
#include <libglom/utils.h>
#include <libglom/db_utils.h>
#include <glib.h> //For g_assert()
#include <iostream>
#include <cstdlib> //For EXIT_SUCCESS and EXIT_FAILURE

static bool test(Glom::Document::HostingMode hosting_mode)
{
  auto document = std::make_shared<Glom::Document>();

  // TODO: For now, we expect this to fail,
  // but we hope that the libgda bug will be fixed one day:
  // See https://bugzilla.gnome.org/show_bug.cgi?id=763534
  /* const bool recreated = */
  test_create_and_selfhost_from_test_example("test_example_music_collection_table_name_with_space.glom", document, hosting_mode);
  return true;

  /*
  if(!recreated)
  {
    std::cerr << G_STRFUNC << ": Recreation failed.\n";
    return false;
  }

  if(!test_example_musiccollection_data(document))
  {
    std::cerr << G_STRFUNC << ": test_example_musiccollection_data() failed.\n";
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
  */

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

  const auto result = test_all_hosting_modes(sigc::ptr_fun(&test));

  Glom::libglom_deinit();

  return result;
}
