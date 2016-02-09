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
  const bool recreated = 
    test_create_and_selfhost_from_example("example_music_collection.glom", document, hosting_mode);
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

  //Test the system preferences for the database:
  //TODO: We should store this only in the document anyway,
  //and make it translatable:
  /* TODO: This is not stored in the examples. Should it be?
  const Glom::SystemPrefs prefs = 
    Glom::DbUtils::get_database_preferences(document);
  g_return_val_if_fail(prefs.m_name == "Music Collection", false);
  g_return_val_if_fail(prefs.m_org_name == "SomeOrganization Incorporated", false);
  g_return_val_if_fail(prefs.m_org_address_street == "Some House", false);
  g_return_val_if_fail(prefs.m_org_address_street2 == "123 Some Street", false);
  g_return_val_if_fail(prefs.m_org_address_town == "Some Town", false);
  g_return_val_if_fail(prefs.m_org_address_county == "Some State", false);
  g_return_val_if_fail(prefs.m_org_address_postcode == "12345", false);
  g_return_val_if_fail(prefs.m_org_address_country == "USA", false);
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
