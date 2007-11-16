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
#include "labelglom.h"
#include <gtkmm/messagedialog.h>
#include "../application.h"
#include <glibmm/i18n.h>

namespace Glom
{

PlaceholderGlom::PlaceholderGlom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /* refGlade */)
: Gtk::Frame(cobject)
{
  setup_menu();

  init();
}

PlaceholderGlom::PlaceholderGlom()
{
  setup_menu();

  init();
}

PlaceholderGlom::~PlaceholderGlom()
{
}

void PlaceholderGlom::init()
{
	set_shadow_type (Gtk::SHADOW_ETCHED_IN);
	
	set_size_request (250, 25);
}

App_Glom* PlaceholderGlom::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<App_Glom*>(pWindow);
}

} // namespace Glom
