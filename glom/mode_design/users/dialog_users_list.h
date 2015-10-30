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

#ifndef GLOM_MODE_DESIGN_DIALOG_USERS_LIST_H
#define GLOM_MODE_DESIGN_DIALOG_USERS_LIST_H

#include <gtkmm/dialog.h>
#include <gtkmm/treeview.h>
#include <gtkmm/builder.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/liststore.h>
#include <glom/base_db.h>

namespace Glom
{

/** A dialog that lists the users in a group,
 * or all users if no group is selected.
 */
class Dialog_UsersList
  : public Gtk::Dialog,
    public Base_DB
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  /** Call fill_list() after instantiating this class.
   */
  Dialog_UsersList(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_UsersList();

  /** Fill the list of users.
   */
  void fill_list();

  void set_group(const Glib::ustring& group_name);

private:

  //Enable/disable buttons, depending on treeview selection:
  void enable_buttons();

  void save_to_document() override;

  //signal handlers:
  void on_button_user_delete();
  void on_button_user_add();
  void on_button_user_remove();
  void on_button_user_new();
  void on_button_user_edit();
  void on_treeview_users_selection_changed();
  void on_combo_group_changed();

  /** Warn if the group is the developer group, and there is only one user remaining.
   * @result Whether the warning was necessary.
   */
  bool warn_about_empty_standard_group();

 // void on_treeview_cell_edited_text(const Glib::ustring& path_string, const Glib::ustring& new_text, const Gtk::TreeModelColumn<Glib::ustring>& model_column);
 // void on_treeview_cell_edited_numeric(const Glib::ustring& path_string, const Glib::ustring& new_text, const Gtk::TreeModelColumn<guint>& model_column);

  class ModelColumnsUsers : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumnsUsers()
    { add(m_col_name); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
  };

  ModelColumnsUsers m_model_columns_users;

  Gtk::TreeView* m_treeview_users;
  Gtk::ComboBoxText* m_combo_group;
  Gtk::Button* m_button_user_add;
  Gtk::Button* m_button_user_remove;
  Gtk::Button* m_button_user_new;
  Gtk::Button* m_button_user_delete;
  Gtk::Button* m_button_user_edit;

  Glib::RefPtr<Gtk::ListStore> m_model_users;
};

} //namespace Glom

#endif //GLOM_MODE_DESIGN_DIALOG_USERS_LIST_H
