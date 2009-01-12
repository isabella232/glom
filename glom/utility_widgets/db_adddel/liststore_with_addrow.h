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

#ifndef GLOM_LISTSTORE_WITH_ADDROW_H
#define GLOM_LISTSTORE_WITH_ADDROW_H

#include <gtkmm/liststore.h>
#include <glom/utility_widgets/db_adddel/treemodel_with_addrow.h>

namespace Glom
{

class ListStoreWithAddRow
  : public Gtk::ListStore,
    public TreeModelWithAddRow
{
  ListStoreWithAddRow(const Gtk::TreeModelColumnRecord& columns, const type_vec_fields& column_fields);

public:
  static Glib::RefPtr<ListStoreWithAddRow> create(const Gtk::TreeModelColumnRecord& columns, const type_vec_fields& column_fields);

  virtual void set_is_not_placeholder(const TreeModel::iterator& iter);
  virtual bool get_is_placeholder(const TreeModel::iterator& iter) const;

  /** Get the last row - usually the placeholder.
   */
  virtual TreeModel::iterator get_last_row();

  /** Get the placeholder row.
   */
  virtual TreeModel::iterator get_placeholder_row();


  virtual void clear();

  /** Removes the given row from the list store.
   * @param iter The iterator to the row to be removed.
   * @result An iterator to the next row, or end() if there is none.
   */
  virtual iterator erase(const iterator& iter);

protected:
};

} //namespace Glom

#endif //GLOM_LISTSTORE_WITH_ADDROW_H

