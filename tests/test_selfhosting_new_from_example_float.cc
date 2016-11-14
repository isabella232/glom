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
#include <libglom/sql_utils.h>
#include <libglom/db_utils.h>
#include <libglom/connectionpool.h>
#include <libglom/data_structure/glomconversions.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <glib.h> //For g_assert()
#include <iostream>
#include <cstdlib> //For EXIT_SUCCESS and EXIT_FAILURE

static bool test(Glom::Document::HostingMode hosting_mode)
{
  auto document = std::make_shared<Glom::Document>();
  const bool recreated =
    test_create_and_selfhost_from_example("example_smallbusiness.glom", document, hosting_mode);
  if(!recreated)
  {
    std::cerr << G_STRFUNC << ": Recreation failed.\n";
    return false;
  }

  const Glib::ustring table_name = "products";
  auto primary_key_field = document->get_field_primary_key(table_name);
  if(!primary_key_field)
  {
    std::cerr << G_STRFUNC << ": Failure: primary_key_field is empty.\n";
    return false;
  }

  //Check that some data is as expected:
  const auto pk_value = Gnome::Gda::Value::create_as_double(2.0l);
  const Gnome::Gda::SqlExpr where_clause =
    Glom::SqlUtils::build_simple_where_expression(table_name, primary_key_field, pk_value);

  Glom::SqlUtils::type_vecLayoutFields fieldsToGet;
  auto field = document->get_field(table_name, "price");
  auto layoutitem = std::make_shared<Glom::LayoutItem_Field>();
  layoutitem->set_full_field_details(field);
  fieldsToGet.emplace_back(layoutitem);

  const Glib::RefPtr<const Gnome::Gda::SqlBuilder> builder =
    Glom::SqlUtils::build_sql_select_with_where_clause(table_name,
      fieldsToGet, where_clause);
  const Glib::RefPtr<const Gnome::Gda::DataModel> data_model =
    Glom::DbUtils::query_execute_select(builder);
  if(!test_model_expected_size(data_model, 1, 1))
  {
    std::cerr << G_STRFUNC << "Failure: Unexpected data model size with query: " <<
      Glom::SqlUtils::sqlbuilder_get_full_query(builder) << std::endl;
    return false;
  }

  const auto count = Glom::DbUtils::count_rows_returned_by(builder);
  if(count != 1 )
  {
    std::cerr << G_STRFUNC << "Failure: The COUNT query returned an unexpected value: " << count << std::endl;
    return false;
  }


  //Get the value from the result:
  const auto value = data_model->get_value_at(0, 0);

  if(!test_check_numeric_value_type(hosting_mode, value))
  {
    std::cerr << G_STRFUNC << ": Failure: The value has an unexpected type: " <<
      g_type_name(value.get_value_type()) << std::endl;
    return false;
  }

  if(Glom::Conversions::get_double_for_gda_value_numeric(value) != 3.5l)
  {
    std::cerr << G_STRFUNC << ": Failure: The value has an unexpected value: " << value.to_string() << " instead of 3.5\n";
    std::cerr << G_STRFUNC << ":     value as string: " << value.to_string() << std::endl;
    std::cerr << G_STRFUNC << ":     value GType: " << g_type_name(value.get_value_type()) << std::endl;
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
