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
#include "canvas_editable.h"
#include "canvas_line_movable.h"
#include "canvas_rect_movable.h"
#include "canvas_text_movable.h"
#include "canvas_group_movable.h"
#include <goocanvasrect.h>
#include <iostream>

class MyCanvas : public Glom::CanvasEditable
{
public:
  
  MyCanvas()
  : m_context_menu(0)
  {
    setup_context_menu();

    set_bounds(0, 0, 500, 500);

    add_vertical_rule(73);
    add_vertical_rule(103);
    add_horizontal_rule(55);
    set_grid_gap(40);

    //Doesn't work until we fix the goocanvas _new() methods: Glib::RefPtr<Goocanvas::Rect> rect = Glib::wrap( goo_canvas_rect_new()
    //Glib::RefPtr<Goocanvas::Rect> rect    = Goocanvas::Rect::create(10, 10, 110, 110);
    Glib::RefPtr<Glom::CanvasRectMovable> rect = Glom::CanvasRectMovable::create(10, 10, 110, 110);
    rect->property_fill_color() = "white"; //This makes the whole area clickable, not just the outline stroke:
    rect->property_line_width() = 2.0f;
    rect->property_stroke_color() = "blue";
    add_item(rect, true /* resizable */);

    Glib::RefPtr<Glom::CanvasRectMovable> rect2 = Glom::CanvasRectMovable::create(20, 10, 110, 140);
    rect2->property_fill_color() = "yellow"; //This makes the whole area clickable, not just the outline stroke:
    rect2->property_line_width() = 1.0f;
    rect2->property_stroke_color() = "red";
    add_item(rect2, true /* resizable */);

    Glib::RefPtr<Glom::CanvasLineMovable> line = Glom::CanvasLineMovable::create();
    double points_coordinates[] = {20.0, 20.0, 100.0, 40.0};
    Goocanvas::Points points(2, points_coordinates);
    line->property_points() = points;
    line->property_line_width() = 3.0f;
    line->property_stroke_color() = "gray";
    line->set_movement_allowed(false, true);
    add_item(line);

    Glib::RefPtr<Glom::CanvasTextMovable> text = Glom::CanvasTextMovable::create("some text");
    text->set_xy(50, 50);
    add_item(text, true /* resizable */);

    line->signal_show_context().connect( sigc::mem_fun(*this, &MyCanvas::on_show_context_menu) );


    Glib::RefPtr<Glom::CanvasGroupMovable> group = Glom::CanvasGroupMovable::create();
    Glib::RefPtr<Glom::CanvasRectMovable> rect_inner = Glom::CanvasRectMovable::create(70, 70, 90, 90);
    rect_inner->property_fill_color() = "blue"; //This makes the whole area clickable, not just the outline stroke:
    rect_inner->property_line_width() = 1.0f;
    rect_inner->property_stroke_color() = "red";
    rect_inner->set_movement_allowed(false, false); //Move only as part of the parent group.
    group->add_child(rect_inner);
    add_item(group);
  }

protected:
  //This is an override:
  virtual void on_show_context_menu(guint button, guint32 activate_time)
  {
    if(m_context_menu)
      m_context_menu->popup(button, activate_time);
  }

  void on_context_menu_edit()
  {
  }

  void on_context_menu_delete()
  {
  }

  void setup_context_menu()
  {
    m_context_menu_action_group = Gtk::ActionGroup::create();

    m_context_menu_action_group->add(Gtk::Action::create("ContextMenu", "Context Menu") );

    Glib::RefPtr<Gtk::Action> action =  Gtk::Action::create("ContextEdit", Gtk::Stock::EDIT);
    m_context_menu_action_group->add(action,
      sigc::mem_fun(*this, &MyCanvas::on_context_menu_edit) );

    action =  Gtk::Action::create("ContextDelete", Gtk::Stock::DELETE);
    m_context_menu_action_group->add(action,
      sigc::mem_fun(*this, &MyCanvas::on_context_menu_delete) );

    m_context_menu_uimanager = Gtk::UIManager::create();
    m_context_menu_uimanager->insert_action_group(m_context_menu_action_group);

    #ifdef GLIBMM_EXCEPTIONS_ENABLED
    try
    {
    #endif
      Glib::ustring ui_info = 
        "<ui>"
        "  <popup name='ContextMenu'>"
        "    <menuitem action='ContextEdit'/>"
        "    <menuitem action='ContextDelete'/>"
        "  </popup>"
        "</ui>";

    #ifdef GLIBMM_EXCEPTIONS_ENABLED
      m_context_menu_uimanager->add_ui_from_string(ui_info);
    }
    catch(const Glib::Error& ex)
    {
      std::cerr << "building menus failed: " <<  ex.what();
    }
    #else
    std::auto_ptr<Glib::Error> error;
    m_context_menu_uimanager->add_ui_from_string(ui_info, error);
    if(error.get() != NULL)
    {
      std::cerr << "building menus failed: " << error->what();
    }
    #endif

    //Get the menu:
    m_context_menu = dynamic_cast<Gtk::Menu*>( m_context_menu_uimanager->get_widget("/ContextMenu") ); 
  }
 
  Gtk::Menu* m_context_menu;
  Glib::RefPtr<Gtk::ActionGroup> m_context_menu_action_group;
  Glib::RefPtr<Gtk::UIManager> m_context_menu_uimanager;
};

int
main(int argc, char* argv[])
{
  Gtk::Main mainInstance(argc, argv);
  Goocanvas::init("test", "0.1", argc, argv ) ;

  Gtk::Window window;
  MyCanvas canvas;
  window.add(canvas);
  canvas.show();
  

  Gtk::Main::run(window);


  return 0;
}





