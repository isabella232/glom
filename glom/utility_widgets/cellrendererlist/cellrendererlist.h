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

#ifndef ADDDEL_CELLRENDERERLIST_H
#define ADDDEL_CELLRENDERERLIST_H

#include <gtkmm/cellrenderercombo.h>
#include <gtkmm/liststore.h>


namespace Glom
{

class CellRendererList : public Gtk::CellRendererCombo
{
public:
  CellRendererList();
  virtual ~CellRendererList();

  void remove_all_list_items();
  void append_list_item(const Glib::ustring& text, const Glib::ustring& extra = Glib::ustring());

  ///Whether the second column will be shown.
  void set_use_second(bool use_second = true);

  void set_restrict_values_to_list(bool val = true);

private:
  void on_editing_started(Gtk::CellEditable* cell_editable, const Glib::ustring& path);

  //Tree model columns for the Combo CellRenderer in the TreeView column:
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns()
    { add(m_col_choice); add(m_col_extra); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_choice;
    Gtk::TreeModelColumn<Glib::ustring> m_col_extra;
  };

  ModelColumns m_model_columns;

  Glib::RefPtr<Gtk::ListStore> m_refModel;
  bool m_use_second;
};

} //namespace Glom

#endif //ADDDEL_CELLRENDERERLIST_H


