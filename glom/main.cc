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
#include <glibmm/i18n.h>



#include "config.h" //For VERSION.

#include "application.h"


class ExampleOptionGroup : public Glib::OptionGroup
{ 
public:
  ExampleOptionGroup();

  //These int instances should live as long as the OptionGroup to which they are added, 
  //and as long as the OptionContext to which those OptionGroups are added.
  std::string m_arg_filename;
};

ExampleOptionGroup::ExampleOptionGroup()
: Glib::OptionGroup("Glom", "Glom options", "Debug options for glom")
{
  Glib::OptionEntry entry;
  entry.set_long_name("file");
  entry.set_short_name('f');
  entry.set_description("The Filename");
  add_entry_filename(entry, m_arg_filename);
}


int 
main(int argc, char* argv[])
{
  //Make this application use the current locale for _() translation:
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);  //LOCALEDIR is defined in the Makefile.am
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  Gnome::Gda::init("glom", VERSION, argc, argv);

  Glib::OptionContext context;

  ExampleOptionGroup group;
  context.set_main_group(group);

/*
  try
  {
    context.parse(argc, argv);
  }
  catch(const Glib::Error& ex)
  {
    std::cout << "Exception: " << ex.what() << std::endl;
  }
*/

  //We use python for calculated-fields:
  Py_Initialize();

  try
  {
    //Initialize gnome_program, so that we can use gnome_help_display().    
    gnome_program_init (PACKAGE, VERSION, LIBGNOME_MODULE, argc, argv,
                            GNOME_PROGRAM_STANDARD_PROPERTIES, 0);

    Gtk::Main mainInstance(argc, argv, context);
    Bakery::init();

    //Get command-line parameters, if any:
    Glib::ustring input_uri = group.m_arg_filename;

    //debugging:
    //input_uri = "file:///home/murrayc/cvs/gnome212/glom/examples/example_smallbusiness.glom";


    // Main app
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_main");
    App_Glom* pApp_Glom = 0;
    refXml->get_widget_derived("window_main", pApp_Glom);

    pApp_Glom->set_command_line_args(argc, argv);

    bool test = pApp_Glom->init(input_uri); //Sets it up and shows it.
    if(test) //The user could cancel the offer of a new or existing database.
    {
      Gtk::Main::run();
    }
    else
      delete pApp_Glom;
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


