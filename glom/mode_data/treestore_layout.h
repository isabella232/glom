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

#ifndef GLOM_MODE_DATA_TREESTORE_LAYOUT_H
#define GLOM_MODE_DATA_TREESTORE_LAYOUT_H

#include <gtkmm/treestore.h>
#include "../data_structure/layout/layoutitem_portal.h"

class TreeStore_Layout : public Gtk::TreeStore
{
protected:
  TreeStore_Layout();

public:

   enum enumType
   {
     TYPE_FIELD,
     TYPE_GROUP,
     TYPE_PORTAL
   };

  //Tree model columns:  
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns()
    { add(m_col_type); add(m_col_name); add(m_col_relationship); add(m_col_field_formatting); add(m_col_title); add(m_col_editable); add(m_col_sequence); add(m_col_columns_count); add(m_col_portal_relationship); add(m_col_portal); }

    Gtk::TreeModelColumn<enumType> m_col_type;
    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    Gtk::TreeModelColumn<Relationship> m_col_relationship; //Only for fields
    Gtk::TreeModelColumn<LayoutItem_Field> m_col_field_formatting; //Only for fields
    Gtk::TreeModelColumn<Glib::ustring> m_col_title; //only for groups
    Gtk::TreeModelColumn<bool> m_col_editable; 
    Gtk::TreeModelColumn<guint> m_col_sequence;
    Gtk::TreeModelColumn<guint> m_col_columns_count; //Only for groups.

    Gtk::TreeModelColumn<Glib::ustring> m_col_portal_relationship; //Only for portals.
    Gtk::TreeModelColumn<LayoutItem_Portal> m_col_portal;
  };

  ModelColumns m_columns;

  static Glib::RefPtr<TreeStore_Layout> create();

  virtual void fill_sequences();
  virtual void fill_sequences(const iterator& iter);

protected:
  //Overridden virtual functions:
  virtual bool row_draggable_vfunc(const Gtk::TreeModel::Path& path) const;
  virtual bool row_drop_possible_vfunc(const Gtk::TreeModel::Path& dest, const Gtk::SelectionData& selection_data) const;

};

#endif //GLOM_MODE_DATA_TREESTORE_LAYOUT_H

