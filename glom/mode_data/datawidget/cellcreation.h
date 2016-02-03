/* Glom
 *
 * Copyright (C) 2010 Murray Cumming
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

#ifndef GLOM_CELL_CREATION_H
#define GLOM_CELL_CREATION_H

#include <gtkmm/cellrenderer.h>
#include <libglom/data_structure/layout/layoutitem.h>

namespace Glom
{

/** Create a Gtk::CellRenderer that's appropriate to display a layout item,
 * for internal use by a DbAddDel or ComboChoices widget.
 */
Gtk::CellRenderer* create_cell(const std::shared_ptr<const LayoutItem>& layout_item, const Glib::ustring& table_name, const std::shared_ptr<const Document>& document, guint fixed_cell_height);

} //namespace Glom

#endif //GLOM_CELL_CREATION_H
