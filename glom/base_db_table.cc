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

#include "base_db_table.h"
#include <libglom/data_structure/glomconversions.h>
#include <glom/appwindow.h>
#include <glom/python_embed/glom_python.h>
#include <sstream>

namespace Glom
{

Base_DB_Table::Base_DB_Table()
{
}

Glib::ustring Base_DB_Table::get_table_name() const
{
  return m_table_name;
}

bool Base_DB_Table::init_db_details(const Glib::ustring& table_name)
{
  m_table_name = table_name;

  if(!ConnectionPool::get_instance()->get_ready_to_connect())
    return false;

  return fill_from_database();
}

} //namespace Glom


