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

Glom::Document document;

static bool check_get_extra_rows()
{
  //Try to get more rows than intended:
  const Gnome::Gda::Value value("Born To Run\" OR \"x\"=\"x");
  Glom::sharedptr<const Glom::Field> where_field = document.get_field("albums", "name");
  const Gnome::Gda::SqlExpr where_clause = 
    Glom::Utils::build_simple_where_expression("albums", where_field, value);
  
  Glom::Utils::type_vecLayoutFields fieldsToGet;
  Glom::sharedptr<const Glom::Field> field = document.get_field("albums", "album_id");
  Glom::sharedptr<Glom::LayoutItem_Field> layoutitem = Glom::sharedptr<Glom::LayoutItem_Field>::create();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);
  field = document.get_field("albums", "name");
  layoutitem = Glom::sharedptr<Glom::LayoutItem_Field>::create();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);

  const Glib::RefPtr<const Gnome::Gda::SqlBuilder> builder = 
    Glom::Utils::build_sql_select_with_where_clause("albums",
      fieldsToGet, where_clause);
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = 
    Glom::DbUtils::query_execute_select(builder);
  if(!test_model_expected_size(data_model, 2, 0)) //No rows should be returned because the match value was stupid, if escaped properly.
  {
    std::cerr << "Failure: Unexpected data model size for query." << std::endl;
    return false;
  }

  return true;
}

static bool check_drop_table()
{
  //Try to get more rows than intended:
  const Gnome::Gda::Value value("True Blue\"; DROP TABLE songs; --");
  Glom::sharedptr<const Glom::Field> where_field = 
    document.get_field("albums", "name");
  const Gnome::Gda::SqlExpr where_clause = 
    Glom::Utils::build_simple_where_expression("albums", where_field, value);
  
  Glom::Utils::type_vecLayoutFields fieldsToGet;
  Glom::sharedptr<const Glom::Field> field = document.get_field("albums", "album_id");
  Glom::sharedptr<Glom::LayoutItem_Field> layoutitem = Glom::sharedptr<Glom::LayoutItem_Field>::create();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);
  field = document.get_field("albums", "name");
  layoutitem = Glom::sharedptr<Glom::LayoutItem_Field>::create();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);

  const Glib::RefPtr<const Gnome::Gda::SqlBuilder> builder = 
    Glom::Utils::build_sql_select_with_where_clause("albums",
      fieldsToGet, where_clause);
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = 
    Glom::DbUtils::query_execute_select(builder);
  if(!test_model_expected_size(data_model, 2, 0)) //No rows should be returned because the match value was stupid, if escaped properly.
  {
    std::cerr << "Failure: Unexpected data model size for query." << std::endl;
    return false;
  }

  if(!test_table_exists("songs", document))
  {
    std::cerr << "Failure: The table may have been dropped." << std::endl;
    return false;
  }

  return true;
}

int main()
{
  Glom::libglom_init();

  const bool recreated = 
    test_create_and_selfhost("example_music_collection.glom", document);
  g_assert(recreated);

  if(!check_get_extra_rows())
  {
    std::cerr << "Failure: check_get_extra_rows() failed." << std::endl;
    test_selfhosting_cleanup();
    return EXIT_FAILURE;
  }

  if(!check_drop_table())
  {
    std::cerr << "Failure: check_drop_table() failed." << std::endl;
    test_selfhosting_cleanup();
    return EXIT_FAILURE;
  }


  test_selfhosting_cleanup();

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
