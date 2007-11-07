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

#include <glom/libglom/data_structure/print_layout.h>
#include <glom/mode_design/print_layouts/canvas_print_layout.h>
#include <glom/libglom/document/document_glom.h>
#include <gtkmm/window.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/toggleaction.h>
#include <gtkmm/ruler.h>
#include <libglademm/xml.h>

namespace Glom
{

class Window_PrintLayout_Edit
: public Gtk::Window,
  public View_Composite_Glom
{
public:
  Window_PrintLayout_Edit(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Window_PrintLayout_Edit();

  virtual bool init_db_details(const Glib::ustring& table_name);

  void set_print_layout(const Glib::ustring& table_name, const sharedptr<const PrintLayout>& print_layout);
  Glib::ustring get_original_name() const;
  sharedptr<PrintLayout> get_print_layout();

protected:

  void enable_buttons();
  void init_menu();

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

  //override:
  virtual bool on_configure_event(GdkEventConfigure* event);

  void setup_context_menu();
  void set_ruler_sizes();

  bool get_is_item_at(double x, double y);
  void set_default_position(const sharedptr<LayoutItem>& item);

  //Box_DB_Table_Definition* m_box;
  Glib::ustring m_name_original;
  Glib::ustring m_table_name;
  bool m_modified;

  sharedptr<PrintLayout> m_print_layout;

  Gtk::Entry* m_entry_name;
  Gtk::Entry* m_entry_title;
  Gtk::Label* m_label_table_name;
  Gtk::Label* m_label_table;
  //Gtk::Label* m_label_table_title;
  Gtk::Button* m_button_close;

  Gtk::VBox* m_box_menu;
  Gtk::VBox* m_box_canvas;
  Gtk::VBox* m_box;
  Gtk::ScrolledWindow m_scrolled_window;
  Canvas_PrintLayout m_canvas;

  Gtk::VRuler* m_vruler;
  Gtk::HRuler* m_hruler;

  //Main menu:
  Glib::RefPtr<Gtk::ActionGroup> m_action_group;
  Glib::RefPtr<Gtk::UIManager> m_uimanager;
  Glib::RefPtr<Gtk::ToggleAction> m_action_showgrid, m_action_showrules;
  Glib::RefPtr<Gtk::ToggleAction> m_action_zoom_fit_page_width;

  //Context menu for clicking on empty space on the canvas:
  Gtk::Menu* m_context_menu;
  Glib::RefPtr<Gtk::ActionGroup> m_context_menu_action_group;
  Glib::RefPtr<Gtk::UIManager> m_context_menu_uimanager;
};

} //namespace Glom

#endif //WINDOW_PRINT_LAYOUT_EDIT_H

