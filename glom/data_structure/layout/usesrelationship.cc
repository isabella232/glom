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
 
#include "fieldformatting.h"
#include <glibmm/i18n.h>

UsesRelationship::UsesRelationship()
{
}

UsesRelationship::UsesRelationship(const UsesRelationship& src)
: m_relationship(src.m_relationship)
{
}

UsesRelationship::~UsesRelationship()
{
}

bool UsesRelationship::operator==(const UsesRelationship& src) const
{
  return (m_relationship == src.m_relationship);
}


UsesRelationship& UsesRelationship::operator=(const UsesRelationship& src)
{
  m_relationship = src.m_relationship;

  return *this;
}

bool UsesRelationship::get_has_relationship_name() const
{
  if(!m_relationship)
    return false;
  else
    return !(m_relationship->get_name().empty());
}

Glib::ustring UsesRelationship::get_relationship_name() const
{
  if(m_relationship)
    return m_relationship->get_name();
  else
    return Glib::ustring();
}

sharedptr<Relationship> UsesRelationship::get_relationship() const
{
  return m_relationship;
}

void UsesRelationship::set_relationship(const sharedptr<Relationship>& relationship)
{
  m_relationship = relationship;
}

