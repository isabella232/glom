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

#include <glom/mode_design/layout/treestore_layout.h>
#include <iostream>

namespace Glom
{

TreeStore_Layout::TreeStore_Layout()
: Gtk::TreeStore()
{
  //We can't just call Gtk::TreeModel(m_columns) in the initializer list
  //because m_columns does not exist when the base class constructor runs.
  //And we can't have a static m_columns instance, because that would be
  //instantiated before the gtkmm type system.
  //So, we use this method, which should only be used just after creation:
  set_column_types(m_columns);
}

Glib::RefPtr<TreeStore_Layout> TreeStore_Layout::create()
{
  return Glib::RefPtr<TreeStore_Layout>( new TreeStore_Layout() );
}

bool TreeStore_Layout::row_draggable_vfunc(const Gtk::TreeModel::Path& path) const
{
  //Allow everything to be dragged.
  return Gtk::TreeStore::row_draggable_vfunc(path);
}

bool TreeStore_Layout::row_drop_possible_vfunc(const Gtk::TreeModel::Path& dest, const Gtk::SelectionData& selection_data) const
{
    //Get the Row that is being dragged:
    //TODO: Add const version of get_from_selection_data(): Glib::RefPtr<const Gtk::TreeModel> refThis = Glib::RefPtr<const Gtk::TreeModel>(this);
    Glib::RefPtr<Gtk::TreeModel> refThis = Glib::RefPtr<Gtk::TreeModel>(const_cast<TreeStore_Layout*>(this));
    refThis->reference(); //, true /* take_copy */)
    Gtk::TreeModel::Path path_dragged_row;
    Gtk::TreeModel::Path::get_from_selection_data(selection_data, refThis, path_dragged_row);

    if(path_dragged_row == dest)
      return false; //Prevent a row from being dragged onto itself.

  //Only allow items to become children of groups.
  //Do not allow items to become children of fields.

  //dest is the path that the row would have after it has been dropped:
  //But in this case we are more interested in the parent row:
  Gtk::TreeModel::Path dest_parent = dest;
  bool dest_is_not_top_level = dest_parent.up();
  if(!dest_is_not_top_level || dest_parent.empty())
  {
    //The user wants to move something to the top-level.
    //Only groups can be at the top level, so we examine the thing being dragged:

    //Get an iterator for the row at this path:
    //We must unconst this. This should not be necessary with a future version of gtkmm.
    auto unconstThis = const_cast<TreeStore_Layout*>(this); //TODO: Add a const version of get_iter to TreeModel:
    const auto iter_dragged = unconstThis->get_iter(path_dragged_row);
    //const auto iter_dragged = get_iter(path_dragged_row);

    if(iter_dragged)
    {
      Gtk::TreeModel::Row row = *iter_dragged;

      std::shared_ptr<LayoutItem> layout_item = row[m_columns.m_col_layout_item];
      std::shared_ptr<LayoutGroup> layout_group = std::dynamic_pointer_cast<LayoutGroup>(layout_item);
      const bool is_group = (bool)layout_group;

      return is_group; //Only groups can be dragged to the top-level.
    }
  }
  else
  {
    if(dest_parent == path_dragged_row)
      return false; //Don't allow an item to be dragged under itself. TODO: Check the whole parent hierarchy.

    //Get an iterator for the row at the requested parent's path:
    //We must unconst this. This should not be necessary with a future version of gtkmm.
    auto unconstThis = const_cast<TreeStore_Layout*>(this); //TODO: Add a const version of get_iter to TreeModel:
    const auto iter_dest_parent = unconstThis->get_iter(dest_parent);
    //const auto iter_dest_parent = get_iter(dest);
    if(iter_dest_parent)
    {
      Gtk::TreeModel::Row row_parent = *iter_dest_parent;

      std::shared_ptr<LayoutItem> layout_item = row_parent[m_columns.m_col_layout_item];
      std::shared_ptr<LayoutGroup> layout_group = std::dynamic_pointer_cast<LayoutGroup>(layout_item);
      const bool is_group = (bool)layout_group;

      return is_group; //Only groups can contain other items.
    }
  }

  //Fallback:
  return Gtk::TreeStore::row_drop_possible_vfunc(dest, selection_data);
}

void TreeStore_Layout::fill_sequences()
{
  guint sequence = 1;
  for(auto iter_children = children().begin(); iter_children != children().end(); ++iter_children)
  {
    Gtk::TreeModel::Row row = *iter_children;
    row[m_columns.m_col_sequence] = sequence;
    ++sequence;

    //Recurse:
    fill_sequences(iter_children);
  }
}

void TreeStore_Layout::fill_sequences(const iterator& iter)
{
  guint sequence = 1;
  for(auto iter_children = iter->children().begin(); iter_children != iter->children().end(); ++iter_children)
  {
    Gtk::TreeModel::Row row = *iter_children;
    row[m_columns.m_col_sequence] = sequence;
    ++sequence;

    //Recurse:
    fill_sequences(iter_children);
  }
}

} //namespace Glom
