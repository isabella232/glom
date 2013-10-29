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
71 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "tests/test_selfhosting_utils.h"
#include <libglom/init.h>
#include <libglom/privs.h>
#include <libglom/db_utils.h>
#include <iostream>

static bool test(Glom::Document::HostingMode hosting_mode)
{
  // Create the document:
  Glom::Document document;

  if(!(test_create_and_selfhost_new_database(document, hosting_mode, "test_db")))
  {
    std::cerr << G_STRFUNC << ": test_create_and_selfhost_new_database() failed" << std::endl;
    return false;
  }
  
  //Test some simple changes to the database:
  try
  {
    Glom::SystemPrefs prefs_in;
    prefs_in.m_name = "test name";
    prefs_in.m_org_name = "test org name";
    prefs_in.m_org_address_street = "test street 1";
    prefs_in.m_org_address_street2 = "test street 2";
    prefs_in.m_org_address_county = "test county";
    prefs_in.m_org_address_postcode = "test postcode";
    prefs_in.m_org_address_country = "test country";
    Glom::DbUtils::set_database_preferences(&document, prefs_in);

    const Glom::SystemPrefs prefs_out = 
      Glom::DbUtils::get_database_preferences(&document);
    if(prefs_out != prefs_in)
    {
      std::cerr << G_STRFUNC << ": The System Preferences read out were not the same as those written." << std::endl;
      return false;
    }
  }
  catch(const Glom::ExceptionConnection& ex)
  {
    std::cerr << G_STRFUNC << ": Exception: " << ex.what() << std::endl;
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
