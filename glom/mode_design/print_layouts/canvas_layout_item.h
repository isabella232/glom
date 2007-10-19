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

#ifndef GLOM_MODE_DESIGN_PRINT_LAYOUTS_CANVAS_LAYOUT_ITEM_H
#define GLOM_MODE_DESIGN_PRINT_LAYOUTS_CANVAS_LAYOUT_ITEM_H

#include <glom/utility_widgets/canvas/canvas_group_resizable.h>
#include <glom/libglom/data_structure/layout/layoutitem.h>

namespace Glom
{

class CanvasLayoutItem : public CanvasGroupResizable
{
protected:
  CanvasLayoutItem(const sharedptr<LayoutItem>& layout_item);
  virtual ~CanvasLayoutItem();

public:
  static Glib::RefPtr<CanvasLayoutItem> create(const sharedptr<LayoutItem>& layout_item);

  sharedptr<LayoutItem> get_layout_item();
  void set_layout_item(const sharedptr<LayoutItem>& item);

protected:
  sharedptr<LayoutItem> m_layout_item;
};

} //namespace Glom


#endif //GLOM_MODE_DESIGN_PRINT_LAYOUTS_CANVAS_LAYOUT_ITEM_H
