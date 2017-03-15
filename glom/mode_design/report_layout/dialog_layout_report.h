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

#ifndef GLOM_DIALOG_LAYOUT_REPORT_H
#define GLOM_DIALOG_LAYOUT_REPORT_H

#include <glom/mode_design/layout/dialog_layout.h>
#include <libglom/data_structure/report.h>
#include <gtkmm/notebook.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <glibmm/weakref.h>
#include "treestore_report_layout.h"
//#include <libglom/data_structure/layout/layoutitem.h>
//#include <libglom/sharedptr.h>

namespace Glom
{

class Dialog_Layout_Report : public Dialog_Layout
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_Layout_Report(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_Layout_Report();

  void set_report(const Glib::ustring& table_name, const std::shared_ptr<const Report>& report);
  std::shared_ptr<Report> get_report();

  Glib::ustring get_original_report_name() const;

private:

  typedef TreeStore_ReportLayout type_model;

  void add_group(const Glib::RefPtr<type_model>& model_parts, const Gtk::TreeModel::iterator& parent, const std::shared_ptr<const LayoutGroup>& group);

  void add_group_children(const Glib::RefPtr<type_model>& model_parts, const Gtk::TreeModel::iterator& parent, const std::shared_ptr<const LayoutGroup>& group);

  void fill_group_children(const std::shared_ptr<LayoutGroup>& group, const Gtk::TreeModel::const_iterator& iter, const Glib::RefPtr<const type_model> model);
  std::shared_ptr<LayoutGroup> fill_group(const Gtk::TreeModel::const_iterator& iter, const Glib::RefPtr<const type_model> model);

  //Enable/disable buttons, depending on treeview selection:
  void enable_buttons() override;

  void save_to_document() override;

  std::shared_ptr<Relationship> offer_relationship_list();

  ///Depends on the active notebook tab.
  Glib::RefPtr<type_model> get_selected_model();
  Glib::RefPtr<const type_model> get_selected_model() const;
  Gtk::TreeView* get_selected_treeview();
  const Gtk::TreeView* get_selected_treeview() const;

  Gtk::TreeModel::iterator get_selected_group_parent() const;

  Gtk::TreeModel::iterator get_selected_available() const;

  void setup_model(Gtk::TreeView& treeview, Glib::RefPtr<type_model>& model);

  //signal handlers:
  void on_button_add();

  void on_button_up();
  void on_button_down();
  void on_button_delete();

  void on_button_edit();
  void on_button_formatting();

  void on_treeview_parts_selection_changed();
  void on_treeview_available_parts_selection_changed();

  void on_cell_data_part(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, const Glib::WeakRef<type_model>& model);
  void on_cell_data_details(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, const Glib::WeakRef<type_model>& model);

  void on_cell_data_available_part(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);

  void on_notebook_switch_page(Gtk::Widget*, guint);

  void fill_report_parts(std::shared_ptr<LayoutGroup>& group, const Glib::RefPtr<const type_model> parts_model);

  Gtk::Notebook* m_notebook_parts;
  Gtk::TreeView* m_treeview_parts_header;
  Gtk::TreeView* m_treeview_parts_footer;
  Gtk::TreeView* m_treeview_parts_main;

  Gtk::TreeView* m_treeview_available_parts;

  Gtk::Button* m_button_up;
  Gtk::Button* m_button_down;
  Gtk::Button* m_button_add;
  Gtk::Button* m_button_delete;
  Gtk::Button* m_button_edit;
  Gtk::Button* m_button_formatting;
  Gtk::Label* m_label_table_name;
  Gtk::Entry* m_entry_name;
  Gtk::Entry* m_entry_title;
  Gtk::CheckButton* m_checkbutton_table_title;


  Glib::RefPtr<type_model> m_model_parts_header;
  Glib::RefPtr<type_model> m_model_parts_footer;
  Glib::RefPtr<type_model> m_model_parts_main;

  Glib::RefPtr<type_model> m_model_available_parts_headerfooter;
  Glib::RefPtr<type_model> m_model_available_parts_main;

  Glib::ustring m_name_original;
  std::shared_ptr<Report> m_report;
  sigc::connection m_signal_connection;
};

} //namespace Glom

#endif //GLOM_DIALOG_LAYOUT_REPORT_H
