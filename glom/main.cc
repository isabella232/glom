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

//#include <gnome.h>
#include <gtkmm/main.h>
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

  try
  {
    Gtk::Main mainInstance(argc, argv);
    Bakery::init();

    // Main app
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_main");
    App_Glom* pApp_Glom = 0;
    refXml->get_widget_derived("window_main", pApp_Glom);

    pApp_Glom->set_command_line_args(argc, argv);
    pApp_Glom->init(); //Sets it up and shows it.

    Gtk::Main::run();
  }
  catch(const std::exception& ex)
  {
    std::cerr << "Glom: exception: \n" << ex.what() << std::endl;
  }

  return 0;
}


