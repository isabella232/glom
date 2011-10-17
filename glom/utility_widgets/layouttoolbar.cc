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
#include <gtkmm/stock.h>
#include <glibmm/i18n.h>
#include <libglom/libglom_config.h>

#include "layoutwidgetbase.h"

namespace Glom
{

LayoutToolbar::LayoutToolbar()
:  m_group_items(_("Items")), 
   m_group_containers(_("Containers")),
   m_drag_group("glom-group.png", LayoutWidgetBase::TYPE_GROUP, 
     _("Group"), _("Drag this to the layout to add a new group.")),
   m_drag_notebook("glom-notebook.png", LayoutWidgetBase::TYPE_NOTEBOOK,
     _("Notebook"), _("Drag this to the layout to add a new notebook.")),
   m_drag_item("glom-field.png", LayoutWidgetBase::TYPE_FIELD,
     _("Database Field"), _("Drag this to the layout to add a new database field.")),
   m_drag_portal("glom-related-records.png", LayoutWidgetBase::TYPE_PORTAL,
     _("Related Records"), _("Drag this to the layout to add a new Related Record.")),
   m_drag_button("glom-button.png", LayoutWidgetBase::TYPE_BUTTON,
     _("Button"), _("Drag this to the layout to add a new button.")),
   m_drag_text("glom-text.png", LayoutWidgetBase::TYPE_TEXT,
     _("Group"), _("Drag this to the layout to add a new static text box.")),
   m_drag_image("glom-image.png", LayoutWidgetBase::TYPE_IMAGE,
     _("Image"), _("Drag this to the layout to add a new static image."))
{
  // Looks ugly otherwise:
  set_size_request(100, 200);

  //TODO: Add a drag item for the related records item.
  
  //Note for translators: These are container layout items, containing child layout items, like container widgets in GTK+.
  m_group_containers.add(m_drag_group);
  m_group_containers.add(m_drag_notebook);

  //Note for translators: These are layout items, like widgets in GTK+.
  m_group_items.add(m_drag_portal);
  m_group_items.add(m_drag_item);
  m_group_items.add(m_drag_button);
  m_group_items.add(m_drag_text);
  m_group_items.add(m_drag_image);
  
  add_group(m_group_containers);
  add_group(m_group_items);

  set_drag_source();
  
  show_all_children();
}

LayoutToolbar::~LayoutToolbar()
{
  
}

} // namespace Glom

