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

namespace
{

Glib::ustring get_icon_path(const Glib::ustring& filename)
{
#ifdef G_OS_WIN32
  gchar* basepath = g_win32_get_package_installation_subdirectory(NULL, NULL, "share/glom/pixmaps");
  Glib::ustring result = Glib::build_filename(basepath, filename);
  g_free(basepath);
  return result;
#else
  return Glib::build_filename(GLOM_ICON_DIR, filename);
#endif
}

} //anonymous namespace

namespace Glom
{

LayoutToolbar::LayoutToolbar()
{
  // Looks ugly otherwise:
  set_size_request(100, 200);

  LayoutToolbarButton* drag_group = 
    Gtk::manage(new LayoutToolbarButton("glom-group.png", LayoutWidgetBase::TYPE_GROUP,
                                        _("Group"), _("Drag this to the layout to add a new group.")));
  LayoutToolbarButton* drag_notebook = 
    Gtk::manage(new LayoutToolbarButton("glom-notebook.png", LayoutWidgetBase::TYPE_NOTEBOOK,
                                        _("Notebook"), _("Drag this to the layout to add a new notebook.")));  
  
  LayoutToolbarButton* drag_item = 
    Gtk::manage(new LayoutToolbarButton("glom-field.png", LayoutWidgetBase::TYPE_FIELD,
                                        _("Database Field"), _("Drag this to the layout to add a new database field.")));
  LayoutToolbarButton* drag_portal = 
    Gtk::manage(new LayoutToolbarButton("glom-related-records.png", LayoutWidgetBase::TYPE_PORTAL,
                                        _("Related Records"), _("Drag this to the layout to add a new Related Record.")));
  LayoutToolbarButton* drag_button = 
    Gtk::manage(new LayoutToolbarButton("glom-button.png", LayoutWidgetBase::TYPE_BUTTON,
                                        _("Button"), _("Drag this to the layout to add a new button.")));
  LayoutToolbarButton* drag_text = 
    Gtk::manage(new LayoutToolbarButton("glom-text.png", LayoutWidgetBase::TYPE_TEXT,
                                        _("Group"), _("Drag this to the layout to add a new static text box.")));  
  LayoutToolbarButton* drag_image = 
    Gtk::manage(new LayoutToolbarButton("glom-image.png", LayoutWidgetBase::TYPE_IMAGE,
                                        _("Image"), _("Drag this to the layout to add a new static image.")));

  //TODO: Add a drag item for the related records item.
  
  //Note for translators: These are container layout items, containing child layout items, like container widgets in GTK+.
  GtkContainer* container_group = GTK_CONTAINER(egg_tool_item_group_new(_("Containers")));
  gtk_container_add(container_group, GTK_WIDGET(drag_group->gobj()));
  gtk_container_add(container_group, GTK_WIDGET(drag_notebook->gobj()));

  //Note for translators: These are layout items, like widgets in GTK+.
  GtkContainer* fields_group = GTK_CONTAINER(egg_tool_item_group_new(_("Items")));
  gtk_container_add(fields_group, GTK_WIDGET(drag_portal->gobj()));
  gtk_container_add(fields_group, GTK_WIDGET(drag_item->gobj()));
  gtk_container_add(fields_group, GTK_WIDGET(drag_button->gobj()));  
  gtk_container_add(fields_group, GTK_WIDGET(drag_text->gobj()));
  gtk_container_add(fields_group, GTK_WIDGET(drag_image->gobj()));
  
  add_group(EGG_TOOL_ITEM_GROUP(container_group));
  add_group(EGG_TOOL_ITEM_GROUP(fields_group));

  set_drag_source();
  
  show_all_children();
}

LayoutToolbar::~LayoutToolbar()
{
  
}

} // namespace Glom

