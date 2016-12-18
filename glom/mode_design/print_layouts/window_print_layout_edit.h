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

#ifndef WINDOW_PRINT_LAYOUT_EDIT_H
#define WINDOW_PRINT_LAYOUT_EDIT_H

#include <gtkmm/box.h>
#include <libglom/data_structure/print_layout.h>
#include <glom/print_layout/canvas_print_layout.h>
#include <glom/mode_design/print_layouts/print_layout_toolbar.h>
#include <glom/mode_design/print_layouts/print_layout_toolbar_button.h>
#include <libglom/document/document.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
//#include <glom/utility_widgets/gimpruler/gimpruler.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/button.h>
#include <gtkmm/builder.h>

namespace Glom
{

class Window_PrintLayout_Edit
: public Gtk::ApplicationWindow,
  public View_Composite_Glom
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Window_PrintLayout_Edit(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Window_PrintLayout_Edit();

  virtual bool init_db_details(const Glib::ustring& table_name);

  void set_print_layout(const Glib::ustring& table_name, const std::shared_ptr<const PrintLayout>& print_layout);
  Glib::ustring get_original_name() const;
  std::shared_ptr<PrintLayout> get_print_layout();

private:

  void enable_buttons();
  void init_menu();

  std::shared_ptr<LayoutItem> create_empty_item(PrintLayoutToolbarButton::enumItems item_type);

  void on_menu_file_page_setup();
  void on_menu_file_print_preview();
  void on_menu_insert_field();
  void on_menu_insert_text();
  void on_menu_insert_image();
  void on_menu_insert_relatedrecords();
  void on_menu_insert_line_horizontal();
  void on_menu_insert_line_vertical();
  void on_menu_insert_create_standard();
  void on_menu_insert_add_page();
  void on_menu_insert_delete_page();

  void on_menu_view_show_grid();
  void on_menu_view_show_rules();
  void on_menu_view_show_outlines();
  void on_menu_view_zoom(int parameter);

  void on_menu_edit_cut();
  void on_menu_edit_copy();
  void on_menu_edit_paste();
  void on_menu_edit_delete();
  void on_menu_edit_selectall();
  void on_menu_edit_unselectall();

  void on_menu_align_top();
  void on_menu_align_bottom();
  void on_menu_align_left();
  void on_menu_align_right();

  bool on_canvas_motion_notify_event(GdkEventMotion* motion_event);
  void on_canvas_show_context_menu(guint button, guint32 activate_time);
  void on_context_menu_insert_field();
  void on_context_menu_insert_text();

  void on_scroll_value_changed();
  void on_button_close();

  //void on_toolbar_item_drag_begin(const Glib::RefPtr<Gdk::DragContext>& drag_context);
  //void on_toolbar_item_drag_end(const Glib::RefPtr<Gdk::DragContext>& drag_context);
  bool on_canvas_drag_drop(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, guint timestamp);
  bool on_canvas_drag_motion(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, guint timestamp);
  void on_canvas_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint timestamp);
  void on_canvas_drag_leave(const Glib::RefPtr<Gdk::DragContext>& drag_context, guint timestamp);

  void on_canvas_selection_changed();
  void on_selected_item_moved(const Glib::RefPtr<CanvasItemMovable>& item, double x_offset, double y_offset);

  void on_spinbutton_x();
  void on_spinbutton_y();
  void on_spinbutton_width();
  void on_spinbutton_height();

  bool on_hruler_button_press_event(GdkEventButton* button_event);
  bool on_vruler_button_press_event(GdkEventButton* button_event);

  //override:
  bool on_configure_event(GdkEventConfigure* configure_event) override;

  void do_menu_view_show_grid(bool active);
  void do_menu_view_show_rules(bool active);
  void do_menu_view_show_outlines(bool active);

  void update_table_title();
  void setup_context_menu();
  void set_ruler_sizes();

  bool get_is_item_at(double x, double y) const;
  void set_default_position(const std::shared_ptr<LayoutItem>& item);

  void canvas_convert_from_drag_pixels(double& x, double& y, bool adjust_for_scrolling = false) const;
  void get_dimensions_of_multiple_selected_items(double& x, double& y, double& width, double& height) const;

  Glib::RefPtr<CanvasLayoutItem> create_canvas_layout_item_and_add(const std::shared_ptr<LayoutItem>& layout_item);

  //Box_DB_Table_Definition* m_box;
  Glib::ustring m_name_original;
  Glib::ustring m_table_name;
  bool m_modified;

  std::shared_ptr<PrintLayout> m_print_layout;

  Gtk::Entry* m_entry_name;
  Gtk::Entry* m_entry_title;
  Gtk::Label* m_label_table_name;
  //Gtk::Label* m_label_table_title;
  Gtk::Button* m_button_close;

  Gtk::Box* m_box_menu;
  Gtk::Box* m_box_canvas;
  Gtk::Box* m_box;
  Gtk::ScrolledWindow m_scrolled_window;
  Canvas_PrintLayout m_canvas;

  //Widgets that let the user edit item position and size:
  Gtk::Box* m_box_item_position;
  Gtk::SpinButton* m_spinbutton_x;
  Gtk::SpinButton* m_spinbutton_y;
  Gtk::SpinButton* m_spinbutton_width;
  Gtk::SpinButton* m_spinbutton_height;
  bool m_ignore_spinbutton_signals;

  //A preview of the item being dragged onto the canvas:
  bool m_drag_preview_requested;
  Glib::RefPtr<CanvasLayoutItem> m_layout_item_dropping;

  bool m_temp_rule_horizontal; //Otherwise vertical.

  //A cache of the selected item,
  //to avoid repeatedly requesting it:
  std::vector<Glib::RefPtr<CanvasLayoutItem>> m_layout_items_selected;

  std::vector<sigc::connection> m_connections_items_selected_moved;

  //A copied item to be pasted later:
  typedef LayoutGroup::type_list_items type_list_items;
  type_list_items m_layout_items_to_paste;

  std::vector<Gtk::TargetEntry> m_drag_targets_rule;
  //GimpRuler* m_vruler;
  //GimpRuler* m_hruler;

  //Main menu:
  Glib::RefPtr<Gtk::Builder> m_builder;
  Glib::RefPtr<Gio::SimpleAction> m_action_showgrid, m_action_showrules, m_action_showoutlines;
  Glib::RefPtr<Gio::SimpleAction> m_action_zoom;

  //Edit menu:
  Glib::RefPtr<Gio::SimpleAction> m_action_edit_cut, m_action_edit_copy,
    m_action_edit_paste, m_action_edit_delete;

  //Align menu:
  Glib::RefPtr<Gio::SimpleAction> m_action_align_top, m_action_align_bottom,
    m_action_align_left, m_action_align_right;

  //Toolbar:
  Gtk::Box* m_palette_box;
  std::vector<Gtk::TargetEntry> m_drag_targets_all;
  PrintLayoutToolbar m_toolbar;

  //Context menu for clicking on empty space on the canvas:
  std::unique_ptr<Gtk::Menu> m_context_menu;
};

} //namespace Glom

#endif //WINDOW_PRINT_LAYOUT_EDIT_H

