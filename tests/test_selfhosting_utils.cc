/* Glom
 *
 * Copyright (C) 2011 Murray Cumming
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

#include <libglom/document/document.h>
#include <libglom/connectionpool.h>
#include <libglom/connectionpool_backends/postgres_self.h>
#include <libglom/init.h>
#include <libglom/privs.h>
#include <libglom/db_utils.h>
#include <libglom/utils.h>
#include <giomm/file.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>
#include <iostream>
#include "config.h"

static void on_initialize_progress()
{
  //std::cout << "Database initialization progress" << std::endl;
}

static void on_startup_progress()
{
  //std::cout << "Database startup progress" << std::endl;
}

static void on_recreate_progress()
{
  //std::cout << "Database re-creation progress" << std::endl;
}

static void on_cleanup_progress()
{
  //std::cout << "Database cleanup progress" << std::endl;
}

static void on_db_creation_progress()
{
  //std::cout << "Database creation progress" << std::endl;
}

static std::string temp_filepath_dir; //Remembered so we can delete it later.
static Glib::ustring temp_file_uri; //Rememered so we can return it sometimes.

static bool check_directory_exists()
{
  if(temp_filepath_dir.empty())
  {
    std::cerr << G_STRFUNC << ": temp_filepath_dir is empty." << std::endl;
    return false;
  }
  
  Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(temp_filepath_dir);
  return file->query_exists();
}

/** Delete a directory, if it exists, and its contents.
 * Unlike g_file_delete(), this does not fail if the directory is not empty.
 */
static bool delete_directory(const Glib::RefPtr<Gio::File>& directory)
{
  if(!(directory->query_exists()))
    return true;

  //(Recursively) Delete any child files and directories,
  //so we can delete this directory.
  Glib::RefPtr<Gio::FileEnumerator> enumerator = directory->enumerate_children();

  Glib::RefPtr<Gio::FileInfo> info = enumerator->next_file();
  while(info)
  {
    Glib::RefPtr<Gio::File> child = directory->get_child(info->get_name());
    bool removed_child = false;
    if(child->query_file_type() == Gio::FILE_TYPE_DIRECTORY)
      removed_child = delete_directory(child);
    else
      removed_child = child->remove();

    if(!removed_child)
       return false;

    info = enumerator->next_file();
  }

  //Delete the actual directory:
  if(!directory->remove())
    return false;

  return true;
}

/** Delete a directory, if it exists, and its contents.
 * Unlike g_file_delete(), this does not fail if the directory is not empty.
 */
static bool delete_directory(const std::string& uri)
{
  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
  return delete_directory(file);
}

void test_selfhosting_cleanup(bool delete_file)
{
  Glom::ConnectionPool* connection_pool = Glom::ConnectionPool::get_instance();

  const bool stopped = connection_pool->cleanup( sigc::ptr_fun(&on_cleanup_progress) );
  g_assert(stopped);

  if(!delete_file)
    return;

  //Make sure the directory is removed at the end:
  if(!temp_filepath_dir.empty())
  {
    Glib::ustring uri;
    
    try
    {
      uri = Glib::filename_to_uri(temp_filepath_dir);
    }
    catch(const Glib::ConvertError& ex)
    {
      std::cerr << G_STRFUNC << ": Glib::filename_to_uri() failed: " << ex.what() << std::endl;
    }

    if(!uri.empty())
      delete_directory(uri);
  }
  
  temp_filepath_dir.clear();
  temp_file_uri.clear(); //Forget this too.
}

bool test_selfhost(Glom::Document& document, const Glib::ustring& user, const Glib::ustring& password)
{
  //TODO: Let this happen automatically on first connection?
  Glom::ConnectionPool* connection_pool = Glom::ConnectionPool::get_instance();

  connection_pool->setup_from_document(&document);

  connection_pool->set_user(user);
  connection_pool->set_password(password);

  const Glom::ConnectionPool::StartupErrors started = connection_pool->startup( sigc::ptr_fun(&on_startup_progress) );
  if(started != Glom::ConnectionPool::Backend::STARTUPERROR_NONE)
  {
    std::cerr << G_STRFUNC << ": connection_pool->startup(): result=" << started << std::endl;
    test_selfhosting_cleanup();
    return false;
  }
  g_assert(started == Glom::ConnectionPool::Backend::STARTUPERROR_NONE);

  return true;
}

bool test_create_and_selfhost_new_empty(Glom::Document& document, Glom::Document::HostingMode hosting_mode, const std::string& subdirectory_path)
{
  if( (hosting_mode != Glom::Document::HOSTING_MODE_POSTGRES_SELF) &&
    (hosting_mode != Glom::Document::HOSTING_MODE_MYSQL_SELF) &&
    (hosting_mode != Glom::Document::HOSTING_MODE_SQLITE) )
  {
    std::cerr << G_STRFUNC << ": This test function does not support the specified hosting_mode: " << hosting_mode << std::endl;
    return false;
  }

  //Save a copy, specifying the path to file in a directory:
  //For instance, /tmp/testglom/testglom.glom");
  const std::string temp_filename = "testglom";
  temp_filepath_dir = 
    Glom::Utils::get_temp_directory_path(temp_filename);
  if(!subdirectory_path.empty())
    temp_filepath_dir = Glib::build_filename(temp_filepath_dir, subdirectory_path);
  const std::string temp_filepath = Glib::build_filename(temp_filepath_dir, temp_filename);

  //Make sure that the file does not exist yet:
  {
    const Glib::ustring uri = Glib::filename_to_uri(temp_filepath_dir);
    delete_directory(uri);
  }

   //Save the example as a real file:
  temp_file_uri = Glib::filename_to_uri(temp_filepath);
  document.set_allow_autosave(false); //To simplify things and to not depend implicitly on autosave.
  document.set_file_uri(temp_file_uri);

  document.set_hosting_mode(hosting_mode);
  document.set_is_example_file(false);
  document.set_network_shared(false);
  const bool saved = document.save();
  g_assert(saved);

  //Specify the backend and backend-specific details to be used by the connectionpool.
  Glom::ConnectionPool* connection_pool = Glom::ConnectionPool::get_instance();
  connection_pool->setup_from_document(&document);

  //We must specify a default username and password:
  Glib::ustring password;
  const Glib::ustring user = Glom::Privs::get_default_developer_user_name(password, hosting_mode);
  connection_pool->set_user(user);
  connection_pool->set_password(password);

  //Create the self-hosting files:
  const Glom::ConnectionPool::InitErrors initialized_errors =
    connection_pool->initialize( sigc::ptr_fun(&on_initialize_progress) );
  g_assert(initialized_errors == Glom::ConnectionPool::Backend::INITERROR_NONE);
  
  if(!check_directory_exists())
  {
    std::cerr << G_STRFUNC << ": Failure: The data directory does not exist after calling initialize()." << std::endl;
    return false;
  }

  //Start self-hosting:
  return test_selfhost(document, user, password);
}

Glib::ustring test_get_temp_file_uri()
{
  return temp_file_uri;
}

bool test_create_and_selfhost_new_database(Glom::Document& document, Glom::Document::HostingMode hosting_mode, const Glib::ustring& database_name, const std::string& subdirectory_path)
{
  if(!test_create_and_selfhost_new_empty(document, hosting_mode, subdirectory_path))
  {
    std::cerr << G_STRFUNC << ": test_create_and_selfhost_new_empty() failed." << std::endl;
    return false;
  } 

  const Glib::ustring db_name = Glom::DbUtils::get_unused_database_name(database_name);
  if(db_name.empty())
  {
    std::cerr << G_STRFUNC << ": DbUtils::get_unused_database_name) failed." << std::endl;
    return false;
  }

  //Create a database:
  const bool created = Glom::DbUtils::create_database(&document, db_name,
    "test title", sigc::ptr_fun(&on_db_creation_progress));
  if(!created)
  {
    std::cerr << G_STRFUNC << ": DbUtils::create_database() failed." << std::endl;
    return false;
  }
  
  return true;
}

static bool test_create_and_selfhost_from_example_full_path(const std::string& example_path, Glom::Document& document, Glom::Document::HostingMode hosting_mode, const std::string& subdirectory_path = std::string())
{
  Glib::ustring uri;
  try
  {
    uri = Glib::filename_to_uri(example_path);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what();
    return false;
  }

  return test_create_and_selfhost_from_uri(uri, document, hosting_mode, subdirectory_path);
}

bool test_create_and_selfhost_from_example(const std::string& example_filename, Glom::Document& document, Glom::Document::HostingMode hosting_mode, const std::string& subdirectory_path)
{
  Glib::ustring uri;
  
  // Get a URI for the example file:
  std::string path;
  try
  {
    path =
       Glib::build_filename(GLOM_DOCDIR_EXAMPLES_NOTINSTALLED,
         example_filename);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what();
    return false;
  }

  return test_create_and_selfhost_from_example_full_path(path, document, hosting_mode, subdirectory_path);
}

bool test_create_and_selfhost_from_test_example(const std::string& example_filename, Glom::Document& document, Glom::Document::HostingMode hosting_mode)
{
  Glib::ustring uri;
  
  // Get a URI for the example file:
  std::string path;
  try
  {
    path =
       Glib::build_filename(GLOM_TEST_EXAMPLES_NOTINSTALLED,
         example_filename);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what();
    return false;
  }

  return test_create_and_selfhost_from_example_full_path(path, document, hosting_mode);
}

static bool after_load(Glom::Document& document, Glom::Document::HostingMode hosting_mode, const std::string& subdirectory_path)
{
  if(!document.get_is_example_file() && !document.get_is_backup_file())
  {
    std::cerr << G_STRFUNC << ": The document is not an example or a backup." << std::endl;
    return false;
  }

  if(!test_create_and_selfhost_new_empty(document, hosting_mode, subdirectory_path))
  {
    std::cerr << G_STRFUNC << ": test_create_and_selfhost_new_empty() failed." << std::endl;
    return false;
  }

  const bool recreated = Glom::DbUtils::recreate_database_from_document(&document, sigc::ptr_fun(&on_recreate_progress) );
  if(!recreated)
    test_selfhosting_cleanup();

  return recreated;
}

bool test_create_and_selfhost_from_uri(const Glib::ustring& example_file_uri, Glom::Document& document, Glom::Document::HostingMode hosting_mode, const std::string& subdirectory_path)
{
  if( (hosting_mode != Glom::Document::HOSTING_MODE_POSTGRES_SELF) &&
    (hosting_mode != Glom::Document::HOSTING_MODE_MYSQL_SELF) &&
    (hosting_mode != Glom::Document::HOSTING_MODE_SQLITE) )
  {
    std::cerr << G_STRFUNC << ": This test function does not support the specified hosting_mode: " << hosting_mode << std::endl;
    return false;
  }

  // Load the document:
  document.set_allow_autosave(false); //To simplify things and to not depend implicitly on autosave.
  document.set_file_uri(example_file_uri);
  int failure_code = 0;
  const bool test = document.load(failure_code);
  //std::cout << "Document load result=" << test << std::endl;

  if(!test)
  {
    std::cerr << G_STRFUNC << ": Document::load() failed with failure_code=" << failure_code << " for uri=" << example_file_uri << std::endl;
    return false;
  }

  return after_load(document, hosting_mode, subdirectory_path);
}

bool test_create_and_selfhost_from_data(const Glib::ustring& example_file_contents, Glom::Document& document, Glom::Document::HostingMode hosting_mode, const std::string& subdirectory_path)
{
  if( (hosting_mode != Glom::Document::HOSTING_MODE_POSTGRES_SELF) &&
    (hosting_mode != Glom::Document::HOSTING_MODE_MYSQL_SELF) &&
    (hosting_mode != Glom::Document::HOSTING_MODE_SQLITE) )
  {
    std::cerr << G_STRFUNC << ": This test function does not support the specified hosting_mode: " << hosting_mode << std::endl;
    return false;
  }

  document.set_allow_autosave(false); //To simplify things and to not depend implicitly on autosave.

  int failure_code = 0;
  const bool test = document.load_from_data((const guchar*)example_file_contents.c_str(), example_file_contents.bytes(), failure_code);

  if(!test)
  {
    std::cerr << G_STRFUNC << ": Document::load_from_data() failed with failure_code=" << failure_code << std::endl;
    return false;
  }

  return after_load(document, hosting_mode, subdirectory_path);
}
bool test_model_expected_size(const Glib::RefPtr<const Gnome::Gda::DataModel>& data_model, guint columns_count, guint rows_count)
{
  if(!data_model)
  {
    std::cerr << G_STRFUNC << ": Failure: data_model was null" << std::endl;
    return false;
  }

  if(data_model->get_n_columns() != (int)columns_count)
  {
    std::cerr << G_STRFUNC << ": Failure: get_n_columns() returned an unexpected value. Expected: " << columns_count << ", Actual: " << data_model->get_n_columns() << std::endl;
    return false;
  }

  if(data_model->get_n_rows() != (int)rows_count)
  {
    std::cerr << G_STRFUNC << ": Failure: get_n_rows() returned an unexpected value. Expected: " << rows_count << ", Actual: " << data_model->get_n_rows() << std::endl;
    return false;
  }

  return true;
}

bool test_table_exists(const Glib::ustring& table_name, const Glom::Document& document)
{
  //Try to get more rows than intended:
  Glom::Utils::type_vecLayoutFields fieldsToGet;
  Glom::sharedptr<const Glom::Field> field = document.get_field_primary_key(table_name); //To to get some field.
  if(!field)
  {
    std::cerr << G_STRFUNC << "Failure: Could not get primary key for table=" << table_name << std::endl;
    return false;
  }

  Glom::sharedptr<Glom::LayoutItem_Field> layoutitem = Glom::sharedptr<Glom::LayoutItem_Field>::create();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);

  const Glib::RefPtr<const Gnome::Gda::SqlBuilder> builder = 
    Glom::Utils::build_sql_select_with_where_clause(table_name,
      fieldsToGet);
  const Glib::RefPtr<const Gnome::Gda::DataModel> data_model = 
    Glom::DbUtils::query_execute_select(builder);
  if(!data_model || !(data_model->get_n_columns()))
  {
    std::cerr << G_STRFUNC << ": Failure: table does not exist: " << table_name << std::endl;
    return false;
  }

  return true;
}

static bool test_example_musiccollection_data_related(const Glom::Document* document, const Gnome::Gda::Value& album_id)
{
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document is null" << std::endl;
    return false;
  }

  Glom::Utils::type_vecLayoutFields fieldsToGet;

  //Normal fields:
  Glom::sharedptr<const Glom::Field> field_album_id = document->get_field("albums", "album_id");
  Glom::sharedptr<Glom::LayoutItem_Field> layoutitem = Glom::sharedptr<Glom::LayoutItem_Field>::create();
  layoutitem->set_full_field_details(field_album_id);
  fieldsToGet.push_back(layoutitem);
  Glom::sharedptr<const Glom::Field> field = document->get_field("albums", "name");
  if(!field)
  {
    std::cerr << G_STRFUNC << "Failure: Could not get field." << std::endl;
    return false;
  }
  layoutitem = Glom::sharedptr<Glom::LayoutItem_Field>::create();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);

  //Related field:
  const Glom::sharedptr<Glom::Relationship> relationship = document->get_relationship("albums", "artist");
  if(!relationship)
  {
    std::cerr << G_STRFUNC << ": Failure: The relationship could not be found." << std::endl;
    return false;
  }
  layoutitem = Glom::sharedptr<Glom::LayoutItem_Field>::create();
  layoutitem->set_relationship(relationship);
  field = document->get_field("artists", "name");
  if(!field)
  {
    std::cerr << G_STRFUNC << "Failure: Could not get field." << std::endl;
    return false;
  }
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);

  const Glib::RefPtr<const Gnome::Gda::SqlBuilder> builder =
    Glom::Utils::build_sql_select_with_key("albums", fieldsToGet, field_album_id, album_id);
  const Glib::RefPtr<const Gnome::Gda::DataModel> data_model = 
    Glom::DbUtils::query_execute_select(builder);
  if(!test_model_expected_size(data_model, 3, 1))
  {
    std::cerr << G_STRFUNC << ": Failure: Unexpected data model size with query: " << 
      Glom::Utils::sqlbuilder_get_full_query(builder) << std::endl;
    return false;
  }
  
  return true;
}

bool test_example_musiccollection_data(const Glom::Document* document)
{
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document is null" << std::endl;
    return false;
  }
  
  //Check that some data is as expected:
  const Gnome::Gda::Value value("Born To Run");
  const Gnome::Gda::SqlExpr where_clause = 
    Glom::Utils::get_find_where_clause_quick(document, "albums", value);
  
  Glom::Utils::type_vecLayoutFields fieldsToGet;
  Glom::sharedptr<const Glom::Field> field = document->get_field("albums", "album_id");
  Glom::sharedptr<Glom::LayoutItem_Field> layoutitem = Glom::sharedptr<Glom::LayoutItem_Field>::create();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);

  field = document->get_field("albums", "name");
  if(!field)
  {
    std::cerr << G_STRFUNC << "Failure: Could not get field." << std::endl;
    return false;
  }
  layoutitem = Glom::sharedptr<Glom::LayoutItem_Field>::create();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);

  const Glib::RefPtr<const Gnome::Gda::SqlBuilder> builder = 
    Glom::Utils::build_sql_select_with_where_clause("albums",
      fieldsToGet, where_clause);
  const Glib::RefPtr<const Gnome::Gda::DataModel> data_model = 
    Glom::DbUtils::query_execute_select(builder);
  if(!test_model_expected_size(data_model, 2, 1))
  {
    std::cerr << G_STRFUNC << "Failure: Unexpected data model size with query: " << 
      Glom::Utils::sqlbuilder_get_full_query(builder) << std::endl;
    return false;
  }

  const int count = Glom::DbUtils::count_rows_returned_by(builder);
  if(count != 1 )
  {
    std::cerr << G_STRFUNC << "Failure: The COUNT query returned an unexpected value: " << count << std::endl;
    return false;
  }


  //Get the album's related artist name:
  const Gnome::Gda::Value album_id = data_model->get_value_at(0, 0);
  return test_example_musiccollection_data_related(document, album_id);
}

static bool test_hosting_mode(const SlotTest& slot, Glom::Document::HostingMode hosting_mode, const Glib::ustring name)
{
  try
  {
    if(!slot(hosting_mode))
    {
      std::cerr << G_STRFUNC << ": Failed with " << name << std::endl;
      test_selfhosting_cleanup();
      return false;
    }
  }
  catch(const std::exception& ex)
  {
    std::cerr << G_STRFUNC << ": Failed with " << name << " with exception: " << ex.what() << std::endl;
    test_selfhosting_cleanup();
    return false;
  }
  catch(...)
  {
    std::cerr << G_STRFUNC << ": Failed with " << name << " with unknown exception: " << std::endl;
    test_selfhosting_cleanup();
    return false;
  }

  return true;
}

int test_all_hosting_modes(const SlotTest& slot)
{
  if(!test_hosting_mode(slot, Glom::Document::HOSTING_MODE_SQLITE, "SQLite"))
    return EXIT_FAILURE;

//Do not test MySQL unless it is enabled in the build,
//though the functionality is always built in libglom.
//We usually use GLOM_ENABLE_MYSQL only in the UI code,
//but let's avoid forcing people to install the MySQL server.
//Also, Ubuntu's AppArmor will not let use start a MySQL process by default anyway.
//See https://bugs.launchpad.net/ubuntu/+source/mysql-5.5/+bug/1095370
#ifdef GLOM_ENABLE_MYSQL
  if(!test_hosting_mode(slot, Glom::Document::HOSTING_MODE_MYSQL_SELF, "MySQL"))
    return EXIT_FAILURE;
#endif //GLOM_ENABLE_MYSQL

  if(!test_hosting_mode(slot, Glom::Document::HOSTING_MODE_POSTGRES_SELF, "PostgreSQL"))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}

bool test_check_numeric_value_type(Glom::Document::HostingMode hosting_mode, const Gnome::Gda::Value& value)
{
  const GType gtype = value.get_value_type();
  //std::cout << "debug: gtype=" << g_type_name(gtype) << std::endl;
  if(hosting_mode == Glom::Document::HOSTING_MODE_SQLITE)
  {
    if(gtype == G_TYPE_DOUBLE)
      return true;
  } else if( (hosting_mode == Glom::Document::HOSTING_MODE_MYSQL_CENTRAL) ||
    (hosting_mode == Glom::Document::HOSTING_MODE_MYSQL_SELF) )
  {
    if(gtype == G_TYPE_DOUBLE)
      return true;
  }

  //The normal type for PostgreSQL:
  return (gtype == GDA_TYPE_NUMERIC);
}
