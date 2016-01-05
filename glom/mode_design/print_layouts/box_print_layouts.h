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

#ifndef BOX_PRINT_LAYOUTS_H
#define BOX_PRINT_LAYOUTS_H

#include <glom/box_db_table.h>
#include <libglom/data_structure/print_layout.h>
#include <glom/utility_widgets/adddel/adddel_withbuttons.h>

namespace Glom
{

class Box_Print_Layouts : public Box_DB_Table
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Box_Print_Layouts(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

private:
  bool fill_from_database() override;

  void fill_row(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const PrintLayout>& print_layout);

  void save_to_document() override;

  //Signal handlers:
  void on_adddel_user_added(const Gtk::TreeModel::iterator& row);
  void on_adddel_user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd);
  void on_adddel_user_requested_edit(const Gtk::TreeModel::iterator& row);
  void on_adddel_user_changed(const Gtk::TreeModel::iterator& row, guint column);

  void on_userlevel_changed(AppState::userlevels userlevel) override;

  guint m_colName;
  guint m_colTitle;

  mutable AddDel_WithButtons m_AddDel; //mutable because its get_ methods aren't const.
};

} //namespace Glom

#endif //BOX_PRINT_LAYOUTS_H

