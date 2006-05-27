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

#include "box_db.h"
#include "application.h" //App_Glom.
#include <glom/libglom/appstate.h>
//#include <libgnomeui/gnome-app-helper.h>

#include <sstream> //For stringstream

namespace Glom
{

Box_DB::Box_DB()
: m_Box_Buttons(false, 6),
  m_Button_Cancel(Gtk::Stock::CANCEL)
{
  //m_pDocument = 0;

  set_border_width(6);
  set_spacing(6);

  //Connect signals:
  m_Button_Cancel.signal_clicked().connect(sigc::mem_fun(*this, &Box_DB::on_Button_Cancel));
}

Box_DB::Box_DB(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /* refGlade */)
: Gtk::VBox(cobject),
  m_Box_Buttons(false, 6),
  m_Button_Cancel(Gtk::Stock::CANCEL)
{
  //m_pDocument = 0;

  set_border_width(6);
  set_spacing(6);

  //Connect signals:
  m_Button_Cancel.signal_clicked().connect(sigc::mem_fun(*this, &Box_DB::on_Button_Cancel));
}

Box_DB::Box_DB(BaseObjectType* cobject)
: Gtk::VBox(cobject),
  m_Box_Buttons(false, 6),
  m_Button_Cancel(Gtk::Stock::CANCEL)
{
}

Box_DB::~Box_DB()
{
}

void Box_DB::on_Button_Cancel()
{
  //Tell the parent dialog that the user has clicked [Cancel]:
  signal_cancelled.emit();
}

const Gtk::Window* Box_DB::get_app_window() const
{
  Box_DB* nonconst = const_cast<Box_DB*>(this);
  return nonconst->get_app_window();
}
  
Gtk::Window* Box_DB::get_app_window()
{
  return dynamic_cast<Gtk::Window*>(get_toplevel());
/*

  Gtk::Widget* pWidget = get_parent();
  while(pWidget)
  {
    //Is this widget a Gtk::Window?:
    Gtk::Window* pWindow = dynamic_cast<Gtk::Window*>(pWidget);
    if(pWindow)
    {
      //Yes, return it.
      return pWindow;
    }
    else
    {
      //Try the parent's parent:
      pWidget = pWidget->get_parent();
    }
  }

  return 0; //not found.
*/
}

/*
void Box_DB::show_hint()
{
  hint_set(m_strHint);
}
*/

void Box_DB::set_button_cancel(Gtk::Button& button)
{
  button.signal_clicked().connect(sigc::mem_fun(*this, &Box_DB::on_Button_Cancel));
}

Gtk::Widget* Box_DB::get_default_button()
{
  return 0; //Override this if the box has a default button.
}


} //namespace Glom

