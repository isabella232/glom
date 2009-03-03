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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "box_db_table.h"
#include <libglom/data_structure/glomconversions.h>
#include "python_embed/glom_python.h"
#include <sstream>

namespace Glom
{

Box_DB_Table::Box_DB_Table()
{
}

Box_DB_Table::Box_DB_Table(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Box_WithButtons(cobject, refGlade)
{
}

Box_DB_Table::~Box_DB_Table()
{
}

const Gtk::Window* Box_DB_Table::get_app_window() const
{
  Box_DB_Table* nonconst = const_cast<Box_DB_Table*>(this);
  return nonconst->get_app_window();
}
  
Gtk::Window* Box_DB_Table::get_app_window()
{
  return dynamic_cast<Gtk::Window*>(get_toplevel());
}



} //namespace Glom


