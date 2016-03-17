/* Glom
 *
 * Copyright (C) 2011 Openismus GmbH
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

// For instance:
// glom_create_from_example /opt/gnome30/share/doc/glom/examples/example_music_collection.glom --output-path=/home/murrayc/ --output-name="something.glom"

#include "config.h"

#include <libglom/document/document.h>
#include <libglom/connectionpool.h>
#include <libglom/connectionpool_backends/postgres_self.h>
#include <libglom/connectionpool_backends/postgres_central.h>
#include <libglom/connectionpool_backends/mysql_self.h>
#include <libglom/connectionpool_backends/mysql_central.h>
#include <libglom/init.h>
#include <libglom/privs.h>
#include <libglom/db_utils.h>
#include <libglom/file_utils.h>
#include <libglom/utils.h>
#include <giomm/file.h>
#include <glibmm/optioncontext.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>
#include <iostream>
#include <stdexcept>

#include <unistd.h> //For getpass().

#include <glibmm/i18n.h>

#include "config.h"

class GlomCreateOptionGroup : public Glib::OptionGroup
{
public:
  GlomCreateOptionGroup();

  //These instances should live as long as the OptionGroup to which they are added,
  //and as long as the OptionContext to which those OptionGroups are added.
  std::string m_arg_filename_input;
  std::string m_arg_filepath_dir_output;
  std::string m_arg_filepath_name_output;
  bool m_arg_version;
  
  //If not using self-hosting:
  Glib::ustring m_arg_server_hostname;
  double m_arg_server_port;
  Glib::ustring m_arg_server_username;
  Glib::ustring m_arg_server_password;

#ifdef GLOM_ENABLE_MYSQL
  bool m_arg_use_mysql;
#endif //GLOM_ENABLE_MYSQL
};

GlomCreateOptionGroup::GlomCreateOptionGroup()
: Glib::OptionGroup("glom_create_from_example", _("Glom options"), _("Command-line options")),
  m_arg_version(false),
  m_arg_server_port(0)
#ifdef GLOM_ENABLE_MYSQL
  ,
  m_arg_use_mysql(false)
#endif //GLOM_ENABLE_MYSQL
{
  Glib::OptionEntry entry;
  entry.set_long_name("input");
  entry.set_short_name('i');
  entry.set_description(_("The example .glom file to open."));
  add_entry_filename(entry, m_arg_filename_input);
  
  entry.set_long_name("output-path");
  entry.set_short_name('o');
  entry.set_description(_("The directory in which to save the created .glom file, or sub-directory if necessary, such as /home/someuser/ ."));
  add_entry_filename(entry, m_arg_filepath_dir_output);
  
  entry.set_long_name("output-name");
  entry.set_short_name('n');
  entry.set_description(_("The name for the created .glom file, such as something.glom ."));
  add_entry_filename(entry, m_arg_filepath_name_output);

  entry.set_long_name("version");
  entry.set_short_name('V');
  entry.set_description(_("The version of this application."));
  add_entry(entry, m_arg_version);


  entry.set_long_name("server-hostname");
  entry.set_short_name('h');
  entry.set_description(_("The hostname of the PostgreSQL server, such as localhost. If this is not specified then a self-hosted database will be created."));
  add_entry(entry, m_arg_server_hostname);
  
  entry.set_long_name("server-port");
  entry.set_short_name('p');
  entry.set_description(_("The port of the PostgreSQL server, such as 5434."));
  add_entry(entry, m_arg_server_port);
  
  entry.set_long_name("server-username");
  entry.set_short_name('u');
  entry.set_description(_("The username for the PostgreSQL server."));
  add_entry(entry, m_arg_server_username);

#ifdef GLOM_ENABLE_MYSQL
  entry.set_long_name("use-mysql");
  entry.set_short_name('m');
  entry.set_description(_("Use MySQL instead of PostgreSQL."));
  add_entry(entry, m_arg_use_mysql);
#endif //GLOM_ENABLE_MYSQL
}

static void on_initialize_progress()
{
  std::cout << "Database initialization progress\n";
}

static void on_startup_progress()
{
  std::cout << "Database startup progress\n";
}

static void on_recreate_progress()
{
  std::cout << "Database re-creation progress\n";
}

static void on_cleanup_progress()
{
  std::cout << "Database cleanup progress\n";
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
  auto enumerator = directory->enumerate_children();

  auto info = enumerator->next_file();
  while(info)
  {
    auto child = directory->get_child(info->get_name());
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
  if(uri.empty())
    return true;

  auto file = Gio::File::create_for_uri(uri);
  return delete_directory(file);
}

std::string filepath_dir;

static Glib::ustring convert_filepath_to_uri(const std::string& filepath)
{
  try
  {
    return Glib::filename_to_uri(filepath);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": Could not convert filepath to URI: " << filepath << std::endl;
    return Glib::ustring();
  }
}

static void cleanup()
{
  auto connection_pool = Glom::ConnectionPool::get_instance();

  const auto stopped = connection_pool->cleanup( sigc::ptr_fun(&on_cleanup_progress) );
  g_assert(stopped);

  //Make sure the directory is removed at the end,
  {
    const auto uri = convert_filepath_to_uri(filepath_dir);
    delete_directory(uri);
  }
}


int main(int argc, char* argv[])
{
  bindtextdomain(GETTEXT_PACKAGE, GLOM_LOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  // Set the locale for any streams to the user's current locale,
  // We should not rely on the default locale of
  // any streams (we should always do an explicit imbue()),
  // but this is maybe a good default in case we forget.
  try
  {
    std::locale::global(std::locale(""));
  }
  catch(const std::runtime_error& ex)
  {
    //This has been known to throw an exception at least once:
    //https://bugzilla.gnome.org/show_bug.cgi?id=619445
    //This should tell us what the problem is:
    std::cerr << G_STRFUNC << ": exception from std::locale::global(std::locale(\"\")): " << ex.what() << std::endl;
    std::cerr << G_STRFUNC << ":   This can happen if the locale is not properly installed or configured.\n";
  }

  
  Glom::libglom_init();
  
  Glib::OptionContext context;
  GlomCreateOptionGroup group;
  context.set_main_group(group);
  
  try
  {
    context.parse(argc, argv);
  }
  catch(const Glib::OptionError& ex)
  {
      std::cout << _("Error while parsing command-line options: ") << std::endl << ex.what() << std::endl;
      std::cout << _("Use --help to see a list of available command-line options.") << std::endl;
      return 0;
  }
  catch(const Glib::Error& ex)
  {
    std::cout << "Error: " << ex.what() << std::endl;
    return 0;
  }

  if(group.m_arg_version)
  {
    std::cout << PACKAGE_STRING << std::endl;
    return 0;
  }

  // Get a URI for a test file:
  Glib::ustring input_uri = group.m_arg_filename_input;

  // The GOption documentation says that options without names will be returned to the application as "rest arguments".
  // I guess this means they will be left in the argv. Murray.
  if(input_uri.empty() && (argc > 1))
  {
     const char* pch = argv[1];
     if(pch)
       input_uri = pch;
  }
  
  if(!input_uri.empty())
  {
    //Get a URI (file://something) from the filepath:
    auto file = Gio::File::create_for_commandline_arg(input_uri);

    if(!file->query_exists())
    {
      std::cerr << _("Glom: The file does not exist.") << std::endl;
      std::cerr << G_STRFUNC << ": uri: " << input_uri << std::endl;

      std::cerr << std::endl << context.get_help() << std::endl;
      return EXIT_FAILURE;
    }

    const auto file_type = file->query_file_type();
    if(file_type == Gio::FILE_TYPE_DIRECTORY)
    {
      std::cerr << _("Glom: The file path is a directory instead of a file.") << std::endl;
      std::cerr << std::endl << context.get_help() << std::endl;
      return EXIT_FAILURE;
    }

    input_uri = file->get_uri();

  }
  
  if(input_uri.empty())
  {
    std::cerr << G_STRFUNC << ": Please specify a glom example file.\n";
    std::cerr << std::endl << context.get_help() << std::endl;
    return EXIT_FAILURE;
  }


  //Check the output directory path: 
  if(group.m_arg_filepath_dir_output.empty())
  {
    std::cerr << G_STRFUNC << ": Please specify an output directory path.\n";
    std::cerr << std::endl << context.get_help() << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    //Get a URI (file://something) from the filepath:
    auto file = Gio::File::create_for_commandline_arg(group.m_arg_filepath_dir_output);

    if(!file->query_exists())
    {
      std::cerr << _("Glom: The output directory does not exist.") << std::endl;
      std::cerr << G_STRFUNC << ": uri: " << group.m_arg_filepath_dir_output << std::endl;

      std::cerr << std::endl << context.get_help() << std::endl;
      return EXIT_FAILURE;
    }

    const auto file_type = file->query_file_type();
    if(file_type != Gio::FILE_TYPE_DIRECTORY)
    {
      std::cerr << _("Glom: The output path is not a directory.") << std::endl;
      std::cerr << std::endl << context.get_help() << std::endl;
      return EXIT_FAILURE;
    }
  }
  
  //Check the output name path: 
  if(group.m_arg_filepath_name_output.empty())
  {
    std::cerr << G_STRFUNC << ": Please specify an output name.\n";
    std::cerr << std::endl << context.get_help() << std::endl;
    return EXIT_FAILURE;
  }


  // Load the document:
  auto document = std::make_shared<Glom::Document>();
  document->set_file_uri(input_uri);
  int failure_code = 0;
  const auto test = document->load(failure_code);
  //std::cout << "Document load result=" << test << std::endl;

  if(!test)
  {
    std::cerr << G_STRFUNC << ": Document::load() failed with failure_code=" << failure_code << std::endl;
    return EXIT_FAILURE;
  }

  g_assert(document->get_is_example_file());;

  auto connection_pool = Glom::ConnectionPool::get_instance();

  //Save a copy, specifying the path to file in a directory:
  filepath_dir = 
    Glom::FileUtils::get_file_path_without_extension(
      Glib::build_filename(
        group.m_arg_filepath_dir_output,
        group.m_arg_filepath_name_output));
  const std::string filepath =
    Glib::build_filename(filepath_dir, group.m_arg_filepath_name_output);

  //Make sure that the file does not exist yet:
  {
    const auto uri = convert_filepath_to_uri(filepath_dir);
    if(uri.empty())
      return EXIT_FAILURE;
        
    auto file = Gio::File::create_for_commandline_arg(uri);
    if(file->query_exists())
    {
      std::cerr << G_STRFUNC << ": The output path already exists: " << filepath_dir << std::endl;
      return EXIT_FAILURE;
    }
  }

  //Save the example as a real file:
  const auto file_uri = convert_filepath_to_uri(filepath);
  if(file_uri.empty())
    return EXIT_FAILURE;

  document->set_file_uri(file_uri);

  const auto self_hosting = group.m_arg_server_hostname.empty();
  if(self_hosting)
  {
    std::cout << "Using self-hosting instead of a central database server.\n";

#if GLOM_ENABLE_MYSQL
    if(group.m_arg_use_mysql)
      document->set_hosting_mode(Glom::Document::HostingMode::MYSQL_SELF);
    else
#endif //GLOM_ENABLE_MYSQL
    {
      document->set_hosting_mode(Glom::Document::HostingMode::POSTGRES_SELF);
    }
  }
  else
  {
    std::cout << "Using the database server with host: " << group.m_arg_server_hostname << std::endl;

#if GLOM_ENABLE_MYSQL
    if(group.m_arg_use_mysql)
      document->set_hosting_mode(Glom::Document::HostingMode::MYSQL_CENTRAL);
    else
#endif //GLOM_ENABLE_MYSQL
    {
      document->set_hosting_mode(Glom::Document::HostingMode::POSTGRES_CENTRAL);
    }
  }
   
  document->set_is_example_file(false);
  document->set_network_shared(false);
  const auto saved = document->save();
  g_assert(saved);

  //Specify the backend and backend-specific details to be used by the connectionpool.
  connection_pool->setup_from_document(document);

  //We must specify a default username and password:
  if(self_hosting)
  {
    Glib::ustring password;
    const auto user = Glom::Privs::get_default_developer_user_name(password, document->get_hosting_mode());
    connection_pool->set_user(user);
    connection_pool->set_password(password);
  }
  else
  {
    //Get the password from stdin.
    //This is not a command-line option because then it would appear in logs.
    //Other command-line utilities such as psql don't do this either.
    //TODO: Support alternatives such as using a file.
    const auto prompt = Glib::ustring::compose(
      _("Please enter the database server's password for the user %1: "), group.m_arg_server_username);

#ifdef G_OS_WIN32
    const char* password = "";
    std::cerr << G_STRFUNC << ": Error: getpass() is not implemented in the Windows build. The connection will fail.\n";
#else
    const auto password = ::getpass(prompt.c_str());
#endif

    //Central hosting:
    connection_pool->set_user(group.m_arg_server_username);
    connection_pool->set_password(password); //TODO: Take this from stdin instead.
    
    auto backend = connection_pool->get_backend();
    auto central = std::dynamic_pointer_cast<Glom::ConnectionPoolBackends::PostgresCentralHosted>(backend);
    g_assert(central);

    central->set_host(group.m_arg_server_hostname);
    
    if(group.m_arg_server_port)
    {
      central->set_port(group.m_arg_server_port);
      central->set_try_other_ports(false);
    }
    else
    {
      //Try all ports:
      central->set_try_other_ports(false);
    }
    
    const Glib::ustring database_name =
      Glom::DbUtils::get_unused_database_name(document->get_connection_database());
    if(database_name.empty())
    {
      std::cerr << G_STRFUNC << ": Could not find an unused database name\n";
    }
    else
      document->set_connection_database(database_name);
  }
        
  //Startup. For instance, create the self-hosting files if necessary:
  const Glom::ConnectionPool::InitErrors initialized_errors =
    connection_pool->initialize( sigc::ptr_fun(&on_initialize_progress) );
  g_assert(initialized_errors == Glom::ConnectionPool::Backend::InitErrors::NONE);

  //Start self-hosting:
  //TODO: Let this happen automatically on first connection?
  const auto started = connection_pool->startup( sigc::ptr_fun(&on_startup_progress) );
  if(started != Glom::ConnectionPool::Backend::StartupErrors::NONE)
  {
    std::cerr << G_STRFUNC << ": connection_pool->startup(): result=" << Glom::Utils::to_utype(started) << std::endl;
    cleanup();
  }
  g_assert(started == Glom::ConnectionPool::Backend::StartupErrors::NONE);

  const auto recreated = Glom::DbUtils::recreate_database_from_document(document, &on_recreate_progress);
  if(!recreated)
    cleanup();
  g_assert(recreated);

  //Tell the user where the file is:
  std::string output_path_used;
  try
  {
    output_path_used = Glib::filename_from_uri(document->get_file_uri());
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": Could not convert URI to output filepath: " << document->get_file_uri() << std::endl;
  }
   
  std::cout << "Glom file created at: " << output_path_used << std::endl;


  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
