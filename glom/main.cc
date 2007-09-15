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

//We use Python for calculated fields.
#include <Python.h> //Include it before anything else to avoid "_POSIX_C_SOURCE redefined".

//#include <gnome.h>
#include <gtkmm/main.h>
#include <libgnome/gnome-init.h> // For gnome_program_init().
#include <libgnomevfsmm/uri.h>
#include <gtksourceviewmm/init.h>
#include <glibmm/i18n.h>



#include "config.h" //For VERSION.

#include "application.h"

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

int 
main(int argc, char* argv[])
{
  //Make this application use the current locale for _() translation:
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);  //LOCALEDIR is defined in the Makefile.am
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  g_thread_init(NULL); //So we can use GMutex.

  Gnome::Gda::init("glom", VERSION, argc, argv);

  Glib::OptionContext context;

  Glom::OptionGroup group;
  context.set_main_group(group);

  try
  {
    context.parse(argc, argv);
  }
  catch(const Glib::OptionError& ex)
  {
    std::cout << _("Error while parsing commmand-line options: ") << std::endl << ex.what() << std::endl;
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
    std::cout << VERSION << std::endl;
    return 0;
  }

  //We use python for calculated-fields:
  Py_Initialize();
  PySys_SetArgv(argc, argv);

  try
  {
    //Initialize gnome_program, so that we can use gnome_help_display().    
    gnome_program_init(PACKAGE, VERSION, LIBGNOME_MODULE, argc, argv,
        GNOME_PARAM_HUMAN_READABLE_NAME, _("Glom"),
        GNOME_PROGRAM_STANDARD_PROPERTIES, NULL);


    Gtk::Main mainInstance(argc, argv, context);
    Bakery::init();

#ifndef ENABLE_CLIENT_ONLY
    gtksourceview::init();
#endif //ENABLE_CLIENT_ONLY

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
      input_uri = Gnome::Vfs::Uri::make_from_shell_arg(input_uri);
      //std::cout << "URI = " << input_uri << std::endl;
    }

    //debugging:
    //input_uri = "file:///home/murrayc/cvs/gnome212/glom/examples/example_smallbusiness.glom";

    bool install_complete;
#ifndef ENABLE_CLIENT_ONLY
    //Check that PostgreSQL is really available:
    install_complete = Glom::ConnectionPool::check_postgres_is_available_with_warning();
    if(!install_complete)
      return -1; //There is no point in going further because the most useful Glom functionality will not work without Postgres. Only a very cut-down Glom client would be useful without self-hosting.
#endif // !ENABLE_CLIENT_ONLY

    //Check that the libgda postgres provider is really available:
    install_complete = Glom::ConnectionPool::check_postgres_gda_client_is_available_with_warning();
    if(!install_complete)
      return -1; //There is no point in going further because Glom would not be able to connect to any Postgres servers.


    // Main app
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_main");
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

  //We use python for calculated-fields:
  Py_Finalize();

  return 0;
}


