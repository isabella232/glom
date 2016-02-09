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

#ifndef GLOM_MODE_DESIGN_PRINT_LAYOUTS_CANVAS_LAYOUT_ITEM_H
#define GLOM_MODE_DESIGN_PRINT_LAYOUTS_CANVAS_LAYOUT_ITEM_H

#include <glom/utility_widgets/canvas/canvas_group_resizable.h>
#include <libglom/data_structure/layout/layoutitem_withformatting.h>
#include <libgdamm/value.h>
#include <goocanvasmm/table.h>

namespace Glom
{

class CanvasTextMovable;
class CanvasTableMovable;
class Formatting;
class LayoutItem_Portal;

/** This has the appropriate child canvas item, depending on the type of the child LayoutItem.
 * You should call set_layout_item() after instantiating a CanvasLayoutItem via create(),
 * and after adding the CanvasLayoutItem to a parent CanvasItem that is already in a Goocanvas::Canvas.
 *
 * If the CanvasLayoutItem is not already (indirectly) in a GooCanvas::Canvas then
 * Goocanvas::Image items will show over-scaled images, due to goocanvas bug:
 * https://bugzilla.gnome.org/show_bug.cgi?id=657592#c16 
 */
class CanvasLayoutItem : public CanvasGroupResizable
{
private:
  CanvasLayoutItem();

public:

  static Glib::RefPtr<CanvasLayoutItem> create();

  //Creates a new canvas item, with an appropriate child canvas item,
  //and sets the position and size of this canvas item to the position in the LayoutItem.
  static Glib::RefPtr<CanvasLayoutItem> create(const std::shared_ptr<LayoutItem>& layout_item);

  std::shared_ptr<LayoutItem> get_layout_item();

  //Create an appropriate child canvas item,
  //and sets the position and size of this canvas item to the position in the LayoutItem.
  void set_layout_item(const std::shared_ptr<LayoutItem>& layout_item);
  
  /// Make the canvas item show actual data instead of, for instance, a field name.
  void set_db_data(const Gnome::Gda::Value& value);

  /// Hide the missing-image pixbuf from images, for instance.
  void remove_empty_indicators();

  /** Make sure that the LayoutItem has the same position info as the CanvasItem that represents it.
   */
  void update_layout_position_from_canvas();

  /* Add table child items if any are missing,
   * for instance if the table has been made bigger.
   */
  void add_portal_rows_if_necessary(guint rows_count);

  static Glib::RefPtr<Goocanvas::Item> get_canvas_table_cell_child(const Glib::RefPtr<Goocanvas::Table>& table, int row, int col); //TODO: Add this to Goocanvas::Table.

private:
  /// Create the appropriate inner canvas item to represent the layout item.
  static Glib::RefPtr<CanvasItemMovable> create_canvas_item_for_layout_item(const std::shared_ptr<LayoutItem>& layout_item);

  static void apply_formatting(const Glib::RefPtr<CanvasTextMovable>& canvas_item, const LayoutItem_WithFormatting& layout_item);
  
  static void add_portal_rows_if_necessary(const Glib::RefPtr<CanvasTableMovable>& canvas_table, const std::shared_ptr<LayoutItem_Portal>& portal, guint rows_count);

  void on_resized();

  std::shared_ptr<LayoutItem> m_layout_item;
};

} //namespace Glom


#endif //GLOM_MODE_DESIGN_PRINT_LAYOUTS_CANVAS_LAYOUT_ITEM_H
