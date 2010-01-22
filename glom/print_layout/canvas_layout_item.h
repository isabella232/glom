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
#include <libglom/data_structure/layout/layoutitem_withformatting.h>
#include <libgdamm/value.h>

namespace Glom
{

class CanvasTextMovable;
class FieldFormatting;
class LayoutItem_Portal;

/** This has the appropriate child canvas item, depending on the type of the child LayoutItem.
 */
class CanvasLayoutItem : public CanvasGroupResizable
{
private:
  CanvasLayoutItem();
  CanvasLayoutItem(const sharedptr<LayoutItem>& layout_item);
  virtual ~CanvasLayoutItem();

public:

  static Glib::RefPtr<CanvasLayoutItem> create();

  //Creates a new canvas item, with an appropriate child canvas item,
  //and sets the position and size of this canvas item to the position in the LayoutItem.
  static Glib::RefPtr<CanvasLayoutItem> create(const sharedptr<LayoutItem>& layout_item);

  sharedptr<LayoutItem> get_layout_item();

  //Create an appropriate child canvas item,
  //and sets the position and size of this canvas item to the position in the LayoutItem.
  void set_layout_item(const sharedptr<LayoutItem>& layout_item);
  
  /// Make the canvas item show actual data instead of, for instance, a field name.
  void set_db_data(const Gnome::Gda::Value& value);

  /// Hide the missing-image pixbuf from images, for instance.
  void remove_empty_indicators();

  static int get_rows_count_for_portal(const sharedptr<const LayoutItem_Portal>& portal, double& row_height);

private:
  /// Create the appropriate inner canvas item to represent the layout item.
  static Glib::RefPtr<CanvasItemMovable> create_canvas_item_for_layout_item(const sharedptr<LayoutItem>& layout_item);

  static void apply_formatting(const Glib::RefPtr<CanvasTextMovable>& canvas_item, const sharedptr<const LayoutItem_WithFormatting>& layout_item);
  
  void on_resized();

  sharedptr<LayoutItem> m_layout_item;
};

} //namespace Glom


#endif //GLOM_MODE_DESIGN_PRINT_LAYOUTS_CANVAS_LAYOUT_ITEM_H
