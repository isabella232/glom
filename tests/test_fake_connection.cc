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
#include <libglom/utils.h>
#include <libglom/db_utils.h>
#include <giomm/file.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>

#include <iostream>

int main()
{
  Glom::libglom_init();

  // Get a URI for a test file:
  Glib::ustring uri;

  try
  {
    const std::string path =
       Glib::build_filename(GLOM_DOCDIR_EXAMPLES_NOTINSTALLED,
         "example_music_collection.glom");
    uri = Glib::filename_to_uri(path);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what();
    return EXIT_FAILURE;
  }

  // Load the document:
  Glom::Document document;
  document.set_file_uri(uri);
  int failure_code = 0;
  const auto test = document.load(failure_code);
  //std::cout << "Document load result=" << test << std::endl;

  if(!test)
  {
    std::cerr << G_STRFUNC << ": Document::load() failed with failure_code=" << failure_code << std::endl;
    return EXIT_FAILURE;
  }

  //Allow a fake connection, so sqlbuilder_get_full_query() can work:
  Glom::DbUtils::set_fake_connection();

  //Build a SQL query and get the string for it:
  const Gnome::Gda::Value value("Born To Run");
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
  const auto query = Glom::Utils::sqlbuilder_get_full_query(builder);
  g_assert(!query.empty());
  if(query.find("album_id") == Glib::ustring::npos)
  {
    std::cerr << G_STRFUNC << ": Failed: The query did not contain an expected field name." << std::endl;
    return EXIT_FAILURE;
  }

  //std::cout << "query: " << query << std::endl;

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
