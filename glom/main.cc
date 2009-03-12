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

#include "config.h" //For VERSION.

//We use Python for calculated fields.
#include <Python.h> //Include it before anything else to avoid "_POSIX_C_SOURCE redefined".

//#include <gnome.h>
#include <gtkmm/main.h>
#include <giomm.h>

// For postgres availability checks:
#ifdef GLOM_ENABLE_POSTGRESQL
#include <libglom/connectionpool_backends/postgres.h>
#ifndef GLOM_ENABLE_CLIENT_ONLY
#include <libglom/connectionpool_backends/postgres_self.h>
#endif //GLOM_ENABLE_CLIENT_ONLY
#endif //GLOM_ENABLE_POSTGRESQL

// For sanity checks:
#include <libglom/data_structure/glomconversions.h> // For GLOM_IMAGE_FORMAT

#ifndef GLOM_ENABLE_CLIENT_ONLY
#include <gtksourceviewmm/init.h>
#include <goocanvasmm/init.h>
#endif // !GLOM_ENABLE_CLIENT_ONLY
#include <glibmm/i18n.h>

#ifdef GLOM_ENABLE_MAEMO
#include <hildonmm/init.h>
#endif

#include "application.h"
#include <libglom/glade_utils.h>

#ifndef G_OS_WIN32
#include <fontconfig/fontconfig.h> //For cleanup.
#else
#define SAVE_DATADIR DATADIR
#undef DATADIR
#include <winsock2.h>
#define DATADIR SAVE_DATADIR
#endif

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
  bool m_arg_debug_sql;
};

OptionGroup::OptionGroup()
: Glib::OptionGroup("Glom", _("Glom options"), _("Command-line options for glom")),
  m_arg_version(false),
  m_arg_debug_sql(false)
{
  Glib::OptionEntry entry;
  entry.set_long_name("file");
  entry.set_short_name('f');
  entry.set_description(_("The Filename"));
  add_entry_filename(entry, m_arg_filename);

  Glib::OptionEntry entry_version;
  entry_version.set_long_name("version");
  entry_version.set_short_name('V');
  entry_version.set_description(_("The version of this application."));
  add_entry(entry_version, m_arg_version);

  Glib::OptionEntry entry_debug_sql;
  entry_version.set_long_name("debug_sql");
  entry_version.set_description(_("Show the generated SQL queries on stdout, for debugging."));
  add_entry(entry_version, m_arg_debug_sql);
}

} //namespace Glom

#ifndef G_OS_WIN32
extern "C" void __libc_freeres(void);
#endif

int 
main(int argc, char* argv[])
{
#ifndef G_OS_WIN32
  //Force some cleanup at exit,
  //to help valgrind to detect memory leaks:
  atexit(__libc_freeres);
#else
  WSADATA data;
  int errcode = WSAStartup(MAKEWORD(2, 0), &data);
  if(errcode != 0)
  {
    std::cerr << "Failed to initialize WinSock: " << errcode << std::endl;
    return -1;
  }
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
  gchar* python_path = g_win32_get_package_installation_subdirectory(NULL, NULL, "python");
  std::string current_path = Glib::getenv("PYTHONPATH");
  if(current_path.empty()) current_path = python_path;
  else current_path += (std::string(";") + python_path); // PATH-like variables are separated by ; on Windows because : is a valid character in paths.
  g_free(python_path);
  std::cout << "Setting " << current_path << ":" << std::endl;
  std::cout << Glib::setenv("PYTHONPATH", current_path) << std::endl;
  std::cout << getenv("PYTHONPATH") << std::endl;
#endif
#endif

#ifdef G_OS_WIN32
  // Add glom's bin directory to PATH so that g_spawn* finds the
  // gspawn-win32-helper.exe helper program. The installer installs it there.
  gchar* app_dir = g_win32_get_package_installation_subdirectory(NULL, NULL, "bin");
  Glib::setenv("PATH", Glib::getenv("PATH") + ";" + app_dir);
  g_free(app_dir);
#endif

#ifdef G_OS_WIN32
  // Load translations relative to glom.exe on Windows
  gchar* dir = g_win32_get_package_installation_subdirectory(NULL, NULL, "share/locale");
  bindtextdomain(GETTEXT_PACKAGE, dir);
  g_free(dir);
#else
  //Make this application use the current locale for _() translation:
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);  //LOCALEDIR is defined in the Makefile.am
#endif
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  g_thread_init(NULL); //So we can use GMutex.

  Gnome::Gda::init();
#ifdef GLOM_ENABLE_MAEMO
  Hildon::init();
#endif

  Glib::OptionContext context;

  Glom::OptionGroup group;
  context.set_main_group(group);
  //We use python for calculated-fields:
  Py_Initialize();
  PySys_SetArgv(argc, argv);
  Gtk::Main mainInstance(argc, argv, context);

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
#else
  std::auto_ptr<Glib::Error> error;
#endif // GLIBMM_EXCEPTIONS_ENABLED
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    context.parse(argc, argv);
#else
    context.parse(argc, argv, error);
#endif // GLIBMM_EXCEPTIONS_ENABLED
  }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  catch(const Glib::OptionError& ex)
#else
  if(error.get() != NULL)
#endif
  {
#ifndef GLIBMM_EXCEPTIONS_ENABLED
    const Glib::OptionError* exptr = dynamic_cast<Glib::OptionError*>(error.get());
    if(exptr)
    {
      const Glib::OptionError& ex = *exptr;
#endif // !GLIBMM_EXCEPTIONS_ENABLED
      std::cout << _("Error while parsing command-line options: ") << std::endl << ex.what() << std::endl;
      std::cout << _("Use --help to see a list of available command-line options.") << std::endl;
      return 0;
#ifndef GLIBMM_EXCEPTIONS_ENABLED
    }
    const Glib::Error& ex = *error.get();
#else
  }
  catch(const Glib::Error& ex)
  {
#endif
    std::cout << "Error: " << ex.what() << std::endl;
    return 0;
  }


  if(group.m_arg_version)
  {
    std::cout << VERSION << std::endl;
    return 0;
  }

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
#endif
  {
    Bakery::init();

#ifndef GLOM_ENABLE_CLIENT_ONLY
    gtksourceview::init();
    Goocanvas::init(PACKAGE, VERSION, argc, argv ) ;
#endif //!GLOM_ENABLE_CLIENT_ONLY

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
      if(file)
        input_uri = file->get_uri(); 
      //std::cout << "URI = " << input_uri << std::endl;
    }

    //debugging:
    //input_uri = "file:///home/murrayc/cvs/gnome212/glom/examples/example_smallbusiness.glom";

#ifdef GLOM_ENABLE_POSTGRESQL
    bool install_complete = false;
#ifndef GLOM_ENABLE_CLIENT_ONLY
    //Check that PostgreSQL is really available:
    install_complete = Glom::ConnectionPoolBackends::PostgresSelfHosted::check_postgres_is_available_with_warning();
    if(!install_complete)
      return -1; //There is no point in going further because the most useful Glom functionality will not work without Postgres. Only a very cut-down Glom client would be useful without self-hosting.
#endif // !GLOM_ENABLE_CLIENT_ONLY

    //Check that the libgda postgres provider is really available:
    install_complete = Glom::ConnectionPoolBackends::Postgres::check_postgres_gda_client_is_available_with_warning();
    if(!install_complete)
      return -1; //There is no point in going further because Glom would not be able to connect to any Postgres servers.

    // Postgres can't be started as root. initdb complains.
    // So just prevent this in general. It is safer anyway.
    if(!Glom::ConnectionPool::check_user_is_not_root())
      return -1;
#endif //GLOM_ENABLE_POSTGRESQL


    // Some more sanity checking:
    // These print errors to the stdout if they fail.
    // In future we might refuse to start if they fail.
    const bool test1 = Glom::Conversions::sanity_check_date_text_representation_uses_4_digit_years();
    const bool test2 = Glom::Conversions::sanity_check_date_parsing();
    if(!test1 || !test2)
    {
      std::cerr << "Glom: ERROR: Date parsing sanity checks failed. Glom will not display dates correctly or interperet entered dates correctly. This needs attention from a translator. Please file a bug. See http://www.glom.org." << std::endl;
    }


#ifdef GLIBMM_EXCEPTIONS_ENABLED
    // Main app
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(Glom::Utils::get_glade_file_path("glom.glade"), "window_main");
#else
    std::auto_ptr<Gnome::Glade::XmlError> error;
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(Glom::Utils::get_glade_file_path("glom.glade"), "window_main", "", error);
    if(error.get())
    {
      std::cerr << "Glom: exception: \n  " << error->what() << std::endl;
      return -1;
    }
#endif


    Glom::App_Glom* pApp_Glom = 0;
    refXml->get_widget_derived("window_main", pApp_Glom);

    pApp_Glom->set_command_line_args(argc, argv);
    pApp_Glom->set_show_sql_debug(group.m_arg_debug_sql);

    bool test = pApp_Glom->init(input_uri); //Sets it up and shows it.
    if(test) //The user could cancel the offer of a new or existing database.
    {
      Gtk::Main::run();
    }
    else
      delete pApp_Glom;
  }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
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
#endif // GLIBMM_EXCEPTIONS_ENABLED

  //We use python for calculated-fields:
  Py_Finalize();

  //Clean up singletons:
  Glom::ConnectionPool::delete_instance();

  //These fail, probably because of previous things that are causing leaks:
  //cairo_debug_reset_static_data(); //This crashes with _cairo_hash_table_destroy: Assertion `hash_table->live_entries == 0' failed.
  //FcFini(); //This crashes with "FcCacheFini: Assertion `fcCacheChains[i] == ((void *)0)' failed."
#ifdef G_OS_WIN32
  WSACleanup();
#endif

  return 0;
}


