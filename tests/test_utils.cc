/* Glom
 *
 * Copyright (C) 2011 Murray Cumming
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

#include "tests/test_utils.h"

Glom::sharedptr<const Glom::LayoutItem_Field> get_field_on_layout(const Glom::Document& document, const Glib::ustring& layout_table_name, const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  const Glom::Document::type_list_layout_groups groups = 
    document.get_data_layout_groups("details", layout_table_name);

  for(Glom::Document::type_list_layout_groups::const_iterator iter = groups.begin(); iter != groups.end(); ++iter)
  {
    const Glom::sharedptr<const Glom::LayoutGroup> group = *iter;
    if(!group)
      continue;
    
    const Glom::LayoutGroup::type_list_const_items items = group->get_items_recursive();
    for(Glom::LayoutGroup::type_list_const_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
    {
      const Glom::sharedptr<const Glom::LayoutItem> layout_item = *iter;
      const Glom::sharedptr<const Glom::LayoutItem_Field> layout_item_field =
        Glom::sharedptr<const Glom::LayoutItem_Field>::cast_dynamic(layout_item);
      if(!layout_item_field)
        continue;

      if( (layout_item_field->get_table_used(layout_table_name) == table_name) &&
        (layout_item_field->get_name() == field_name) )
      {
        return layout_item_field;
      }
    }
  }
  
  return Glom::sharedptr<const Glom::LayoutItem_Field>();
}


