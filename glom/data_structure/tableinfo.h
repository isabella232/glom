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
 
#ifndef GLOM_DATASTRUCTURE_TABLEINFO_H
#define GLOM_DATASTRUCTURE_TABLEINFO_H

#include <glibmm/ustring.h>

class TableInfo
{
public:

  TableInfo();
  TableInfo(const TableInfo& src);
  TableInfo& operator=(const TableInfo& src);

  Glib::ustring get_name() const; //For the predicate, when using std::find_if().
  
  Glib::ustring m_name;
  guint m_sequence; //TODO: Use this?
  Glib::ustring m_title;
  bool m_hidden;
};

#endif //GLOM_DATASTRUCTURE_TABLEINFO_H



