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

#ifndef GLOM_MODE_DESIGN_DIALOG_LAYOUT_H
#define GLOM_MODE_DESIGN_DIALOG_LAYOUT_H

#include <gtkmm/dialog.h>
#include <glom/utility_widgets/dialog_properties.h>
#include <libglom/document/document.h>
#include <glom/box_withbuttons.h>
#include <gtkmm/togglebutton.h>

namespace Glom
{

class Dialog_Layout :
  public Gtk::Dialog,
  public Base_DB //Give it access to the document, and to the database utilities
{
public:
  Dialog_Layout(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, bool with_table_title = true);

  virtual bool get_modified() const;

  /**
   * @param layout_name "list" or "details"
   * @param layout_platform As in the document. Empty or "maemo".
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   * @param table_fields: The actual fields in the table, in case the document does not yet know about them all.
   */
  virtual void init(const Glib::ustring& layout_name, const Glib::ustring& layout_platform, Document* document, const Glib::ustring& table_name, const type_vecConstLayoutFields& table_fields);
  
protected:

  void treeview_fill_sequences(const Glib::RefPtr<Gtk::TreeModel> model, const Gtk::TreeModelColumn<guint>& sequence_column);
  virtual void enable_buttons();

  void save_to_document() override;

  void move_treeview_selection_down(Gtk::TreeView* treeview, const Gtk::TreeModelColumn<guint>& sequence_column);
  void move_treeview_selection_up(Gtk::TreeView* treeview, const Gtk::TreeModelColumn<guint>& sequence_column);

  //signal handlers:
  //TODO: Do these need to be virtual?
  void on_treemodel_row_changed(const Gtk::TreeModel::Path& path, const Gtk::TreeModel::iterator& iter);
  void on_entry_table_title_changed();
  void on_button_close();

  void make_sensitivity_depend_on_toggle_button(Gtk::ToggleButton& toggle_button, Gtk::Widget& widget);
  void on_sensitivity_toggle_button(Gtk::ToggleButton* toggle_button, Gtk::Widget* widget);

  Gtk::Entry* m_entry_table_title;
  Gtk::Label* m_label_table_title;

  Glib::ustring m_table_name;
  Glib::ustring m_layout_name, m_layout_platform; //As in the document.

  //Whether the layout is for a view that allows editing.
  //If so, various editing options will be available in the formatting.
  bool m_editable_layout;
  
  bool m_modified;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_LAYOUT_H
