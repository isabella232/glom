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

#ifndef BOX_DATA_LIST_H
#define BOX_DATA_LIST_H

#include "box_data.h"

class Box_Data_List : public Box_Data
{
public: 
  Box_Data_List();
  virtual ~Box_Data_List();

  virtual Glib::ustring get_primary_key_value(const Gtk::TreeModel::iterator& row);
  virtual Glib::ustring get_primary_key_value_selected();
  
  virtual Field get_Entered_Field(guint index);
  
  virtual guint get_records_count() const;
    
  //Primary Key value:
  typedef sigc::signal<void, Glib::ustring> type_signal_user_requested_details;
  type_signal_user_requested_details signal_user_requested_details();

  //Signal Handlers:
  virtual void on_details_nav_first();
  virtual void on_details_nav_previous();
  virtual void on_details_nav_next();
  virtual void on_details_nav_last();
  virtual void on_Details_record_deleted(Glib::ustring strPrimaryKey);

protected:
  virtual void fill_from_database(); //override.
  virtual void fill_column_titles();

  //Signal handlers:
  virtual void on_AddDel_user_requested_add();
  virtual void on_AddDel_user_requested_edit(const Gtk::TreeModel::iterator& row);
  virtual void on_AddDel_user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd);
  virtual void on_AddDel_user_added(const Gtk::TreeModel::iterator& row);
  virtual void on_AddDel_user_reordered_columns();

  virtual void on_AddDel_user_changed(const Gtk::TreeModel::iterator& row, guint col);

  virtual void on_record_added(const Glib::ustring& strPrimaryKey); //Not a signal handler. To be overridden.

  
  //Member widgers:
  mutable AddDel_WithButtons m_AddDel; //mutable because its get_ methods aren't const.

  bool m_has_one_or_more_records;
  guint m_first_col;

  type_signal_user_requested_details m_signal_user_requested_details;
};

#endif
