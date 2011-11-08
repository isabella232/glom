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
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <libgda/gda-blob-op.h>
#include <glib.h> //For g_assert()
#include <iostream>
#include <cstdlib> //For EXIT_SUCCESS and EXIT_FAILURE
#include <cstring> //For memcmp().

static bool do_test(Glom::Document::HostingMode hosting_mode, const Glib::ustring& first_table_name, const Glib::ustring& renamed_table_name)
{
  Glom::Document document;
  const bool recreated = 
    test_create_and_selfhost_from_example("example_smallbusiness.glom", document, hosting_mode);
  if(!recreated)
  {
    std::cerr << "Recreation failed." << std::endl;
    return false;
  }
  
  if(!Glom::DbUtils::create_table_with_default_fields(&document, first_table_name))
  {
    std::cerr << "Failure: create_table_with_default_fields() failed." << std::endl;
    return false;
  }

  if(!Glom::DbUtils::rename_table(first_table_name, renamed_table_name))
  {
    std::cerr << "Failure: rename_table() failed." << std::endl;
    return false;
  }

  if(!Glom::DbUtils::drop_table(renamed_table_name))
  {
    std::cerr << "Failure: drop_table() failed." << std::endl;
    return false;
  }

  test_selfhosting_cleanup();
 
  return true; 
}

static bool test(Glom::Document::HostingMode hosting_mode)
{
  const Glib::ustring table_name = "sometable";
  const Glib::ustring new_table_name = "renamedtable";

  bool result = do_test(hosting_mode, table_name, new_table_name);
  if(!result)
    return false;

  result = do_test(hosting_mode, table_name + "-plusahyphen", new_table_name + "-plusahyphen");
  if(!result)
    return false;

  /** TODO: Make this work:
  result = do_test(hosting_mode, table_name + "with\"quote", new_table_name + "with\"quote");
  if(!result)
    return false;
  */

  return result;
}

int main()
{
  Glom::libglom_init();

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
