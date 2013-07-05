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
 
#include <libglom/data_structure/layout/report_parts/layoutitem_groupby.h>
#include <libglom/data_structure/layout/layoutitem_field.h>
#include <glibmm/i18n.h>

namespace Glom
{

LayoutItem_GroupBy::LayoutItem_GroupBy()
{
  m_group_secondary_fields = std::make_shared<LayoutGroup>(); //So that we dont need to create it from outside.
}

LayoutItem_GroupBy::LayoutItem_GroupBy(const LayoutItem_GroupBy& src)
: LayoutGroup(src),
  m_field_group_by(src.m_field_group_by),
  m_group_secondary_fields(src.m_group_secondary_fields),
  m_fields_sort_by(src.m_fields_sort_by)
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
    m_fields_sort_by = src.m_fields_sort_by;
  }

  return *this;
}

std::shared_ptr<LayoutItem_Field> LayoutItem_GroupBy::get_field_group_by()
{
  return m_field_group_by;
}

std::shared_ptr<const LayoutItem_Field> LayoutItem_GroupBy::get_field_group_by() const
{
  return m_field_group_by;
}

bool LayoutItem_GroupBy::get_has_field_group_by() const
{
  if(!m_field_group_by)
    return false;

  return m_field_group_by->get_name_not_empty();
}

LayoutItem_GroupBy::type_list_sort_fields LayoutItem_GroupBy::get_fields_sort_by()
{
  return m_fields_sort_by;
}

LayoutItem_GroupBy::type_list_sort_fields LayoutItem_GroupBy::get_fields_sort_by() const
{
  return m_fields_sort_by;
}

bool LayoutItem_GroupBy::get_has_fields_sort_by() const
{
  return !m_fields_sort_by.empty();
}

Glib::ustring LayoutItem_GroupBy::get_part_type_name() const
{
  //Translators: This is the name of a UI element (a layout part name).
  return _("Group By");
}

void LayoutItem_GroupBy::set_field_group_by(const std::shared_ptr<LayoutItem_Field>& field)
{
  m_field_group_by = field;
}

void LayoutItem_GroupBy::set_fields_sort_by(const type_list_sort_fields& fields)
{
  m_fields_sort_by = fields;
}

Glib::ustring LayoutItem_GroupBy::get_layout_display_name() const
{
  Glib::ustring result;

  //TODO: Internationalize this properly:
  if(get_has_field_group_by())
    result = get_field_group_by()->get_layout_display_name();

  if(get_has_fields_sort_by())
  {
    result += "(sort by: ";

    Glib::ustring sort_fields_names;

    //List all the sort fields:
    for(type_list_sort_fields::const_iterator iter = m_fields_sort_by.begin(); iter != m_fields_sort_by.end(); ++iter)
    {
      if(!sort_fields_names.empty())
        sort_fields_names += ", ";

      sort_fields_names += iter->first->get_layout_display_name();
    }

    result += sort_fields_names + ')';
  }

  return result;
}

Glib::ustring LayoutItem_GroupBy::get_report_part_id() const
{
  return "group_by";
}

std::shared_ptr<LayoutGroup> LayoutItem_GroupBy::get_secondary_fields()
{
  return m_group_secondary_fields;
}

std::shared_ptr<const LayoutGroup> LayoutItem_GroupBy::get_secondary_fields() const
{
  return m_group_secondary_fields;
}

LayoutItem_GroupBy::type_list_sort_fields LayoutItem_GroupBy::get_sort_by() const
{
  return m_fields_sort_by;
}

void LayoutItem_GroupBy::set_sort_by(const type_list_sort_fields& sort_by)
{
  m_fields_sort_by = sort_by;
}

} //namespace Glom

