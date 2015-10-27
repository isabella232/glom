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

#ifndef BOX_TABLES_H
#define BOX_TABLES_H

#include <glom/box_withbuttons.h>
#include <glom/base_db.h>
#include <glom/utility_widgets/adddel/adddel_withbuttons.h>
#include <gtkmm/builder.h>
#include <gtkmm/checkbutton.h>

namespace Glom
{

/** This widget offers a list of tables in the database,
  * allowing the user to select a table,
  * or add or delete a table.
  */
class Box_Tables 
: public Box_WithButtons,
  public Base_DB
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Box_Tables(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Box_Tables();

private:
  bool fill_from_database() override;

  void fill_table_row(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const TableInfo>& table_info);

  //Signal handlers:
#ifndef GLOM_ENABLE_CLIENT_ONLY
  void save_to_document() override;

  void on_adddel_Add(const Gtk::TreeModel::iterator& row);
  void on_adddel_Delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd);
  void on_adddel_changed(const Gtk::TreeModel::iterator& row, guint column);
#endif //GLOM_ENABLE_CLIENT_ONLY
  void on_adddel_Edit(const Gtk::TreeModel::iterator& row);

  void on_show_hidden_toggled();

  virtual void on_userlevel_changed(AppState::userlevels userlevel);
      
  Gtk::CheckButton* m_pCheckButtonShowHidden;
  guint m_colTableName;
  guint m_colHidden;
  guint m_colTitle;
  guint m_colDefault;
  guint m_colTitleSingular;

  mutable AddDel_WithButtons m_AddDel; //mutable because its get_ methods aren't const.
};

} //namespace Glom

#endif //BOX_TABLES_H

