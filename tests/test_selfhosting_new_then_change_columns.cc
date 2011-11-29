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
#include <libglom/connectionpool.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <libgda/gda-blob-op.h>
#include <glib.h> //For g_assert()
#include <iostream>
#include <cstdlib> //For EXIT_SUCCESS and EXIT_FAILURE
#include <cstring> //For memcmp().

static bool test(Glom::Document::HostingMode hosting_mode)
{
  Glom::Document document;
  const bool recreated = 
    test_create_and_selfhost_from_example("example_smallbusiness.glom", document, hosting_mode);
  if(!recreated)
  {
    std::cerr << "Recreation failed." << std::endl;
    return false;
  }
  
  const Glib::ustring table_name = "contacts";
  const Glib::ustring field_name_original = "date_of_birth";
  const Glom::sharedptr<const Glom::Field> field = document.get_field(table_name, field_name_original);
  if(!field)
  {
    std::cerr << "Failure: Could not get field." << std::endl;
    return false;
  }

  Glom::sharedptr<Glom::Field> field_new = Glom::glom_sharedptr_clone(field);
  if(!field_new)
  {
    std::cerr << "Failure: field_new is null." << std::endl;
    return false;
  }
  field_new->set_glom_type(Glom::Field::TYPE_TEXT);

  Glom::ConnectionPool* connection_pool = Glom::ConnectionPool::get_instance();
  if(!connection_pool)
  {
    std::cerr << "Failure: connection_pool is null." << std::endl;
    return false;
  }

  //Test that change_column() does not fail horribly:
  //TODO: Start with some data that can be converted meaningfully,
  //and check that the result is as expected:
  try
  {
    const bool test = connection_pool->change_column(table_name, field, field_new);
    if(!test)
    {
      std::cerr << "Failure: change_column() failed." << std::endl;
      return false;
    }
  }
  catch(const Glib::Error& ex)
  { 
    std::cerr << "Failure: change_column() threw an exception: " << ex.what() << std::endl;
    return false;
  }

  //Try another change:
  field_new->set_glom_type(Glom::Field::TYPE_NUMERIC);
  try
  {
    const bool test = connection_pool->change_column(table_name, field, field_new);
    if(!test)
    {
      std::cerr << "Failure: change_column() failed." << std::endl;
      return false;
    }
  }
  catch(const Glib::Error& ex)
  { 
    std::cerr << "Failure: change_column() threw an exception: " << ex.what() << std::endl;
    return false;
  }

  //Try another change:
  field_new->set_name("somenewfieldname");
  try
  {
    const bool test = connection_pool->change_column(table_name, field, field_new);
    if(!test)
    {
      std::cerr << "Failure: change_column() failed." << std::endl;
      return false;
    }
  }
  catch(const Glib::Error& ex)
  { 
    std::cerr << "Failure: change_column() threw an exception: " << ex.what() << std::endl;
    return false;
  }

  //Anything using this code would then update the Glom::Document,
  //for instance by calling Document::set_table_fields(),
  //but we are not testing that here.

  test_selfhosting_cleanup();
 
  return true; 
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
