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

#ifndef GLOM_DATASTRUCTURE_DATABASE_TITLE_H
#define GLOM_DATASTRUCTURE_DATABASE_TITLE_H

#include <libglom/data_structure/translatable_item.h>

namespace Glom
{

/** This is a separate class, instead of just deriving Document from
 * TranslatableItem, to avoid the need to use Document via std::shared_ptr.
 */
class DatabaseTitle
 : public TranslatableItem
{
public:
  DatabaseTitle();
  DatabaseTitle(const DatabaseTitle& src);
  DatabaseTitle(DatabaseTitle&& src) = delete;
  DatabaseTitle& operator=(const DatabaseTitle& src);
  DatabaseTitle& operator=(DatabaseTitle&& src) = delete;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_DATABASE_TITLE_H



