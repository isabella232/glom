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
	set_snap_edge (Gtk::POS_LEFT);
	set_handle_position (Gtk::POS_TOP);
	
  add (m_box);
  show_all_children();
}

SideBar::~SideBar()
{
  
}

void SideBar::add_button(Gtk::Button& button)
{
	button.set_relief (Gtk::RELIEF_HALF);
	m_box.pack_start (button, false, false, 0);
}

void SideBar::remove_button(Gtk::Button& button)
{
	m_box.remove (button);	
}

} // namespace Glom
