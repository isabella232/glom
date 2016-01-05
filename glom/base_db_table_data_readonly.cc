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

#include "config.h"
#include "base_db_table_data.h"
#include <libglom/data_structure/glomconversions.h>
#include <glom/appwindow.h>
#include <glom/python_embed/glom_python.h>
#include <glom/utils_ui.h>
#include <libglom/db_utils.h>
#include <sstream>
#include <glibmm/i18n.h>

namespace Glom
{

Base_DB_Table_Data_ReadOnly::Base_DB_Table_Data_ReadOnly()
{
}

bool Base_DB_Table_Data_ReadOnly::refresh_data_from_database()
{
  if(!ConnectionPool::get_instance()->get_ready_to_connect())
    return false;

  return fill_from_database();
}

} //namespace Glom
