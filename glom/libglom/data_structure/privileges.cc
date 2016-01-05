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

#include <libglom/data_structure/privileges.h>

namespace Glom
{

Privileges::Privileges()
: m_view(false),
  m_edit(false),
  m_create(false),
  m_delete(false)
{
}

Privileges::Privileges(const Privileges& src)
{
  operator=(src);
}

Privileges::Privileges(Privileges&& src)
: m_view(std::move(src.m_view)),
  m_edit(std::move(src.m_edit)),
  m_create(std::move(src.m_create)),
  m_delete(std::move(src.m_delete))
{
}

Privileges& Privileges::operator=(const Privileges& src)
{
  m_view = src.m_view;
  m_edit = src.m_edit;
  m_create = src.m_create;
  m_delete = src.m_delete;

  return *this;
}

Privileges& Privileges::operator=(Privileges&& src)
{
  m_view = std::move(src.m_view);
  m_edit = std::move(src.m_edit);
  m_create = std::move(src.m_create);
  m_delete = std::move(src.m_delete);

  return *this;
}

bool Privileges::operator==(const Privileges& src) const
{
  return (m_view == src.m_view)
         && (m_edit == src.m_edit)
         && (m_create == src.m_create)
         && (m_delete == src.m_delete);
}

} //namespace Glom
