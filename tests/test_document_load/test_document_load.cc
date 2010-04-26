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

  // Get a URI for a test file:
  Glib::ustring uri;

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    const std::string path = 
       Glib::build_filename(GLOM_DOCDIR_EXAMPLES_NOTINSTALLED, 
         "example_music_collection.glom");
    uri = Glib::filename_to_uri(path);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << "Exception from Glib::filename_to_uri(): " << ex.what();
    return EXIT_FAILURE;
  }
  #else
  std::auto_ptr<Glib::Error> ex;
  uri = Glib::filename_to_uri("/usr/share/glom/doc/examples/example_music_collection.glom", ex);
  #endif

  //std::cout << "URI=" << uri << std::endl;


  // Load the document:
  Glom::Document document;
  document.set_file_uri(uri);
  int failure_code = 0;
  const bool test = document.load(failure_code);
  //std::cout << "Document load result=" << test << std::endl;

  if(!test)
  {
    std::cerr << "Document::load() failed with failure_code=" << failure_code << std::endl;
    return EXIT_FAILURE;
  }

  //Test some known details:
  g_assert(document.get_is_example_file());
  g_assert(document.get_database_title() == "Music Collection");

  const std::vector<Glib::ustring> table_names = document.get_table_names();
  g_assert(contains(table_names, "albums"));
  g_assert(contains(table_names, "artists"));
  g_assert(contains(table_names, "publishers"));
  g_assert(contains(table_names, "songs"));
  g_assert(!contains(table_names, "nosuchtable"));

  //Test known details of one table:
  const Glom::Document::type_vec_fields fields = document.get_table_fields("albums");
  g_assert(contains_named(fields, "album_id"));
  g_assert(contains_named(fields, "name"));
  g_assert(contains_named(fields, "artist_id"));
  g_assert(contains_named(fields, "year"));
  g_assert(!contains_named(fields, "nosuchfield"));

  const Glom::Document::type_vec_relationships relationships = document.get_relationships("albums");
  g_assert(contains_named(relationships, "artist"));
  g_assert(contains_named(relationships, "publisher"));
  g_assert(contains_named(relationships, "songs"));

  //Check some fields:
  Glom::sharedptr<const Glom::Field> field = document.get_field("albums", "album_id");
  g_assert(field);
  g_assert(field->get_glom_type() == Glom::Field::TYPE_NUMERIC);
  g_assert(field->get_auto_increment());
  field = document.get_field("albums", "year");
  g_assert(field);
  g_assert(field->get_glom_type() == Glom::Field::TYPE_NUMERIC);
  g_assert(!field->get_auto_increment());
  g_assert(!field->get_unique_key());
    
  //Check a relationship:
  const Glom::sharedptr<const Glom::Relationship> relationship = document.get_relationship("albums", "artist");
  g_assert(relationship);
  g_assert(relationship->get_from_field() == "artist_id");
  g_assert(relationship->get_to_table() == "artists");
  g_assert(relationship->get_to_field() == "artist_id");
  
  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
