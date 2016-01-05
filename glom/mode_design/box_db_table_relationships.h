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

#ifndef BOX_DB_TABLE_RELATIONSHIPS_H
#define BOX_DB_TABLE_RELATIONSHIPS_H

#include <glom/box_db_table.h>
#include <glom/utility_widgets/adddel/adddel_withbuttons.h>

namespace Glom
{

class Box_DB_Table_Relationships : public Box_DB_Table
{
public:
  Box_DB_Table_Relationships();
  Box_DB_Table_Relationships(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  void init(); //avoid duplication in constructors.

  void save_to_document() override;

private:
  bool fill_from_database() override;

  //Signal handlers:
  void on_adddel_user_activated(const Gtk::TreeModel::iterator& row, guint col);
  void on_adddel_user_changed(const Gtk::TreeModel::iterator& row, guint col);
  void on_adddel_user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd);
  void on_adddel_user_added(const Gtk::TreeModel::iterator& row);

  guint m_colName, m_colTitle, m_colFromField, m_colToTable, m_colToField, m_colAllowEdit, m_colAutoCreate, m_colTitleSingular;

  mutable AddDel_WithButtons m_AddDel; //mutable because its get_ methods aren't const.
  Gtk::Button m_Button_Guess;
};

} //namespace Glom

#endif //BOX_DB_TABLE_RELATIONSHIPS_H
