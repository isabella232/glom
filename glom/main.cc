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

#include "config.h"

//We use Python for calculated fields.
//#include <Python.h> //Include it before anything else to avoid "_POSIX_C_SOURCE redefined".
#include <boost/python.hpp>

//#include <gnome.h>
#include <gtkmm/messagedialog.h>
#include <glom/libglom/init.h>
#include <glom/glade_utils.h>
#include <gtkmm/main.h>

#include <giomm/file.h>
#include <glibmm/optioncontext.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>

// For postgres availability checks:
#ifdef GLOM_ENABLE_POSTGRESQL
#include <libglom/connectionpool_backends/postgres.h>
#ifndef GLOM_ENABLE_CLIENT_ONLY
#include <libglom/connectionpool_backends/postgres_self.h>
#endif //GLOM_ENABLE_CLIENT_ONLY
#endif //GLOM_ENABLE_POSTGRESQL

// For sanity checks:
#include <libglom/data_structure/glomconversions.h>
#include <glom/python_embed/glom_python.h>

#ifndef GLOM_ENABLE_CLIENT_ONLY
#include <gtksourceviewmm/init.h>
#include <goocanvasmm/init.h>
#endif // !GLOM_ENABLE_CLIENT_ONLY

#include <glom/application.h>
#include <glom/glade_utils.h>
#include <glom/utils_ui.h>

#include <evince-view.h>

#include <glibmm/i18n.h>

#ifdef G_OS_WIN32
#include <winsock2.h>
#else
#include <fontconfig/fontconfig.h> //For cleanup.
#endif

namespace
{

#ifdef G_OS_WIN32
static BOOL
pgwin32_get_dynamic_tokeninfo(HANDLE token, TOKEN_INFORMATION_CLASS class_,
                char **InfoBuffer, char *errbuf, int errsize)
{
  DWORD    InfoBufferSize;

  if(GetTokenInformation(token, class_, 0, 0, &InfoBufferSize))
  {
    snprintf(errbuf, errsize, "could not get token information: got zero size\n");
    return false;
  }

  if(GetLastError() != ERROR_INSUFFICIENT_BUFFER)
  {
    snprintf(errbuf, errsize, "could not get token information: error code %d\n",
         (int) GetLastError());
    return false;
  }

  *InfoBuffer = static_cast<char*>(malloc(InfoBufferSize));
  if(*InfoBuffer == 0)
  {
    snprintf(errbuf, errsize, "could not allocate %d bytes for token information\n",
         (int) InfoBufferSize);
    return false;
  }

  if(!GetTokenInformation(token, class_, *InfoBuffer,
               InfoBufferSize, &InfoBufferSize))
  {
    snprintf(errbuf, errsize, "could not get token information: error code %d\n",
         (int) GetLastError());
    return false;
  }

  return true;
}

int
pgwin32_is_admin(void)
{
  HANDLE    AccessToken;
  char     *InfoBuffer = 0;
  char    errbuf[256];
  PTOKEN_GROUPS Groups;
  PSID    AdministratorsSid;
  PSID    PowerUsersSid;
  SID_IDENTIFIER_AUTHORITY NtAuthority = {SECURITY_NT_AUTHORITY};
  UINT    x;
  BOOL    success;

  if(!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &AccessToken))
  {
    throw std::runtime_error(Glib::ustring::compose("Could not open process token: error code %1", (int)GetLastError()));
  }

  if(!pgwin32_get_dynamic_tokeninfo(AccessToken, TokenGroups,
                     &InfoBuffer, errbuf, sizeof(errbuf)))
  {
    CloseHandle(AccessToken);
    throw std::runtime_error(errbuf);
  }

  Groups = (PTOKEN_GROUPS) InfoBuffer;

  CloseHandle(AccessToken);

  if(!AllocateAndInitializeSid(&NtAuthority, 2,
     SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0,
                  0, &AdministratorsSid))
  {
    free(InfoBuffer);
    throw std::runtime_error(Glib::ustring::compose("could not get SID for Administrators group: error code %1", (int)GetLastError()));
  }

  if(!AllocateAndInitializeSid(&NtAuthority, 2,
  SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_POWER_USERS, 0, 0, 0, 0, 0,
                  0, &PowerUsersSid))
  {
    free(InfoBuffer);
    FreeSid(AdministratorsSid);
    throw std::runtime_error(Glib::ustring::compose("could not get SID for PowerUsers group: error code %1", (int) GetLastError()));
  }

  success = false;

  for (x = 0; x < Groups->GroupCount; ++x)
  {
    if((EqualSid(AdministratorsSid, Groups->Groups[x].Sid) && (Groups->Groups[x].Attributes & SE_GROUP_ENABLED)) ||
      (EqualSid(PowerUsersSid, Groups->Groups[x].Sid) && (Groups->Groups[x].Attributes & SE_GROUP_ENABLED)))
    {
      success = true;
      break;
    }
  }

  free(InfoBuffer);
  FreeSid(AdministratorsSid);
  FreeSid(PowerUsersSid);
  return success;
}

#endif // G_OS_WIN32
} // anonymous namespace

namespace Glom
{

class OptionGroup : public Glib::OptionGroup
{
public:
  OptionGroup();

  //These int instances should live as long as the OptionGroup to which they are added,
  //and as long as the OptionContext to which those OptionGroups are added.
  std::string m_arg_filename;
  bool m_arg_version;
  bool m_arg_restore;
  bool m_arg_stop_auto_server_shutdown;
  bool m_arg_debug_sql;
  bool m_arg_debug_date_check;
};

OptionGroup::OptionGroup()
: Glib::OptionGroup("Glom", _("Glom options"), _("Command-line options for glom")),
  m_arg_version(false),
  m_arg_restore(false),
  m_arg_stop_auto_server_shutdown(false),
  m_arg_debug_sql(false),
  m_arg_debug_date_check(false)
{
  Glib::OptionEntry entry;
  entry.set_long_name("file");
  entry.set_short_name('f');
  entry.set_description(_("The Filename"));
  add_entry_filename(entry, m_arg_filename);

  entry.set_long_name("version");
  entry.set_short_name('V');
  entry.set_description(_("The version of this application."));
  add_entry(entry, m_arg_version);

  entry.set_long_name("restore");
  entry.set_short_name(0);
  entry.set_description(_("Whether the filename is a .tar.gz backup to be restored."));
  add_entry(entry, m_arg_restore);

  entry.set_long_name("stop-auto-server-shutdown");
  entry.set_short_name(0);
  entry.set_description(_("Do not automatically stop the database server if Glom quits. This is helpful for debugging with gdb."));
  add_entry(entry, m_arg_stop_auto_server_shutdown);

  entry.set_long_name("debug_sql");
  entry.set_short_name(0);
  entry.set_description(_("Show the generated SQL queries on stdout, for debugging."));
  add_entry(entry, m_arg_debug_sql);

  entry.set_long_name("debug-date-check");
  entry.set_short_name(0);
  entry.set_description(_("Show how Glom outputs a date in this locale, then stop."));
  add_entry(entry, m_arg_debug_date_check);
}

#ifdef GLOM_ENABLE_POSTGRESQL
bool check_user_is_not_root_with_warning()
{
  Glib::ustring message;
#ifdef G_OS_WIN32
  try
  {
    if(pgwin32_is_admin())
    {
      message = _("You seem to be running Glom as a user with administrator privileges. Glom may not be run with such privileges for security reasons.\nPlease login to your system as a normal user.");
    }
  }
  catch(const std::runtime_error& ex)
  {
    message = ex.what();
  }
#else
  //std::cout << "debug: " << G_STRFUNC << ": geteuid()=" << geteuid() << ", getgid()=" << getgid() << std::endl;

  //This is very linux-specific. We should ifdef this out for other platforms.
  if(geteuid() == 0)
  {
    //Warn the user:
    message = _("You seem to be running Glom as root. Glom may not be run as root.\nPlease login to your system as a normal user.");
  }
#endif

  if(!message.empty())
  {
    Gtk::MessageDialog dialog(Utils::bold_message(_("Running As Root")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true /* modal */);
    dialog.set_secondary_text(message);
    dialog.run();

    return false; /* Is root. Bad. */
  }

  return true; /* Not root. It's OK. */
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
// Message to packagers:
// If your Glom package does not depend on PostgreSQL, for some reason,
// then your distro-specific patch should uncomment this #define.
// and implement ConnectionPool::install_posgres().
// But please, just make your Glom package depend on PostgreSQL instead,
// because this is silly.
//
//#define DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED 1

/** Check whether PostgreSQL is really available for self-hosting,
 * in case the distro package has incorrect dependencies.
 *
 * @results True if everything is OK.
 */
bool check_postgres_is_available_with_warning()
{
  const std::string binpath = Glom::ConnectionPoolBackends::PostgresSelfHosted::get_path_to_postgres_executable("postgres", false /* not quoted */);

  // TODO: At least on Windows we should probably also check for initdb and
  // pg_ctl. Perhaps it would also be a good idea to access these files as
  // long as glom runs so they cannot be (re)moved.
  if(!binpath.empty())
  {
    const Glib::ustring uri_binpath = Glib::filename_to_uri(binpath);
    if(Utils::file_exists(uri_binpath))
      return true;
  }

  #ifdef DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED

  //Show message to the user about the broken installation:
  //This is a packaging bug, but it would probably annoy packagers to mention that in the dialog:
  Gtk::MessageDialog dialog(Utils::bold_message(_("Incomplete Glom Installation")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_NONE, true /* modal */);
  dialog.set_secondary_text(_("Your installation of Glom is not complete, because PostgreSQL is not available on your system. PostgreSQL is needed for self-hosting of Glom databases.\n\nYou may now install PostgreSQL to complete the Glom installation."));
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(_("Install PostgreSQL"), Gtk::RESPONSE_OK);
  const int response = dialog.run();
  if(response != Gtk::RESPONSE_OK)
    return false; //Failure. Glom should now quit.
  else
    return install_postgres(&dialog);

  #else  //DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED

  //Show message to the user about the broken installation:
  Gtk::MessageDialog dialog(Utils::bold_message(_("Incomplete Glom Installation")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true /* modal */);
  dialog.set_secondary_text(_("Your installation of Glom is not complete, because PostgreSQL is not available on your system. PostgreSQL is needed for self-hosting of Glom databases.\n\nPlease report this bug to your vendor, or your system administrator so it can be corrected."));
  dialog.run();
  return false;

  #endif //DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED
}
#endif //GLOM_ENABLE_CLIENT_ONLY

#endif //GLOM_ENABLE_POSTGRESQL

bool check_pyglom_is_available_with_warning()
{
  if(glom_python_module_is_available())
    return true;

   /* The python module could not be imported by Glom, so warn the user: */
   const Glib::ustring message = _("Your installation of Glom is not complete, because the Glom Python module is not available on your system.\n\nPlease report this bug to your vendor, or your system administrator so it can be corrected.");

  Gtk::MessageDialog dialog(Utils::bold_message(_("Glom Python Module Not Installed")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true /* modal */);
  dialog.set_secondary_text(message);
  dialog.run();

  return false;
}

bool check_gir_is_available_with_warning()
{
  if(gir_python_module_is_available())
    return true;

  /* The python module could not be imported by Glom, so warn the user: */
  const Glib::ustring message = _("Your installation of Glom is not complete, because the gi.repository Python module is not available on your system.\n\nPlease report this bug to your vendor, or your system administrator so it can be corrected.");

  Gtk::MessageDialog dialog(Utils::bold_message(_("gi.repository Python Module Not Installed")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true /* modal */);
  dialog.set_secondary_text(message);
  dialog.run();

  return false;
}

bool check_pygda_is_available_with_warning()
{
  if(gda_python_module_is_available())
    return true;

  /* The python module could not be imported by Glom, so warn the user: */
  const Glib::ustring message = _("Your installation of Glom is not complete, because the gi.repository.Gda python module is not available on your system.\n\nPlease report this bug to your vendor, or your system administrator so it can be corrected.");

  Gtk::MessageDialog dialog(Utils::bold_message(_("gi.repository.Gda Python Module Not Installed")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true /* modal */);
  dialog.set_secondary_text(message);
  dialog.run();

  return false;
}

} //namespace Glom

#ifdef __linux__
extern "C" void __libc_freeres(void);
#endif

int
main(int argc, char* argv[])
{
#ifdef __linux__
  //Force some cleanup at exit,
  //to help valgrind to detect memory leaks:
  atexit(__libc_freeres);
#else
#  ifdef G_OS_WIN32
  WSADATA data;
  int errcode = WSAStartup(MAKEWORD(2, 0), &data);
  if(errcode != 0)
  {
    std::cerr << "Failed to initialize WinSock: " << errcode << std::endl;
    return EXIT_FAILURE;
  }

  gchar* installation_dir_c = g_win32_get_package_installation_directory_of_module(0);
  const std::string installation_dir(installation_dir_c);
  g_free(installation_dir_c);
#  endif
#endif

  // TODO: I am not sure why, but this does not work. PYTHONPATH is set
  // correctly according to getenv(), but python still does not look in it.
  // For now, the installer installs all the python stuff directly into the
  // application directory, although I would like to move this to a python/
  // subdirectory.
#if 0
#ifdef G_OS_WIN32
  // Set PYTHONPATH to point to python/ because that's where the installer
  // installs all the python modules into.
  std::string python_path = Glib::build_filename(installation_dir, "python");
  std::string current_path = Glib::getenv("PYTHONPATH");
  if(current_path.empty()) current_path = python_path;
  else current_path += (std::string(";") + python_path); // PATH-like variables are separated by ; on Windows because : is a valid character in paths.
  std::cout << "Setting " << current_path << ":" << std::endl;
  std::cout << Glib::setenv("PYTHONPATH", current_path) << std::endl;
  std::cout << getenv("PYTHONPATH") << std::endl;
#endif
#endif

#ifdef G_OS_WIN32
  // Add glom's bin directory to PATH so that g_spawn* finds the
  // gspawn-win32-helper.exe helper program. The installer installs it there.
  Glib::setenv("PATH", Glib::getenv("PATH") + ";" + Glib::build_filename(installation_dir, "bin"));
#endif

#ifdef G_OS_WIN32
  // Load translations relative to glom.exe on Windows
  bindtextdomain(GETTEXT_PACKAGE,
      Glib::build_filename(installation_dir, "share" G_DIR_SEPARATOR_S "locale").c_str());
#else
  //Make this application use the current locale for _() translation:
  bindtextdomain(GETTEXT_PACKAGE, GLOM_LOCALEDIR);
#endif
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
    std::cerr << "  This can happen if the locale is not properly installed or configured." << std::endl;
  }

  Glom::libglom_init(); //Also initializes python.

  Glib::OptionContext context;

  Glom::OptionGroup group;
  context.set_main_group(group);

  //We use python for calculated-fields:
  PySys_SetArgv(argc, argv);

  std::auto_ptr<Gtk::Main> mainInstance;
  try
  {
    mainInstance = std::auto_ptr<Gtk::Main>( new Gtk::Main(argc, argv, context) );
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "Glom: Error while initializing gtkmm: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  try
  {
    context.parse(argc, argv);
  }
  catch(const Glib::OptionError& ex)
  {
      std::cout << _("Error while parsing command-line options: ") << std::endl << ex.what() << std::endl;
      std::cout << _("Use --help to see a list of available command-line options.") << std::endl;
      return EXIT_FAILURE;
  }
  catch(const Glib::Error& ex)
  {
    std::cout << "Error: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }


  if(group.m_arg_version)
  {
    std::cout << PACKAGE_STRING << std::endl;
    return EXIT_SUCCESS;
  }

  try
  {
#ifndef GLOM_ENABLE_CLIENT_ONLY
    Gsv::init();
    Goocanvas::init();
#endif //!GLOM_ENABLE_CLIENT_ONLY

    ev_init();

    //Get command-line parameters, if any:
    Glib::ustring input_uri = group.m_arg_filename;

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
      Glib::RefPtr<Gio::File> file = Gio::File::create_for_commandline_arg(input_uri);

      if(!file->query_exists())
      {
        std::cerr << _("Glom: The file does not exist.") << std::endl;
        std::cerr << "uri: " << input_uri << std::endl;

        std::cerr << std::endl << context.get_help() << std::endl;
        return EXIT_FAILURE;
      }

      const Gio::FileType file_type = file->query_file_type();
      if(file_type == Gio::FILE_TYPE_DIRECTORY)
      {
        std::cerr << _("Glom: The file path is a directory instead of a file.") << std::endl;

        std::cerr << std::endl << context.get_help() << std::endl;
        return EXIT_FAILURE;
      }

      input_uri = file->get_uri();

      //std::cout << "URI = " << input_uri << std::endl;
    }

    //debugging:
    //input_uri = "file:///home/murrayc/cvs/gnome212/glom/examples/example_smallbusiness.glom";

#ifdef GLOM_ENABLE_POSTGRESQL

    //Check that the libgda postgres provider is really available:
    bool install_complete = Glom::ConnectionPoolBackends::Postgres::check_postgres_gda_client_is_available();
    if(!install_complete)
    {
      /* The Postgres provider was not found, so warn the user: */
      const Glib::ustring message = _("Your installation of Glom is not complete, because the PostgreSQL libgda provider is not available on your system. This provider is needed to access Postgres database servers.\n\nPlease report this bug to your vendor, or your system administrator so it can be corrected.");

      Gtk::MessageDialog dialog(Glom::Utils::bold_message(_("Incomplete Glom Installation")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true /* modal */);
      dialog.set_secondary_text(message);
      dialog.run();

      return EXIT_FAILURE; //There is no point in going further because Glom would not be able to connect to any Postgres servers.
    }

    #ifndef GLOM_ENABLE_CLIENT_ONLY
    //Check that the postgres executable is really available:
    if(!Glom::check_postgres_is_available_with_warning())
      return EXIT_FAILURE; //There is no point in going further because the most useful Glom functionality will not work without Postgres. Only a very cut-down Glom client would be useful without self-hosting.
    #endif // !GLOM_ENABLE_CLIENT_ONLY

    // Postgres can't be started as root. initdb complains.
    // So just prevent this in general. It is safer anyway.
    if(!Glom::check_user_is_not_root_with_warning())
      return EXIT_FAILURE;
#endif //GLOM_ENABLE_POSTGRESQL

    if(!Glom::check_gir_is_available_with_warning())
      return EXIT_FAILURE;

    if(!Glom::check_pygda_is_available_with_warning())
      return EXIT_FAILURE;

    if(!Glom::check_pyglom_is_available_with_warning())
      return EXIT_FAILURE;


    // Some more sanity checking:
    // These print errors to the stdout if they fail.
    // In future we might refuse to start if they fail.
    bool date_check_ok = true;
    const bool test1 =
      Glom::Conversions::sanity_check_date_text_representation_uses_4_digit_years(group.m_arg_debug_date_check /* show debug output */);
    if(!test1)
    {
      std::cerr << "Glom: ERROR: Date presentation sanity checks failed. Glom will not display dates correctly. This needs attention from a translator. Please file a bug. See http://www.glom.org." << std::endl;
      date_check_ok = false;
    }

    const bool test2 = Glom::Conversions::sanity_check_date_parsing();
    if(!test2)
    {
      std::cerr << "Glom: ERROR: Date parsing sanity checks failed. Glom will not interpret dates correctly. This needs attention from a translator. Please file a bug. See http://www.glom.org." << std::endl;
      date_check_ok = false;
    }

    if(group.m_arg_debug_date_check)
    {
      return date_check_ok ? EXIT_SUCCESS : EXIT_FAILURE; //This command-line option is documented as stopping afterwards.
    }

    Glom::Application* pApplication = 0;
    Glom::Utils::get_glade_widget_derived_with_warning(pApplication);
    g_assert(pApplication);

    pApplication->set_command_line_args(argc, argv);
    pApplication->set_show_sql_debug(group.m_arg_debug_sql);
    pApplication->set_stop_auto_server_shutdown(group.m_arg_stop_auto_server_shutdown);
    Glom::ConnectionPool::get_instance()->set_show_debug_output(group.m_arg_debug_sql);

    const bool test = pApplication->init(input_uri, group.m_arg_restore); //Sets it up and shows it.

    if(test) //The user could cancel the offer of a new or existing database.
      Gtk::Main::run(*pApplication); //Quit when the window is closed.

    //Cleanup:
    delete pApplication;
    pApplication = 0;
  }
  catch(const Glib::Exception& ex)
  {
    //If this happens then comment out the try/catch, and let the debugger show the call stack.
    std::cerr << "Glom: exception: \n  " << ex.what() << std::endl;
  }
  catch(const std::exception& ex)
  {
    //If this happens then comment out the try/catch, and let the debugger show the call stack.
    std::cerr << "Glom: exception: \n  " << ex.what() << std::endl;
  }

  Glom::libglom_deinit();

  //Tell libxml to clean things up to make valgrind more useful:
  xmlCleanupParser();

  //These fail, probably because of previous things that are causing leaks:
  //cairo_debug_reset_static_data(); //This crashes with _cairo_hash_table_destroy: Assertion `hash_table->live_entries == 0' failed.
  //FcFini(); //This crashes with "FcCacheFini: Assertion `fcCacheChains[i] == ((void *)0)' failed."
#ifdef G_OS_WIN32
  WSACleanup();
#endif

  return EXIT_SUCCESS;
}
