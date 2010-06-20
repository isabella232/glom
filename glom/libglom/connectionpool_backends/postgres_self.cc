/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
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

#include <config.h> // For POSTGRES_UTILS_PATH

#include <libglom/connectionpool_backends/postgres_self.h>
#include <libglom/utils.h>
#include <libglom/spawn_with_feedback.h>
#include <giomm.h>
#include <glib/gstdio.h> // For g_remove

#include <glibmm/i18n.h>

#include <libglom/gst-package.h>
#include <sstream> //For stringstream

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

namespace
{

static Glib::ustring port_as_string(int port_num)
{
  Glib::ustring result;
  char* cresult = g_strdup_printf("%d", port_num);
  if(cresult)
    result = cresult;
  g_free(cresult);

  return result;
}

} // anonymous namespace

namespace Glom
{

namespace ConnectionPoolBackends
{

//TODO: Do we need these sameuser lines?

// We need both <=8.3 and >=8.4 versions, because the ident line changed syntax 
// incompatibly: http://www.postgresql.org/about/press/features84#security
 
#define DEFAULT_CONFIG_PG_HBA_LOCAL_8p3 \
"# TYPE  DATABASE    USER        CIDR-ADDRESS          METHOD\n" \
"\n" \
"# local is for Unix domain socket connections only\n" \
"# trust allows connection from the current PC without a password:\n" \
"local   all         all                               trust\n" \
"local   all         all                               ident sameuser\n" \
"local   all         all                               md5\n" \
"\n" \
"# TCP connections from the same computer, with a password:\n" \
"host    all         all         127.0.0.1    255.255.255.255    md5\n" \
"# IPv6 local connections:\n" \
"host    all         all         ::1/128               md5\n"

#define DEFAULT_CONFIG_PG_HBA_LOCAL_8p4 \
"# TYPE  DATABASE    USER        CIDR-ADDRESS          METHOD\n" \
"\n" \
"# local is for Unix domain socket connections only\n" \
"# trust allows connection from the current PC without a password:\n" \
"local   all         all                               trust\n" \
"local   all         all                               ident\n" \
"local   all         all                               md5\n" \
"\n" \
"# TCP connections from the same computer, with a password:\n" \
"host    all         all         127.0.0.1    255.255.255.255    md5\n" \
"# IPv6 local connections:\n" \
"host    all         all         ::1/128               md5\n"

#define DEFAULT_CONFIG_PG_HBA_REMOTE_EXTRA \
"\n" \
"# IPv4 local connections:\n" \
"host    all         all         0.0.0.0/0          md5\n" \
"# IPv6 local connections:\n" \
"host    all         all         ::1/128               md5\n"

#define PORT_POSTGRESQL_SELF_HOSTED_START 5433
#define PORT_POSTGRESQL_SELF_HOSTED_END 5500


#define DEFAULT_CONFIG_PG_HBA_REMOTE_8p3 \
DEFAULT_CONFIG_PG_HBA_LOCAL_8p3 \
DEFAULT_CONFIG_PG_HBA_REMOTE_EXTRA

#define DEFAULT_CONFIG_PG_HBA_REMOTE_8p4 \
DEFAULT_CONFIG_PG_HBA_LOCAL_8p3 \
DEFAULT_CONFIG_PG_HBA_REMOTE_EXTRA

#define PORT_POSTGRESQL_SELF_HOSTED_START 5433
#define PORT_POSTGRESQL_SELF_HOSTED_END 5500

#define DEFAULT_CONFIG_PG_IDENT ""

PostgresSelfHosted::PostgresSelfHosted()
: m_port(0),
   m_network_shared(false)
{
}


std::string PostgresSelfHosted::get_path_to_postgres_executable(const std::string& program)
{
#ifdef G_OS_WIN32
  // Add the .exe extension on Windows:
  std::string real_program = program + EXEEXT;
    
  // Have a look at the bin directory of the application executable first.
  // The installer installs postgres there. postgres needs to be installed
  // in a directory called bin for its relocation stuff to work, so that
  // it finds the share data in share. Unfortunately it does not look into
  // share/postgresql which would be nice to separate the postgres stuff
  // from the other shared data. We can perhaps still change this later by
  // building postgres with another prefix than /local/pgsql.
  gchar* installation_directory = g_win32_get_package_installation_directory_of_module(0);
  std::string test = Glib::build_filename(installation_directory, Glib::build_filename("bin", real_program));
  g_free(installation_directory);

  if(Glib::file_test(test, Glib::FILE_TEST_IS_EXECUTABLE))
    return test;

  // Look in PATH otherwise
  return Glib::find_program_in_path(real_program);
#else // G_OS_WIN32
  return Glib::build_filename(POSTGRES_UTILS_PATH, program + EXEEXT);
#endif // !G_OS_WIN32
}

void PostgresSelfHosted::set_self_hosting_data_uri(const std::string& data_uri)
{
  if(m_self_hosting_data_uri != data_uri)
  {
    // Can't change data uri if we are running the server on another data uri
    g_assert(!get_self_hosting_active());
    m_self_hosting_data_uri = data_uri;
  }
}

bool PostgresSelfHosted::get_self_hosting_active() const
{
  return m_port != 0;
}

int PostgresSelfHosted::get_port() const
{
  return m_port;
}

/** Try to install postgres on the distro, though this will require a
 * distro-specific patch to the implementation.
 */
bool PostgresSelfHosted::install_postgres(const SlotProgress& /* slot_progress */)
{
#if 0
  // This  is example code for Ubuntu, and possibly Debian,
  // using code from the gnome-system-tools Debian/Ubuntu patches.
  // (But please, just fix the dependencies instead. PostgreSQL is not optional.)
  //
  // You will also need to remove the "ifdef 0"s around the code in gst-package.[h|c],
  // and define DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED above.

  //Careful. Maybe you want a different version.
  //Also, Glom will start its own instance of PostgreSQL, on its own port, when it needs to,
  //so there is no need to start a Glom service after installation at system startup, 
  //though it will not hurt Glom if you do that.
  const gchar *packages[] = { "postgresql-8.1", 0 };
  const bool result = gst_packages_install(parent_window->gobj() /* parent window */, packages);
  if(result)
  {
    std::cout << "Glom: gst_packages_install() reports success." << std::endl;
    //Double-check, because gst_packages_install() incorrectly returns TRUE if it fails because 
    //a) synaptic is already running, or
    //b) synaptic did not know about the package (no warning is shown in this case.)
    //Maybe gst_packages_install() never returns FALSE.
    return check_postgres_is_available_with_warning(); //This is recursive, but clicking Cancel will stop everything.
  }
  else
  {
    std::cout << "Glom: gst_packages_install() reports failure." << std::endl;
    return false; //Failed to install postgres.
  }
#else
  return false; //Failed to install postgres because no installation technique was implemented.
#endif // #if 0
}

Backend::InitErrors PostgresSelfHosted::initialize(const SlotProgress& slot_progress, const Glib::ustring& initial_username, const Glib::ustring& password, bool network_shared)
{
  m_network_shared = network_shared;

  if(m_self_hosting_data_uri.empty())
  {
    std::cerr << "PostgresSelfHosted::initialize: m_self_hosting_data_uri is empty." << std::endl;
    return INITERROR_OTHER;
  }
  
  if(initial_username.empty())
  {
    std::cerr << "PostgresSelfHosted::initialize(). Username was empty while attempting to create self-hosting database" << std::endl;
    return INITERROR_OTHER;
  }

  //Get the filepath of the directory that we should create:
  const std::string dbdir_uri = m_self_hosting_data_uri;
  //std::cout << "debug: dbdir_uri=" << dbdir_uri << std::endl;

  if(directory_exists_uri(dbdir_uri))
    return INITERROR_DIRECTORY_ALREADY_EXISTS;

  const std::string dbdir = Glib::filename_from_uri(dbdir_uri);
  //std::cout << "debug: dbdir=" << dbdir << std::endl;
  g_assert(!dbdir.empty());

  //0770 means "this user and his group can read and write and "execute" (meaning add sub-files) this non-executable file".
  //The 0 prefix means that this is octal.
  int mkdir_succeeded = g_mkdir_with_parents(dbdir.c_str(), 0770);
  if(mkdir_succeeded == -1)
  {
    std::cerr << "Error from g_mkdir_with_parents() while trying to create directory: " << dbdir << std::endl;
    perror("Error from g_mkdir_with_parents");
    
    return INITERROR_COULD_NOT_CREATE_DIRECTORY;
  }

  //Create the config directory:
  const std::string dbdir_config = dbdir + "/config";
  mkdir_succeeded = g_mkdir_with_parents(dbdir_config.c_str(), 0770);
  if(mkdir_succeeded == -1)
  {
    std::cerr << "Error from g_mkdir_with_parents() while trying to create directory: " << dbdir_config << std::endl;
    perror("Error from g_mkdir_with_parents");

    return INITERROR_COULD_NOT_CREATE_DIRECTORY;
  }

  //Create these files: environment  pg_hba.conf  pg_ident.conf  start.conf
  set_network_shared(slot_progress, m_network_shared); //Creates pg_hba.conf and pg_ident.conf

  //Check that there is not an existing data directory:
  const std::string dbdir_data = dbdir + "/data";
  mkdir_succeeded = g_mkdir_with_parents(dbdir_data.c_str(), 0770);
  g_assert(mkdir_succeeded != -1);

  
  // initdb creates a new postgres database cluster:

  //Get file:// URI for the tmp/ directory:
  const std::string temp_pwfile = Glib::build_filename(Glib::get_tmp_dir(), "glom_initdb_pwfile");
  Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(temp_pwfile);
  const std::string temp_pwfile_uri = file->get_uri();
  const bool pwfile_creation_succeeded = create_text_file(temp_pwfile_uri, password);
  g_assert(pwfile_creation_succeeded);

  // Make sure to use double quotes for the executable path, because the
  // CreateProcess() API used on Windows does not support single quotes.
  const std::string command_initdb = "\"" + get_path_to_postgres_executable("initdb") + "\" -D \"" + dbdir_data + "\"" +
                                        " -U " + initial_username + " --pwfile=\"" + temp_pwfile + "\"";

  //Note that --pwfile takes the password from the first line of a file. It's an alternative to supplying it when prompted on stdin.
  const bool result = Glom::Spawn::execute_command_line_and_wait(command_initdb, slot_progress);
  if(!result)
  {
    std::cerr << "Error while attempting to create self-hosting database." << std::endl;
  }

  const int temp_pwfile_removed = g_remove(temp_pwfile.c_str()); //Of course, we don't want this to stay around. It would be a security risk.
  g_assert(temp_pwfile_removed == 0);
 
  return result ? INITERROR_NONE : INITERROR_COULD_NOT_START_SERVER;
}

Glib::ustring PostgresSelfHosted::get_postgresql_utils_version(const SlotProgress& slot_progress)
{
  Glib::ustring result;

  const std::string command = "\"" + get_path_to_postgres_executable("pg_ctl") + "\" --version";

  //The first command does not return, but the second command can check whether it succeeded:
  std::string output;
  const bool spawn_result = Glom::Spawn::execute_command_line_and_wait(command, slot_progress, output);
  if(!spawn_result)
  {
    std::cerr << "Error while attempting to discover the pg_ctl version." << std::endl;
    return result;
  }

  //Use a regex to get the version number:
  Glib::RefPtr<Glib::Regex> regex;

  //We want the characters at the end:  
  const gchar* VERSION_REGEX = "pg_ctl \\(PostgreSQL\\) (.*)";

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    regex = Glib::Regex::create(VERSION_REGEX);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "Glom: Glib::Regex::create() failed: " << ex.what() << std::endl;
    return result;
  } 
  #else
  std::auto_ptr<Glib::Error> ex;
  regex = Glib::Regex::create(VERSION_REGEX, static_cast<Glib::RegexCompileFlags>(0), static_cast<Glib::RegexMatchFlags>(0), ex);
  if(ex.get())
  {
    std::cerr << "Glom: Glib::Regex::create() failed: " << ex->what() << std::endl;
    return result;
  }
  #endif
 
  if(!regex)
    return result;

  typedef std::vector<Glib::ustring> type_vec_strings;
  const type_vec_strings vec = regex->split(output, Glib::REGEX_MATCH_NOTEMPTY);
  //std::cout << "DEBUG: output == " << output << std::endl;
  //std::cout << "DEBUG: vec.size() == " << vec.size() << std::endl;

  // We get, for instance, "\n" and 8.4.1" and "\n".
  for(type_vec_strings::const_iterator iter = vec.begin();
       iter != vec.end();
       ++iter)
  {
    const Glib::ustring str = *iter;
    if(!str.empty())
      return str; //Found.
  }
 
  return result;
}

float PostgresSelfHosted::get_postgresql_utils_version_as_number(const SlotProgress& slot_progress)
{
  float result = 0;

  const Glib::ustring version_str = get_postgresql_utils_version(slot_progress);

  Glib::RefPtr<Glib::Regex> regex;

  //We want the characters at the end:  
  const gchar* VERSION_REGEX = "^(\\d*)\\.(\\d*)";

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    regex = Glib::Regex::create(VERSION_REGEX);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "Glom: Glib::Regex::create() failed: " << ex.what() << std::endl;
    return result;
  } 
  #else
  std::auto_ptr<Glib::Error> ex;
  regex = Glib::Regex::create(VERSION_REGEX, static_cast<Glib::RegexCompileFlags>(0), static_cast<Glib::RegexMatchFlags>(0), ex);
  if(ex.get())
  {
    std::cerr << "Glom: Glib::Regex::create() failed: " << ex->what() << std::endl;
    return result;
  }
  #endif
 
  if(!regex)
    return result;

  typedef std::vector<Glib::ustring> type_vec_strings;
  const type_vec_strings vec = regex->split(version_str, Glib::REGEX_MATCH_NOTEMPTY);
  //std::cout << "DEBUG: str == " << version_str << std::endl;
  //std::cout << "DEBUG: vec.size() == " << vec.size() << std::endl;

  //We need to loop over the numbers because we get some "" items that we want to ignore:
  guint count = 0; //We want 2 numbers.
  for(type_vec_strings::const_iterator iter = vec.begin();
       iter != vec.end();
       ++iter)
  {
    //std::cout << "regex item: START" << *iter << "END" << std::endl;

    const Glib::ustring str = *iter;
    if(str.empty())
      continue;

    const float num = atoi(str.c_str());
    if(count == 0)
      result = num;
    else if(count == 1)
    {
      result += (0.1 * num);
      break;
    }

    ++count;
  }

  return result;
}


bool PostgresSelfHosted::startup(const SlotProgress& slot_progress, bool network_shared)
{
  m_network_shared = network_shared;

  // Don't risk random crashes, although this really shouldn't be called
  // twice of course.
  //g_assert(!get_self_hosting_active());

  if(get_self_hosting_active())
    return true; //Just do it once.

  const std::string dbdir_uri = m_self_hosting_data_uri;

  if(!(directory_exists_uri(dbdir_uri)))
  {
    //TODO: Use a return enum or exception so we can tell the user about this:
    std::cerr << "ConnectionPool::create_self_hosting(): The data directory could not be found: " << dbdir_uri << std::endl;
    return false;
  }

  //Attempt to ensure that the config files are correct:
  set_network_shared(slot_progress, m_network_shared); //Creates pg_hba.conf and pg_ident.conf

  const std::string dbdir = Glib::filename_from_uri(dbdir_uri);
  g_assert(!dbdir.empty());

  const std::string dbdir_data = Glib::build_filename(dbdir, "data");
  const std::string dbdir_data_uri = Glib::filename_to_uri(dbdir_data);
  if(!(directory_exists_uri(dbdir_data_uri)))
  {
    //TODO: Use a return enum or exception so we can tell the user about this:
    std::cerr << "ConnectionPool::create_self_hosting(): The data sub-directory could not be found." << dbdir_data_uri << std::endl;
    return false;
  }


  const int available_port = discover_first_free_port(PORT_POSTGRESQL_SELF_HOSTED_START, PORT_POSTGRESQL_SELF_HOSTED_END);
  //std::cout << "ConnectionPool::create_self_hosting():() : debug: Available port for self-hosting: " << available_port << std::endl;
  if(available_port == 0)
  {
    //TODO: Use a return enum or exception so we can tell the user about this:
    std::cerr << "ConnectionPool::create_self_hosting(): No port was available between " << PORT_POSTGRESQL_SELF_HOSTED_START << " and " << PORT_POSTGRESQL_SELF_HOSTED_END << std::endl;
    return false;
  }

  //TODO: Performance:
  const std::string port_as_text = Glib::Ascii::dtostr(available_port);

  // -D specifies the data directory.
  // -c config_file= specifies the configuration file
  // -k specifies a directory to use for the socket. This must be writable by us.
  // POSTGRES_UTILS_PATH is defined in config.h, based on the configure.
  // Make sure to use double quotes for the executable path, because the
  // CreateProcess() API used on Windows does not support single quotes.
  const std::string command_postgres_start = "\"" + get_path_to_postgres_executable("postgres") + "\" -D \"" + dbdir_data + "\" "
                                  + " -p " + port_as_text
                                  + " -i " //Equivalent to -h "*", which in turn is equivalent to listen_addresses in postgresql.conf. Listen to all IP addresses, so any client can connect (with a username+password)
                                  + " -c hba_file=\"" + dbdir + "/config/pg_hba.conf\""
                                  + " -c ident_file=\"" + dbdir + "/config/pg_ident.conf\""
                                  + " -k \"" + dbdir + "\""
                                  + " --external_pid_file=\"" + dbdir + "/pid\"";

  // Make sure to use double quotes for the executable path, because the
  // CreateProcess() API used on Windows does not support single quotes.
  const std::string command_check_postgres_has_started = "\"" + get_path_to_postgres_executable("pg_ctl") + "\" status -D \"" + dbdir_data + "\"";

  //For postgres 8.1, this is "postmaster is running".
  //For postgres 8.2, this is "server is running".
  //This is a big hack that we should avoid. murrayc.
  //
  //pg_ctl actually seems to return a 0 result code for "is running" and a 1 for not running, at least with Postgres 8.2,
  //so maybe we can avoid this in future.  
  //Please do test it with your postgres version, using "echo $?" to see the result code of the last command.
  const std::string second_command_success_text = "is running"; //TODO: This is not a stable API. Also, watch out for localisation.

  //The first command does not return, but the second command can check whether it succeeded:
  const bool result = Glom::Spawn::execute_command_line_and_wait_until_second_command_returns_success(command_postgres_start, command_check_postgres_has_started, slot_progress, second_command_success_text);
  if(!result)
  {
    std::cerr << "Error while attempting to self-host a database." << std::endl;
    return false;
  }

  m_port = available_port; //Remember it for later.
  
  return true;
}

bool PostgresSelfHosted::cleanup(const SlotProgress& slot_progress)
{
  // This seems to be called twice sometimes, so we don't assert here until
  // this is fixed.
  //g_assert(get_self_hosting_active());

  if(!get_self_hosting_active())
    return true; //Don't try to stop it if we have not started it.

  const std::string dbdir_uri = m_self_hosting_data_uri;
  const std::string dbdir = Glib::filename_from_uri(dbdir_uri);
  g_assert(!dbdir.empty());

  const std::string dbdir_data = Glib::build_filename(dbdir, "data");


  // TODO: Detect other instances on the same computer, and use a different port number, 
  // or refuse to continue, showing an error dialog.

  // -D specifies the data directory.
  // -c config_file= specifies the configuration file
  // -k specifies a directory to use for the socket. This must be writable by us.
  // POSTGRES_UTILS_PATH is defined in config.h, based on the configure.
  // We use "-m fast" instead of the default "-m smart" because that waits for clients to disconnect (and sometimes never succeeds).
  // TODO: Warn about connected clients on other computers? Warn those other users?
  // Make sure to use double quotes for the executable path, because the
  // CreateProcess() API used on Windows does not support single quotes.
  const std::string command_postgres_stop = "\"" + get_path_to_postgres_executable("pg_ctl") + "\" -D \"" + dbdir_data + "\" stop -m fast";
  const bool result = Glom::Spawn::execute_command_line_and_wait(command_postgres_stop, slot_progress);
  if(!result)
  {
    std::cerr << "Error while attempting to stop self-hosting of the database. Trying again."  << std::endl;

    //I've seen it fail when running under valgrind, and there are reports of failures in bug #420962.
    //Maybe it will help to try again:
    const bool result = Glom::Spawn::execute_command_line_and_wait(command_postgres_stop, slot_progress);
    if(!result)
    {
      std::cerr << "Error while attempting (for a second time) to stop self-hosting of the database."  << std::endl;
      return false;
    }
  }

  m_port = 0;

  return true;
}



bool PostgresSelfHosted::set_network_shared(const SlotProgress& slot_progress, bool network_shared)
{
  //TODO: Use slot_progress, while doing async IO for create_text_file().

  m_network_shared = network_shared;

  const std::string dbdir_uri = m_self_hosting_data_uri;
  const std::string dbdir = Glib::filename_from_uri(dbdir_uri);

  const std::string dbdir_uri_config = dbdir_uri + "/config";
  const char* default_conf_contents = 0;

  // Choose the configuration contents based on the postgresql version 
  // and whether we want to be network-shared:
  const float postgresql_version = get_postgresql_utils_version_as_number(slot_progress);
  //std::cout << "DEBUG: postgresql_version=" << postgresql_version << std::endl;

  if(postgresql_version >= 8.4f)
    default_conf_contents = m_network_shared ? DEFAULT_CONFIG_PG_HBA_REMOTE_8p4 : DEFAULT_CONFIG_PG_HBA_LOCAL_8p4;
  else
    default_conf_contents = m_network_shared ? DEFAULT_CONFIG_PG_HBA_REMOTE_8p3 : DEFAULT_CONFIG_PG_HBA_LOCAL_8p3;

  //std::cout << "DEBUG: default_conf_contents=" << default_conf_contents << std::endl;

  const bool hba_conf_creation_succeeded = create_text_file(dbdir_uri_config + "/pg_hba.conf", default_conf_contents);
  g_assert(hba_conf_creation_succeeded);
  if(!hba_conf_creation_succeeded)
    return false;

  const bool ident_conf_creation_succeeded = create_text_file(dbdir_uri_config + "/pg_ident.conf", DEFAULT_CONFIG_PG_IDENT);
  g_assert(ident_conf_creation_succeeded);

  return hba_conf_creation_succeeded;
}

static bool on_timeout_delay(const Glib::RefPtr<Glib::MainLoop>& mainloop)
{
  //Allow our mainloop.run() to return:
  if(mainloop)
    mainloop->quit();
    
  return false;
}


Glib::RefPtr<Gnome::Gda::Connection> PostgresSelfHosted::connect(const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password)
{
  if(!get_self_hosting_active())
  {
    throw ExceptionConnection(ExceptionConnection::FAILURE_NO_BACKEND); //TODO: But there is a backend. It's just not ready.
    return Glib::RefPtr<Gnome::Gda::Connection>();
  }

  std::auto_ptr<ExceptionConnection> ex;

  Glib::RefPtr<Gnome::Gda::Connection> result;
  bool keep_trying = true;
  guint count_retries = 0;
  const guint MAX_RETRIES_KNOWN_PASSWORD = 30; /* seconds */
  const guint MAX_RETRIES_EVER = 60; /* seconds */
  while(keep_trying)
  { 
    try
    {
      result = attempt_connect("localhost", port_as_string(m_port), database, username, password);
    }
    catch(const ExceptionConnection& ex)
    {
      if(ex.get_failure_type() == ExceptionConnection::FAILURE_NO_SERVER)
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
      }

      std::cout << "DEBUG: Glom::PostgresSelfHosted::connect(): Waiting and retrying the connection due to suspected too-early success of pg_ctl." << std::endl; 

      //Wait:
      Glib::RefPtr<Glib::MainLoop> mainloop = Glib::MainLoop::create(false);
        sigc::connection connection_timeout = Glib::signal_timeout().connect(
        sigc::bind(sigc::ptr_fun(&on_timeout_delay), sigc::ref(mainloop)), 
        1000 /* 1 second */);
      mainloop->run();
      connection_timeout.disconnect();
      
      keep_trying = true;
      continue;
    }
    
    keep_trying = false;
  }

  return result;
}

bool PostgresSelfHosted::create_database(const Glib::ustring& database_name, const Glib::ustring& username, const Glib::ustring& password)
{
  return attempt_create_database(database_name, "localhost", port_as_string(m_port), username, password);
}

int PostgresSelfHosted::discover_first_free_port(int start_port, int end_port)
{
  //Open a socket so we can try to bind it to a port:
  const int fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd == -1)
  {
#ifdef G_OS_WIN32
    std::cerr << "Create socket: " << WSAGetLastError() << std::endl;
#else
    perror("Create socket");
#endif
    return 0;
  }

  //This code was originally suggested by Lennart Poettering.

  struct ::sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;

  int port_to_try = start_port;
  while (port_to_try <= end_port)
  {
    sa.sin_port = htons(port_to_try);

    const int result = bind(fd, (sockaddr*)&sa, sizeof(sa));
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
      //std::cout << "debug: ConnectionPool::discover_first_free_port(): port in use: " << port_to_try << std::endl;
    }

    if(available)
    {
      #ifdef G_OS_WIN32
      closesocket(fd);
      #else
      close(fd);
      #endif //G_OS_WIN32

      //std::cout << "debug: ConnectionPool::discover_first_free_port(): Found: returning " << port_to_try << std::endl;
      return port_to_try;
    }

    ++port_to_try;
  }

#ifdef G_OS_WIN32
  closesocket(fd);
#else
  close(fd);
#endif

  std::cerr << "debug: ConnectionPool::discover_first_free_port(): No port was available." << std::endl;
  return 0;
}

bool PostgresSelfHosted::create_text_file(const std::string& file_uri, const std::string& contents)
{
  if(file_uri.empty())
    return false;

  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(file_uri);
  Glib::RefPtr<Gio::FileOutputStream> stream;

  //Create the file if it does not already exist:
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    if(file->query_exists())
    {
      stream = file->replace(); //Instead of append_to().
    }
    else
    {
      //By default files created are generally readable by everyone, but if we pass FILE_CREATE_PRIVATE in flags the file will be made readable only to the current user, to the level that is supported on the target filesystem.
      //TODO: Do we want to specify 0660 exactly? (means "this user and his group can read and write this non-executable file".)
      stream = file->create_file();
    }
  }
  catch(const Gio::Error& ex)
  {
#else
  std::auto_ptr<Gio::Error> error;
  stream.create(error);
  if(error.get())
  {
    const Gio::Error& ex = *error.get();
#endif
    // If the operation was not successful, print the error and abort
    std::cerr << "ConnectionPool::create_text_file(): exception while creating file." << std::endl
      << "  file uri:" << file_uri << std::endl
      << "  error:" << ex.what() << std::endl;
    return false; // print_error(ex, output_uri_string);
  }


  if(!stream)
    return false;


  gsize bytes_written = 0;
  const std::string::size_type contents_size = contents.size();
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    //Write the data to the output uri
    bytes_written = stream->write(contents.data(), contents_size);
  }
  catch(const Gio::Error& ex)
  {
#else
  bytes_written = stream->write(contents.data(), contents_size, error);
  if(error.get())
  {
    Gio::Error& ex = *error.get();
#endif
    // If the operation was not successful, print the error and abort
    std::cerr << "ConnectionPool::create_text_file(): exception while writing to file." << std::endl
      << "  file uri:" << file_uri << std::endl
      << "  error:" << ex.what() << std::endl;
    return false; //print_error(ex, output_uri_string);
  }

  if(bytes_written != contents_size)
  {
    std::cerr << "ConnectionPool::create_text_file(): not all bytes written when writing to file." << std::endl
      << "  file uri:" << file_uri << std::endl;
    return false;
  }

  return true; //Success.
}

bool PostgresSelfHosted::directory_exists_uri(const std::string& uri)
{
  if(uri.empty())
    return false;

  const Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
  return file && file->query_exists();
}

} // namespace ConnectionPoolBackends

} // namespcae Glom
