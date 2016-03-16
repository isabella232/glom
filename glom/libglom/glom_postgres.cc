/* Glom
 *
 * Copyright (C) 2001-2006 Murray Cumming
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

#include "glom_postgres.h"
#include <libglom/utils.h>
#include <libglom/string_utils.h>

namespace Glom
{

GlomPostgres::type_vec_strings GlomPostgres::pg_list_separate(const Glib::ustring& str)
{
  //Remove the first { and the last }:
  Glib::ustring without_brackets = Utils::string_trim(str, "{");
  without_brackets = Utils::string_trim(without_brackets, "}");

  //Get the comma-separated items:
  return Utils::string_separate(without_brackets, ",");
}

} //namespace Glom
