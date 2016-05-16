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

#ifndef GLOM_DATA_STRUCTURE_PRIVILEGES_H
#define GLOM_DATA_STRUCTURE_PRIVILEGES_H


#include <glibmm/ustring.h>

namespace Glom
{

#define GLOM_STANDARD_GROUP_NAME_DEVELOPER "glom_developer"

class Privileges
{
public:
  Privileges();
  Privileges(const Privileges& src);
  Privileges(Privileges&& src);

  Privileges& operator=(const Privileges& src);
  Privileges& operator=(Privileges&& src);

  bool operator==(const Privileges& src) const;


  bool m_view, m_edit, m_create, m_delete;
};

} //namespace Glom

#endif //GLOM_DATA_STRUCTURE_PRIVILEGES_H
