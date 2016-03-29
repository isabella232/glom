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


#ifndef GLOM_BOX_DB_TABLE_H
#define GLOM_BOX_DB_TABLE_H

#include <glom/box_withbuttons.h>
#include <glom/base_db_table.h>
#include <libglom/data_structure/field.h>
#include <gtkmm/builder.h>

namespace Glom
{

/** A Box that has access to a database table's structure.
 */
class Box_DB_Table
: public Box_WithButtons,
  public Base_DB_Table
{
public: 
  Box_DB_Table();
  Box_DB_Table(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

protected:
  void handle_error(const Glib::Exception& ex);
  void handle_error(const std::exception& ex); //TODO_port: This is probably useless now.
};

} //namespace Glom

#endif // GLOM_BOX_DB_TABLE_H
