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

#ifndef WINDOW_PRINT_LAYOUT_EDIT_H
#define WINDOW_PRINT_LAYOUT_EDIT_H

#include <gtkmm/box.h>
#include <libglom/data_structure/print_layout.h>
#include <glom/print_layout/canvas_print_layout.h>
#include <glom/mode_design/print_layouts/print_layout_toolbar.h>
#include <glom/mode_design/print_layouts/print_layout_toolbar_button.h>
#include <libglom/document/document.h>
#include <gtkmm/window.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/uimanager.h>
//TODO: #include <gtkmm/ruler.h>
#include <glom/utility_widgets/gimpruler/gimpruler.h>
#include <gtkmm/builder.h>

namespace Glom
{

class Window_PrintLayout_Edit
: public Gtk::Window,
  public View_Composite_Glom
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Window_PrintLayout_Edit(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Window_PrintLayout_Edit();

  virtual bool init_db_details(const Glib::ustring& table_name);

  void set_print_layout(const Glib::ustring& table_name, const sharedptr<const PrintLayout>& print_layout);
  Glib::ustring get_original_name() const;
  sharedptr<PrintLayout> get_print_layout();

private:

  void enable_buttons();
  void init_menu();

  sharedptr<LayoutItem> create_empty_item(PrintLayoutToolbarButton::enumItems item_type);

  void on_menu_file_page_setup();
  void on_menu_insert_field();
  void on_menu_insert_text();
  void on_menu_insert_image();
  void on_menu_insert_relatedrecords();
  void on_menu_insert_line_horizontal();
  void on_menu_insert_line_vertical();
  void on_menu_view_show_grid();
  void on_menu_view_show_rules();
  void on_menu_view_zoom(guint percent);
  void on_menu_view_fitpagewidth();

  void on_canvas_show_context_menu(guint button, guint32 activate_time);
  void on_context_menu_insert_field();
  void on_context_menu_insert_text();

  void on_scroll_value_changed();
  void on_button_close();

  //void on_toolbar_item_drag_begin(const Glib::RefPtr<Gdk::DragContext>& drag_context);
  //void on_toolbar_item_drag_end(const Glib::RefPtr<Gdk::DragContext>& drag_context);
  void on_toolbar_item_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& drag_context, Gtk::SelectionData& selection_data, guint info, guint time);
  bool on_canvas_drag_drop(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, guint timestamp);
  bool on_canvas_drag_motion(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, guint timestamp);
  void on_canvas_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint timestamp);
  void on_canvas_drag_leave(const Glib::RefPtr<Gdk::DragContext>& drag_context, guint timestamp);

  //override:
  virtual bool on_configure_event(GdkEventConfigure* event);

  Glib::RefPtr<Gdk::Pixbuf> get_icon_for_toolbar_item(Gtk::ToolItem& item);

  void update_table_title();
  void setup_context_menu();
  void set_ruler_sizes();

  bool get_is_item_at(double x, double y);
  void set_default_position(const sharedptr<LayoutItem>& item);
  
  void canvas_convert_from_drag_pixels(double& x, double& y) const;

  //Box_DB_Table_Definition* m_box;
  Glib::ustring m_name_original;
  Glib::ustring m_table_name;
  bool m_modified;

  sharedptr<PrintLayout> m_print_layout;

  Gtk::Entry* m_entry_name;
  Gtk::Entry* m_entry_title;
  Gtk::Label* m_label_table_name;
  //Gtk::Label* m_label_table_title;
  Gtk::Button* m_button_close;

  Gtk::VBox* m_box_menu;
  Gtk::VBox* m_box_canvas;
  Gtk::VBox* m_box;
  Gtk::ScrolledWindow m_scrolled_window;
  Canvas_PrintLayout m_canvas;

  //A preview of the item being dragged onto the canvas:
  bool m_drag_preview_requested;
  Glib::RefPtr<CanvasLayoutItem> m_layout_item_dropping;

  GimpRuler* m_vruler;
  GimpRuler* m_hruler;

  //Main menu:
  Glib::RefPtr<Gtk::ActionGroup> m_action_group;
  Glib::RefPtr<Gtk::UIManager> m_uimanager;
  Glib::RefPtr<Gtk::ToggleAction> m_action_showgrid, m_action_showrules;
  Glib::RefPtr<Gtk::ToggleAction> m_action_zoom_fit_page_width;

  //Toolbar:
  Gtk::HandleBox* m_palette_handle_box; //TODO: The toolbar is already a HandleBox.
  std::vector<Gtk::TargetEntry> m_drag_targets;
  PrintLayoutToolbar m_toolbar;
  
  //Context menu for clicking on empty space on the canvas:
  Gtk::Menu* m_context_menu;
  Glib::RefPtr<Gtk::ActionGroup> m_context_menu_action_group;
  Glib::RefPtr<Gtk::UIManager> m_context_menu_uimanager;
};

} //namespace Glom

#endif //WINDOW_PRINT_LAYOUT_EDIT_H

