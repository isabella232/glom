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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include <gtkmm/window.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/application.h>
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
  auto button1 = Gtk::manage(new Gtk::Entry());
  button1->set_text("one");
  button1->show();
  //button1->set_size_request(100, 100);
  vec_child_widgets.push_back(button1);

  auto button2 = Gtk::manage(new Gtk::Entry());
  button2->set_text("two");
  flowtable.add_widgets(*button1, *button2);
  button2->show();
  //button2->set_size_request(100, 100);
  vec_child_widgets.push_back(button2);

  auto button3 = Gtk::manage(new Gtk::Label());
  button3->set_text("three"); //TODO: valgrind says that something here is leaked.
  button3->show();
  //button1->set_size_request(100, 100);
  vec_child_widgets.push_back(button3);

  auto button4 = Gtk::manage(new Gtk::Entry());
  button4->set_text("four");
  flowtable.add_widgets(*button3, *button4);
  button4->show();
  vec_child_widgets.push_back(button4);

  auto button5 = Gtk::manage(new Gtk::Entry());
  button5->set_text("five");
  auto button6 = Gtk::manage(new Gtk::Entry());
  button6->set_text("size");
  flowtable.add_widgets(*button5, *button6);
  button5->show();
  button6->show();
  vec_child_widgets.push_back(button5);
  vec_child_widgets.push_back(button6);
}

static void clear_flowtable(Glom::FlowTable& flowtable)
{
  flowtable.remove_all();

  //std::cout << G_STRFUNC << ": debug 1" << std::endl;
  for(const auto& widget : vec_child_widgets)
  {
    //std::cout << "  loop: widget=" << widget << std::endl;
    delete widget; //TODO: This crashes 
  }

  vec_child_widgets.clear();
}

int
main(int argc, char* argv[])
{
  Glib::RefPtr<Gtk::Application> app = 
    Gtk::Application::create(argc, argv, "org.glom.test_flowtable");

  Gtk::Window window;
  //Gtk::Box flowtable;
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

  return app->run(window);
}
