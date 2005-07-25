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

LayoutItem_Portal::LayoutItem_Portal()
{
}

LayoutItem_Portal::LayoutItem_Portal(const LayoutItem_Portal& src)
: LayoutGroup(src),
  m_relationship(src.m_relationship)
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

  m_relationship = src.m_relationship;

  return *this;
}


Glib::ustring LayoutItem_Portal::get_relationship() const
{
  return m_relationship.get_name();
}

void LayoutItem_Portal::set_relationship(const Glib::ustring& relationship)
{
  m_relationship.set_name(relationship);
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
  for(LayoutGroup::type_map_items::iterator iterItem = m_map_items.begin(); iterItem != m_map_items.end(); ++iterItem)
  {
    LayoutItem_Field* field_item = dynamic_cast<LayoutItem_Field*>(iterItem->second);
    if(field_item)
    {
      if(field_item->get_has_relationship_name()) //If it's a related table (this would be a self-relationship)
      {
        if(field_item->m_relationship.get_to_table() == table_name)
        {
          if(field_item->get_name() == field_name)
            field_item->set_name(field_name_new); //Change it.
        }
      }
      else
      {
        if((m_relationship.get_to_table() == table_name) && (field_item->get_name() == field_name))
          field_item->set_name(field_name_new); //Change it.
      }
    }
    else
    {
      LayoutGroup* sub_group = dynamic_cast<LayoutGroup*>(iterItem->second);
      if(sub_group)
        sub_group->change_field_item_name(table_name, field_name, field_name_new);
    }
  }
}

void LayoutItem_Portal::change_relationship_name(const Glib::ustring& table_name, const Glib::ustring& name, const Glib::ustring& name_new)
{
  const bool is_parent_table = (m_relationship.get_from_table() == table_name);
  const bool is_child_table = (m_relationship.get_to_table() == table_name);
  
  if(is_parent_table)
  {
    if(m_relationship.get_name() == name)
      m_relationship.set_name(name_new);
  }
  else if(is_child_table)
  {
    //Look at each item:
    for(LayoutGroup::type_map_items::iterator iterItem = m_map_items.begin(); iterItem != m_map_items.end(); ++iterItem)
    {
      LayoutItem_Field* field_item = dynamic_cast<LayoutItem_Field*>(iterItem->second);
      if(field_item)
      {
        if(field_item->get_has_relationship_name())
        {
          if(field_item->m_relationship.get_name() == name)
            field_item->m_relationship.set_name(name_new);
        }
      }
      else
      {
        LayoutGroup* sub_group = dynamic_cast<LayoutGroup*>(iterItem->second);
        if(sub_group)
          sub_group->change_relationship_name(table_name, name, name_new);
      }
    }
  }
}

void LayoutItem_Portal::change_related_relationship_name(const Glib::ustring& /* table_name */, const Glib::ustring& /* name */, const Glib::ustring& /* name_new */)
{
  
}

void LayoutItem_Portal::debug(guint level) const
{
  g_warning("LayoutItem_Portal::debug: level = %d", level);
  LayoutGroup::debug(level);
}
