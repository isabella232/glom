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
#include <glom/libglom/init.h>
#include <glom/libglom/connectionpool.h>
#include <glom/python_embed/glom_python.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/utils.h>
#include <libglom/db_utils.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>
#include <boost/python.hpp>
#include <iostream>

static bool get_field_result(const Glom::Document& document,
  const Glib::RefPtr<Gnome::Gda::Connection>& gda_connection,
  const Glib::ustring& table_name,
  const Glom::sharedptr<const Glom::Field>& primary_key_field,
  const Gnome::Gda::Value& primary_key_value,
  const Glom::type_map_fields field_values,
  const Glib::ustring& field_name, const Gnome::Gda::Value& expected_value)
{
  const Glib::ustring calculation = "return record[\"" + field_name + "\"]";

  const GType expected_value_gtype = expected_value.get_value_type();

  //Execute a python function:
  Gnome::Gda::Value value;
  Glib::ustring error_message;
  try
  {
    value = Glom::glom_evaluate_python_function_implementation(
      Glom::Field::get_glom_type_for_gda_type(expected_value_gtype),
      calculation, field_values,
      &document, table_name,
      primary_key_field, primary_key_value,
      gda_connection,
      error_message);
  }
  catch(const std::exception& ex)
  {
    std::cerr << G_STRFUNC << ": Exception: " << ex.what() << std::endl;
    return false;
  }
  catch(const boost::python::error_already_set& ex)
  {
    std::cerr << G_STRFUNC << ": Exception: boost::python::error_already_set" << std::endl;
    return false;
  }

  //std::cout << "type=" << g_type_name(value.get_value_type()) << std::endl;

  //Check that there was no python error:
  if(!error_message.empty())
  {
    std::cerr << G_STRFUNC << ": Python error: " << error_message << std::endl;
    return false;
  }

  //Check that the return value is of the expected type:
  if(value.get_value_type() != expected_value_gtype)
  {
    std::cerr << G_STRFUNC << ": Unexpected value type: " << g_type_name( value.get_value_type()) << " instead of " << g_type_name(expected_value_gtype) << std::endl;
    return false;
  }

  //Check that the return value is of the expected value:
  if(value != expected_value)
  {
    std::cerr << G_STRFUNC << ": Unexpected value: " << value.to_string() << " instead of " << expected_value.to_string() << std::endl;
    return false;
  }

  return true;
}

static bool test(Glom::Document::HostingMode hosting_mode)
{
  //Connect to a Glom database
  Glom::Document document;
  const bool recreated = 
    test_create_and_selfhost_from_example("example_smallbusiness.glom", document, hosting_mode);
  if(!recreated)
  {
    std::cerr << G_STRFUNC << ": Recreation failed." << std::endl;
    return false;
  }

  Glom::ConnectionPool* connection_pool = Glom::ConnectionPool::get_instance();
  Glom::sharedptr<Glom::SharedConnection> connection = connection_pool->connect();
  g_assert(connection);

  const Glib::RefPtr<Gnome::Gda::Connection> gda_connection = connection->get_gda_connection();
  g_assert(connection->get_gda_connection());


  //Some python code just to exercise our PyGlomRecord API:
  const Glib::ustring table_name = "products";
  const Glom::sharedptr<const Glom::Field> primary_key_field =
    document.get_field_primary_key(table_name);
  if(!primary_key_field)
  {
    std::cerr << G_STRFUNC << ": Failure: primary_key_field is empty." << std::endl;
    return false;
  }

  const Gnome::Gda::Value primary_key_value(2);

  const Glom::type_map_fields field_values =
    Glom::DbUtils::get_record_field_values(&document,
      table_name,
      primary_key_field,
      primary_key_value);
  if(field_values.empty())
  {
    std::cerr << G_STRFUNC << ": field_values was empty: " << std::endl;
    return false;
  }

  // Text:
  if(!get_field_result(document, gda_connection,
      table_name, primary_key_field, primary_key_value, field_values,
      "name", Gnome::Gda::Value("Widget")))
  {
    return false;
  }

  // Numeric:
  if(!get_field_result(document, gda_connection,
      table_name, primary_key_field, primary_key_value, field_values,
      "price", Glom::Conversions::parse_value((double)3.50l)))
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
