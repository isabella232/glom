/***************************************************************************
                          main.cc  -  description
                             -------------------
    begin                : Thu Jul 27 2000
    copyright            : (C) 2000 by Murray Cumming
    email                : murrayc@usa.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//#include <gnome.h>
#include <gtkmm/main.h>
#include "config.h" //For VERSION.

#include "application.h"


int 
main(int argc, char* argv[])
{
  //bindtextdomain("glom", "");  // GNOMELOCALEDIR
  //textdomain("glom");

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


