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

#include <libglom/document/document.h>
#include <libglom/init.h>
#include <giomm/file.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>

#include <iostream>

template<typename T_Container>
bool contains(const T_Container& container, const Glib::ustring& name)
{
  typename T_Container::const_iterator iter =
    std::find(container.begin(), container.end(), name);
  return iter != container.end();
}

template<typename T_Container>
bool contains_named(const T_Container& container, const Glib::ustring& name)
{
  typedef typename T_Container::value_type::object_type type_item;
  typename T_Container::const_iterator iter =
    std::find_if(container.begin(), container.end(),
      Glom::predicate_FieldHasName<type_item>(name));
  return iter != container.end();
}

int main()
{
  Glom::libglom_init();

  Glom::Document document;

  //Test some simple get/set operations:
  const char* title = "Music Collection";
  document.set_database_title(title);
  g_assert(document.get_database_title() == title);

  const char* value = "someuser";
  document.set_connection_user(value);
  g_assert(document.get_connection_user() == value);

  value = "someserver";
  document.set_connection_server(value);
  g_assert(document.get_connection_server() == value);

  value = "somedb";
  document.set_connection_database(value);
  g_assert(document.get_connection_database() == value);

  const guint port = 12345;
  document.set_connection_port(port);
  g_assert(document.get_connection_port() == port);

  const bool try_other_ports = false;
  document.set_connection_try_other_ports(try_other_ports);
  g_assert(document.get_connection_try_other_ports() == try_other_ports);

  value = "somescriptcontents";
  document.set_startup_script(value);
  g_assert(document.get_startup_script() == value);

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
