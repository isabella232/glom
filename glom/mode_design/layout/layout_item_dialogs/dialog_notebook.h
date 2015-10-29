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

#ifndef GLOM_MODE_DESIGN_DIALOG_NOTEBOOK_H
#define GLOM_MODE_DESIGN_DIALOG_NOTEBOOK_H

#include <glom/base_db.h>
#include <glom/mode_design/layout/dialog_layout.h>
#include <libglom/data_structure/layout/layoutitem_notebook.h>
#include <gtkmm/builder.h>

namespace Glom
{

class Dialog_Notebook
 : public Dialog_Layout //It has some useful stuff
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_Notebook(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_Notebook();


  void set_notebook(const std::shared_ptr<const LayoutItem_Notebook>& start_notebook);
  std::shared_ptr<LayoutItem_Notebook> get_notebook() const;

private:

  //Enable/disable buttons, depending on treeview selection:
  void enable_buttons() override;

  //signal handlers:
  void on_button_up();
  void on_button_down();
  void on_button_add();
  void on_button_delete();
  void on_treeview_selection_changed();

  //Tree model columns:
  class ModelColumns_Tabs : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns_Tabs()
    { add(m_col_name); add(m_col_title); add(m_col_sequence); add(m_col_item); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    Gtk::TreeModelColumn<Glib::ustring> m_col_title;
    Gtk::TreeModelColumn<guint> m_col_sequence;
    Gtk::TreeModelColumn< std::shared_ptr<LayoutGroup> > m_col_item;
  };

  ModelColumns_Tabs m_ColumnsTabs;

  //Tree model columns:
  Gtk::TreeView* m_treeview;
  Gtk::Button* m_button_up;
  Gtk::Button* m_button_down;
  Gtk::Button* m_button_add;
  Gtk::Button* m_button_delete;

  Glib::RefPtr<Gtk::ListStore> m_model;

  std::shared_ptr<const LayoutItem_Notebook> m_layout_item;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_NOTEBOOK_H
