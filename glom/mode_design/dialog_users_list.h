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
#include "../base_db.h"

class Dialog_UsersList
  : public Gtk::Dialog,
    public Base_DB
{
public:
  Dialog_UsersList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_UsersList();

  /**
   * @param layout "list" or "details"
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   * @param table_fields: The actual fields in the table, in case the document does not yet know about them all.
   */
  //virtual void set_document(const Glib::ustring& layout, Document_Glom* document, const Glib::ustring& table_name, const type_vecLayoutFields& table_fields);

protected:

  void fill_list();

  //Enable/disable buttons, depending on treeview selection:
  virtual void enable_buttons();

  virtual void save_to_document();

  //signal handlers:
  virtual void on_button_delete();
  virtual void on_button_add();
  virtual void on_button_edit();
  virtual void on_treeview_selection_changed();

 // virtual void on_treeview_cell_edited_text(const Glib::ustring& path_string, const Glib::ustring& new_text, const Gtk::TreeModelColumn<Glib::ustring>& model_column);
 // virtual void on_treeview_cell_edited_numeric(const Glib::ustring& path_string, const Glib::ustring& new_text, const Gtk::TreeModelColumn<guint>& model_column);

  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns()
    { add(m_col_user); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_user;
  };

  ModelColumns m_model_columns;

  Gtk::TreeView* m_treeview_users;
  Gtk::Button* m_button_add;
  Gtk::Button* m_button_delete;
  Gtk::Button* m_button_edit;

  Glib::RefPtr<Gtk::ListStore> m_model_items;
};

#endif //GLOM_MODE_DESIGN_DIALOG_USERS_LIST_H
