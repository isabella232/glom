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

#ifndef GLOM_UTILITY_WIDGETS_CELLRENDERERLIST_H
#define GLOM_UTILITY_WIDGETS_CELLRENDERERLIST_H

#include <gtkmm/cellrenderercombo.h>
#include <gtkmm/liststore.h>


namespace Glom
{

/** A CellRendererCombo with a single (text) column.
 */
class CellRendererList : public Gtk::CellRendererCombo
{
public:
  CellRendererList();

  void remove_all_list_items();
  void append_list_item(const Glib::ustring& text);

  void set_restrict_values_to_list(bool val = true);

private:

  //Tree model columns for the Combo CellRenderer in the TreeView column:
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns()
    { add(m_col_choice); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_choice;
  };

  ModelColumns m_model_columns;

  Glib::RefPtr<Gtk::ListStore> m_model;
};

} //namespace Glom

#endif // GLOM_UTILITY_WIDGETS_CELLRENDERERLIST_H
