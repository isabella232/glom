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

#ifndef GLOM_DIALOG_RELATIONSHIPS_OVERVIEW
#define GLOM_DIALOG_RELATIONSHIPS_OVERVIEW

#include <libglom/document/view.h>
#include "glom/utility_widgets/canvas/canvas_editable.h"
#include "canvas_group_dbtable.h"
#include <gtkmm/dialog.h>
#include <gtkmm/widget.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/menubar.h>
#include <gtkmm/printoperation.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/toggleaction.h>
#include <gtkmm/builder.h>
#include <goocanvasmm/canvas.h>
#include <map>
#include <vector>

//#include "relationships_canvas.h"

namespace Glom
{

class Dialog_RelationshipsOverview
 : public Gtk::Dialog,
   public View_Composite_Glom
{
public:
  Dialog_RelationshipsOverview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_RelationshipsOverview();
  
  virtual void load_from_document(); //overridden.

private:
  class TableView;
  
  void draw_tables();
  void draw_lines();
  void setup_context_menu();
  void update_relationships(TableView* table_from);
  void print_or_preview(Gtk::PrintOperationAction print_action);
  void on_response(int id);

  void on_menu_file_print();
  void on_menu_file_page_setup();
  void on_menu_file_save();
  void on_menu_view_showgrid();

  void on_table_moved(Glib::RefPtr<CanvasGroupDbTable> table);
  void on_table_show_context(guint button, guint32 activate_time, Glib::RefPtr<CanvasGroupDbTable> table);

  void on_context_menu_edit_fields(Glib::RefPtr<CanvasGroupDbTable> table);
  void on_context_menu_edit_relationships(Glib::RefPtr<CanvasGroupDbTable> table);

  void on_scroll_value_changed();

  Glib::RefPtr<CanvasGroupDbTable> get_table_group(const Glib::ustring& table_name);
  
  Glib::RefPtr<Gtk::UIManager> m_refUIManager;
  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
  Glib::RefPtr<Gtk::ToggleAction> m_action_showgrid;
  Gtk::MenuBar* m_menu;

  bool m_modified;
  CanvasEditable m_canvas;
  Gtk::ScrolledWindow* m_scrolledwindow_canvas;

  //typedef std::map<Glib::RefPtr<Goocanvas::Item>, TableView*> type_map_item_tables;
  //type_map_item_tables m_tables;

  static int m_last_size_x, m_last_size_y;

  Glib::RefPtr<Goocanvas::Group> m_group_tables;
  Glib::RefPtr<Goocanvas::Group> m_group_lines;

  typedef std::list<sigc::connection> type_list_connections;
  type_list_connections m_list_table_connections;
  
  //Context menu:
  Gtk::Menu* m_context_menu;
  Glib::RefPtr<Gtk::ActionGroup> m_context_menu_action_group;
  Glib::RefPtr<Gtk::UIManager> m_context_menu_uimanager;
  Glib::RefPtr<Gtk::Action> m_action_edit_fields, m_action_edit_relationships;
  sigc::connection m_connection_edit_fields, m_connection_edit_relationships;

  //Printing:
  Glib::RefPtr<Gtk::PrintSettings> m_refSettings;
  Glib::RefPtr<Gtk::PageSetup> m_refPageSetup;
};

} //namespace Glom

#endif //GLOM_DIALOG_RELATIONSHIPS_OVERVIEW
