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



#include "config.h" //For VERSION.

#include "application.h"


int 
main(int argc, char* argv[])
{
  //Make this application use the current locale for gettext() translation:
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);  //LOCALEDIR is defined in the Makefile.am
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  Gnome::Gda::init("glom", VERSION, argc, argv);

  //Get command-line parameters, if any:
  Glib::ustring input_uri;
  if(argc > 1 )
  {
    input_uri = argv[1];

    //Ignore arguements starting with "--", or "-".
    if(input_uri.size() && input_uri[0] == '-')
      input_uri = Glib::ustring();
  }

  //We use python for calculated-fields:
  Py_Initialize();
  
  try
  {
    //Initialize gnome_program, so that we can use gnome_help_display().    
    gnome_program_init (PACKAGE, VERSION, LIBGNOME_MODULE, argc, argv,
                            GNOME_PROGRAM_STANDARD_PROPERTIES, 0);
                            
    Gtk::Main mainInstance(argc, argv);
    Bakery::init();

    // Main app
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_main");
    App_Glom* pApp_Glom = 0;
    refXml->get_widget_derived("window_main", pApp_Glom);

    pApp_Glom->set_command_line_args(argc, argv);

    pApp_Glom->init(input_uri); //Sets it up and shows it.

    Gtk::Main::run();
  }
  catch(const std::exception& ex)
  {
    std::cerr << "Glom: exception: \n" << ex.what() << std::endl;
  }

  //We use python for calculated-fields:
  Py_Finalize();
  
  return 0;
}


