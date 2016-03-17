/* Glom
 *
 * Copyright (C) 2001-2016 Murray Cumming
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

#include <libglom/layout_utils.h>
#include <libglom/db_utils.h>
#include <libglom/connectionpool.h>
#include <libglom/string_utils.h>
#include "privs.h"

#include <giomm/file.h>
#include <giomm/resource.h>
#include <glibmm/convert.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <iostream>


namespace Glom
{

Glib::ustring Utils::get_list_of_layout_items_for_display(const LayoutGroup::type_list_items& list_layout_fields)
{
  Glib::ustring result;
  for(const auto& item : list_layout_fields)
  {
    if(item)
    {
      if(!result.empty())
       result += ", ";

      result += item->get_layout_display_name();
    }
  }

  return result;
}

Glib::ustring Utils::get_list_of_layout_items_for_display(const std::shared_ptr<const LayoutGroup>& layout_group)
{
  if(layout_group)
    return get_list_of_layout_items_for_display(layout_group->m_list_items);
  else
    return Glib::ustring();
}

Glib::ustring Utils::get_list_of_sort_fields_for_display(const Formatting::type_list_sort_fields& sort_fields)
{
  Glib::ustring text;
  for(const auto& the_pair : sort_fields)
  {
    const auto item = the_pair.first;
    if(!item)
      continue;
    
    if(!text.empty())
      text += ", ";

    text += item->get_layout_display_name();
    //TODO: Show Ascending/Descending?
  }

  return text;
}

LayoutGroup::type_list_const_items Utils::get_layout_items_plus_primary_key(const LayoutGroup::type_list_const_items& items, const std::shared_ptr<const Document>& document, const Glib::ustring& table_name)
{
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document was null.\n";
    return items;
  }

  const auto field_primary_key = document->get_field_primary_key(table_name);
  if(!field_primary_key)
  {
    std::cerr << G_STRFUNC << ": Could not find the primary key.\n";
    return items;
  }

  auto pk_layout_item = std::make_shared<LayoutItem_Field>();
  pk_layout_item->set_hidden();
  pk_layout_item->set_full_field_details(field_primary_key);
  
  if(find_if_layout_item_field_is_same_field_exists(items, pk_layout_item))
    return items; //It is already in the list:

  LayoutGroup::type_list_const_items items_plus_pk = items;
  items_plus_pk.emplace_back(pk_layout_item);
  return items_plus_pk;
}

//TODO: Avoid the horrible code duplication with the const version.
LayoutGroup::type_list_items Utils::get_layout_items_plus_primary_key(const LayoutGroup::type_list_items& items, const std::shared_ptr<const Document>& document, const Glib::ustring& table_name)
{
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document was null.\n";
    return items;
  }

  const auto field_primary_key = document->get_field_primary_key(table_name);
  if(!field_primary_key)
  {
    std::cerr << G_STRFUNC << ": Could not find the primary key.\n";
    return items;
  }

  auto pk_layout_item = std::make_shared<LayoutItem_Field>();
  pk_layout_item->set_hidden();
  pk_layout_item->set_full_field_details(field_primary_key);
  
  if(find_if_layout_item_field_is_same_field_exists(items, pk_layout_item))
    return items; //It is already in the list:

  LayoutGroup::type_list_items items_plus_pk = items;
  items_plus_pk.emplace_back(pk_layout_item);
  return items_plus_pk;
}

namespace {

static bool get_field_primary_key_index_for_fields(const Utils::type_vec_fields& fields, guint& field_column)
{
  //Initialize input parameter:
  field_column = 0;

  //TODO_performance: Cache the primary key?
  guint col = 0;
  guint cols_count = fields.size();
  while(col < cols_count)
  {
    if(fields[col]->get_primary_key())
    {
      field_column = col;
      return true;
    }
    else
    {
      ++col;
    }
  }

  return false; //Not found.
}

static void get_table_fields_to_show_for_sequence_add_group(const std::shared_ptr<const Document>& document, const Glib::ustring& table_name, const Privileges& table_privs, const Utils::type_vec_fields& all_db_fields, const std::shared_ptr<LayoutGroup>& group, Utils::type_vecConstLayoutFields& vecFields)
{
  //g_warning("Box_Data::get_table_fields_to_show_for_sequence_add_group(): table_name=%s, all_db_fields.size()=%d, group->name=%s", table_name.c_str(), all_db_fields.size(), group->get_name().c_str());

  for(const auto& item : group->get_items())
  {
    auto item_field = std::dynamic_pointer_cast<LayoutItem_Field>(item);
    if(item_field)
    {
      //Get the field info:
      const auto field_name = item->get_name();

      if(item_field->get_has_relationship_name()) //If it's a field in a related table.
      {
        //TODO_Performance: get_fields_for_table_one_field() is probably very inefficient
        auto field = DbUtils::get_fields_for_table_one_field(document, item_field->get_table_used(table_name), item->get_name());
        if(field)
        {
          auto layout_item = item_field;
          layout_item->set_full_field_details(field); //Fill in the full field information for later.


          //TODO_Performance: We do this once for each related field, even if there are 2 from the same table:
          const auto privs_related = Privs::get_current_privs(item_field->get_table_used(table_name));
          layout_item->m_priv_view = privs_related.m_view;
          layout_item->m_priv_edit = privs_related.m_edit;

          vecFields.emplace_back(layout_item);
        }
        else
        {
          std::cerr << G_STRFUNC << ": related field not found: field=" << item->get_layout_display_name() << std::endl;
        }
      }
      else //It's a regular field in the table:
      {
        const auto iterFind = find_if_same_name(all_db_fields, field_name);

        //If the field does not exist anymore then we won't try to show it:
        if(iterFind != all_db_fields.end() )
        {
          auto layout_item = item_field;
          layout_item->set_full_field_details(*iterFind); //Fill the LayoutItem with the full field information.

          //std::cout << "debug: " << G_STRFUNC << ": name=" << layout_item->get_name() << std::endl;

          //Prevent editing of the field if the user may not edit this table:
          layout_item->m_priv_view = table_privs.m_view;
          layout_item->m_priv_edit = table_privs.m_edit;

          vecFields.emplace_back(layout_item);
        }
      }
    }
    else
    {
      auto item_group = std::dynamic_pointer_cast<LayoutGroup>(item);
      if(item_group)
      {
        auto item_portal = std::dynamic_pointer_cast<LayoutItem_Portal>(item);
        if(!item_portal) //Do not recurse into portals. They are filled by means of a separate SQL query.
        {
          //Recurse:
          get_table_fields_to_show_for_sequence_add_group(document, table_name, table_privs, all_db_fields, item_group, vecFields);
        }
      }
    }
  }

  if(vecFields.empty())
  {
    //std::cerr << G_STRFUNC << ": Returning empty list.\n";
  }
}

} //anonymous namespace

Utils::type_vecConstLayoutFields Utils::get_table_fields_to_show_for_sequence(const std::shared_ptr<const Document>& document, const Glib::ustring& table_name, const Document::type_list_layout_groups& mapGroupSequence)
{
  //Get field definitions from the database, with corrections from the document:
  auto all_fields = DbUtils::get_fields_for_table(document, table_name);

  const auto table_privs = Privs::get_current_privs(table_name);

  //Get fields that the document says we should show:
  type_vecConstLayoutFields result;
  if(document)
  {
    if(mapGroupSequence.empty())
    {
      //No field sequence has been saved in the document, so we use all fields by default, so we start with something visible:

      //Start with the Primary Key as the first field:
      guint iPrimaryKey = 0;
      bool bPrimaryKeyFound = get_field_primary_key_index_for_fields(all_fields, iPrimaryKey);
      Glib::ustring primary_key_field_name;
      if(bPrimaryKeyFound)
      {
        auto layout_item = std::make_shared<LayoutItem_Field>();
        layout_item->set_full_field_details(all_fields[iPrimaryKey]);

        //Don't use thousands separators with ID numbers:
        layout_item->m_formatting.m_numeric_format.m_use_thousands_separator = false;

        layout_item->set_editable(true); //A sensible default.

        //Prevent editing of the field if the user may not edit this table:
        layout_item->m_priv_view = table_privs.m_view;
        layout_item->m_priv_edit = table_privs.m_edit;

        result.emplace_back(layout_item);
      }

      //Add the rest:
      for(const auto& field_info : all_fields)
      {
        if(field_info->get_name() != primary_key_field_name) //We already added the primary key.
        {
          auto layout_item = std::make_shared<LayoutItem_Field>();
          layout_item->set_full_field_details(field_info);

          layout_item->set_editable(true); //A sensible default.

          //Prevent editing of the field if the user may not edit this table:
          layout_item->m_priv_view = table_privs.m_view;
          layout_item->m_priv_edit = table_privs.m_edit;

          result.emplace_back(layout_item);
        }
      }
    }
    else
    {
      //We will show the fields that the document says we should:
      for(const auto& group : mapGroupSequence)
      {
        if(true) //!group->get_hidden())
        {
          //Get the fields:
          get_table_fields_to_show_for_sequence_add_group(document, table_name, table_privs, all_fields, group, result);
        }
      }
    }
  }

  if(result.empty())
  {
    //std::cerr << G_STRFUNC << ": Returning empty list.\n";
  }

  return result;
}



} //namespace Glom
