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

#ifndef DIALOG_LAYOUT_H
#define DIALOG_LAYOUT_H

#include <gtkmm/dialog.h>
#include "../utility_widgets/dialog_properties.h"
#include "../document/document_glom.h"
#include "../box_db.h"
#include "../utility_widgets/adddel/cellrendererlist.h"

/**
  *@author Murray Cumming
  */


class Dialog_Layout : public Gtk::Dialog
{
public:
  Dialog_Layout(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_Layout();

  typedef std::vector< Field > type_vecFields;
  
  /**
   * @param layout "list" or "details"
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   * @param table_fields: The actual fields in the table, in case the document does not yet know about them all.
   */
  void set_document(const Glib::ustring& layout, Document_Glom* document, const Glib::ustring& table_name, const type_vecFields& table_fields);

  void set_show_groups(bool val = true);
protected:

  //Enable/disable buttons, depending on treeview selection:
  virtual void enable_buttons();
  virtual void fill_cellrenderer_groups();
  //virtual void fill_groups();
  virtual void fill_groups_fields();
  virtual void treeview_fill_sequences(const Glib::RefPtr<Gtk::TreeModel> model, const Gtk::TreeModelColumn<guint>& sequence_column);
  
  virtual void save_to_document();

  void move_treeview_selection_down(Gtk::TreeView* treeview, const Gtk::TreeModelColumn<guint>& sequence_column);
  void move_treeview_selection_up(Gtk::TreeView* treeview, const Gtk::TreeModelColumn<guint>& sequence_column);
    
  //signal handlers:
  virtual void on_button_field_up();
  virtual void on_button_field_down();
  virtual void on_button_group_up();
  virtual void on_button_group_down();
  virtual void on_button_group_add();
  virtual void on_button_group_delete();
  virtual void on_button_close();
  virtual void on_treeview_fields_selection_changed();
  virtual void on_treeview_groups_selection_changed();
  virtual void on_treeview_groups_name_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text);
  virtual void on_treeview_fields_group_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text);
  virtual void on_treemodel_row_changed(const Gtk::TreeModel::Path& path, const Gtk::TreeModel::iterator& iter);
  virtual void on_entry_table_title_changed();

  //Tree model columns:
  class ModelColumns_Fields : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns_Fields()
    { add(m_col_name); add(m_col_sequence); add(m_col_hidden); add(m_col_group); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    Gtk::TreeModelColumn<guint> m_col_sequence;
    Gtk::TreeModelColumn<bool> m_col_hidden;
    Gtk::TreeModelColumn<Glib::ustring> m_col_group;
  };

  ModelColumns_Fields m_ColumnsFields;

  //Tree model columns:
  class ModelColumns_Groups : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns_Groups()
    { add(m_col_name); add(m_col_title), add(m_col_sequence); add(m_col_fields); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    Gtk::TreeModelColumn<Glib::ustring> m_col_title;
    Gtk::TreeModelColumn<guint> m_col_sequence;
    Gtk::TreeModelColumn<Glib::ustring> m_col_fields; //A "list" of fields in this group.
  };

  ModelColumns_Groups m_ColumnsGroups;
  
  Gtk::TreeView* m_treeview_fields;
  Gtk::TreeView* m_treeview_groups;
  Gtk::Button* m_button_field_up;
  Gtk::Button* m_button_field_down;  
  Gtk::Button* m_button_field_add;
  Gtk::Button* m_button_field_delete;
  Gtk::Button* m_button_field_edit;
  Gtk::Button* m_button_group_up;
  Gtk::Button* m_button_group_down;
  Gtk::Button* m_button_group_add;
  Gtk::Button* m_button_group_delete;
  Gtk::Frame* m_frame_groups;
  Gtk::TreeView::Column* m_treeviewcolumn_field_groups;
  Gtk::Label* m_label_table_name;
  Gtk::Entry* m_entry_table_title;;
      
  Glib::RefPtr<Gtk::ListStore> m_model_fields, m_model_groups;

  Glib::ustring m_table_name;
  Glib::ustring m_layout_name;

  CellRendererList m_cellrenderer_field_group;

  Document_Glom* m_document;  
  bool m_modified;
};

#endif //DIALOG_LAYOUT_H
