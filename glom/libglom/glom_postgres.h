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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef DB_POSTGRES_H
#define DB_POSTGRES_H

//#include <glom/base_db.h>
#include <libglom/document/document.h>
#include <libglom/connectionpool.h>

namespace Glom
{

class GlomPostgres
{
public:

  typedef std::vector<Glib::ustring> type_vec_strings;

protected:
  //Utility functions to help with the odd formats of postgres internal catalog fields:
  static type_vec_strings pg_list_separate(const Glib::ustring& str);

};

} //namespace Glom

#endif //DB_POSTGRES_H

