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

#include <libglom/data_structure/translatable_item.h>
#include <libglom/data_structure/has_title_singular.h>

namespace Glom
{

class TableInfo
 : public TranslatableItem,
   public HasTitleSingular
{
public:
  TableInfo();
  TableInfo(const TableInfo& src);
  TableInfo& operator=(const TableInfo& src);

  guint m_sequence; //TODO: Use this?
  bool m_hidden;
  bool m_default;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_TABLEINFO_H



