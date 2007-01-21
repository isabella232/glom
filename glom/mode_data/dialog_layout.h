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

#ifndef GLOM_MODE_DATA_DIALOG_LAYOUT_H
#define GLOM_MODE_DATA_DIALOG_LAYOUT_H

#include <gtkmm/dialog.h>
#include "../utility_widgets/dialog_properties.h"
#include <glom/libglom/document/document_glom.h>
#include "../box_db.h"

namespace Glom
{

class Dialog_Layout :
  public Gtk::Dialog,
  public Base_DB //Give it access to the document, and to the database utilities
{
public:
  Dialog_Layout(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade, bool with_table_title = true);
  virtual ~Dialog_Layout();

  /**
   * @param layout "list" or "details"
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   * @param table_fields: The actual fields in the table, in case the document does not yet know about them all.
   */
  virtual void set_document(const Glib::ustring& layout, Document_Glom* document, const Glib::ustring& table_name, const type_vecLayoutFields& table_fields);

  virtual bool get_modified() const;

protected:

  virtual void treeview_fill_sequences(const Glib::RefPtr<Gtk::TreeModel> model, const Gtk::TreeModelColumn<guint>& sequence_column);
  virtual void enable_buttons();

  virtual void save_to_document();

  void move_treeview_selection_down(Gtk::TreeView* treeview, const Gtk::TreeModelColumn<guint>& sequence_column);
  void move_treeview_selection_up(Gtk::TreeView* treeview, const Gtk::TreeModelColumn<guint>& sequence_column);

  //signal handlers:
  virtual void on_treemodel_row_changed(const Gtk::TreeModel::Path& path, const Gtk::TreeModel::iterator& iter);
  virtual void on_entry_table_title_changed();
  virtual void on_button_close();

  void make_sensitivity_depend_on_toggle_button(Gtk::ToggleButton& toggle_button, Gtk::Widget& widget);
  void on_sensitivity_toggle_button(Gtk::ToggleButton* toggle_button, Gtk::Widget* widget);

  Gtk::Entry* m_entry_table_title;
  Gtk::Label* m_label_table_title;

  Glib::ustring m_table_name;
  Glib::ustring m_layout_name;


  //Document_Glom* m_document;  
  bool m_modified;
};

} //namespace Glom

#endif //GLOM_MODE_DATA_DIALOG_LAYOUT_H
