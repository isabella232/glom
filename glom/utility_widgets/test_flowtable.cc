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

  Gtk::Entry button1; //"one");
  button1.show();
  button1.set_text("debug");

  FlowTable inner_table;
  inner_table.set_padding(20);
  inner_table.set_design_mode();
  inner_table.set_columns_count(1);
  inner_table.show();
  flowtable.add(inner_table);

  Gtk::Entry button2; button2.set_text("two");
  inner_table.add(button1, button2);
  button2.show();
  button2.set_size_request(40, 40);

  Gtk::Entry button3; button3.set_text("three");
  button3.show();

  Gtk::Entry button4; button4.set_text("four");
  inner_table.add(button3, button4);
  button4.show();
  button4.set_size_request(20, 30);

  Gtk::Entry button5; button5.set_text("five");
  button5.show();

  Gtk::Entry button6; button6.set_text("six");
  inner_table.add(button5, button6);
  button6.show();

  Gtk::Entry button7; button7.set_text("seven");;
  button7.show();
  //button7.set_size_request(100, 100);

  Gtk::Entry button8; button8.set_text("eight");
  flowtable.add(button7, button8);
  button8.show();
  button8.set_size_request(100, 300);

  Gtk::Entry button9; button9.set_text("nine");;
  button9.show();
  //button7.set_size_request(100, 100);

  Gtk::Entry button10; button10.set_text("ten");
  flowtable.add(button9, button10);
  button10.show();

  window.add(flowtable);
  flowtable.set_design_mode();
  flowtable.show();


  flowtable.set_padding(20);

  Gtk::Main::run(window);


  return 0;
}





