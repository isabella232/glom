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

#include "print_layout_toolbar.h"
#include "print_layout_toolbar_button.h"
#include <gtkmm/stock.h>
#include <glibmm/i18n.h>
#include <libglom/libglom_config.h>

//#include "layoutwidgetbase.h"
#include <gtk/gtktoolitemgroup.h>

namespace Glom
{

PrintLayoutToolbar::PrintLayoutToolbar()
{
  // Looks ugly otherwise:
  set_size_request(100, 200);
  
  PrintLayoutToolbarButton* drag_field = 
    Gtk::manage(new PrintLayoutToolbarButton("glom-field.png", PrintLayoutToolbarButton::ITEM_FIELD,
                                        _("Database Field"), _("Drag this to the layout to add a new database field.")));
  PrintLayoutToolbarButton* drag_text = 
    Gtk::manage(new PrintLayoutToolbarButton("glom-text.png", PrintLayoutToolbarButton::ITEM_TEXT,
                                        _("Text"), _("Drag this to the layout to add a new static text box.")));  
  PrintLayoutToolbarButton* drag_image = 
    Gtk::manage(new PrintLayoutToolbarButton("glom-image.png", PrintLayoutToolbarButton::ITEM_IMAGE,
                                        _("Image"), _("Drag this to the layout to add a new static image.")));

  PrintLayoutToolbarButton* drag_line_horizontal = 
    Gtk::manage(new PrintLayoutToolbarButton("glom-line-horizontal.png", PrintLayoutToolbarButton::ITEM_LINE_HORIZONTAL,
                                        _("Horizontal Line"), _("Drag this to the layout to add a new horizontal line.")));
  PrintLayoutToolbarButton* drag_line_vertical = 
    Gtk::manage(new PrintLayoutToolbarButton("glom-line-vertical.png", PrintLayoutToolbarButton::ITEM_LINE_VERTICAL,
                                        _("Vertical Line"), _("Drag this to the layout to add a new vertical line.")));

  PrintLayoutToolbarButton* drag_related_records = 
    Gtk::manage(new PrintLayoutToolbarButton("glom-related-records.png", PrintLayoutToolbarButton::ITEM_PORTAL,
                                        _("Related Records"), _("Drag this to the layout to add a new related records portal.")));
  
  //Note for translators: These are layout items, like widgets in GTK+.
  GtkContainer* items_group = GTK_CONTAINER(gtk_tool_item_group_new(_("Items")));
  gtk_container_add(items_group, GTK_WIDGET(drag_field->gobj()));
  gtk_container_add(items_group, GTK_WIDGET(drag_text->gobj()));
  gtk_container_add(items_group, GTK_WIDGET(drag_image->gobj()));

  //Note for translators: These are layout items, like widgets in GTK+.
  GtkContainer* lines_group = GTK_CONTAINER(gtk_tool_item_group_new(_("Lines")));
  gtk_container_add(lines_group, GTK_WIDGET(drag_line_horizontal->gobj()));
  gtk_container_add(lines_group, GTK_WIDGET(drag_line_vertical->gobj()));
  
  //Note for translators: These are layout items, like widgets in GTK+.
  GtkContainer* related_group = GTK_CONTAINER(gtk_tool_item_group_new(_("Records")));
  gtk_container_add(related_group, GTK_WIDGET(drag_related_records->gobj()));

  add_group(GTK_TOOL_ITEM_GROUP(items_group));
  add_group(GTK_TOOL_ITEM_GROUP(lines_group));
  add_group(GTK_TOOL_ITEM_GROUP(related_group));

  set_drag_source();
  
  show_all_children();
}

PrintLayoutToolbar::~PrintLayoutToolbar()
{
  
}

} // namespace Glom

