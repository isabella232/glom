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
#include <libglom/privs.h>
#include <libglom/utils.h>
#include <libglom/db_utils.h>
#include <giomm/file.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>
#include <iostream>

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

static void on_db_creation_progress()
{
  std::cout << "Database creation progress" << std::endl;
}

static Glom::ConnectionPool* connection_pool = 0;
static std::string temp_filepath_dir;

static void cleanup()
{
  //Stop:
  const bool stopped = connection_pool->cleanup( sigc::ptr_fun(&on_cleanup_progress) );
  g_assert(stopped);

  //Make sure the directory is removed at the end,
  const Glib::ustring uri = Glib::filename_to_uri(temp_filepath_dir);
  Glom::Utils::delete_directory(uri);
}

static bool test(Glom::Document::HostingMode hosting_mode)
{
  connection_pool = 0;
  temp_filepath_dir.clear();

  // Create the document:
  Glom::Document document;

  //Save a copy, specifying the path to file in a directory:
  //For instance, /tmp/testfileglom/testfile.glom");
  const std::string temp_filename = "testglom";
  temp_filepath_dir = 
    Glom::Utils::get_temp_directory_path(temp_filename);
  const std::string temp_filepath = Glib::build_filename(temp_filepath_dir, temp_filename);

  //Make sure that the file does not exist yet:
  {
    const Glib::ustring uri = Glib::filename_to_uri(temp_filepath_dir);
    Glom::Utils::delete_directory(uri);
  }

  //Save the file. TODO: Do we need to do this for the test?
  const Glib::ustring file_uri = Glib::filename_to_uri(temp_filepath);
  document.set_allow_autosave(false); //To simplify things and to not depend implicitly on autosave.
  document.set_file_uri(file_uri);

  document.set_hosting_mode(hosting_mode);
  document.set_is_example_file(false);
  document.set_network_shared(false);
  const bool saved = document.save();
  g_assert(saved);

  //Specify the backend and backend-specific details to be used by the connectionpool.
  connection_pool = Glom::ConnectionPool::get_instance();
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
  const Glom::ConnectionPool::StartupErrors started = connection_pool->startup( sigc::ptr_fun(&on_startup_progress) );
  if(started != Glom::ConnectionPool::Backend::STARTUPERROR_NONE)
  {
    std::cerr << "connection_pool->startup(): result=" << started << std::endl;
  }
  g_assert(started == Glom::ConnectionPool::Backend::STARTUPERROR_NONE);

  //Test this function a little:
  const Glib::ustring db_name = Glom::DbUtils::get_unused_database_name("test_db");
  if(db_name.empty())
  {
    std::cerr << "DbUtils::get_unused_database_name) failed." << std::endl;
    cleanup();
    return false;
  }

  //Create a database:
  const bool created = Glom::DbUtils::create_database(&document, db_name, 
    "test title", sigc::ptr_fun(&on_db_creation_progress));
  if(!created)
  {
    std::cerr << "DbUtils::create_database() failed." << std::endl;
    cleanup();
    return false;
  }

  
  //Test some simple changes to the database:
  try
  {
    Glom::SystemPrefs prefs;
    prefs.m_name = "test name";
    prefs.m_org_name = "test org name";
    Glom::DbUtils::set_database_preferences(&document, prefs);
  }
  catch(const Glom::ExceptionConnection& ex)
  {
    std::cerr << "Exception: " << ex.what() << std::endl;
    cleanup();
    return false;
  }
  
  cleanup();
  
  return true;
}

int main()
{
  Glom::libglom_init();

  if(!test(Glom::Document::HOSTING_MODE_POSTGRES_SELF))
  {
    std::cerr << "Failed with PostgreSQL" << std::endl;
    return EXIT_FAILURE;
  }
  
  if(!test(Glom::Document::HOSTING_MODE_SQLITE))
  {
    std::cerr << "Failed with SQLite" << std::endl;
    return EXIT_FAILURE;
  }

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
