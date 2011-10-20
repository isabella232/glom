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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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
#include <iostream>

static void on_initialize_progress()
{
  std::cout << "Database initialization progress" << std::endl;
}

static void on_startup_progress()
{
  std::cout << "Database startup progress" << std::endl;
}

static void on_recreate_progress()
{
  std::cout << "Database re-creation progress" << std::endl;
}

static void on_cleanup_progress()
{
  std::cout << "Database cleanup progress" << std::endl;
}

std::string temp_filepath_dir;

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

void test_selfhosting_cleanup()
{
  Glom::ConnectionPool* connection_pool = Glom::ConnectionPool::get_instance();

  const bool stopped = connection_pool->cleanup( sigc::ptr_fun(&on_cleanup_progress) );
  g_assert(stopped);

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
}


bool test_create_and_selfhost(const std::string& example_filename, Glom::Document& document, Glom::Document::HostingMode hosting_mode)
{
  if( (hosting_mode != Glom::Document::HOSTING_MODE_POSTGRES_SELF) &&
    (hosting_mode != Glom::Document::HOSTING_MODE_SQLITE) )
  {
    std::cerr << G_STRFUNC << ": This test function does not support the specified hosting_mode: " << hosting_mode << std::endl;
    return false;
  }
 
  // Get a URI for a test file:
  Glib::ustring uri;

  try
  {
    const std::string path =
       Glib::build_filename(GLOM_DOCDIR_EXAMPLES_NOTINSTALLED,
         example_filename);
    uri = Glib::filename_to_uri(path);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what();
    return false;
  }

  //std::cout << "URI=" << uri << std::endl;


  // Load the document:
  document.set_file_uri(uri);
  int failure_code = 0;
  const bool test = document.load(failure_code);
  //std::cout << "Document load result=" << test << std::endl;

  if(!test)
  {
    std::cerr << "Document::load() failed with failure_code=" << failure_code << std::endl;
    return false;
  }

  g_assert(document.get_is_example_file());;

  Glom::ConnectionPool* connection_pool = Glom::ConnectionPool::get_instance();

  //Save a copy, specifying the path to file in a directory:
  //For instance, /tmp/testfileglom/testfile.glom");
  const std::string temp_filename = "testglom";
  temp_filepath_dir = Glib::build_filename(Glib::get_tmp_dir(),
    temp_filename);
  const std::string temp_filepath = Glib::build_filename(temp_filepath_dir, temp_filename);

  //Make sure that the file does not exist yet:
  {
    const Glib::ustring uri = Glib::filename_to_uri(temp_filepath_dir);
    delete_directory(uri);
  }

  //Save the example as a real file:
  const Glib::ustring file_uri = Glib::filename_to_uri(temp_filepath);
  document.set_file_uri(file_uri);

  document.set_hosting_mode(hosting_mode);
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
  
  if(!check_directory_exists())
  {
    std::cerr << "Failure: The data directory does not exist after calling initialize()." << std::endl; 
  }

  //Start self-hosting:
  //TODO: Let this happen automatically on first connection?
  const Glom::ConnectionPool::StartupErrors started = connection_pool->startup( sigc::ptr_fun(&on_startup_progress) );
  if(started != Glom::ConnectionPool::Backend::STARTUPERROR_NONE)
  {
    std::cerr << "connection_pool->startup(): result=" << started << std::endl;
    test_selfhosting_cleanup();
  }
  g_assert(started == Glom::ConnectionPool::Backend::STARTUPERROR_NONE);

  const bool recreated = Glom::DbUtils::recreate_database_from_document(&document, sigc::ptr_fun(&on_recreate_progress) );
  if(!recreated)
    test_selfhosting_cleanup();

  return recreated;
}

bool test_model_expected_size(const Glib::RefPtr<Gnome::Gda::DataModel>& data_model, guint columns_count, guint rows_count)
{
  if(!data_model)
  {
    std::cerr << "Failure: data_model was null" << std::endl;
    return false;
  }

  if(data_model->get_n_columns() != (int)columns_count)
  {
    std::cerr << "Failure: get_n_columns() returned an unexpected value. Expected: " << columns_count << ", Actual: " << data_model->get_n_columns() << std::endl;
    return false;
  }

  if(data_model->get_n_rows() != (int)rows_count)
  {
    std::cerr << "Failure: get_n_rows() returned an unexpected value. Expected: " << rows_count << ", Actual: " << data_model->get_n_rows() << std::endl;
    return false;
  }

  return true;
}

bool test_table_exists(const Glib::ustring& table_name, const Glom::Document& document)
{
  //Try to get more rows than intended:
  Glom::Utils::type_vecLayoutFields fieldsToGet;
  Glom::sharedptr<const Glom::Field> field = document.get_field_primary_key(table_name); //To to get some field.
  Glom::sharedptr<Glom::LayoutItem_Field> layoutitem = Glom::sharedptr<Glom::LayoutItem_Field>::create();
  layoutitem->set_full_field_details(field);
  fieldsToGet.push_back(layoutitem);

  const Glib::RefPtr<const Gnome::Gda::SqlBuilder> builder = 
    Glom::Utils::build_sql_select_with_where_clause(table_name,
      fieldsToGet);
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = 
    Glom::DbUtils::query_execute_select(builder);
  if(!data_model || !(data_model->get_n_columns()))
  {
    std::cerr << "Failure: table does not exist: " << table_name << std::endl;
    return false;
  }

  return true;
}

