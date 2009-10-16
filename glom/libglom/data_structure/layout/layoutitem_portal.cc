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

#include "layoutitem_portal.h"
#include <glibmm/i18n.h>

namespace Glom
{

LayoutItem_Portal::LayoutItem_Portal()
: m_print_layout_row_height(20), //arbitrary default.
  m_navigation_type(LayoutItem_Portal::NAVIGATION_AUTOMATIC)
{
}

LayoutItem_Portal::LayoutItem_Portal(const LayoutItem_Portal& src)
: LayoutGroup(src),
  UsesRelationship(src),
  //HasTitleSingular(src),
  m_navigation_relationship_specific(src.m_navigation_relationship_specific),
  m_print_layout_row_height(src.m_print_layout_row_height),
  m_navigation_type(src.m_navigation_type)
{
}

LayoutItem_Portal::~LayoutItem_Portal()
{
}

LayoutItem* LayoutItem_Portal::clone() const
{
  return new LayoutItem_Portal(*this);
}


LayoutItem_Portal& LayoutItem_Portal::operator=(const LayoutItem_Portal& src)
{
  LayoutGroup::operator=(src);
  UsesRelationship::operator=(src);
  //HasTitleSingular::operator=(src);

  m_navigation_relationship_specific = src.m_navigation_relationship_specific;
  m_print_layout_row_height = src.m_print_layout_row_height;
  m_navigation_type = src.m_navigation_type;

  return *this;
}

Glib::ustring LayoutItem_Portal::get_part_type_name() const
{
  return _("Portal");
}


void LayoutItem_Portal::change_related_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new)
{
  LayoutGroup::change_related_field_item_name(table_name, field_name, field_name_new);
}

void LayoutItem_Portal::change_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new)
{
  //Look at each item:
  for(LayoutGroup::type_list_items::iterator iterItem = m_list_items.begin(); iterItem != m_list_items.end(); ++iterItem)
  {
    sharedptr<LayoutItem> item = *iterItem;
    sharedptr<LayoutItem_Field> field_item = sharedptr<LayoutItem_Field>::cast_dynamic(item);
    if(field_item)
    {
      if(field_item->get_table_used(Glib::ustring()) == table_name) //If it's a related table (this would be a self-relationship)
      {
        if(field_item->get_name() == field_name)
          field_item->set_name(field_name_new); //Change it.
      }
      else
      {
        sharedptr<const Relationship> relationship = get_relationship();
        if(relationship && (relationship->get_to_table() == table_name) && (field_item->get_name() == field_name))
          field_item->set_name(field_name_new); //Change it.
      }
    }
    else
    {
      sharedptr<LayoutGroup> sub_group = sharedptr<LayoutGroup>::cast_dynamic(item);
      if(sub_group)
        sub_group->change_field_item_name(table_name, field_name, field_name_new);
    }
  }
}

sharedptr<UsesRelationship> LayoutItem_Portal::get_navigation_relationship_specific()
{
  if(get_navigation_type() == LayoutItem_Portal::NAVIGATION_SPECIFIC)
    return m_navigation_relationship_specific;
  else
    return sharedptr<UsesRelationship>();
}

sharedptr<const UsesRelationship> LayoutItem_Portal::get_navigation_relationship_specific() const
{
  if(get_navigation_type() == LayoutItem_Portal::NAVIGATION_SPECIFIC)
    return m_navigation_relationship_specific;
  else
    return sharedptr<UsesRelationship>();
}

void LayoutItem_Portal::set_navigation_relationship_specific(const sharedptr<UsesRelationship>& relationship)
{
  m_navigation_relationship_specific = relationship;
  m_navigation_type = LayoutItem_Portal::NAVIGATION_SPECIFIC;
}

void LayoutItem_Portal::reset_navigation_relationship()
{
    m_navigation_relationship_specific = sharedptr<UsesRelationship>();
    m_navigation_type = LayoutItem_Portal::NAVIGATION_AUTOMATIC;
}

Glib::ustring LayoutItem_Portal::get_from_table() const
{
  Glib::ustring from_table;

  sharedptr<const Relationship> relationship = get_relationship();
  if(relationship)
    from_table = relationship->get_from_table();

  return from_table;
}

double LayoutItem_Portal::get_print_layout_row_height() const
{
  return m_print_layout_row_height;
}

void LayoutItem_Portal::set_print_layout_row_height(double row_height)
{
  m_print_layout_row_height = row_height;
}

LayoutItem_Portal::navigation_type LayoutItem_Portal::get_navigation_type() const
{
  return m_navigation_type;
}

void LayoutItem_Portal::set_navigation_type(LayoutItem_Portal::navigation_type type)
{
  m_navigation_type = type;
}

/*
void LayoutItem_Portal::debug(guint level) const
{
  g_warning("LayoutItem_Portal::debug: level = %d", level);
  //LayoutGroup::debug(level);
}
*/

} //namespace Glom
