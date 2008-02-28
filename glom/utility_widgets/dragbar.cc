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

#include "layoutwidgetbase.h"
#include "eggtoolpalette/eggtoolitemgroup.h"

namespace Glom
{

DragBar::DragBar()
{
	// Does look ugly otherwise
	set_size_request(100, 200);
	
  Gtk::Image* image_item = 
    Gtk::manage (new Gtk::Image(GLOM_ICON_DIR "/glom-field.png"));
  Gtk::Image* image_button = 
    Gtk::manage (new Gtk::Image(GLOM_ICON_DIR "/glom-button.png"));
  Gtk::Image* image_text = 
    Gtk::manage (new Gtk::Image(GLOM_ICON_DIR "/glom-text.png"));
  
  Gtk::Image* image_group = 
    Gtk::manage (new Gtk::Image(GLOM_ICON_DIR "/glom-group.png"));
  Gtk::Image* image_notebook = 
    Gtk::manage (new Gtk::Image(GLOM_ICON_DIR "/glom-notebook.png"));
  
	
  DragButton* drag_group = Gtk::manage(new DragButton(*image_group, LayoutWidgetBase::TYPE_GROUP));  
  DragButton* drag_notebook = Gtk::manage(new DragButton(*image_notebook, LayoutWidgetBase::TYPE_NOTEBOOK));  

  DragButton* drag_item = Gtk::manage(new DragButton(*image_item, LayoutWidgetBase::TYPE_FIELD));
  DragButton* drag_button = Gtk::manage(new DragButton(*image_button, LayoutWidgetBase::TYPE_BUTTON));
  DragButton* drag_text = Gtk::manage(new DragButton(*image_text, LayoutWidgetBase::TYPE_TEXT));  
  
	GtkContainer* container_group = GTK_CONTAINER(egg_tool_item_group_new(_("Container")));
	gtk_container_add (container_group, GTK_WIDGET(drag_group->gobj()));
	gtk_container_add (container_group, GTK_WIDGET(drag_notebook->gobj()));

	GtkContainer* fields_group = GTK_CONTAINER(egg_tool_item_group_new(_("Fields")));
	gtk_container_add (fields_group, GTK_WIDGET(drag_item->gobj()));
	gtk_container_add (fields_group, GTK_WIDGET(drag_button->gobj()));  
	gtk_container_add (fields_group, GTK_WIDGET(drag_text->gobj()));
	
	add_group (EGG_TOOL_ITEM_GROUP(container_group));
	add_group (EGG_TOOL_ITEM_GROUP(fields_group));
	
  set_drag_source();
  
  show_all_children();
}

DragBar::~DragBar()
{
  
}

} // namespace Glom

