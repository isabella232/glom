/* Glom
 *
 * Copyright (C) 2015 Openismus GmbH
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

#include <glom/libglom/init.h>
#include <glom/libglom/connectionpool_backends/postgres_self.h>
#include <iostream>

static bool test_postgres_utils_version()
{
  const auto str = Glom::ConnectionPoolBackends::PostgresSelfHosted::get_postgresql_utils_version_from_string("pg_ctl (PostgreSQL) 9.4.4");
  //std::cout << "debug: str: " << str << std::endl;
  return str == "9.4.4";
}

static bool test_postgres_utils_version_as_number()
{
  const auto number = Glom::ConnectionPoolBackends::PostgresSelfHosted::get_postgresql_utils_version_as_number_from_string("pg_ctl (PostgreSQL) 9.4.4");
  //std::cout << "debug: number: " << number << std::endl;
  return number == 9.4f;
}

int main()
{
  Glom::libglom_init();

  if(! test_postgres_utils_version())
    return EXIT_FAILURE;

  if(!test_postgres_utils_version_as_number())
    return EXIT_FAILURE;

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
