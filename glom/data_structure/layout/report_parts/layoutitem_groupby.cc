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
 
#include "layoutitem_groupby.h"
#include "../layoutitem_field.h"
#include <glibmm/i18n.h>

LayoutItem_GroupBy::LayoutItem_GroupBy()
{
  m_group_secondary_fields = sharedptr<LayoutGroup>::create(); //So that we dont need to create it from outside.
}

LayoutItem_GroupBy::LayoutItem_GroupBy(const LayoutItem_GroupBy& src)
: LayoutGroup(src),
  m_group_secondary_fields(src.m_group_secondary_fields),
  m_field_group_by(src.m_field_group_by),
  m_field_sort_by(src.m_field_sort_by)
{
}

LayoutItem_GroupBy::~LayoutItem_GroupBy()
{
  remove_all_items();
}


LayoutItem* LayoutItem_GroupBy::clone() const
{
  return new LayoutItem_GroupBy(*this);
}


LayoutItem_GroupBy& LayoutItem_GroupBy::operator=(const LayoutItem_GroupBy& src)
{
  if(this != &src)
  {
    LayoutGroup::operator=(src);

    m_group_secondary_fields = src.m_group_secondary_fields;
    m_field_group_by = src.m_field_group_by;
    m_field_sort_by = src.m_field_sort_by;
  }

  return *this;
}

sharedptr<LayoutItem_Field> LayoutItem_GroupBy::get_field_group_by()
{
  return m_field_group_by;
}

sharedptr<const LayoutItem_Field> LayoutItem_GroupBy::get_field_group_by() const
{
  return m_field_group_by;
}

bool LayoutItem_GroupBy::get_has_field_group_by() const
{
  if(!m_field_group_by)
    return false;

  return m_field_group_by->get_name_not_empty();
}

sharedptr<LayoutItem_Field> LayoutItem_GroupBy::get_field_sort_by()
{
  return m_field_sort_by;
}

sharedptr<const LayoutItem_Field> LayoutItem_GroupBy::get_field_sort_by() const
{
  return m_field_sort_by;
}

bool LayoutItem_GroupBy::get_has_field_sort_by() const
{
  if(!m_field_sort_by)
    return false;

  return m_field_sort_by->get_name_not_empty();
}


Glib::ustring LayoutItem_GroupBy::get_part_type_name() const
{
  return _("Group By");
}

void LayoutItem_GroupBy::set_field_group_by(const sharedptr<LayoutItem_Field>& field)
{
  m_field_group_by = field;
}

void LayoutItem_GroupBy::set_field_sort_by(const sharedptr<LayoutItem_Field>& field)
{
  m_field_sort_by = field;
}

