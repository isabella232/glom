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

#include "relationship.h"

Relationship::Relationship()
{
}

Relationship::Relationship(const Relationship& src)
{
  operator=(src);
}

Relationship::~Relationship()
{
}

Relationship& Relationship::operator=(const Relationship& src)
{
  m_strName = src.m_strName;
  m_strTitle = src.m_strTitle;  
  m_strFrom_Table = src.m_strFrom_Table;
  m_strFrom_Field = src.m_strFrom_Field;
  m_strTo_Table = src.m_strTo_Table;
  m_strTo_Field = src.m_strTo_Field;

  return *this;
}

bool Relationship::operator==(const Relationship& src) const
{
  bool bEqual = (m_strFrom_Table == src.m_strFrom_Table);
  bEqual = bEqual && (m_strTitle == src.m_strTitle);
  bEqual = bEqual && (m_strFrom_Field == src.m_strFrom_Field);
  bEqual = bEqual && (m_strTo_Table == src.m_strTo_Table);
  bEqual = bEqual && (m_strTo_Field == src.m_strTo_Field);
  bEqual = bEqual && (m_strName == src.m_strName);

  return bEqual;
}

Glib::ustring Relationship::get_from_table() const
{
  return m_strFrom_Table;
}

Glib::ustring Relationship::get_from_field() const
{
  return m_strFrom_Field;
}

Glib::ustring Relationship::get_to_table() const
{
  return m_strTo_Table;
}

Glib::ustring Relationship::get_to_field() const
{
  return m_strTo_Field;
}

void Relationship::set_from_table(const Glib::ustring& strVal)
{
  m_strFrom_Table = strVal;
}

void Relationship::set_from_field(const Glib::ustring& strVal)
{
  m_strFrom_Field = strVal;
}

void Relationship::set_to_table(const Glib::ustring& strVal)
{
  m_strTo_Table = strVal;
}

void Relationship::set_to_field(const Glib::ustring& strVal)
{
  m_strTo_Field = strVal;
}

Glib::ustring Relationship::get_name() const
{
  return m_strName;
}

void Relationship::set_name(const Glib::ustring& strVal)
{
  m_strName = strVal;
}

Glib::ustring Relationship::get_title() const
{
  return m_strTitle;
}

void Relationship::set_title(const Glib::ustring& strVal)
{
  m_strTitle = strVal;
}

Glib::ustring Relationship::get_title_or_name() const
{
  if(m_strTitle.empty())
    return m_strName;
  else
    return m_strTitle;
}
