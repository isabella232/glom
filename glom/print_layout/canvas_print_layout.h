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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_PRINT_LAYOUT_CANVAS_PRINT_LAYOUT_H
#define GLOM_PRINT_LAYOUT_CANVAS_PRINT_LAYOUT_H

#include <glom/base_db.h>
#include <glom/utility_widgets/canvas/canvas_editable.h>
#include <glom/print_layout/canvas_layout_item.h>
#include <libglom/data_structure/print_layout.h>
#include <libglom/data_structure/layout/layoutitem_line.h>
#include <giomm/simpleactiongroup.h>
#include <gtkmm/pagesetup.h>
#include <gtkmm/menu.h>
#include <glibmm/weakref.h>

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

  void set_print_layout(const Glib::ustring& table_name, const std::shared_ptr<PrintLayout>& print_layout);
  std::shared_ptr<PrintLayout> get_print_layout();

  void set_page_setup(const Glib::RefPtr<Gtk::PageSetup>& page_setup);
  Glib::RefPtr<Gtk::PageSetup> get_page_setup();
  Glib::RefPtr<const Gtk::PageSetup> get_page_setup() const;

  void set_page_count(guint count);
  guint get_page_count() const;

  void set_zoom_percent(guint percent);

  /** Hide the bounds rectangle and the margin lines:
   */
  void hide_page_bounds();

  void add_canvas_layout_item(const Glib::RefPtr<CanvasLayoutItem>& item);

  void remove_canvas_layout_item(const Glib::RefPtr<CanvasLayoutItem>& item);

  /** If the item is a field from the System Preferences table,
   * show the content instead of the field name,
   * because it will be the same for all records.
   */
  void fill_with_data_system_preferences(const Glib::RefPtr<CanvasLayoutItem>& canvas_item, const std::shared_ptr<Document>& document);

  void fill_with_data(const FoundSet& found_set, bool avoid_page_margins);

  void set_grid_gap(double gap = 20.0) override;

  void set_outlines_visibility(bool visible = true);

  /** Get any items that have get_selected()==true.
   */
  type_vec_items get_selected_items() override;

  /** Set all items as selected or unselected.
   * @param selected Use false to unselect all.
   */
  void select_all(bool selected = true);


  Goocanvas::Bounds get_page_bounds(guint page_num) const;

  /** Look for any items that overlap the @a canvas_item and move them down so that the no longer overlap.
   * @param y_start Ignore any items whose y position is less than this.
   * @param offset Move items down by this amount
   * @param result The highest item that should be moved down to the start of the next page, if any:
   */
  Glib::RefPtr<CanvasLayoutItem> move_items_down(double y_start, double offset);

private:

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void setup_context_menu();
#endif

  void add_layout_group(const std::shared_ptr<LayoutGroup>& group, bool is_top_level = false);
  void add_layout_group_children(const std::shared_ptr<LayoutGroup>& group);
  void fill_layout_group(const std::shared_ptr<LayoutGroup>& group);

  //These are not static, because they need access to the document:
  void fill_with_data(const Glib::RefPtr<Goocanvas::Group>& canvas_group, const FoundSet& found_set, bool avoid_page_margins);
  void fill_with_data_portal(const Glib::RefPtr<CanvasLayoutItem>& canvas_item, const Gnome::Gda::Value& foreign_key_value);
  static void set_canvas_item_field_value(const Glib::RefPtr<Goocanvas::Item>& canvas_item, const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value);

  type_vecConstLayoutFields get_portal_fields_to_show(const std::shared_ptr<const LayoutItem_Portal>& portal);

  void create_canvas_layout_item_and_add(const std::shared_ptr<LayoutItem>& layout_item);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  std::shared_ptr<LayoutItem_Portal> offer_related_records(const std::shared_ptr<LayoutItem_Portal>& portal, Gtk::Window* parent);
  std::shared_ptr<LayoutItem_Line> offer_line(const std::shared_ptr<LayoutItem_Line>& portal, Gtk::Window* parent);

  //TODO: Make the signal send the item, so we can pass it by const reference:
  void on_item_show_context_menu(guint button, guint32 activate_time, const Glib::WeakRef<CanvasLayoutItem>& item);
  void on_context_menu_edit();
  void on_context_menu_formatting();
  void on_context_menu_delete();
  bool on_background_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* button_event);

  void on_dialog_format_hide();
#endif

  void update_page_bounds();

  Glib::RefPtr<Goocanvas::Polyline> create_margin_line(double x1, double y1, double x2, double y2);

  double get_page_height() const;
  double get_page_height(double& margin_top, double& margin_bottom) const;

  Glib::ustring m_table_name;
  bool m_modified; //TODO: Actually check this?

  //A group containing the layout items, so we can remove them without removing anything else:
  Glib::RefPtr<Goocanvas::Group> m_items_group;

  //A rectangle to show the bounds:
  Glib::RefPtr<Goocanvas::Group> m_bounds_group; //the page and its margins.
  Glib::RefPtr<Goocanvas::Rect> m_bounds_rect;
  Glib::RefPtr<Goocanvas::Polyline> m_margin_left, m_margin_right;

  typedef std::vector< Glib::RefPtr<Goocanvas::Polyline> > type_vec_margins;
  type_vec_margins m_vec_margin_tops;
  type_vec_margins m_vec_margin_bottoms;

  //Context menu for existing items:
  std::unique_ptr<Gtk::Menu> m_context_menu;
  Glib::RefPtr<Gio::SimpleActionGroup> m_context_menu_action_group;
  Glib::RefPtr<Gio::SimpleAction> m_action_edit, m_action_formatting, m_action_delete;
  Glib::RefPtr<CanvasLayoutItem> m_context_item; //The selected item when showing the context menu.,

  Glib::RefPtr<Gtk::PageSetup> m_page_setup;

  Dialog_TextFormatting* m_dialog_format;

  bool m_outline_visibility;
  guint m_page_count;
};

} //namespace Glom

#endif // GLOM_PRINT_LAYOUT_CANVAS_PRINT_LAYOUT_H
