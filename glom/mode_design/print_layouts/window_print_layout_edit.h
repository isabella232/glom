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

  void on_menu_insert_field();
  void on_menu_insert_text();
  void on_menu_insert_image();
  void on_menu_view_showgrid();

  void on_canvas_show_context_menu(guint button, guint32 activate_time);
  void on_context_menu_insert_field();
  void on_context_menu_insert_text();

  void on_button_close();
  void setup_context_menu();
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
  Canvas_PrintLayout m_canvas;

  //Main menu:
  Glib::RefPtr<Gtk::ActionGroup> m_action_group;
  Glib::RefPtr<Gtk::UIManager> m_uimanager;
  Glib::RefPtr<Gtk::ToggleAction> m_action_showgrid;

  //Context menu for clicking on empty space on the canvas:
  Gtk::Menu* m_context_menu;
  Glib::RefPtr<Gtk::ActionGroup> m_context_menu_action_group;
  Glib::RefPtr<Gtk::UIManager> m_context_menu_uimanager;
};

} //namespace Glom

#endif //WINDOW_PRINT_LAYOUT_EDIT_H

