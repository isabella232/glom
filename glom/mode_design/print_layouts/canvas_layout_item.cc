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
#include <glibmm/i18n.h>

namespace Glom
{

CanvasLayoutItem::CanvasLayoutItem(const sharedptr<LayoutItem>& layout_item)
: CanvasGroupResizable()
{
  set_layout_item(layout_item);
}

CanvasLayoutItem::~CanvasLayoutItem()
{
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
  //Remove the current child:
  if(get_n_children())
    remove_child(0);

  //Add the new child:
  m_layout_item = item;

  Glib::RefPtr<CanvasItemMovable> child;
  sharedptr<LayoutItem_Text> text = sharedptr<LayoutItem_Text>::cast_dynamic(m_layout_item);
  if(text)
  {
    Glib::RefPtr<CanvasTextMovable> canvas_item = CanvasTextMovable::create();
    canvas_item->property_text() = text->get_text();
    child = canvas_item;
  }
  else
  {
    sharedptr<LayoutItem_Image> image = sharedptr<LayoutItem_Image>::cast_dynamic(m_layout_item);
    if(image)
    {
      Glib::RefPtr<CanvasRectMovable> canvas_item = CanvasRectMovable::create();
      canvas_item->property_fill_color() = "white"; //This makes the whole area clickable, not just the outline stroke:
      child = canvas_item;
    }
    else
    {
      sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(m_layout_item);
      if(field)
      {
        Glib::RefPtr<CanvasTextMovable> canvas_item = CanvasTextMovable::create();

        Glib::ustring name = field->get_name();
          if(name.empty())
            name = _("Choose Field");
          
        canvas_item->property_text() = name;

        child = canvas_item;
      }
      else
      {
        std::cerr << "CanvasLayoutItem::set_layout_item(): Unhandled LayoutItem type." << std::endl;
      }
    }
  }

  if(child)
  {
    //Set the position and dimensions:
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;
    item->get_print_layout_position(x, y, width, height);

    child->set_xy(x, y);
    child->set_width_height(width, height);

    //We do this after setting the child dimensions, 
    //so that the manipulators are correct, 
    //though it would be nice if the manipulators moved when we repositioned the child explicitly.
    set_child(child);
  }
}

} //namespace Glom

