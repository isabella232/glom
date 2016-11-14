/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
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

#include <libglom/libglom_config.h>
#include <libglom/data_structure/layout/layoutitem_notebook.h>
#include <glibmm/i18n-lib.h>

namespace Glom
{

LayoutItem_Notebook::LayoutItem_Notebook()
{
}

LayoutItem* LayoutItem_Notebook::clone() const
{
  return new LayoutItem_Notebook(*this);
}

Glib::ustring LayoutItem_Notebook::get_part_type_name() const
{
  //Translators: This is the name of a UI element (a layout part name).
  //"Notebook" means a GtkNotebook-type widget.
  return _("Notebook");
}

} //namespace Glom

