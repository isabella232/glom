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

#ifndef GLOM_MODE_DESIGN_DIALOG_USERS_LIST_H
#define GLOM_MODE_DESIGN_DIALOG_USERS_LIST_H

#include <gtkmm.h>
#include <libglademm.h>
#include "../../base_db.h"
#include "../fields/combo_textglade.h"

class Dialog_UsersList
  : public Gtk::Dialog,
    public Base_DB
{
public:
  Dialog_UsersList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_UsersList();

  virtual void set_group(const Glib::ustring& group_name);

protected:

  void fill_list();

  //Enable/disable buttons, depending on treeview selection:
  virtual void enable_buttons();

  virtual void save_to_document();

  //signal handlers:
  virtual void on_button_user_delete();
  virtual void on_button_user_add();
  virtual void on_button_user_remove();
  virtual void on_button_user_new();
  virtual void on_button_user_edit();
  virtual void on_treeview_users_selection_changed();
  virtual void on_combo_group_changed();

  /** Warn if the group is the developer group, and there is only one user remaining.
   * @result Whether the warning was necessary.
   */
  virtual bool warn_about_empty_standard_group();

 // virtual void on_treeview_cell_edited_text(const Glib::ustring& path_string, const Glib::ustring& new_text, const Gtk::TreeModelColumn<Glib::ustring>& model_column);
 // virtual void on_treeview_cell_edited_numeric(const Glib::ustring& path_string, const Glib::ustring& new_text, const Gtk::TreeModelColumn<guint>& model_column);

  class ModelColumnsUsers : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumnsUsers()
    { add(m_col_name); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
  };

  ModelColumnsUsers m_model_columns_users;

  Gtk::TreeView* m_treeview_users;
  Combo_TextGlade* m_combo_group;
  Gtk::Button* m_button_user_add;
  Gtk::Button* m_button_user_remove;
  Gtk::Button* m_button_user_new;
  Gtk::Button* m_button_user_delete;
  Gtk::Button* m_button_user_edit;

  Glib::RefPtr<Gtk::ListStore> m_model_users;
};

#endif //GLOM_MODE_DESIGN_DIALOG_USERS_LIST_H
