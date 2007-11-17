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
: m_navigation_relationship_specific_main(false)
{
}

LayoutItem_Portal::LayoutItem_Portal(const LayoutItem_Portal& src)
: LayoutGroup(src),
  UsesRelationship(src),
  m_navigation_relationship_specific_main(src.m_navigation_relationship_specific_main),
  m_navigation_relationship_specific(src.m_navigation_relationship_specific)
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

  m_navigation_relationship_specific_main = src.m_navigation_relationship_specific_main;
  m_navigation_relationship_specific = src.m_navigation_relationship_specific;

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
        if(m_relationship && (m_relationship->get_to_table() == table_name) && (field_item->get_name() == field_name))
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

sharedptr<UsesRelationship> LayoutItem_Portal::get_navigation_relationship_specific(bool& main_relationship)
{
  main_relationship = m_navigation_relationship_specific_main;
  if(!main_relationship)
    return m_navigation_relationship_specific;
  else
    return sharedptr<UsesRelationship>();
}

sharedptr<const UsesRelationship> LayoutItem_Portal::get_navigation_relationship_specific(bool& main_relationship) const
{
  main_relationship = m_navigation_relationship_specific_main;
  if(!main_relationship)
    return m_navigation_relationship_specific;
  else
    return sharedptr<UsesRelationship>();
}

void LayoutItem_Portal::set_navigation_relationship_specific(bool main_relationship, const sharedptr<UsesRelationship>& relationship)
{
  m_navigation_relationship_specific_main = main_relationship;
  if(!m_navigation_relationship_specific_main)
    m_navigation_relationship_specific = relationship;
  else
    m_navigation_relationship_specific = sharedptr<UsesRelationship>();
}

/*
void LayoutItem_Portal::debug(guint level) const
{
  g_warning("LayoutItem_Portal::debug: level = %d", level);
  //LayoutGroup::debug(level);
}
*/

} //namespace Glom
