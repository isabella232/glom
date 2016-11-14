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

#include <libglom/data_structure/relationship.h>

namespace Glom
{

Relationship::Relationship()
: m_allow_edit(true), m_auto_create(false)
{
   m_translatable_item_type = enumTranslatableItemType::RELATIONSHIP;
}

bool Relationship::operator==(const Relationship& src) const
{
  return TranslatableItem::operator==(src)
         && HasTitleSingular::operator==(src)
         && (m_strFrom_Table == src.m_strFrom_Table)
         && (m_strFrom_Field == src.m_strFrom_Field)
         && (m_strTo_Table == src.m_strTo_Table)
         && (m_strTo_Field == src.m_strTo_Field)
         && (m_allow_edit == src.m_allow_edit)
         && (m_auto_create == src.m_auto_create);
}

Relationship* Relationship::clone() const
{
  return new Relationship(*this);
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

bool Relationship::get_auto_create() const
{
  return m_auto_create;
}

void Relationship::set_auto_create(bool val)
{
  m_auto_create = val;
}

bool Relationship::get_allow_edit() const
{
  return m_allow_edit;
}

void Relationship::set_allow_edit(bool val)
{
  m_allow_edit = val;
}

bool Relationship::get_has_fields() const
{
  return !m_strTo_Field.empty() && !m_strFrom_Field.empty() && !m_strTo_Table.empty() && !m_strFrom_Table.empty();
}

bool Relationship::get_has_to_table() const
{
  return !m_strTo_Table.empty();
}

} //namespace Glom
