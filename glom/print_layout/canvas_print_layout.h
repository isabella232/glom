/* Glom
 *
 * Copyright (C) 2007 Murray Cumming
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

#ifndef GLOM_PRINT_LAYOUT_CANVAS_PRINT_LAYOUT_H
#define GLOM_PRINT_LAYOUT_CANVAS_PRINT_LAYOUT_H

#include <glom/base_db.h>
#include <glom/utility_widgets/canvas/canvas_editable.h>
#include <glom/print_layout/canvas_layout_item.h>
#include <libglom/data_structure/print_layout.h>
#include <libglom/data_structure/layout/layoutitem_line.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/toggleaction.h>
#include <gtkmm/pagesetup.h>

namespace Glom
{

class Dialog_TextFormatting;
class LayoutItem_Portal;

/// A canvas that contains CanvasLayoutItem items.
class Canvas_PrintLayout
 : public CanvasEditable,
   public Base_DB
{
public:
  Canvas_PrintLayout();
  virtual ~Canvas_PrintLayout();

  void set_print_layout(const Glib::ustring& table_name, const sharedptr<const PrintLayout>& print_layout);
  sharedptr<PrintLayout> get_print_layout();

  void set_page_setup(const Glib::RefPtr<Gtk::PageSetup>& page_setup);
  Glib::RefPtr<Gtk::PageSetup> get_page_setup();

  void set_zoom_percent(guint percent);
 
  /** Hide the bounds rectangle and the margin lines:
   */
  void hide_page_bounds();

  //Override: TODO: Is it really?
  void add_canvas_layout_item(const Glib::RefPtr<CanvasLayoutItem>& item);

  void remove_canvas_layout_item(const Glib::RefPtr<CanvasLayoutItem>& item);

  void fill_with_data(const FoundSet& found_set);

  virtual void set_grid_gap(double gap = 20.0);

  void set_outlines_visibility(bool visible = true);

  /** Get any items that have get_selected()==true.
   */
  virtual type_vec_items get_selected_items();

  /** Set all items as selected or unselected. 
   * @param selected Use false to unselect all.
   */
  void select_all(bool selected = true);
  
private:

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void setup_context_menu();
#endif

  void add_layout_group(const sharedptr<LayoutGroup>& group, bool is_top_level = false);
  void add_layout_group_children(const sharedptr<LayoutGroup>& group);
  void fill_layout_group(const sharedptr<LayoutGroup>& group);

  //These are not static, because they need access to the document:
  void fill_with_data(const Glib::RefPtr<Goocanvas::Group>& canvas_group, const FoundSet& found_set);
  void fill_with_data_portal(const Glib::RefPtr<CanvasLayoutItem>& canvas_item, const Gnome::Gda::Value& foreign_key_value);
  static void set_canvas_item_field_value(const Glib::RefPtr<Goocanvas::Item> canvas_item, const sharedptr<LayoutItem_Field> field, const Gnome::Gda::Value& value);

  static Glib::RefPtr<Goocanvas::Item> get_canvas_table_cell_child(const Glib::RefPtr<Goocanvas::Table>& table, int row, int col); //TODO: Add this to Goocanvas::Table.
  
  type_vecConstLayoutFields get_portal_fields_to_show(const sharedptr<LayoutItem_Portal>& portal);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  sharedptr<LayoutItem_Portal> offer_related_records(const sharedptr<LayoutItem_Portal>& portal, Gtk::Window* parent);
  sharedptr<LayoutItem_Line> offer_line(const sharedptr<LayoutItem_Line>& portal, Gtk::Window* parent);
  
  //TODO: Make the signal send the item, so we can pass it by const reference:
  void on_item_show_context_menu(guint button, guint32 activate_time, Glib::RefPtr<CanvasLayoutItem> item);
  void on_context_menu_edit();
  void on_context_menu_formatting();
  void on_context_menu_delete();
  bool on_background_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);

  void on_dialog_format_hide();
#endif

  Glib::RefPtr<Goocanvas::Polyline> create_margin_line(double x1, double y1, double x2, double y2);

  Glib::ustring m_table_name;
  bool m_modified; //TODO: Actually check this?

  //A group containing the layout items, so we can remove them without removing anything else:
  Glib::RefPtr<Goocanvas::Group> m_items_group;

  //A rectangle to show the bounds:
  Glib::RefPtr<Goocanvas::Group> m_bounds_group; //the page and its margins.
  Glib::RefPtr<Goocanvas::Rect> m_bounds_rect;
  Glib::RefPtr<Goocanvas::Polyline> m_margin_left, m_margin_right, m_margin_top, m_margin_bottom;

  //Context menu for existing items:
  Gtk::Menu* m_context_menu;
  Glib::RefPtr<Gtk::ActionGroup> m_context_menu_action_group;
  Glib::RefPtr<Gtk::UIManager> m_context_menu_uimanager;

  Glib::RefPtr<Gtk::Action> m_action_edit, m_action_formatting, m_action_delete;
  Glib::RefPtr<CanvasLayoutItem> m_context_item; //The selected item when showing the context menu.,

  Glib::RefPtr<Gtk::PageSetup> m_page_setup;

  Dialog_TextFormatting* m_dialog_format;

  bool m_outline_visibility;
};

} //namespace Glom

#endif // GLOM_PRINT_LAYOUT_CANVAS_PRINT_LAYOUT_H
