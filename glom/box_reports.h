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

#ifndef BOX_REPORTS_H
#define BOX_REPORTS_H

#include "box_db_table.h"
#include <libglom/data_structure/report.h>

namespace Glom
{

class Box_Reports : public Box_DB_Table
{
public:
  Box_Reports(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Box_Reports();

private:
  virtual bool fill_from_database(); //override

  virtual void fill_row(const Gtk::TreeModel::iterator& iter, const sharedptr<const Report>& report);

  virtual void save_to_document();

  //Signal handlers:
  virtual void on_adddel_Add(const Gtk::TreeModel::iterator& row);
  virtual void on_adddel_Delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd);
  virtual void on_adddel_Edit(const Gtk::TreeModel::iterator& row);
  virtual void on_adddel_changed(const Gtk::TreeModel::iterator& row, guint column);

  virtual void on_userlevel_changed(AppState::userlevels userlevel);

  Gtk::Label* m_pLabelFrameTitle;
  guint m_colReportName;
  guint m_colTitle;

  mutable AddDel_WithButtons m_AddDel; //mutable because its get_ methods aren't const.
};

} //namespace Glom

#endif //BOX_REPORTS_H

