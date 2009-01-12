/* Glom
 *
 * Copyright (C) 2009 Openismus GmbH
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

#ifndef GLOM_TREEMODEL_WITH_ADDROW_H
#define GLOM_TREEMODEL_WITH_ADDROW_H

#include <gtkmm/treemodel.h>
#include <glom/base_db.h>

namespace Glom
{


class TreeModelWithAddRow
  // We can't share this base because it would need TreeModel to be a virtual base (derived virtually from its own base): 
  //: public Gtk::TreeModel
  // We would need this to use Glib::RefPtr but let's avoid the risk of getting the MI wrong: public virtual Glib::ObjectBase //This is a suitable MI base class because it virtual derives from its own base.
{
public:
  typedef Base_DB::type_vecLayoutFields type_vec_fields;

  virtual void set_is_not_placeholder(const Gtk::TreeModel::iterator& iter) = 0;
  virtual bool get_is_placeholder(const Gtk::TreeModel::iterator& iter) const = 0;

  /** Get the last row - usually the placeholder.
   */
  virtual Gtk::TreeModel::iterator get_last_row() = 0;

  /** Get the placeholder row.
   */
  virtual Gtk::TreeModel::iterator get_placeholder_row() = 0;

  virtual void clear() = 0;

  /** Removes the given row from the list store.
   * @param iter The iterator to the row to be removed.
   * @result An iterator to the next row, or end() if there is none.
   */
  virtual Gtk::TreeModel::iterator erase(const Gtk::TreeModel::iterator& iter) = 0;

protected:
};

} //namespace Glom

#endif //GLOM_TREEMODEL_WITH_ADDROW_H

