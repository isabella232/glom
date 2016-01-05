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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_PRINT_LAYOUT_TOOLBAR_H
#define GLOM_PRINT_LAYOUT_TOOLBAR_H

#include <gtkmm/box.h>
#include <glom/mode_design/print_layouts/print_layout_toolbar_button.h>
#include <gtkmm/window.h>
#include <gtkmm/toolpalette.h>
#include <gtkmm/toolitemgroup.h>

namespace Glom
{

class PrintLayoutToolbar : public Gtk::ToolPalette
{
public:
  PrintLayoutToolbar();

private:
  Gtk::ToolItemGroup m_group_items, m_group_lines, m_group_records;
  PrintLayoutToolbarButton m_drag_field, m_drag_text, m_drag_image, 
    m_drag_line_horizontal, m_drag_line_vertical, m_drag_related_records;
};

} //namespace Glom

#endif // GLOM_PRINT_LAYOUT_TOOLBAR_H
