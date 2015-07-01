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

#include <libglom/data_structure/layout/layoutgroup.h>
#include <libglom/data_structure/layout/layoutitem_field.h>
#include <libglom/data_structure/layout/layoutitem_portal.h>
#include <glibmm/i18n.h>
#include <iostream> 

namespace Glom
{

LayoutGroup::LayoutGroup()
: m_columns_count(1), //A sensible default
  m_border_width(0)
{
}

LayoutGroup::LayoutGroup(const LayoutGroup& src)
: LayoutItem(src),
  m_columns_count(src.m_columns_count),
  m_border_width(src.m_border_width)
{
  //Deep copy of the items map:
  for(type_list_items::const_iterator iter = src.m_list_items.begin(); iter != src.m_list_items.end(); ++iter)
  {
    if(*iter)
      m_list_items.push_back( glom_sharedptr_clone(*iter) );
  }
}

LayoutGroup::~LayoutGroup()
{
  remove_all_items();
}

void LayoutGroup::remove_all_items()
{
  //Delete the items:
  m_list_items.clear();
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

    m_border_width = src.m_border_width;

    //Deep copy of the items map:
    remove_all_items();
    for(type_list_items::const_iterator iter = src.m_list_items.begin(); iter != src.m_list_items.end(); ++iter)
    {
      if(*iter)
        m_list_items.push_back( glom_sharedptr_clone(*iter) );
    }
  }

  return *this;
}

bool LayoutGroup::has_field(const Glib::ustring& parent_table_name, const Glib::ustring& table_name, const Glib::ustring& field_name) const
{
  for(type_list_items::const_iterator iter = m_list_items.begin(); iter != m_list_items.end(); ++iter)
  {
    auto item = *iter;
    auto field_item = std::dynamic_pointer_cast<LayoutItem_Field>(item);
    if(field_item)
    {
      if( (field_item->get_name() == field_name) &&
        (field_item->get_table_used(parent_table_name) == table_name))
      {
        return true;
      }
    }
    else
    {
      //Recurse into the child groups:
      auto group_item = std::dynamic_pointer_cast<LayoutGroup>(item);
      if(group_item)
      {
        if(group_item->has_field(parent_table_name, table_name, field_name))
          return true;
      }
    }
  }

  return false;
}

bool LayoutGroup::has_any_fields() const
{
  for(type_list_items::const_iterator iter = m_list_items.begin(); iter != m_list_items.end(); ++iter)
  {
    auto item = *iter;
    auto field_item = std::dynamic_pointer_cast<LayoutItem_Field>(item);
    if(field_item)
    {
      return true;
    }
    else
    {
      //Recurse into the child groups:
      auto group_item = std::dynamic_pointer_cast<LayoutGroup>(item);
      if(group_item)
      {
        if(group_item->has_any_fields())
          return true;
      }
    }
  }

  return false;
}

void LayoutGroup::add_item(const std::shared_ptr<LayoutItem>& item)
{
  m_list_items.push_back(item);
}

void LayoutGroup::add_item(const std::shared_ptr<LayoutItem>& item, const std::shared_ptr<const LayoutItem>& position)
{
  //Find the position of the item.
  auto unconst = std::const_pointer_cast<LayoutItem>(position);
  auto iter = std::find(m_list_items.begin(), m_list_items.end(), unconst);

  //std::vector::insert() adds before rather than after:
  // jhs: We want to add after rather than before - at least for dnd
  //++iter;

  m_list_items.insert(iter, item);
}

void LayoutGroup::remove_item (const std::shared_ptr<LayoutItem>& item)
{
  auto unconst = std::const_pointer_cast<LayoutItem>(item);
  auto iter = std::find(m_list_items.begin(), m_list_items.end(), unconst);
  m_list_items.erase(iter);
}

LayoutGroup::type_list_items LayoutGroup::get_items()
{
  return m_list_items;
}

LayoutGroup::type_list_const_items LayoutGroup::get_items() const
{
  //Get a const map from the non-const map:
  //TODO_Performance: Surely we should not need to copy the structure just to constize it?
  type_list_const_items result;

  for(type_list_items::const_iterator iter = m_list_items.begin(); iter != m_list_items.end(); ++iter)
  {
    result.push_back(*iter);
  }

  return result;
}

LayoutGroup::type_list_const_items LayoutGroup::get_items_recursive() const
{
  type_list_const_items result;

  for(type_list_items::const_iterator iter = m_list_items.begin(); iter != m_list_items.end(); ++iter)
  {
    const std::shared_ptr<const LayoutItem> item = *iter;
    
    std::shared_ptr<const LayoutGroup> group = std::dynamic_pointer_cast<const LayoutGroup>(item);
    if(group)
    {
      const auto sub_result = group->get_items_recursive();
      result.insert(result.end(), sub_result.begin(), sub_result.end());
    }
    else
      result.push_back(item);
  }

  return result;
}

LayoutGroup::type_list_items LayoutGroup::get_items_recursive()
{
  type_list_items result;

  for(type_list_items::const_iterator iter = m_list_items.begin(); iter != m_list_items.end(); ++iter)
  {
    const auto item = *iter;
    
    auto group = std::dynamic_pointer_cast<LayoutGroup>(item);
    if(group)
    {
      const auto sub_result = group->get_items_recursive();
      result.insert(result.end(), sub_result.begin(), sub_result.end());
    }
    else
      result.push_back(item);
  }

  return result;
}

LayoutGroup::type_list_const_items LayoutGroup::get_items_recursive_with_groups() const
{
  type_list_const_items result;

  for(type_list_items::const_iterator iter = m_list_items.begin(); iter != m_list_items.end(); ++iter)
  {
    const std::shared_ptr<const LayoutItem> item = *iter;
    
    //Add the item itself:
    result.push_back(item);
    
    std::shared_ptr<const LayoutGroup> group = std::dynamic_pointer_cast<const LayoutGroup>(item);
    if(group)
    {
      const auto sub_result = group->get_items_recursive_with_groups();
      result.insert(result.end(), sub_result.begin(), sub_result.end());
    }
  }

  return result;
}

void LayoutGroup::remove_relationship(const std::shared_ptr<const Relationship>& relationship)
{
  auto iterItem = m_list_items.begin();
  while(iterItem != m_list_items.end())
  {
    auto item = *iterItem;
    auto uses_rel = std::dynamic_pointer_cast<UsesRelationship>(item);
    if(uses_rel)
    {
      if(uses_rel->get_has_relationship_name())
      {
        if(*(uses_rel->get_relationship()) == *relationship) //TODO_Performance: Slow if there are lots of translations.
        {
          m_list_items.erase(iterItem);
          iterItem = m_list_items.begin(); //Start again, because we changed the container.AddDel
          continue;
        }
      }
    }

    auto sub_group = std::dynamic_pointer_cast<LayoutGroup>(item);
    if(sub_group)
      sub_group->remove_relationship(relationship);

    ++iterItem;
  }
}

void LayoutGroup::remove_field(const Glib::ustring& parent_table_name, const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  //Look at each item:
  auto iterItem = m_list_items.begin();
  while(iterItem != m_list_items.end())
  {
    auto item = *iterItem;
    auto field_item = std::dynamic_pointer_cast<LayoutItem_Field>(item);
    if(field_item)
    {
      if(field_item->get_table_used(parent_table_name) == table_name)
      {
        if(field_item->get_name() == field_name)
        {
          m_list_items.erase(iterItem);
          iterItem = m_list_items.begin(); //Start again, because we changed the container.AddDel
          continue;
        }
      }
    }
    else
    {
      auto sub_group = std::dynamic_pointer_cast<LayoutGroup>(item);
      if(sub_group)
        sub_group->remove_field(parent_table_name, table_name, field_name);
    }

    ++iterItem;
  }
}

void LayoutGroup::change_related_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new)
{
  //Look at each item:
  for(LayoutGroup::type_list_items::iterator iterItem = m_list_items.begin(); iterItem != m_list_items.end(); ++iterItem)
  {
    auto item = *iterItem;
    auto field_item = std::dynamic_pointer_cast<LayoutItem_Field>(item);
    if(field_item)
    {
      if(field_item->get_has_relationship_name()) //If it's related table.
      {
        std::shared_ptr<const Relationship> relationship = field_item->get_relationship();
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
      auto sub_group = std::dynamic_pointer_cast<LayoutGroup>(item);
      if(sub_group)
        sub_group->change_field_item_name(table_name, field_name, field_name_new);
    }
  }
}

void LayoutGroup::change_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new)
{
  //Look at each item:
  for(LayoutGroup::type_list_items::iterator iterItem = m_list_items.begin(); iterItem != m_list_items.end(); ++iterItem)
  {
    auto item = *iterItem;
    std::shared_ptr<LayoutItem_Field> field_item = 
      std::dynamic_pointer_cast<LayoutItem_Field>(item);
    
    //Field layout items:
    if(field_item)
    {
      if(field_item->get_has_relationship_name()) //If it's a related table (this would be a self-relationship)
      {
        std::shared_ptr<const Relationship> rel = field_item->get_relationship();
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
    }
    else
    {
      //Formatting:
      std::shared_ptr<LayoutItem_WithFormatting> with_formatting = 
        std::dynamic_pointer_cast<LayoutItem_WithFormatting>(item);
      if(with_formatting)
      {
        auto formatting = with_formatting->m_formatting;
        formatting.change_field_item_name(table_name, field_name, field_name_new);
      }
   
      //Recurse into sub-groups:
      auto sub_group = std::dynamic_pointer_cast<LayoutGroup>(item);
      if(sub_group)
        sub_group->change_field_item_name(table_name, field_name, field_name_new);
    }
  }
}

/*
void LayoutGroup::change_relationship_name(const Glib::ustring& table_name, const Glib::ustring& name, const Glib::ustring& name_new)
{
  //Look at each item:
  for(LayoutGroup::type_list_items::iterator iterItem = m_list_items.begin(); iterItem != m_list_items.end(); ++iterItem)
  {
    auto field_item = dynamic_cast<LayoutItem_Field*>(*iterItem);
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
      auto sub_group = dynamic_cast<LayoutGroup*>(*iterItem);
      if(sub_group)
        sub_group->change_relationship_name(table_name, name, name_new);
    }
  }
}
*/


Glib::ustring LayoutGroup::get_part_type_name() const
{
  //Translators: This is the name of a UI element (a layout part name).
  return _("Group");
}

Glib::ustring LayoutGroup::get_report_part_id() const
{
  return "group";
}

guint LayoutGroup::get_items_count() const
{
  return m_list_items.size();
}

/*
void LayoutGroup::debug(guint level) const
{
  for(int i = 0; i < level; ++i)
    std::cout << " ";

  std::cout << "LayoutGroup::debug() level =" << level << std::endl;

  for(type_list_items::const_iterator iter = m_list_items.begin(); iter != m_list_items.end(); ++iter)
  {
    auto group = std::dynamic_pointer_cast<LayoutGroup>(*iter);
    if(group)
      group->debug(level + 1);
    else
    {
      auto field = std::dynamic_pointer_cast<LayoutItem_Field>(*iter);
      if(field)
      {
        for(int i = 0; i < level; ++i)
          std::cout << " ";

        std::cout << " field: name=" << field->get_name() << ", relationship=" << field->get_relationship_name() << std::endl;
      }
    }
  }
}
*/

guint LayoutGroup::get_columns_count() const
{
  return m_columns_count;
}

void LayoutGroup::set_columns_count(guint columns_count)
{
  m_columns_count = columns_count;
}

double LayoutGroup::get_border_width() const
{
  return m_border_width;
}

void LayoutGroup::set_border_width(double border_width)
{
  m_border_width = border_width;
}

} //namespace Glom
