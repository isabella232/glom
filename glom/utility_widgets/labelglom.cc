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

#include "labelglom.h"
#include <gtkmm/messagedialog.h>
#include "../application.h"
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

LabelGlom::LabelGlom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /* refGlade */)
: Gtk::Label(cobject)
{
  setup_menu();
  init();
}

LabelGlom::LabelGlom()
{
  setup_menu();
  init();
}

LabelGlom::LabelGlom(const Glib::ustring& label, float xalign, float yalign, bool mnemonic)
: Gtk::Label(label, xalign, yalign, mnemonic)
{
  setup_menu();
  init();
}

LabelGlom::~LabelGlom()
{
}

void LabelGlom::init()
{

}

App_Glom* LabelGlom::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<App_Glom*>(pWindow);
}

