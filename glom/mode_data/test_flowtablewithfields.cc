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
#include <iostream>


//#include "dragwindow.h"

/*
void on_drag_data_get_label(const Glib::RefPtr<Gdk::DragContext>&, Gtk::SelectionData& selection_data, guint, guint)
{
  selection_data.set(selection_data.get_target(), 8, (const guchar*)"label", 5);
}

void on_drag_data_get_entry(const Glib::RefPtr<Gdk::DragContext>&, Gtk::SelectionData& selection_data, guint, guint)
{
  selection_data.set(selection_data.get_target(), 8, (const guchar*)"entry", 5);
}
*/

typedef std::list<Gtk::Widget*> type_vec_widgets;
type_vec_widgets vec_child_widgets;

static void fill_flowtable(Glom::FlowTable& flowtable)
{
  Gtk::Entry* button1 = Gtk::manage(new Gtk::Entry());
  button1->set_text("seven");
  button1->show();
  //button1->set_size_request(100, 100);
  vec_child_widgets.push_back(button1);

  Gtk::Entry* button2 = Gtk::manage(new Gtk::Entry());
  button2->set_text("eight");
  flowtable.add(*button1, *button2);
  button2->show();
  //button2->set_size_request(100, 100);
  vec_child_widgets.push_back(button2);

  Gtk::Label* button3 = Gtk::manage(new Gtk::Label());
  button3->set_text("nine"); //TODO: valgrind says that something here is leaked.
  button3->show();
  //button1->set_size_request(100, 100);
  vec_child_widgets.push_back(button3);

  Gtk::Entry* button4 = Gtk::manage(new Gtk::Entry());
  button4->set_text("ten");
  flowtable.add(*button3, *button4);
  button4->show();
  vec_child_widgets.push_back(button4);

  Gtk::Entry* button5 = Gtk::manage(new Gtk::Entry());
  button5->set_text("eleven");
  Gtk::Entry* button6 = Gtk::manage(new Gtk::Entry());
  button5->set_text("eleven");
  flowtable.add(*button5, *button6);
  button5->show();
  button6->show();
  vec_child_widgets.push_back(button5);
  vec_child_widgets.push_back(button6);
}

static void clear_flowtable(Glom::FlowTable& flowtable)
{
  //std::cout << G_STRFUNC << ": debug 1" << std::endl;
  for(type_vec_widgets::iterator iter = vec_child_widgets.begin(); iter != vec_child_widgets.end(); ++iter)
  {
    Gtk::Widget* widget = *iter;
    //std::cout << "  loop: widget=" << widget << std::endl;
    delete widget;
  }

  vec_child_widgets.clear();

  flowtable.remove_all();
}

int
main(int argc, char* argv[])
{
  Gtk::Main mainInstance(argc, argv);

  Gtk::Window window;
  //Gtk::VBox flowtable;
  Glom::FlowTable flowtable;
  flowtable.set_lines(2);
  flowtable.set_horizontal_spacing(6);
  flowtable.set_vertical_spacing(6);

  fill_flowtable(flowtable);
  clear_flowtable(flowtable);
  fill_flowtable(flowtable);

  window.add(flowtable);
  flowtable.set_design_mode();
  flowtable.show();

//  Glom::DragWindow drag_window;
//  drag_window.show();

  Gtk::Main::run(window);

  return 0;
}
