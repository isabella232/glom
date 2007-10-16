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

#include "canvas_layout_item.h"
#include <glom/utility_widgets/canvas/canvas_rect_movable.h>
#include <glom/utility_widgets/canvas/canvas_text_movable.h>
#include <glom/utility_widgets/canvas/canvas_line_movable.h>
#include <glom/utility_widgets/canvas/canvas_group_movable.h>

#include <glom/libglom/data_structure/layout/layoutitem_button.h>
#include <glom/libglom/data_structure/layout/layoutitem_text.h>
#include <glom/libglom/data_structure/layout/layoutitem_image.h>
#include <glom/libglom/data_structure/layout/layoutitem_field.h>

namespace Glom
{

CanvasLayoutItem::CanvasLayoutItem(const sharedptr<LayoutItem>& layout_item)
{
  m_layout_item = layout_item;

  sharedptr<LayoutItem_Text> text = sharedptr<LayoutItem_Text>::cast_dynamic(layout_item);
  if(false) //text)
  {
    Glib::RefPtr<CanvasTextMovable> canvas_item = CanvasTextMovable::create();
    canvas_item->property_x() = 10.0;
    canvas_item->property_y() = 10.0;
    canvas_item->property_text() = text->get_text();
    //set_child(canvas_item);
  }
  else
  {
    Glib::RefPtr<CanvasRectMovable> canvas_item = CanvasRectMovable::create();
    canvas_item->property_x() = 10.0;
    canvas_item->property_y() = 10.0;
    canvas_item->property_height() = 20.0;
    canvas_item->property_width() = 100.0;
    set_child(canvas_item);
  }
}

Glib::RefPtr<CanvasLayoutItem> CanvasLayoutItem::create(const sharedptr<LayoutItem>& layout_item)
{
  return Glib::RefPtr<CanvasLayoutItem>(new CanvasLayoutItem(layout_item));
}

sharedptr<LayoutItem> CanvasLayoutItem::get_layout_item()
{
  return m_layout_item;
}

void CanvasLayoutItem::set_layout_item(const sharedptr<LayoutItem>& item)
{
  m_layout_item = item;
}

} //namespace Glom

