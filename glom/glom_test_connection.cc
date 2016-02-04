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
// glom_test_connection --server-hostname=localhost --server-port=5433 --server-username=someuser

#include "config.h"

#include <libglom/connectionpool.h>
#include <libglom/connectionpool_backends/postgres_central.h>
#include <libglom/connectionpool_backends/mysql_central.h>
#include <libglom/init.h>
#include <libglom/privs.h>
#include <libglom/utils.h>
#include <giomm/file.h>
#include <glibmm/optioncontext.h>
#include <glibmm/convert.h>
#include <iostream>
#include <stdexcept>

#include <glibmm/i18n.h>

class GlomCreateOptionGroup : public Glib::OptionGroup
{
public:
  GlomCreateOptionGroup();

  //These instances should live as long as the OptionGroup to which they are added,
  //and as long as the OptionContext to which those OptionGroups are added.
  bool m_arg_version;
  Glib::ustring m_arg_server_hostname;
  double m_arg_server_port;
  Glib::ustring m_arg_server_username;
  Glib::ustring m_arg_server_password;
  Glib::ustring m_arg_server_database;

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

  entry.set_long_name("version");
  entry.set_short_name('V');
  entry.set_description(_("The version of this application."));
  add_entry(entry, m_arg_version);

  entry.set_long_name("server-hostname");
  entry.set_short_name('h');
  entry.set_description(_("The hostname of the PostgreSQL server, such as localhost."));
  add_entry(entry, m_arg_server_hostname);
  
  entry.set_long_name("server-port");
  entry.set_short_name('p');
  entry.set_description(_("The port of the PostgreSQL server, such as 5434."));
  add_entry(entry, m_arg_server_port);
  
  entry.set_long_name("server-username");
  entry.set_short_name('u');
  entry.set_description(_("The username for the PostgreSQL server."));
  add_entry(entry, m_arg_server_username);

  //Optional:
  entry.set_long_name("server-database");
  entry.set_short_name('d');
  entry.set_description(_("The specific database on the PostgreSQL server (Optional)."));
  add_entry(entry, m_arg_server_database);


#ifdef GLOM_ENABLE_MYSQL
  entry.set_long_name("use-mysql");
  entry.set_short_name('m');
  entry.set_description(_("Use MySQL instead of PostgreSQL."));
  add_entry(entry, m_arg_use_mysql);
#endif //GLOM_ENABLE_MYSQL
}

static void print_options_hint()
{
  //TODO: How can we just print them out?
  std::cout << _("Use --help to see a list of available command-line options.") << std::endl;
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
    std::cerr << G_STRFUNC << ":   This can happen if the locale is not properly installed or configured." << std::endl;
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
    print_options_hint();
    return EXIT_FAILURE;
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Error: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  if(group.m_arg_version)
  {
    std::cout << PACKAGE_STRING << std::endl;
    return EXIT_SUCCESS;
  }

  if(group.m_arg_server_hostname.empty())
  {
    std::cerr << G_STRFUNC << ": Please provide a database hostname." << std::endl;
    print_options_hint();
    return EXIT_FAILURE;
  }

  if(group.m_arg_server_username.empty())
  {
    std::cerr << _("Please provide a database username.") << std::endl;
    print_options_hint();
    return EXIT_FAILURE;
  }

  //Get the password from stdin.
  //This is not a command-line option because then it would appear in logs.
  //Other command-line utilities such as psql don't do this either.
  //TODO: Support alternatives such as using a file.
  const auto prompt = Glib::ustring::compose(
    _("Please enter the PostgreSQL server's password for the user %1: "), group.m_arg_server_username);

#ifdef G_OS_WIN32
  const char* password = "";
  std::cerr << G_STRFUNC << ": Error: getpass() is not implemented in the Windows build. The connection will fail." << std::endl;
#else
  const auto password = ::getpass(prompt.c_str());
#endif

  //Setup the connection, assuming that we are testing central hosting:
  auto connection_pool = Glom::ConnectionPool::get_instance();

  //Specify the backend and backend-specific details to be used by the connectionpool.
  //This is usually done by ConnectionPool::setup_from_document():
  std::shared_ptr<Glom::ConnectionPoolBackends::Backend> backend;
#ifdef GLOM_ENABLE_MYSQL
  if(group.m_arg_use_mysql)
  {
    //TODO: Move some of the *CentralHosted API into a multiply-inherited Server base class,
    //to avoid the duplication?
    auto derived_backend = std::make_shared<Glom::ConnectionPoolBackends::MySQLCentralHosted;

    //Use a specified port, or try all suitable ports:
    if(group.m_arg_server_port)
    {
      derived_backend->set_port(group.m_arg_server_port);
      derived_backend->set_try_other_ports(false);
    }
    else
    {
      derived_backend->set_try_other_ports(true);
    }

    derived_backend->set_host(group.m_arg_server_hostname);
    backend = derived_backend;
  }
  else
#endif //GLOM_ENABLE_MYSQL
  {
    auto derived_backend = std::make_shared<Glom::ConnectionPoolBackends::PostgresCentralHosted>();

    //Use a specified port, or try all suitable ports:
    if(group.m_arg_server_port)
    {
      derived_backend->set_port(group.m_arg_server_port);
      derived_backend->set_try_other_ports(false);
    }
    else
    {
      derived_backend->set_try_other_ports(true);
    }

    derived_backend->set_host(group.m_arg_server_hostname);
    backend = derived_backend;
  }

  connection_pool->set_user(group.m_arg_server_username);
  connection_pool->set_password(password);
  connection_pool->set_backend(std::shared_ptr<Glom::ConnectionPool::Backend>(backend));

  if(group.m_arg_server_database.empty())
  {
    //Prevent it from trying to connect to a database with the same name as the user,
    //which is more likely to exist by chance than this silly name:
    connection_pool->set_database("somenonexistantdatbasename");
  }
  else
  {
    connection_pool->set_database(group.m_arg_server_database);
  }

  connection_pool->set_ready_to_connect();

  try
  {
    connection_pool->connect();
  }
  catch(const Glom::ExceptionConnection& ex)
  {
    if(ex.get_failure_type() == Glom::ExceptionConnection::failure_type::NO_SERVER)
    {
      std::cerr << _("Error: Could not connect to the server even without specifying a database.") << std::endl;
      return EXIT_FAILURE;
    }
    else if(ex.get_failure_type() == Glom::ExceptionConnection::failure_type::NO_DATABASE)
    {
      //We expect this exception if we did not specify a database:
      if(!(group.m_arg_server_database.empty()))
      {  
        std::cerr << _("Error: Could not connect to the specified database.") << std::endl;
        return EXIT_FAILURE;
      }
    }
  }

  std::cout << _("Successful connection.") << std::endl;
        
  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
