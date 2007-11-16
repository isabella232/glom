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

#include "dragbar.h"
#include "dragbutton.h"
#include <gtkmm/stock.h>
#include <glibmm/i18n.h>
#include "config.h"

namespace Glom
{

DragBar::DragBar()
{ 
  Gtk::Image* image_item = 
    Gtk::manage (new Gtk::Image(GLOM_ICON_DIR "/glom-field.png"));
  Gtk::Image* image_group = 
    Gtk::manage (new Gtk::Image(GLOM_ICON_DIR "/glom-group.png"));
  
  
  DragButton* drag_item = Gtk::manage(new DragButton(*image_item, "LayoutItem"));
  DragButton* drag_group = Gtk::manage(new DragButton(*image_group, "LayoutGroup"));  
  
	add_button (*drag_item);
	add_button (*drag_group);
	
  show_all_children();
}

DragBar::~DragBar()
{
  
}

} // namespace Glom
