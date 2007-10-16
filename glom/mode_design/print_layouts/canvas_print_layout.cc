
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

#include "canvas_print_layout.h"
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>

namespace Glom
{

Canvas_PrintLayout::Canvas_PrintLayout()
: m_modified(false)
{
 
}

Canvas_PrintLayout::~Canvas_PrintLayout()
{
}


void Canvas_PrintLayout::set_print_layout(const Glib::ustring& table_name, const sharedptr<const PrintLayout>& print_layout)
{
  m_modified = false;

  add_group_children(print_layout->m_layout_group);

  m_modified = false;
}

sharedptr<PrintLayout> Canvas_PrintLayout::get_print_layout()
{
  sharedptr<PrintLayout> result;
  return result;
}

void Canvas_PrintLayout::add_group_children(const sharedptr<const LayoutGroup>& group)
{
  for(LayoutGroup::type_map_items::const_iterator iter = group->m_map_items.begin(); iter != group->m_map_items.end(); ++iter)
  {
    sharedptr<const LayoutItem> item = iter->second;
    sharedptr<const LayoutGroup> group = sharedptr<const LayoutGroup>::cast_dynamic(item);
    if(group)
    {
      add_group(group);
    }
    else
    {
      //row[model_parts->m_columns.m_col_item] = glom_sharedptr_clone(item);
    }
  }

  m_modified = true;
}

void Canvas_PrintLayout::add_group(const sharedptr<const LayoutGroup>& group)
{
  //row[model_parts->m_columns.m_col_item] = sharedptr<LayoutItem>(static_cast<LayoutItem*>(group->clone()));

  add_group_children(group);

  m_modified = true;
}


} //namespace Glom


