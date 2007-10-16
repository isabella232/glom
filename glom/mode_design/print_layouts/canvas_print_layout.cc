
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

  remove_all_items();
  add_group_children(print_layout->m_layout_group);

  m_modified = false;
}

sharedptr<PrintLayout> Canvas_PrintLayout::get_print_layout()
{
  sharedptr<PrintLayout> result = sharedptr<PrintLayout>::create();
  fill_layout_group(result->m_layout_group);
  return result;
}

Glib::RefPtr<CanvasLayoutItem> Canvas_PrintLayout::create_canvas_item(const sharedptr<LayoutItem>& item)
{
  Glib::RefPtr<CanvasLayoutItem> result = CanvasLayoutItem::create(item);

  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;
  item->get_print_layout_position(x, y, width, height);

  result->set_xy(x, y);
  result->set_width_height(width, height);

  return result;
}

void Canvas_PrintLayout::add_group_children(const sharedptr<LayoutGroup>& group)
{
  for(LayoutGroup::type_map_items::const_iterator iter = group->m_map_items.begin(); iter != group->m_map_items.end(); ++iter)
  {
    sharedptr<LayoutItem> item = iter->second;
    sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::cast_dynamic(item);
    if(group)
    {
      add_group(group);
    }
    else
    {
      Glib::RefPtr<CanvasLayoutItem> canvas_item = create_canvas_item(item);
      if(canvas_item)
      {
        add_item(canvas_item);
      }
    }
  }

  m_modified = true;
}

void Canvas_PrintLayout::add_group(const sharedptr<LayoutGroup>& group)
{
  //row[model_parts->m_columns.m_col_item] = sharedptr<LayoutItem>(static_cast<LayoutItem*>(group->clone()));

  add_group_children(group);

  m_modified = true;
}


void Canvas_PrintLayout::fill_layout_group(const sharedptr<LayoutGroup>& group)
{
  Glib::RefPtr<Goocanvas::Item> root = get_root_item();
  if(!root)
    return;

  const int count = root->get_n_children();
  for(int i = 0; i < count; ++i)
  {
    Glib::RefPtr<Goocanvas::Item> base_canvas_item = root->get_child(i);
    Glib::RefPtr<CanvasLayoutItem> canvas_item = Glib::RefPtr<CanvasLayoutItem>::cast_dynamic(base_canvas_item);
    if(canvas_item)
    {
      //Get the actual position:
      double x = 0;
      double y = 0;
      canvas_item->get_xy(x, y);
      double width = 0;
      double height = 0;
      canvas_item->get_width_height(width, height);
      if((width != 0) && (height != 0)) //Avoid bogus items.
      {
        sharedptr<LayoutItem> layout_item = canvas_item->get_layout_item();
        layout_item->set_print_layout_position(x, y, width, height);

        group->add_item(layout_item);
      }
    }

    //TODO: Recurse.
  }
}



} //namespace Glom


