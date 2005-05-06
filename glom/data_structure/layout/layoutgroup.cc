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
#include <glibmm/i18n.h>

LayoutGroup::LayoutGroup()
: m_columns_count(1) //A sensible default
{
}

LayoutGroup::LayoutGroup(const LayoutGroup& src)
: LayoutItem(src),
  m_title(src.m_title),
  m_columns_count(src.m_columns_count)
{
  //Deep copy of the items map:
  for(type_map_items::const_iterator iter = src.m_map_items.begin(); iter != src.m_map_items.end(); ++iter)
  {
    m_map_items[iter->first] = iter->second->clone();
  }   
}

LayoutGroup::~LayoutGroup()
{
  remove_all_items();
}

void LayoutGroup::remove_all_items()
{
  //Delete the items:
  for(type_map_items::iterator iter = m_map_items.begin(); iter != m_map_items.end(); ++iter)
  {
    LayoutItem* item = iter->second;
    if(item)
      delete item;
  }

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

    m_title = src.m_title;
    m_columns_count = src.m_columns_count;

    //Deep copy of the items map:
    remove_all_items();
    for(type_map_items::const_iterator iter = src.m_map_items.begin(); iter != src.m_map_items.end(); ++iter)
    {
      m_map_items[iter->first] = iter->second->clone();
    }
  }
  
  return *this;
}

bool LayoutGroup::has_field(const Glib::ustring& field_name) const
{
  for(type_map_items::const_iterator iter = m_map_items.begin(); iter != m_map_items.end(); ++iter)
  {
    LayoutItem_Field* field_item = dynamic_cast<LayoutItem_Field*>(iter->second);
    if(field_item)
    {
      if(field_item->get_name() == field_name)
        return true;
    }
    else
    {
      //Recurse into the child groups:
      LayoutGroup* group_item = dynamic_cast<LayoutGroup*>(iter->second);
      if(group_item)
      {
        if(group_item->has_field(field_name))
          return true;
      }
    }    
  }

  return false;
}

void LayoutGroup::add_item(const LayoutItem& item)
{
  //Get next available sequence:
  guint sequence = 0;
   if(!m_map_items.empty())
     sequence = m_map_items.rbegin()->first;
  ++sequence;

  add_item(item, sequence);
}

void LayoutGroup::add_item(const LayoutItem& item, guint sequence)
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
  m_map_items[sequence] = item.clone();
  m_map_items[sequence]->m_sequence = sequence;
}

void LayoutGroup::remove_item(guint sequence)
{
  //Delete any existing item at this position:
  type_map_items::iterator iterFind = m_map_items.find(sequence);
  if(iterFind != m_map_items.end())
  {
    delete iterFind->second;
    iterFind->second = 0;

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

void LayoutGroup::change_field_item_name(const Glib::ustring& field_name, const Glib::ustring& field_name_new)
{
  //Look at each item:
  for(LayoutGroup::type_map_items::iterator iterItem = m_map_items.begin(); iterItem != m_map_items.end(); ++iterItem)
  {
    LayoutItem_Field* field_item = dynamic_cast<LayoutItem_Field*>(iterItem->second);
    if(field_item)
    {
      if(iterItem->second->get_name() == field_name)
        iterItem->second->set_name(field_name_new); //Change it.
    }
  }
}

Glib::ustring LayoutGroup::get_part_type_name() const
{
  return _("Group");
}
