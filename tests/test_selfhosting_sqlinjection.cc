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
#include <libglom/connectionpool.h>
#include <glib.h> //For g_assert()
#include <iostream>
#include <cstdlib> //For EXIT_SUCCESS and EXIT_FAILURE

Glom::Document document;

static bool check_get_extra_rows(const Glib::ustring& quote_char)
{
  //Try to get more rows than intended:
  const Gnome::Gda::Value value("Born To Run" + quote_char + " OR " + quote_char + "x" + quote_char + "=" + quote_char + "x");
  std::shared_ptr<const Glom::Field> where_field = document.get_field("albums", "name");
  const Gnome::Gda::SqlExpr where_clause = 
    Glom::Utils::build_simple_where_expression("albums", where_field, value);
  
  Glom::Utils::type_vecLayoutFields fieldsToGet;
  std::shared_ptr<const Glom::Field> field = document.get_field("albums", "album_id");
  std::shared_ptr<Glom::LayoutItem_Field> layoutitem = std::make_shared<Glom::LayoutItem_Field>();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);
  field = document.get_field("albums", "name");
  layoutitem = std::make_shared<Glom::LayoutItem_Field>();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);

  const Glib::RefPtr<const Gnome::Gda::SqlBuilder> builder = 
    Glom::Utils::build_sql_select_with_where_clause("albums",
      fieldsToGet, where_clause);
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = 
    Glom::DbUtils::query_execute_select(builder);
  if(!test_model_expected_size(data_model, 2, 0)) //No rows should be returned because the match value was stupid, if escaped properly.
  {
    std::cerr << G_STRFUNC << ": Failure: Unexpected data model size for query, with quote_char=" << quote_char << std::endl;
    return false;
  }

  return true;
}

static bool check_drop_table(const Glib::ustring& quote_char)
{
  //Try to drop the table in a second SQL statement:
  const Gnome::Gda::Value value("True Blue" + quote_char + "; DROP TABLE songs; --");
  std::shared_ptr<const Glom::Field> where_field = 
    document.get_field("albums", "name");
  const Gnome::Gda::SqlExpr where_clause = 
    Glom::Utils::build_simple_where_expression("albums", where_field, value);
  
  Glom::Utils::type_vecLayoutFields fieldsToGet;
  std::shared_ptr<const Glom::Field> field = document.get_field("albums", "album_id");
  std::shared_ptr<Glom::LayoutItem_Field> layoutitem = std::make_shared<Glom::LayoutItem_Field>();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);
  field = document.get_field("albums", "name");
  layoutitem = std::make_shared<Glom::LayoutItem_Field>();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);

  const Glib::RefPtr<const Gnome::Gda::SqlBuilder> builder = 
    Glom::Utils::build_sql_select_with_where_clause("albums",
      fieldsToGet, where_clause);
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = 
    Glom::DbUtils::query_execute_select(builder);
  if(!test_model_expected_size(data_model, 2, 0)) //No rows should be returned because the match value was stupid, if escaped properly.
  {
    std::cerr << G_STRFUNC << ": Failure: Unexpected data model size for query, with quote_char=" << quote_char << std::endl;
    return false;
  }

  if(!test_table_exists("songs", document))
  {
    std::cerr << G_STRFUNC << ": Failure: The table may have been dropped." << std::endl;
    return false;
  }

  return true;
}

static bool check_avoid_quotes_and_drop_table_with_false_value_type()
{
  //Try to drop the table in a second SQL statement,
  //by using a text value for a field whose type should not need quoting:
  const Gnome::Gda::Value value("1;DROP TABLE songs");
  std::shared_ptr<const Glom::Field> where_field = 
    document.get_field("albums", "album_id");
  const Gnome::Gda::SqlExpr where_clause = 
    Glom::Utils::build_simple_where_expression("albums", where_field, value);
  
  Glom::Utils::type_vecLayoutFields fieldsToGet;
  std::shared_ptr<const Glom::Field> field = document.get_field("albums", "album_id");
  std::shared_ptr<Glom::LayoutItem_Field> layoutitem = std::make_shared<Glom::LayoutItem_Field>();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);
  field = document.get_field("albums", "name");
  layoutitem = std::make_shared<Glom::LayoutItem_Field>();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);

  const Glib::RefPtr<const Gnome::Gda::SqlBuilder> builder = 
    Glom::Utils::build_sql_select_with_where_clause("albums",
      fieldsToGet, where_clause);

  std::cout << "This test expects some std::cerr output about exceptions now:" << std::endl;
  
  //Glom::ConnectionPool::get_instance()->set_show_debug_output(true);

  bool result = false;
  Glib::RefPtr<Gnome::Gda::DataModel> data_model
    = Glom::DbUtils::query_execute_select(builder);
  if(!data_model)
  {
    result = true; //This should have failed because the value was of the wrong type.
  }
  else
  {
    //Allow this because it fails (correctly) with PostgreSQL but not with SQLite.
    //though even with SQLite there is quoting that prevents the SQL injection.
    result = true;
    //result = false;
    //std::cerr << G_STRFUNC << ": Failure: The SQL query should have failed." << std::endl;
  }

  //We should not get this far, but if we do, tell us more about what happened:
  if(!test_table_exists("songs", document))
  {
    std::cerr << G_STRFUNC << ": Failure: The table may have been dropped." << std::endl;
    return false;
  }

  //It should have failed earlier.
  return result;
}

static bool check_avoid_quotes_and_drop_table_with_false_field_type()
{
  //Try to drop the table in a second SQL statement,
  //by using a text value for a field whose type should not need quoting:
  const Gnome::Gda::Value value("\"Born To Run\";DROP TABLE songs");

  //Specify a field with incorrect type information:
  std::shared_ptr<Glom::Field> where_field = 
    document.get_field("albums", "name");
  where_field->set_glom_type(Glom::Field::glom_field_type::NUMERIC);
  //const GType gda_type = Glom::Field::get_gda_type_for_glom_type(Glom::TYPE_NUMERIC); 

  const Gnome::Gda::SqlExpr where_clause = 
    Glom::Utils::build_simple_where_expression("albums", where_field, value);
 
  Glom::Utils::type_vecLayoutFields fieldsToGet;
  std::shared_ptr<const Glom::Field> field = document.get_field("albums", "album_id");
  std::shared_ptr<Glom::LayoutItem_Field> layoutitem = std::make_shared<Glom::LayoutItem_Field>();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);
  field = document.get_field("albums", "name");
  layoutitem = std::make_shared<Glom::LayoutItem_Field>();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);

  const Glib::RefPtr<const Gnome::Gda::SqlBuilder> builder = 
    Glom::Utils::build_sql_select_with_where_clause("albums",
      fieldsToGet, where_clause);

  Glib::RefPtr<Gnome::Gda::DataModel> data_model
    = Glom::DbUtils::query_execute_select(builder);
  if(!test_model_expected_size(data_model, 2, 0)) //No rows should be returned because the match value was stupid, if escaped properly.
  {
    std::cerr << G_STRFUNC << ": Failure: Unexpected data model size for query." << std::endl;
    return false;
  }

  if(!test_table_exists("songs", document))
  {
    std::cerr << G_STRFUNC << ": Failure: The table may have been dropped." << std::endl;
    return false;
  }

  return true;
}

static bool test(Glom::Document::HostingMode hosting_mode)
{
  const bool recreated = 
    test_create_and_selfhost_from_example("example_music_collection.glom", document, hosting_mode);
  if(!recreated)
  {
    std::cerr << G_STRFUNC << ": Recreation failed." << std::endl;
    return false;
  }

  if(!check_get_extra_rows("\""))
  {
    std::cerr << G_STRFUNC << ": Failure: check_get_extra_rows() failed." << std::endl;
    return false;
  }
  
  if(!check_get_extra_rows("'"))
  {
    std::cerr << G_STRFUNC << ": Failure: check_get_extra_rows() failed." << std::endl;
    return false;
  }

  if(!check_drop_table("\""))
  {
    std::cerr << G_STRFUNC << ": Failure: check_drop_table() failed." << std::endl;
    return false;
  }
  
  if(!check_drop_table("'"))
  {
    std::cerr << G_STRFUNC << ": Failure: check_drop_table() failed." << std::endl;
    return false;
  }

  if(!check_avoid_quotes_and_drop_table_with_false_value_type())
  {
    std::cerr << G_STRFUNC << ": Failure: check_avoid_quotes_and_drop_table_with_false_value_type() failed." << std::endl;
    return false;
  }

  if(!check_avoid_quotes_and_drop_table_with_false_field_type())
  {
    std::cerr << G_STRFUNC << ": Failure: check_avoid_quotes_and_drop_table_with_false_field_type() failed." << std::endl;
    return false;
  }

  test_selfhosting_cleanup();

  return true;
}

int main()
{
  Glom::libglom_init();

  const auto result = test_all_hosting_modes(sigc::ptr_fun(&test));

  Glom::libglom_deinit();

  return result;
}
