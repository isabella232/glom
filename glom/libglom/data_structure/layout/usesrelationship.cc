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

#include <libglom/data_structure/layout/formatting.h>

namespace Glom
{

UsesRelationship::UsesRelationship()
{
}

bool UsesRelationship::operator==(const UsesRelationship& src) const
{
  return (m_relationship == src.m_relationship)
         && (m_related_relationship == src.m_related_relationship);
}

bool UsesRelationship::get_has_relationship_name() const
{
  if(!m_relationship)
    return false;
  else
    return !(m_relationship->get_name().empty());
}

bool UsesRelationship::get_has_related_relationship_name() const
{
  if(!m_related_relationship)
    return false;
  else
    return !(m_related_relationship->get_name().empty());
}


Glib::ustring UsesRelationship::get_relationship_name() const
{
  if(m_relationship)
    return m_relationship->get_name();
  else
    return Glib::ustring();
}

Glib::ustring UsesRelationship::get_related_relationship_name() const
{
  if(m_related_relationship)
    return m_related_relationship->get_name();
  else
    return Glib::ustring();
}

std::shared_ptr<const Relationship> UsesRelationship::get_relationship() const
{
  return m_relationship;
}

void UsesRelationship::set_relationship(const std::shared_ptr<const Relationship>& relationship)
{
  m_relationship = relationship;
}

std::shared_ptr<const Relationship> UsesRelationship::get_related_relationship() const
{
  return m_related_relationship;
}

void UsesRelationship::set_related_relationship(const std::shared_ptr<const Relationship>& relationship)
{
  m_related_relationship = relationship;
}

Glib::ustring UsesRelationship::get_sql_table_or_join_alias_name(const Glib::ustring& parent_table) const
{
  if(get_has_relationship_name() || get_has_related_relationship_name())
  {
    const auto result = get_sql_join_alias_name();
    if(result.empty())
    {
      //Non-linked-fields relationship:
      return get_table_used(parent_table);
    }
    else
      return result;
  }
  else
    return parent_table;
}

Glib::ustring UsesRelationship::get_table_used(const Glib::ustring& parent_table) const
{
  //std::cout << "debug: " << G_STRFUNC << ": relationship=" << glom_get_sharedptr_name(m_relationship) << "related_relationship=" << glom_get_sharedptr_name(m_related_relationship) << std::endl;

  if(m_related_relationship)
    return m_related_relationship->get_to_table();
  else if(m_relationship)
    return m_relationship->get_to_table();
  else
    return parent_table;
}

Glib::ustring UsesRelationship::get_title_used(const Glib::ustring& parent_table_title, const Glib::ustring& locale) const
{
  if(m_related_relationship)
    return m_related_relationship->get_title_or_name(locale);
  else if(m_relationship)
    return m_relationship->get_title_or_name(locale);
  else
    return parent_table_title;
}

Glib::ustring UsesRelationship::get_title_singular_used(const Glib::ustring& parent_table_title, const Glib::ustring& locale) const
{
  auto used = m_related_relationship;
  if(!used)
    used = m_relationship;

  if(!used)
    return Glib::ustring();

  const auto result = used->get_title_singular(locale);
  if(!result.empty())
    return result;
  else
    return get_title_used(parent_table_title, locale);
}

Glib::ustring UsesRelationship::get_to_field_used() const
{
  if(m_related_relationship)
    return m_related_relationship->get_to_field();
  else if(m_relationship)
    return m_relationship->get_to_field();
  else
    return Glib::ustring();
}

Glib::ustring UsesRelationship::get_relationship_name_used() const
{
  if(m_related_relationship)
    return m_related_relationship->get_name();
  else if(m_relationship)
    return m_relationship->get_name();
  else
    return Glib::ustring();
}

bool UsesRelationship::get_relationship_used_allows_edit() const
{
  if(m_related_relationship)
    return m_related_relationship->get_allow_edit();
  else if(m_relationship)
    return m_relationship->get_allow_edit();
  else
    return false; /* Arbitrary default. */
}

Glib::ustring UsesRelationship::get_sql_join_alias_name() const
{
  Glib::ustring result;

  if(get_has_relationship_name() && m_relationship->get_has_fields()) //relationships that link to tables together via a field
  {
    //We use relationship_name.field_name instead of related_table_name.field_name,
    //because, in the JOIN below, will specify the relationship_name as an alias for the related table name
    result += ("relationship_" + m_relationship->get_name());

    /*
    const auto field_table_name = relationship->get_to_table();
    if(field_table_name.empty())
    {
      std::cerr << G_STRFUNC << ": field_table_name is null. relationship name = " << relationship->get_name() << std::endl;
    }
    */

    if(get_has_related_relationship_name() && m_related_relationship->get_has_fields())
    {
      result += ('_' + m_related_relationship->get_name());
    }
  }

  return result;
}

Glib::ustring UsesRelationship::get_relationship_display_name() const
{
  Glib::ustring result;

  if(get_has_relationship_name())
    result = get_relationship_name();

  if(get_has_related_relationship_name())
    result += ("::" + get_related_relationship_name());

  return result;
}

} //namespace Glom
