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

#include "liststore_with_addrow.h"



namespace Glom
{

ListStoreWithAddRow::ListStoreWithAddRow(const Gtk::TreeModelColumnRecord& columns, const type_vec_fields& /* column_fields */)
: Gtk::ListStore(columns)
{
}

Glib::RefPtr<ListStoreWithAddRow> ListStoreWithAddRow::create(const Gtk::TreeModelColumnRecord& columns, const type_vec_fields& column_fields)
{
  return Glib::RefPtr<ListStoreWithAddRow>(new ListStoreWithAddRow(columns, column_fields));
}

void ListStoreWithAddRow::set_is_not_placeholder(const TreeModel::iterator& iter)
{
  //If this is the last row then add an extra row so that can be the placeholder instead.
  if(iter == get_last_row())
    append();
}

bool ListStoreWithAddRow::get_is_placeholder(const TreeModel::iterator& iter) const
{
  //The last row is always the placeholder row.
  ListStoreWithAddRow* nonconst = const_cast<ListStoreWithAddRow*>(this); 
  return iter == nonconst->get_last_row();
}


Gtk::TreeModel::iterator ListStoreWithAddRow::get_last_row()
{
 //The last row is always the placeholder row.
  return Gtk::ListStore::children().end();
}

Gtk::TreeModel::iterator ListStoreWithAddRow::get_placeholder_row()
{
  return get_last_row();
}

void ListStoreWithAddRow::clear()
{
  Gtk::ListStore::clear();
}

Gtk::TreeModel::iterator ListStoreWithAddRow::erase(const iterator& iter)
{
  return Gtk::ListStore::erase(iter);
}

} //namespace Glom


