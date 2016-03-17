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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "print_layout_toolbar.h"
#include <glibmm/i18n.h>


namespace Glom
{

PrintLayoutToolbar::PrintLayoutToolbar()
:  m_group_items(_("Items")),
   m_group_lines(_("Lines")),
   m_group_records(_("Records")),
   m_drag_field("glom-field.png", PrintLayoutToolbarButton::enumItems::FIELD,
     _("Database Field"), _("Drag this to the layout to add a new database field.")),
   m_drag_text("glom-text.png", PrintLayoutToolbarButton::enumItems::TEXT,
     _("Text"), _("Drag this to the layout to add a new static text box.")),
   m_drag_image("glom-image.png", PrintLayoutToolbarButton::enumItems::IMAGE,
     _("Image"), _("Drag this to the layout to add a new static image.")),
   m_drag_line_horizontal("glom-line-horizontal.png", PrintLayoutToolbarButton::enumItems::LINE_HORIZONTAL,
     _("Horizontal Line"), _("Drag this to the layout to add a new horizontal line.")),
   m_drag_line_vertical("glom-line-vertical.png", PrintLayoutToolbarButton::enumItems::LINE_VERTICAL,
     _("Vertical Line"), _("Drag this to the layout to add a new vertical line.")),
   m_drag_related_records("glom-related-records.png", PrintLayoutToolbarButton::enumItems::PORTAL,
     _("Related Records"), _("Drag this to the layout to add a new related records portal."))
{
  // Looks ugly otherwise:
  set_size_request(100, 200);

  //Note for translators: These are layout items, like widgets in GTK+.
  m_group_items.add(m_drag_field);
  m_group_items.add(m_drag_text);
  m_group_items.add(m_drag_image);

  //Note for translators: These are layout items, like widgets in GTK+.
  m_group_lines.add(m_drag_line_horizontal);
  m_group_lines.add(m_drag_line_vertical);

  //Note for translators: These are layout items, like widgets in GTK+.
  m_group_records.add(m_drag_related_records);

  add(m_group_items);
  add(m_group_lines);
  add(m_group_records);

  set_drag_source();

  show_all_children();
}


} // namespace Glom
