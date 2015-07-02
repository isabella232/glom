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

#include "treestore_report_layout.h"
#include <libglom/data_structure/layout/report_parts/layoutitem_groupby.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_summary.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_verticalgroup.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_header.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_footer.h>
#include <libglom/data_structure/layout/layoutitem_field.h>
#include <libglom/data_structure/layout/layoutitem_text.h>
#include <iostream>

namespace Glom
{

TreeStore_ReportLayout::TreeStore_ReportLayout()
: Gtk::TreeStore()
{
  //We can't just call Gtk::TreeModel(m_columns) in the initializer list
  //because m_columns does not exist when the base class constructor runs.
  //And we can't have a static m_columns instance, because that would be
  //instantiated before the gtkmm type system.
  //So, we use this method, which should only be used just after creation:
  set_column_types(m_columns);
}

Glib::RefPtr<TreeStore_ReportLayout> TreeStore_ReportLayout::create()
{
  return Glib::RefPtr<TreeStore_ReportLayout>( new TreeStore_ReportLayout() );
}

bool TreeStore_ReportLayout::row_draggable_vfunc(const Gtk::TreeModel::Path& path) const
{
  //Allow everything to be dragged.
  return Gtk::TreeStore::row_draggable_vfunc(path);
}

bool TreeStore_ReportLayout::row_drop_possible_vfunc(const Gtk::TreeModel::Path& dest, const Gtk::SelectionData& selection_data) const
{
  //Get the Row that is being dragged:
  //TODO: Add const version of get_from_selection_data(): Glib::RefPtr<const Gtk::TreeModel> refThis = Glib::RefPtr<const Gtk::TreeModel>(this);
  Glib::RefPtr<Gtk::TreeModel> refThis = Glib::RefPtr<Gtk::TreeModel>(const_cast<TreeStore_ReportLayout*>(this));
  refThis->reference(); //, true /* take_copy */)
  Gtk::TreeModel::Path path_dragged_row;
  Gtk::TreeModel::Path::get_from_selection_data(selection_data, refThis, path_dragged_row);

  if(path_dragged_row == dest)
    return false; //Prevent a row from being dragged onto itself.


  auto iter_dragged_row = refThis->get_iter(path_dragged_row);
  if(!iter_dragged_row)
    return false;

  std::shared_ptr<LayoutItem> item_dragged_row = (*iter_dragged_row)[m_columns.m_col_item];

  //dest is the path that the row would have after it has been dropped:
  //But in this case we are more interested in the parent row:
  Gtk::TreeModel::Path dest_parent = dest;
  bool dest_is_not_top_level = dest_parent.up();
  if(!dest_is_not_top_level || dest_parent.empty())
  {
    return may_be_child_of(std::shared_ptr<const LayoutItem>() /* parent */, item_dragged_row);
  }
  else
  {
    if(dest_parent == path_dragged_row)
      return false; //Don't allow an item to be dragged under itself. TODO: Check the whole parent hierarchy.

    //Get an iterator for the row at the requested parent's path:
    //We must unconst this. This should not be necessary with a future version of gtkmm.
    TreeStore_ReportLayout* unconstThis = const_cast<TreeStore_ReportLayout*>(this); //TODO: Add a const version of get_iter to TreeModel:
    const_iterator iter_dest_parent = unconstThis->get_iter(dest_parent);
    //const_iterator iter_dest_parent = get_iter(dest);
    if(iter_dest_parent)
    {
      Gtk::TreeModel::Row row_parent = *iter_dest_parent;
      std::shared_ptr<LayoutItem> item_parent = (row_parent)[m_columns.m_col_item];

      return may_be_child_of(item_parent, item_dragged_row);
    }
  }

  //Fallback:
  return Gtk::TreeStore::row_drop_possible_vfunc(dest, selection_data);
}

void TreeStore_ReportLayout::fill_sequences()
{
  guint sequence = 1;
  for(iterator iter_children = children().begin(); iter_children != children().end(); ++iter_children)
  {
    Gtk::TreeModel::Row row = *iter_children;
    row[m_columns.m_col_sequence] = sequence;
    ++sequence;

    //Recurse:
    fill_sequences(iter_children);
  }
}

void TreeStore_ReportLayout::fill_sequences(const iterator& iter)
{
  guint sequence = 1;
  for(iterator iter_children = iter->children().begin(); iter_children != iter->children().end(); ++iter_children)
  {
    Gtk::TreeModel::Row row = *iter_children;
    row[m_columns.m_col_sequence] = sequence;
    ++sequence;

    //Recurse:
    fill_sequences(iter_children);
  }
}

bool TreeStore_ReportLayout::may_be_child_of(const std::shared_ptr<const LayoutItem>& parent, const std::shared_ptr<const LayoutItem>& suggested_child)
{
  if(!parent)
    return true; //Anything may be at the top-level.

  if(!(std::dynamic_pointer_cast<const LayoutGroup>(parent)))
    return false; //Only LayoutGroup (and derived types) may have children.

  const bool child_fieldsummary = (bool)std::dynamic_pointer_cast<const LayoutItem_FieldSummary>(suggested_child);

  std::shared_ptr<const LayoutItem_Summary> summary = std::dynamic_pointer_cast<const LayoutItem_Summary>(parent);

  //A Summary may only have FieldSummary children:
  if(summary && !(bool)child_fieldsummary)
      return false;

  //FieldSummary may only be a member of Summary:
  if(child_fieldsummary && !summary)
    return false;


  const bool header = (bool)std::dynamic_pointer_cast<const LayoutItem_Header>(parent);

  const bool footer = (bool)std::dynamic_pointer_cast<const LayoutItem_Footer>(parent);

  const bool child_groupby = (bool)std::dynamic_pointer_cast<const LayoutItem_GroupBy>(suggested_child);
  const bool child_summary = (bool)std::dynamic_pointer_cast<const LayoutItem_Summary>(suggested_child);

  if((header || footer) && (child_summary || child_groupby))
    return false; //Nothing that needs data can be in a Header or Footer. A field is allowed because it could show constans from System Preferences.

  return true;
}

} //namespace Glom
