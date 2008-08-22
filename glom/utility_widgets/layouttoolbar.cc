/* Glom
 *
 * Copyright (C) 2007, 2008 Openismus GmbH
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

#include "layouttoolbar.h"
#include "layouttoolbarbutton.h"
#include <gtkmm/stock.h>
#include <glibmm/i18n.h>
#include "config.h"

#include "layoutwidgetbase.h"
#include "egg/toolpalette/eggtoolitemgroup.h"

namespace Glom
{

LayoutToolbar::LayoutToolbar()
{
  // Looks ugly otherwise:
  set_size_request(100, 200);
	
  Gtk::Image* image_item = 
    Gtk::manage (new Gtk::Image(GLOM_ICON_DIR "/glom-field.png"));
  Gtk::Image* image_button = 
    Gtk::manage (new Gtk::Image(GLOM_ICON_DIR "/glom-button.png"));
  Gtk::Image* image_text = 
    Gtk::manage (new Gtk::Image(GLOM_ICON_DIR "/glom-text.png"));
  Gtk::Image* image_image = 
    Gtk::manage (new Gtk::Image(GLOM_ICON_DIR "/glom-image.png"));
  
  Gtk::Image* image_group = 
    Gtk::manage (new Gtk::Image(GLOM_ICON_DIR "/glom-group.png"));
  Gtk::Image* image_notebook = 
    Gtk::manage (new Gtk::Image(GLOM_ICON_DIR "/glom-notebook.png"));
  
	
  LayoutToolbarButton* drag_group = 
    Gtk::manage(new LayoutToolbarButton(*image_group, LayoutWidgetBase::TYPE_GROUP,
                                        _("Group"), _("Drag to document to add a new group")));
  LayoutToolbarButton* drag_notebook = 
    Gtk::manage(new LayoutToolbarButton(*image_notebook, LayoutWidgetBase::TYPE_NOTEBOOK,
                                        _("Notebook"), _("Drag to document to add a new notebook")));  
  
  LayoutToolbarButton* drag_item = 
    Gtk::manage(new LayoutToolbarButton(*image_item, LayoutWidgetBase::TYPE_FIELD,
                                        _("Database field"), _("Drag to document to add a new database field")));
  LayoutToolbarButton* drag_button = 
    Gtk::manage(new LayoutToolbarButton(*image_button, LayoutWidgetBase::TYPE_BUTTON,
                                        _("Button"), _("Drag to document to cadd a new button")));
  LayoutToolbarButton* drag_text = 
    Gtk::manage(new LayoutToolbarButton(*image_text, LayoutWidgetBase::TYPE_TEXT,
                                        _("Group"), _("Drag to document to add a new text box")));  
  LayoutToolbarButton* drag_image = 
    Gtk::manage(new LayoutToolbarButton(*image_image, LayoutWidgetBase::TYPE_IMAGE,
                                        _("Image"), _("Drag to document to add a new image")));
  
  //Note for translators: These are container layout items, containing child layout items, like container widgets in GTK+.
  GtkContainer* container_group = GTK_CONTAINER(egg_tool_item_group_new(_("Containers")));
  gtk_container_add (container_group, GTK_WIDGET(drag_group->gobj()));
  gtk_container_add (container_group, GTK_WIDGET(drag_notebook->gobj()));

  //Note for translators: These are layout items, like widgets in GTK+.
  GtkContainer* fields_group = GTK_CONTAINER(egg_tool_item_group_new(_("Items")));
  gtk_container_add (fields_group, GTK_WIDGET(drag_item->gobj()));
  gtk_container_add (fields_group, GTK_WIDGET(drag_button->gobj()));  
  gtk_container_add (fields_group, GTK_WIDGET(drag_text->gobj()));
  gtk_container_add (fields_group, GTK_WIDGET(drag_image->gobj()));
  
  add_group (EGG_TOOL_ITEM_GROUP(container_group));
  add_group (EGG_TOOL_ITEM_GROUP(fields_group));
	
  set_drag_source();
  
  show_all_children();
}

LayoutToolbar::~LayoutToolbar()
{
  
}

} // namespace Glom

