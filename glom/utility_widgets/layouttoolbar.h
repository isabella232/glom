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

#ifndef GLOM_UTILITY_WIDGETS_LAYOUTTOOLBAR_H
#define GLOM_UTILITY_WIDGETS_LAYOUTTOOLBAR_H

#include <gtkmm/box.h>
#include <glom/utility_widgets/layouttoolbarbutton.h>
#include <gtkmm/window.h>
#include <gtkmm/toolpalette.h>

namespace Glom
{

class LayoutToolbar : public Gtk::ToolPalette
{
public:
  LayoutToolbar();

private:
  Gtk::ToolItemGroup m_group_items, m_group_containers;

  LayoutToolbarButton m_drag_group, m_drag_notebook,
    m_drag_item, m_drag_portal, m_drag_button, m_drag_text, m_drag_image;
};

} //namespace Glom

#endif // GLOM_UTILITY_WIDGETS_LAYOUTTOOLBAR_H
