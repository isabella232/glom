/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * glom
 * Copyright (C) Johannes Schmid 2007 <jhs@gnome.org>
 *
 * glom is free software.
 *
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * glom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glom.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "placeholder-glom.h"
#include <gtkmm/messagedialog.h>
#include <glom/application.h>
#include <glibmm/i18n.h>
#include <string.h> // for memset

namespace Glom
{

PlaceholderGlom::PlaceholderGlom() :
  Glib::ObjectBase("glom_placeholder"),
  Gtk::Widget()
{
  set_has_window(false);
}

PlaceholderGlom::~PlaceholderGlom()
{
}

Application* PlaceholderGlom::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<Application*>(pWindow);
}

void PlaceholderGlom::on_size_request(Gtk::Requisition* requisition)
{
  //Initialize the output parameter:
  *requisition = Gtk::Requisition();

  // Take some mimimum size, we later want to cover the whole space available
  requisition->height = 30;
  requisition->width = 200;
}

void PlaceholderGlom::on_size_allocate(Gtk::Allocation& allocation)
{
  //Use the offered allocation for this container:
  set_allocation(allocation);

  if(m_refGdkWindow)
  {
    m_refGdkWindow->move_resize( allocation.get_x(), allocation.get_y(),
            allocation.get_width(), allocation.get_height() );
  }
}

void PlaceholderGlom::on_realize()
{
  //Call base class:
  Gtk::Widget::on_realize();

  ensure_style();


  if(!m_refGdkWindow)
  {
    //Create the GdkWindow:
    GdkWindowAttr attributes;
    memset(&attributes, 0, sizeof(attributes));

    Gtk::Allocation allocation = get_allocation();

    //Set initial position and size of the Gdk::Window:
    attributes.x = allocation.get_x();
    attributes.y = allocation.get_y();
    attributes.width = allocation.get_width();
    attributes.height = allocation.get_height();

    attributes.event_mask = get_events () | Gdk::EXPOSURE_MASK;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.wclass = GDK_INPUT_OUTPUT;


    m_refGdkWindow = Gdk::Window::create(get_window() /* parent */, &attributes,
            GDK_WA_X | GDK_WA_Y);
    set_has_window();
    set_window(m_refGdkWindow);

    //set colors
    modify_fg(Gtk::STATE_NORMAL , Gdk::Color("black"));

    //make the widget receive expose events
    m_refGdkWindow->set_user_data(gobj());
  }
}

void PlaceholderGlom::on_unrealize()
{
  m_refGdkWindow.reset();
  Gtk::Widget::on_unrealize();
}

bool PlaceholderGlom::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  // Paint the background:
  Gdk::Cairo::set_source_color(cr, get_style()->get_bg(Gtk::STATE_NORMAL));
  cr->paint();

  // Draw the foreground:
  Gdk::Cairo::set_source_color(cr, get_style()->get_fg(Gtk::STATE_NORMAL));
  cr->set_line_width(4);
  cr->rectangle(0, 0,  get_allocation().get_width(), get_allocation().get_height());
  cr->stroke();

  return true;
}

} // namespace Glom
