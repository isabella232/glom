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
 
#include <gtkmm.h>
#include "flowtable.h"


int
main(int argc, char* argv[])
{
  Gtk::Main mainInstance(argc, argv);

  Gtk::Window window;
  //Gtk::VBox flowtable;
  FlowTable flowtable;
  flowtable.set_columns_count(2);
  
  Gtk::Button button1("one");
  button1.show();
  
  Gtk::Button button2("two");
  flowtable.add(button1, button2);
  button2.show();
  button2.set_size_request(40, 40);
  
  Gtk::Button button3("three");
  button3.show();
  
  Gtk::Button button4("four");
  flowtable.add(button3, button4);
  button4.show();
  button4.set_size_request(20, 30);
  
  Gtk::Button button5("five");
  button5.show();

  Gtk::Button button6("six");
  flowtable.add(button5, button6);
  button6.show();

  Gtk::Button button7("seven");;
  button7.show();
  //button7.set_size_request(100, 100);

  Gtk::Button button8("eight");
  flowtable.add(button7, button8);
  button8.show();
  button8.set_size_request(100, 100);

  Gtk::Button button9("nine");;
  button9.show();
  //button7.set_size_request(100, 100);

  Gtk::Button button10("ten");
  flowtable.add(button9, button10);
  button10.show();
  
  window.add(flowtable);
  flowtable.show();


  flowtable.set_padding(6);

  Gtk::Main::run(window);


  return 0;
}





