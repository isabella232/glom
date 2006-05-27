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

#include "privileges.h"

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

Privileges::~Privileges()
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

bool Privileges::operator==(const Privileges& src) const
{
  return (m_view == src.m_view)
         && (m_edit == src.m_edit)
         && (m_create == src.m_create)
         && (m_delete == src.m_delete);
}

} //namespace Glom
