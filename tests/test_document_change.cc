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

#include <libglom/document/document.h>
#include <libglom/init.h>
#include <giomm/file.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>

#include <iostream>

int main()
{
  Glom::libglom_init();

  Glom::Document document;
  document.set_allow_autosave(false); //Avoid warnings about it having no URI.

  //Test some simple get/set operations:
  const char* title = "Music Collection";
  document.set_database_title_original(title);
  g_assert(document.get_database_title_original() == title);

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


  const Glib::ustring table_name = "sometable";
  auto table_info = std::make_shared<Glom::TableInfo>();
  table_info->set_name(table_name);
  
  const Glib::ustring table_title = "sometabletitle";
  table_info->set_title_original(table_title);
  g_assert(table_info->get_title_original() == table_title);  
  document.add_table(table_info);

  const float x = 20.0f;
  const float y = 30.0f;
  document.set_table_overview_position(table_name, x, y);
  float x_out = 0;
  float y_out = 0;
  document.get_table_overview_position(table_name, x_out, y_out);
  g_assert(x == x_out);
  g_assert(y == y_out);

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
