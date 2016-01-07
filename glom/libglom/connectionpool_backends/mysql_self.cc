/* Glom
 *
 * Copyright (C) 2001-2013 Murray Cumming
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

#include <libglom/connectionpool_backends/mysql_self.h>
#include <libglom/connectionpool.h>
#include <libglom/utils.h>
#include <libglom/db_utils.h>
#include <libglom/spawn_with_feedback.h>
#include <giomm/file.h>
#include <glib/gstdio.h> // For g_remove

#include <glibmm/convert.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <glibmm/stringutils.h>
#include <glibmm/timer.h>
#include <glibmm/main.h>
#include <glibmm/shell.h>
#include <glibmm/i18n.h>

#include <sstream> //For stringstream
#include <iostream>

#ifdef G_OS_WIN32
# include <windows.h>
# include <winsock2.h>
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <errno.h>
# include <netinet/in.h> //For sockaddr_in
#endif

#include <signal.h> //To catch segfaults

// Uncomment to see debug messages
//#define GLOM_CONNECTION_DEBUG

namespace Glom
{

namespace ConnectionPoolBackends
{

static const int PORT_MYSQL_SELF_HOSTED_START = 3306;
static const int PORT_MYSQL_SELF_HOSTED_END = 3350;

static const char FILENAME_DATA[] = "data";
static const char FILENAME_BACKUP[] = "backup";

static const char DEFAULT_DATABASE_NAME[] = "INFORMATION_SCHEMA";

MySQLSelfHosted::MySQLSelfHosted()
: m_network_shared(false),
  m_temporary_password_active(false)
{
  m_host = "localhost";
}

bool MySQLSelfHosted::get_self_hosting_active() const
{
  return m_port != 0;
}

unsigned int MySQLSelfHosted::get_port() const
{
  return m_port;
}

/** Try to install mysql on the distro, though this will require a
 * distro-specific patch to the implementation.
 */
bool MySQLSelfHosted::install_mysql(const SlotProgress& /* slot_progress */)
{
#if 0
  // This  is example code for Ubuntu, and possibly Debian,
  // using code from the gnome-system-tools Debian/Ubuntu patches.
  // (But please, just fix the dependencies instead. MySQL is not optional.)
  //
  // You will also need to remove the "ifdef 0"s around the code in gst-package.[h|c],
  // and define DISTRO_SPECIFIC_MYSQL_INSTALL_IMPLEMENTED above.

  //Careful. Maybe you want a different version.
  //Also, Glom will start its own instance of MySQL, on its own port, when it needs to,
  //so there is no need to start a Glom service after installation at system startup,
  //though it will not hurt Glom if you do that.
  const gchar *packages[] = { "mysqlql-8.1", 0 };
  const auto result = gst_packages_install(parent_window->gobj() /* parent window */, packages);
  if(result)
  {
    std::cout << "Glom: gst_packages_install() reports success." << std::endl;
    //Double-check, because gst_packages_install() incorrectly returns TRUE if it fails because
    //a) synaptic is already running, or
    //b) synaptic did not know about the package (no warning is shown in this case.)
    //Maybe gst_packages_install() never returns FALSE.
    return check_mysql_is_available_with_warning(); //This is recursive, but clicking Cancel will stop everything.
  }
  else
  {
    std::cout << "Glom: gst_packages_install() reports failure." << std::endl;
    return false; //Failed to install mysql.
  }
#else
  return false; //Failed to install mysql because no installation technique was implemented.
#endif // #if 0
}

Backend::InitErrors MySQLSelfHosted::initialize(const SlotProgress& slot_progress, const Glib::ustring& initial_username, const Glib::ustring& password, bool network_shared)
{
  m_network_shared = network_shared;

  if(m_database_directory_uri.empty())
  {
    std::cerr << G_STRFUNC << ": initialize: m_self_hosting_data_uri is empty." << std::endl;
    return InitErrors::OTHER;
  }

  if(initial_username.empty())
  {
    std::cerr << G_STRFUNC << ": MySQLSelfHosted::initialize(). Username was empty while attempting to create self-hosting database" << std::endl;
    return InitErrors::OTHER;
  }

  //Get the filepath of the directory that we should create:
  const auto dbdir_uri = m_database_directory_uri;
  //std::cout << "debug: dbdir_uri=" << dbdir_uri << std::endl;

  if(file_exists_uri(dbdir_uri))
    return InitErrors::DIRECTORY_ALREADY_EXISTS;

  const auto dbdir = Glib::filename_from_uri(dbdir_uri);
  //std::cout << "debug: dbdir=" << dbdir << std::endl;
  g_assert(!dbdir.empty());

  const auto dbdir_created = create_directory_filepath(dbdir);
  if(!dbdir_created)
  {
    std::cerr << G_STRFUNC << ": Couldn't create directory: " << dbdir << std::endl;

    return InitErrors::COULD_NOT_CREATE_DIRECTORY;
  }

  //Create these files: environment
  set_network_shared(slot_progress, m_network_shared);

  //Check that there is not an existing data directory:
  const auto dbdir_data = get_self_hosting_data_path(true /* create */);
  if(dbdir_data.empty())
  {
    std::cerr << G_STRFUNC << ": Couldn't create the data directory: " << dbdir << std::endl;

    return InitErrors::COULD_NOT_CREATE_DIRECTORY;
  }

  // initdb creates a new mysql database cluster:

  // Make sure to use double quotes for the executable path, because the
  // CreateProcess() API used on Windows does not support single quotes.
  // We don't use mysql_secure_installation because it only takes the details via prompts.
  // TODO: With MySQL 5.6, use the new --random-passwords option, because otherwise the root password will be blank,
  // and, at least on Ubuntu, we will then not be able to connect with mysqladmin.
  //
  // Note: This will fail on Ubuntu because its AppArmor configuration prevents use of MySQL in non-default directories:
  // https://bugs.launchpad.net/ubuntu/+source/mysql-5.5/+bug/1095370
  const auto command_initdb = get_path_to_mysql_executable("mysql_install_db")
    + " --no-defaults" //Otherwise Ubuntu specifies --user=mysql
    + " --datadir=" + Glib::shell_quote(dbdir_data);
    //TODO: + " --random-passwords";
  //std::cout << "debug: command_initdb=" << command_initdb << std::endl;
  const auto result = Glom::Spawn::execute_command_line_and_wait(command_initdb, slot_progress);
  if(!result)
  {
    std::cerr << G_STRFUNC << ": Error while attempting to create self-hosting MySQL database." << std::endl;
  }
  else
  {
    //std::cout << "debug: command_initdb succeeded" << ", this=" << this << std::endl;
  
    //This is used during the first start:
    m_initial_password_to_set = password;
    m_initial_username_to_set = initial_username;

    //TODO: With MySQL 5.6, use the new --random-passwords option (see above)
    m_temporary_password = "";
    m_temporary_password_active = true;
    m_saved_username = "root";
    m_saved_password = "";

    //Startup (and shutdown) so we can set the initial username and password.
    //TODO: This is inefficient, because the caller probably wants to start the server soon anyway,
    //but that might be in a different instance of this backend,
    //and we cannot take the risk of leaving the database with a default password.
    if(startup(slot_progress, false) != StartupErrors::NONE)
    {
      std::cerr << G_STRFUNC << ": Error while attempting to create self-hosting MySQL database, while starting for the first time, to set the initial username and password." << std::endl;
      return InitErrors::OTHER;
    }
    else
    {
      if(!cleanup(slot_progress))
      {
        std::cerr << G_STRFUNC << ": Error while attempting to create self-hosting MySQL database, while shutting down, after setting the initial username and password." << std::endl;
        return InitErrors::OTHER;
      }
    }

    //Get the temporary random password,
    //which will be used when first starting the server.
    /*
    const auto temporary_password_file = Glib::build_filename(
      Glib::get_home_dir(), ".mysql.secret");
    try
    {
      m_temporary_password = Glib::file_get_contents(temporary_password_file);
      m_temporary_password_active = true;
    }
    catch(const Glib::Error& ex)
    {
      std::cerr << G_STRFUNC << "file_get_contents() failed: " << ex.what() << std::endl;
    }

    if(m_temporary_password.empty())
    {
       std::cerr << G_STRFUNC << " Unable to discover the initial MySQL password." << std::endl;
       result = false;
    }
    */
  }

  return result ? InitErrors::NONE : InitErrors::COULD_NOT_START_SERVER;
}

Glib::ustring MySQLSelfHosted::get_mysqlql_utils_version(const SlotProgress& /* slot_progress */)
{
  return Glib::ustring(); //TODO
}

float MySQLSelfHosted::get_mysqlql_utils_version_as_number(const SlotProgress& /* slot_progress */)
{
  return 0; //TODO
}

static Glib::ustring build_query_change_username(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& old_username, const Glib::ustring& new_username)
{
  if(old_username.empty())
  {
    std::cerr << G_STRFUNC << ": old_username is empty." << std::endl;
    return Glib::ustring();
  }

  if(new_username.empty())
  {
    std::cerr << G_STRFUNC << ": new_username is empty." << std::endl;
    return Glib::ustring();
  }

  //TODO: Try to avoid specifing @localhost.
  //We do this to avoid this error:
  //mysql> RENAME USER root TO glom_dev_user;
  //ERROR 1396 (HY000): Operation RENAME USER failed for 'root'@'%'
  //mysql> RENAME USER root@localhost TO glom_dev_user;
  //Query OK, 0 rows affected (0.00 sec)
  const auto user = connection->quote_sql_identifier(old_username) + "@localhost";

  //Login will fail after restart if we don't specify @localhost here too:
  const auto new_user = connection->quote_sql_identifier(new_username) + "@localhost";

  return "RENAME USER " + user + " TO " + new_user;
}

Backend::StartupErrors MySQLSelfHosted::startup(const SlotProgress& slot_progress, bool network_shared)
{
  m_network_shared = network_shared;

  // Don't risk random crashes, although this really shouldn't be called
  // twice of course.
  //g_assert(!get_self_hosting_active());

  if(get_self_hosting_active())
  {
    std::cerr << G_STRFUNC << ": Already started." << std::endl;
    return StartupErrors::NONE; //Just do it once.
  }

  const auto dbdir_uri = m_database_directory_uri;

  if(!(file_exists_uri(dbdir_uri)))
  {
    std::cerr << G_STRFUNC << ": The data directory could not be found: " << dbdir_uri << std::endl;
    return StartupErrors::FAILED_NO_MAIN_DIRECTORY;
  }

  const auto dbdir = Glib::filename_from_uri(dbdir_uri);
  g_assert(!dbdir.empty());

  const auto dbdir_data = Glib::build_filename(dbdir, FILENAME_DATA);
  const auto dbdir_data_uri = Glib::filename_to_uri(dbdir_data);
  if(!(file_exists_uri(dbdir_data_uri)))
  {
    const auto dbdir_backup = Glib::build_filename(dbdir, FILENAME_BACKUP);
    const auto dbdir_backup_uri = Glib::filename_to_uri(dbdir_backup);
    if(file_exists_uri(dbdir_backup_uri))
    {
      std::cerr << G_STRFUNC << ": There is no data, but there is backup data." << std::endl;
      //Let the caller convert the backup to real data and then try again:
      return StartupErrors::FAILED_NO_DATA_HAS_BACKUP_DATA;
    }
    else
    {
      std::cerr << G_STRFUNC << ": The data sub-directory could not be found." << dbdir_data_uri << std::endl;
      return StartupErrors::FAILED_NO_DATA;
    }
  }

  //Attempt to ensure that the config files are correct:
  set_network_shared(slot_progress, m_network_shared);

  const unsigned int available_port = discover_first_free_port(PORT_MYSQL_SELF_HOSTED_START, PORT_MYSQL_SELF_HOSTED_END);
  //std::cout << "debug: " << G_STRFUNC << ":() : debug: Available port for self-hosting: " << available_port << std::endl;
  if(available_port == 0)
  {
    std::cerr << G_STRFUNC << ": No port was available between " << PORT_MYSQL_SELF_HOSTED_START << " and " << PORT_MYSQL_SELF_HOSTED_END << std::endl;
    return StartupErrors::FAILED_NO_PORT_AVAILABLE;
  }

  //TODO: Performance:
  const auto port_as_text = Glib::Ascii::dtostr(available_port);

  // Make sure to use double quotes for the executable path, because the
  // CreateProcess() API used on Windows does not support single quotes.
  const auto dbdir_pid = Glib::build_filename(dbdir, "pid");
  const auto dbdir_socket = Glib::build_filename(dbdir, "mysqld.sock");
  const auto command_mysql_start = get_path_to_mysql_executable("mysqld_safe")
                                  + " --no-defaults"
                                  + " --port=" + port_as_text
                                  + " --datadir=" + Glib::shell_quote(dbdir_data)
                                  + " --socket=" + Glib::shell_quote(dbdir_socket)
                                  + " --pid-file=" + Glib::shell_quote(dbdir_pid);
  //std::cout << G_STRFUNC << ": debug: command_mysql_start=" << command_mysql_start << std::endl;

  m_port = available_port; //Needed by get_mysqladmin_command().
  const auto command_check_mysql_has_started = get_mysqladmin_command(m_saved_username, m_saved_password) //TODO: Get the temporary password in a callback.
    + " ping";
  const std::string second_command_success_text = "mysqld is alive"; //TODO: This is not a stable API. Also, watch out for localisation.
  //std::cout << G_STRFUNC << ": debug: command_check_mysql_has_started=" << command_check_mysql_has_started << std::endl;

  if(!Glom::Spawn::execute_command_line_and_wait_until_second_command_returns_success(command_mysql_start, command_check_mysql_has_started, slot_progress, second_command_success_text))
  {
    m_port = 0;

    std::cerr << G_STRFUNC << "Error while attempting to self-host a MySQL database." << std::endl;
    return StartupErrors::FAILED_UNKNOWN_REASON;
  }

  m_port = available_port; //Remember it for later.

  //If necessary, set the initial root password and rename the root user:
  if(m_temporary_password_active)
  {
    if(m_initial_password_to_set.empty()) {
      //If this is empty then mysqladmin will ask for it on stdout, blocking us.
      std::cerr << G_STRFUNC << "Error while attempting to self-host a MySQL database: m_initial_password_to_set is empty." << std::endl;
      return StartupErrors::FAILED_UNKNOWN_REASON;
    }

    //Set the root password:
    const auto command_initdb_set_initial_password = get_mysqladmin_command("root", m_temporary_password)
      + " password " + Glib::shell_quote(m_initial_password_to_set);
    //std::cout << "debug: command_initdb_set_initial_password=" << command_initdb_set_initial_password << std::endl;

    if(!Glom::Spawn::execute_command_line_and_wait(command_initdb_set_initial_password, slot_progress))
    {
      std::cerr << G_STRFUNC << ": Error while attempting to start self-hosting MySQL database, when setting the initial password." << std::endl;
      return StartupErrors::FAILED_UNKNOWN_REASON;
    }

    m_temporary_password_active = false;
    m_temporary_password.clear();

    //Rename the root user,
    //so we can connnect as the expected username:
    //We connect to the INFORMATION_SCHEMA database, because libgda needs us to specify some database.
    const auto gda_connection = connect(DEFAULT_DATABASE_NAME, "root", m_initial_password_to_set);
    if(!gda_connection)
    {
      std::cerr << G_STRFUNC << "Error while attempting to start self-hosting MySQL database, when setting the initial username: connection failed." << std::endl;
      return StartupErrors::FAILED_UNKNOWN_REASON;
    }
    m_saved_password = m_initial_password_to_set;

    const auto query = build_query_change_username(gda_connection, "root", m_initial_username_to_set);
    //std::cout << G_STRFUNC << std::cout << "  DEBUG: rename user query=" << query << std::endl;

    try
    {
      /* const bool test = */ gda_connection->statement_execute_non_select(query);
      //This returns false even when the UPDATE succeeded,
      //but throws an exception when it fail.
      /*
      if(!test)
      {
        std::cerr << G_STRFUNC << "Error while attempting to start self-hosting MySQL database, when setting the initial username: UPDATE failed." << std::endl;
       return StartupErrors::FAILED_UNKNOWN_REASON;
      }
      */
    }
    catch(const Glib::Error& ex)
    {
      std::cerr << G_STRFUNC  << "Error while attempting to start self-hosting MySQL database, when setting the initial username: UPDATE failed: " << ex.what() << std::endl;
      return StartupErrors::FAILED_UNKNOWN_REASON;
    }
  }

  m_saved_username = m_initial_username_to_set;
  m_initial_username_to_set.clear();

  return StartupErrors::NONE;
}

//TODO: Avoid copy/paste with PostgresSelfHosted:
void MySQLSelfHosted::show_active_connections()
{
/* TODO_MySQL
  auto builder =
      Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
  builder->select_add_field("*", "pg_stat_activity");
  builder->select_add_target("pg_stat_activity");
 
  auto gda_connection = connect(m_saved_database_name, m_saved_username, m_saved_password);
  if(!gda_connection)
    std::cerr << G_STRFUNC << ": connection failed." << std::endl;
  
  auto datamodel = DbUtils::query_execute_select(builder);
  if(!datamodel)
    std::cerr << G_STRFUNC << ": pg_stat_activity SQL query failed." << std::endl;
  
  const auto rows_count = datamodel->get_n_rows(); 
  if(datamodel->get_n_rows() < 1)
    std::cerr << G_STRFUNC << ": pg_stat_activity SQL query returned no rows." << std::endl;

  std::cout << "Active connections according to a pg_stat_activity SQL query:" << std::endl;
  const auto cols_count = datamodel->get_n_columns();
  for(int row = 0; row < rows_count; ++row)
  {
    for(int col = 0; col < cols_count; ++col)
    {
      if(col != 0)
        std::cout << ", ";
        
      std::cout << datamodel->get_value_at(col, row).to_string();
    }
    
    std::cout << std::endl;
  }
  
  //Make sure that this connection does not stop a further attempt to stop the server.
  gda_connection->close();
*/
}

std::string MySQLSelfHosted::get_mysqladmin_command(const Glib::ustring& username, const Glib::ustring& password)
{
  if(username.empty())
  {
    std::cerr << G_STRFUNC << ": username is empty." << std::endl;
  }

  const auto port_as_text = Glib::Ascii::dtostr(m_port);

  auto command = get_path_to_mysql_executable("mysqladmin")
    + " --no-defaults"
    + " --port=" + port_as_text
    + " --protocol=tcp" //Otherwise we cannot connect as root. TODO: However, maybe we could use --skip-networking if network sharing is not enabled.
    + " --user=" + Glib::shell_quote(username);

  //--password='' is not always interpreted the same as specifying no --password.
  if(!password.empty())
    command += " --password=" + Glib::shell_quote(password);

  return command;
}

bool MySQLSelfHosted::cleanup(const SlotProgress& slot_progress)
{
  // This seems to be called twice sometimes, so we don't assert here until
  // this is fixed.
  //g_assert(get_self_hosting_active());

  if(!get_self_hosting_active())
  {
    //std::cout << G_STRFUNC << ": self-hosting is not active." << std::endl;
    return true; //Don't try to stop it if we have not started it.
  }

  const auto port_as_text = Glib::Ascii::dtostr(m_port);

  // -D specifies the data directory.
  // -c config_file= specifies the configuration file
  // -k specifies a directory to use for the socket. This must be writable by us.
  // We use "-m fast" instead of the default "-m smart" because that waits for clients to disconnect (and sometimes never succeeds).
  // TODO: Warn about connected clients on other computers? Warn those other users?
  // Make sure to use double quotes for the executable path, because the
  // CreateProcess() API used on Windows does not support single quotes.
  const auto command_mysql_stop = get_mysqladmin_command(m_saved_username, m_saved_password)
    + " shutdown";
  //std::cout << "DEBUGcleanup before shutdown: command=" << command_mysql_stop << std::endl;
  const auto result = Glom::Spawn::execute_command_line_and_wait(command_mysql_stop, slot_progress);

  //Give it time to succeed, because mysqladmin shutdown does not wait when using protocol=tcp.
  //TODO: Wait for a second command, such as myadmin ping, though its error message is less precise in this case.
  Glib::usleep(5000 * 1000);

  //std::cout << "DEBUGcleanup after shutdown: result=" << result << std::endl;

  if(!result)
  {
    std::cerr << G_STRFUNC << ": Error while attempting to stop self-hosting of the MySQL database. Trying again."  << std::endl;
    
    //Show open connections for debugging:
    try
    {
      show_active_connections();
    }
    catch(const Glib::Error& ex)
    {
      std::cerr << G_STRFUNC << ": exception while trying to show active MySQL connections: " << ex.what() << std::endl;
    }
    
    //I've seen it fail when running under valgrind, and there are reports of failures in bug #420962.
    //Maybe it will help to try again:
    if(!Glom::Spawn::execute_command_line_and_wait(command_mysql_stop, slot_progress))
    {
      std::cerr << G_STRFUNC << ": Error while attempting (for a second time) to stop self-hosting of the database."  << std::endl;
      return false;
    }
  }

  m_port = 0;

  return true;
}



bool MySQLSelfHosted::set_network_shared(const SlotProgress& /* slot_progress */, bool network_shared)
{
  //TODO: Use slot_progress, while doing async IO for create_text_file().

  m_network_shared = network_shared;

  return true;
}

static bool on_timeout_delay(const Glib::RefPtr<Glib::MainLoop>& mainloop)
{
  //Allow our mainloop.run() to return:
  if(mainloop)
    mainloop->quit();

  return false;
}


Glib::RefPtr<Gnome::Gda::Connection> MySQLSelfHosted::connect(const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, bool fake_connection)
{
  if(database.empty())
  {
    std::cerr << G_STRFUNC << ": The database name is empty. This is strange." << std::endl;
    return Glib::RefPtr<Gnome::Gda::Connection>();
  }

  if(!get_self_hosting_active())
  {
    throw ExceptionConnection(ExceptionConnection::failure_type::NO_BACKEND); //TODO: But there is a backend. It's just not ready.
  }

  Glib::RefPtr<Gnome::Gda::Connection> result;
  bool keep_trying = true;
  guint count_retries = 1;
  const guint MAX_RETRIES_KNOWN_PASSWORD = 20;
  const guint MAX_RETRIES_EVER = 20;
  while(keep_trying)
  {
    try
    {
      result = attempt_connect(port_as_string(m_port), database, username, password, fake_connection);
    }
    catch(const ExceptionConnection& ex)
    {
      if(ex.get_failure_type() == ExceptionConnection::failure_type::NO_SERVER)
      {
        //It must be using a default password, so any failure would not be due to a wrong password.
        //However, pg_ctl sometimes reports success before it is really ready to let us connect,
        //so in this case we can just keep trying until it works, with a very long timeout.
        count_retries++;
        const guint max_retries = m_network_shared ? MAX_RETRIES_EVER : MAX_RETRIES_KNOWN_PASSWORD;
        if(count_retries > max_retries)
        {
          keep_trying = false;
          continue;
        }

        std::cout << "debug: " << G_STRFUNC << ": Waiting and retrying the connection due to suspected too-early success of pg_ctl. retries=" << count_retries << ", max_retries=" << m_network_shared << std::endl;

        //Wait:
        auto mainloop = Glib::MainLoop::create(false);
          auto connection_timeout = Glib::signal_timeout().connect(
          sigc::bind(sigc::ptr_fun(&on_timeout_delay), sigc::ref(mainloop)),
          1000 /* 1 second */);
        mainloop->run();
        connection_timeout.disconnect();

        keep_trying = true;
        continue;
      }
      else
      {
        throw;
      }
    }

    keep_trying = false;
  }

  //Save the connection details _only_ for later debug use:
  m_saved_username = username;
  m_saved_password = password;
  return result;
}

bool MySQLSelfHosted::create_database(const SlotProgress& slot_progress, const Glib::ustring& database_name, const Glib::ustring& username, const Glib::ustring& password)
{
  return attempt_create_database(slot_progress, database_name, "localhost", port_as_string(m_port), username, password);
}

unsigned int MySQLSelfHosted::discover_first_free_port(unsigned int start_port, unsigned int end_port)
{
  //Open a socket so we can try to bind it to a port:
  const auto fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd == -1)
  {
#ifdef G_OS_WIN32
    std::cerr << G_STRFUNC << ": Create socket: " << WSAGetLastError() << std::endl;
#else
    perror("Create socket");
#endif //G_OS_WIN32
    return 0;
  }

  //This code was originally suggested by Lennart Poettering.

  struct ::sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;

  guint16 port_to_try = start_port;
  while (port_to_try <= end_port)
  {
    sa.sin_port = htons(port_to_try);

    const auto result = bind(fd, (sockaddr*)&sa, sizeof(sa));
    bool available = false;
    if(result == 0)
       available = true;
    else if(result < 0)
    {
      #ifdef G_OS_WIN32
      available = (WSAGetLastError() != WSAEADDRINUSE);
      #endif // G_OS_WIN32

      //Some BSDs don't have this.
      //But watch out - if you don't include errno.h then this won't be
      //defined on Linux either, but you really do need to check for it.
      #ifdef EADDRINUSE
      available = (errno != EADDRINUSE);
      #endif

      #ifdef EPORTINUSE //Linux doesn't have this.
      available = (errno != EPORTINUSE);
      #endif
    }
    else
    {
      //std::cout << "debug: " << G_STRFUNC << ": port in use: " << port_to_try << std::endl;
    }

    if(available)
    {
      #ifdef G_OS_WIN32
      closesocket(fd);
      #else
      close(fd);
      #endif //G_OS_WIN32

      //std::cout << "debug: " << G_STRFUNC << ": Found: returning " << port_to_try << std::endl;
      return port_to_try;
    }

    ++port_to_try;
  }

#ifdef G_OS_WIN32
  closesocket(fd);
#else
  close(fd);
#endif //G_OS_WIN32

  std::cerr << G_STRFUNC << ": No port was available." << std::endl;
  return 0;
}

} // namespace ConnectionPoolBackends

} // namespcae Glom
