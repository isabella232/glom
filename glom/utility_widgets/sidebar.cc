/* Glom
 *
 * Copyright (C) 2007 Johannes Schmid <johannes.schmid@openismus.com>
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

#include "sidebar.h"

namespace Glom
{

SideBar::SideBar()
{  
	palette = EGG_TOOL_PALETTE(egg_tool_palette_new ());
	Gtk::Container* container = Glib::wrap(GTK_CONTAINER(palette));
	
	add(*container);
  show_all_children();
}

SideBar::~SideBar()
{
  
}

void SideBar::add_group(EggToolItemGroup* group)
{
	gtk_container_add(GTK_CONTAINER(palette), GTK_WIDGET(group));
}

void SideBar::remove_group(EggToolItemGroup* group)
{
	gtk_container_remove(GTK_CONTAINER(palette), GTK_WIDGET(group));
}

void SideBar::set_drag_source()
{
  // It's important to call this AFTER all groups have been added
  egg_tool_palette_set_drag_source (palette);
}

} // namespace Glom

