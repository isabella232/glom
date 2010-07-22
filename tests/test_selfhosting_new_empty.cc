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
#include <libglom/connectionpool.h>
#include <libglom/connectionpool_backends/postgres_self.h>
#include <libglom/init.h>
#include <glom/glom_privs.h>
#include <giomm/file.h>

static void on_initialize_progress()
{
  std::cout << "Database initialization progress" << std::endl;
}

static void on_startup_progress()
{
  std::cout << "Database startup progress" << std::endl;
}

static void on_cleanup_progress()
{
  std::cout << "Database cleanup progress" << std::endl;
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

  g_assert(document.get_is_example_file());;

  Glom::ConnectionPool* connection_pool = Glom::ConnectionPool::get_instance();

  //Save a copy, specifying the path to file in a directory:
  //For instance, /tmp/testfileglom/testfile.glom");
  const std::string temp_filename = "testglom";
  const std::string temp_filepath_dir = Glib::build_filename(Glib::get_tmp_dir(),
    temp_filename);
  const std::string temp_filepath = Glib::build_filename(temp_filepath_dir, temp_filename);

  //Make sure that the file does not exist yet:
  {
    const Glib::ustring uri = Glib::filename_to_uri(temp_filepath_dir);
    Glom::Utils::delete_directory(uri);
  }

  //Save the example as a real file:
  const Glib::ustring file_uri = Glib::filename_to_uri(temp_filepath);
  document.set_file_uri(file_uri);

  document.set_hosting_mode(Glom::Document::HOSTING_MODE_POSTGRES_SELF);
  document.set_is_example_file(false);
  document.set_network_shared(false);
  const bool saved = document.save();
  g_assert(saved);

  //Specify the backend and backend-specific details to be used by the connectionpool.
  connection_pool->setup_from_document(&document);

  //We must specify a default username and password:
  Glib::ustring password;
  const Glib::ustring user = Glom::Privs::get_default_developer_user_name(password);
  connection_pool->set_user(user);
  connection_pool->set_password(password);

  //Create the self-hosting files:
  const Glom::ConnectionPool::InitErrors initialized_errors =
    connection_pool->initialize( sigc::ptr_fun(&on_initialize_progress) );
  g_assert(initialized_errors == Glom::ConnectionPool::Backend::INITERROR_NONE);

  //Start self-hosting:
  //TODO: Let this happen automatically on first connection?
  const bool started = connection_pool->startup( sigc::ptr_fun(&on_startup_progress) );
  g_assert(started == Glom::ConnectionPool::Backend::STARTUPERROR_NONE);

  const bool stopped = connection_pool->cleanup( sigc::ptr_fun(&on_cleanup_progress) );
  g_assert(stopped);

  //Make sure the directory is removed at the end,
  {
    const Glib::ustring uri = Glib::filename_to_uri(temp_filepath_dir);
    Glom::Utils::delete_directory(uri);
  }

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
