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

#include "layoutgroup.h"
#include "layoutitem_field.h"
#include "layoutitem_portal.h"
#include <glibmm/i18n.h>

LayoutGroup::LayoutGroup()
: m_columns_count(1) //A sensible default
{
}

LayoutGroup::LayoutGroup(const LayoutGroup& src)
: LayoutItem(src),
  m_columns_count(src.m_columns_count)
{
  //Deep copy of the items map:
  for(type_map_items::const_iterator iter = src.m_map_items.begin(); iter != src.m_map_items.end(); ++iter)
  {
    if(iter->second)
      m_map_items[iter->first] = glom_sharedptr_clone(iter->second);
  }
}

LayoutGroup::~LayoutGroup()
{
  remove_all_items();
}

void LayoutGroup::remove_all_items()
{
  //Delete the items:
  m_map_items.clear();
}

LayoutItem* LayoutGroup::clone() const
{
  return new LayoutGroup(*this);
}


LayoutGroup& LayoutGroup::operator=(const LayoutGroup& src)
{
  if(this != &src)
  {
    LayoutItem::operator=(src);

    m_columns_count = src.m_columns_count;

    //Deep copy of the items map:
    remove_all_items();
    for(type_map_items::const_iterator iter = src.m_map_items.begin(); iter != src.m_map_items.end(); ++iter)
    {
      if(iter->second)
        m_map_items[iter->first] = glom_sharedptr_clone(iter->second);
    }
  }

  return *this;
}

bool LayoutGroup::has_field(const Glib::ustring& field_name) const
{
  for(type_map_items::const_iterator iter = m_map_items.begin(); iter != m_map_items.end(); ++iter)
  {
    sharedptr<LayoutItem> item = iter->second;
    sharedptr<LayoutItem_Field> field_item = sharedptr<LayoutItem_Field>::cast_dynamic(item);
    if(field_item)
    {
      if(field_item->get_name() == field_name)
        return true;
    }
    else
    {
      //Recurse into the child groups:
      sharedptr<LayoutGroup> group_item = sharedptr<LayoutGroup>::cast_dynamic(item);
      if(group_item)
      {
        if(group_item->has_field(field_name))
          return true;
      }
    }
  }

  return false;
}

sharedptr<LayoutItem> LayoutGroup::add_item(const sharedptr<LayoutItem>& item)
{
  //Get next available sequence:
  guint sequence = 0;
   if(!m_map_items.empty())
     sequence = m_map_items.rbegin()->first;

  ++sequence;

  return add_item(item, sequence);
}

sharedptr<LayoutItem> LayoutGroup::add_item(const sharedptr<LayoutItem>& item, guint sequence)
{
  sharedptr<LayoutItem> result;

  if(item)
  {
  /*
    if(m_map_items.find(sequence) != m_map_items.end())
    {
      g_warning("LayoutGroup::add_item(item, sequence): Replacing item: item.name = %s, sequence=%d", item.get_name().c_str(), sequence);
    }
  */

    //Delete any existing item at this position:
    remove_item(sequence);

    //Add the new item:
    result = item;
    m_map_items[sequence] = result;
    m_map_items[sequence]->m_sequence = sequence;
  }

  return result;
}

void LayoutGroup::remove_item(guint sequence)
{
  //Delete any existing item at this position:
  type_map_items::iterator iterFind = m_map_items.find(sequence);
  if(iterFind != m_map_items.end())
  {
    m_map_items.erase(iterFind);
  }
}

/*
void LayoutGroup::add_item(const LayoutGroup& item)
{
  //Get next available sequence:
  guint sequence = 0;
   if(!m_map_items.empty())
     sequence = m_map_items.rbegin()->first;
  ++sequence;

  add_item(item, sequence);
}

void LayoutGroup::add_item(const LayoutGroup& item, guint sequence)
{
  //Delete any existing item at this position:
  remove_item(sequence);

  //Add the new item:
  m_map_items[sequence] = new LayoutGroup(item);
  m_map_items[sequence]->m_sequence = sequence;
}
*/
LayoutGroup::type_map_items LayoutGroup::get_items()
{
  return m_map_items;
}

LayoutGroup::type_map_const_items LayoutGroup::get_items() const
{
  //Get a const map from the non-const map:
  //TODO_Performance: Surely we should not need to copy the structure just to constize it?
  type_map_const_items result;

  for(type_map_items::const_iterator iter = m_map_items.begin(); iter != m_map_items.end(); ++iter)
  {
    result[iter->first] = iter->second;
  }

  return result;
}

void LayoutGroup::remove_relationship(const sharedptr<const Relationship>& relationship)
{
  LayoutGroup::type_map_items::iterator iterItem = m_map_items.begin();
  while(iterItem != m_map_items.end())
  {
    sharedptr<LayoutItem> item = iterItem->second;
    sharedptr<UsesRelationship> uses_rel = sharedptr<UsesRelationship>::cast_dynamic(item);
    if(uses_rel)
    {
      if(uses_rel->get_has_relationship_name())
      {
        if(*(uses_rel->get_relationship()) == *relationship) //TODO_Performance: Slow if there are lots of translations.
        {
          m_map_items.erase(iterItem);
          iterItem = m_map_items.begin(); //Start again, because we changed the container.AddDel 
          continue;
        }
      }
    }

    sharedptr<LayoutGroup> sub_group = sharedptr<LayoutGroup>::cast_dynamic(item);
    if(sub_group)
      sub_group->remove_relationship(relationship);

    ++iterItem;
  }
}

void LayoutGroup::remove_field(const Glib::ustring& field_name)
{
  //Look at each item:
  LayoutGroup::type_map_items::iterator iterItem = m_map_items.begin();
  while(iterItem != m_map_items.end())
  {
    sharedptr<LayoutItem> item = iterItem->second;
    sharedptr<LayoutItem_Field> field_item = sharedptr<LayoutItem_Field>::cast_dynamic(item);
    if(field_item)
    {
      if(!(field_item->get_has_relationship_name())) //If it's not a related table.
      {
        if(field_item->get_name() == field_name)
        {
          m_map_items.erase(iterItem);
          iterItem = m_map_items.begin(); //Start again, because we changed the container.AddDel 
          continue;
        }
      }
    }
    else
    {
      sharedptr<LayoutItem_Portal> sub_portal = sharedptr<LayoutItem_Portal>::cast_dynamic(item);
      if(!sub_portal) //It could only be a related field in a portal - use remove_field(table, field) for that.
      {
        sharedptr<LayoutGroup> sub_group = sharedptr<LayoutGroup>::cast_dynamic(item);
        if(sub_group)
          sub_group->remove_field(field_name);
      }
    }

    ++iterItem;
  }
}

void LayoutGroup::remove_field(const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  //Look at each item:
  LayoutGroup::type_map_items::iterator iterItem = m_map_items.begin();
  while(iterItem != m_map_items.end())
  {
    sharedptr<LayoutItem> item = iterItem->second;
    sharedptr<LayoutItem_Field> field_item = sharedptr<LayoutItem_Field>::cast_dynamic(item);
    if(field_item)
    {
      if(field_item->get_has_relationship_name()) //If it's related table.
      {
        sharedptr<const Relationship> relationship = field_item->get_relationship();
        if(relationship)
        {
          if(relationship->get_to_table() == table_name)
          {
            if(field_item->get_name() == field_name)
            {
              m_map_items.erase(iterItem);
              iterItem = m_map_items.begin(); //Start again, because we changed the container.AddDel 
              continue;
            }
          }
        }
      }
    }
    else
    {
      sharedptr<LayoutGroup> sub_group = sharedptr<LayoutGroup>::cast_dynamic(item);
      if(sub_group)
        sub_group->remove_field(table_name, field_name);
    }

    ++iterItem;
  }
}

void LayoutGroup::change_related_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new)
{
  //Look at each item:
  for(LayoutGroup::type_map_items::iterator iterItem = m_map_items.begin(); iterItem != m_map_items.end(); ++iterItem)
  {
    sharedptr<LayoutItem> item = iterItem->second;
    sharedptr<LayoutItem_Field> field_item = sharedptr<LayoutItem_Field>::cast_dynamic(item);
    if(field_item)
    {
      if(field_item->get_has_relationship_name()) //If it's related table.
      {
        sharedptr<const Relationship> relationship = field_item->get_relationship();
        if(relationship)
        {
          if(relationship->get_to_table() == table_name)
          {
            if(field_item->get_name() == field_name)
              field_item->set_name(field_name_new); //Change it.
          }
        }
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

void LayoutGroup::change_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new)
{
  //Look at each item:
  for(LayoutGroup::type_map_items::iterator iterItem = m_map_items.begin(); iterItem != m_map_items.end(); ++iterItem)
  {
    sharedptr<LayoutItem> item = iterItem->second;
    sharedptr<LayoutItem_Field> field_item = sharedptr<LayoutItem_Field>::cast_dynamic(item);
    if(field_item)
    {
      if(field_item->get_has_relationship_name()) //If it's a related table (this would be a self-relationship)
      {
        sharedptr<const Relationship> rel = field_item->get_relationship();
        if(rel)
        {
          if(rel->get_to_table() == table_name)
          {
            if(field_item->get_name() == field_name)
              field_item->set_name(field_name_new); //Change it.
          }
        }
      }
      else
      {
        if(field_item->get_name() == field_name)
          field_item->set_name(field_name_new); //Change it.
      }

      field_item->m_formatting.change_field_name(table_name, field_name, field_name_new);
    }
    else
    {
      sharedptr<LayoutGroup> sub_group = sharedptr<LayoutGroup>::cast_dynamic(item);
      if(sub_group)
        sub_group->change_field_item_name(table_name, field_name, field_name_new);
    }
  }
}

/*
void LayoutGroup::change_relationship_name(const Glib::ustring& table_name, const Glib::ustring& name, const Glib::ustring& name_new)
{
  //Look at each item:
  for(LayoutGroup::type_map_items::iterator iterItem = m_map_items.begin(); iterItem != m_map_items.end(); ++iterItem)
  {
    LayoutItem_Field* field_item = dynamic_cast<LayoutItem_Field*>(iterItem->second);
    if(field_item)
    {
      if(field_item->get_has_relationship_name())
      {
        if(field_item->get_relationship_name() == name)
        {
          field_item->set_relationship_name(name_new);
        }
      }

      field_item->m_formatting.change_relationship_name(table_name, name, name_new);
    }
    else
    {
      LayoutGroup* sub_group = dynamic_cast<LayoutGroup*>(iterItem->second);
      if(sub_group)
        sub_group->change_relationship_name(table_name, name, name_new);
    }
  }
}
*/


Glib::ustring LayoutGroup::get_part_type_name() const
{
  return _("Group");
}

/*
void LayoutGroup::debug(guint level) const
{
  g_warning("LayoutGroup::debug() level =%d", level);

  for(type_map_items::const_iterator iter = m_map_items.begin(); iter != m_map_items.end(); ++iter)
  {
    const LayoutGroup* group = dynamic_cast<const LayoutGroup*>(iter->second);
    if(group)
      group->debug(level + 1);
    else
    {
      const LayoutItem_Field* field = dynamic_cast<const LayoutItem_Field*>(iter->second);
      if(field)
      {
        g_warning("  field: name=%s, relationship=%s", field->get_name().c_str(), field->get_relationship_name().c_str());
      }
    }
  }
}
*/

