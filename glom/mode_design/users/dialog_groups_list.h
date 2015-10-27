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

#ifndef GLOM_MODE_DESIGN_DIALOG_GROUPS_LIST_H
#define GLOM_MODE_DESIGN_DIALOG_GROUPS_LIST_H

#include <gtkmm/dialog.h>
#include <gtkmm/treeview.h>
#include <gtkmm/builder.h>
#include <gtkmm/liststore.h>
#include <glom/base_db.h>

namespace Glom
{

class Dialog_GroupsList
  : public Gtk::Dialog,
    public Base_DB
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_GroupsList(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_GroupsList();

  //Refresh the UI when we get the document, from add_view:
  virtual void load_from_document() override;

  /**
   * @param layout "list" or "details"
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   * @param table_fields: The actual fields in the table, in case the document does not yet know about them all.
   */
  //virtual void set_document(const Glib::ustring& layout, Document* document, const Glib::ustring& table_name, const type_vecLayoutFields& table_fields);

private:

  void fill_group_list();
  void fill_table_list(const Glib::ustring& group_name);
  Glib::ustring get_selected_group() const;

  void treeview_append_bool_column(Gtk::TreeView& treeview, const Glib::ustring& title, Gtk::TreeModelColumn<bool>& model_column, const sigc::slot<void, const Glib::ustring&>& slot_toggled);

  //Enable/disable buttons, depending on treeview selection:
  void enable_buttons() override;

  void save_to_document() override;

  //signal handlers:

  virtual void on_button_group_delete();
  virtual void on_button_group_new();
  virtual void on_button_group_users();
  virtual void on_treeview_groups_selection_changed();
  virtual void on_treeview_tables_selection_changed();

  virtual void on_treeview_tables_toggled_view(const Glib::ustring& path_string);
  virtual void on_treeview_tables_toggled_edit(const Glib::ustring& path_string);
  virtual void on_treeview_tables_toggled_create(const Glib::ustring& path_string);
  virtual void on_treeview_tables_toggled_delete(const Glib::ustring& path_string);

  enum class enumPriv
  {
    VIEW,
    EDIT,
    CREATE,
    DELETE
  };

  bool set_table_privilege(const Glib::ustring& table_name, const Glib::ustring& group_name, bool grant, enumPriv priv);

  void on_cell_data_group_name(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);

 // virtual void on_treeview_cell_edited_text(const Glib::ustring& path_string, const Glib::ustring& new_text, const Gtk::TreeModelColumn<Glib::ustring>& model_column);
 // virtual void on_treeview_cell_edited_numeric(const Glib::ustring& path_string, const Glib::ustring& new_text, const Gtk::TreeModelColumn<guint>& model_column);

  class ModelColumnsTables : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumnsTables()
    { add(m_col_name); add(m_col_title); add(m_col_view); add(m_col_edit); add(m_col_create); add(m_col_delete); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_name; //Not shown in view.
    Gtk::TreeModelColumn<Glib::ustring> m_col_title;
    Gtk::TreeModelColumn<bool> m_col_view, m_col_edit, m_col_create, m_col_delete;
  };

  ModelColumnsTables m_model_columns_tables;


  Gtk::TreeView* m_treeview_tables;
  Glib::RefPtr<Gtk::ListStore> m_model_tables;


  class ModelColumnsGroups : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumnsGroups()
    { add(m_col_name);  add(m_col_description); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_name, m_col_description;
  };

  ModelColumnsGroups m_model_columns_groups;


  Gtk::TreeView* m_treeview_groups;
  Gtk::Button* m_button_group_new;
  Gtk::Button* m_button_group_delete;
  Gtk::Button* m_button_group_users;

  Glib::RefPtr<Gtk::ListStore> m_model_groups;
};

} //namespace Glom

#endif //GLOM_MODE_DESIGN_DIALOG_GROUPS_LIST_H
