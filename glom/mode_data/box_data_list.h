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
#include "../utility_widgets/db_adddel/db_adddel_withbuttons.h"

class Box_Data_List : public Box_Data
{
public: 
  Box_Data_List();
  virtual ~Box_Data_List();

  virtual Gnome::Gda::Value get_primary_key_value(const Gtk::TreeModel::iterator& row);
  virtual Gnome::Gda::Value get_primary_key_value_selected();
  
  virtual Gnome::Gda::Value get_entered_field_data(const Field& field) const;
  virtual void set_entered_field_data(const Field& field, const Gnome::Gda::Value& value);
  
  virtual guint get_records_count() const;
    
  //Primary Key value:
  typedef sigc::signal<void, Gnome::Gda::Value> type_signal_user_requested_details;
  type_signal_user_requested_details signal_user_requested_details();

  //Signal Handlers:
  virtual void on_details_nav_first();
  virtual void on_details_nav_previous();
  virtual void on_details_nav_next();
  virtual void on_details_nav_last();
  virtual void on_Details_record_deleted(Gnome::Gda::Value primary_key_value);

protected:
  virtual void fill_from_database(); //override.
  virtual void fill_column_titles();

  virtual bool get_field_primary_key_index(guint& field_column) const; //TODO: visible 
  virtual bool get_field_primary_key(Field& field) const;

  void do_lookups(const Gtk::TreeModel::iterator& row, const Field& field_changed, const Gnome::Gda::Value& field_value, const Field& primary_key, const Gnome::Gda::Value& primary_key_value);

  //Signal handlers:
  virtual void on_adddel_user_requested_add();
  virtual void on_adddel_user_requested_edit(const Gtk::TreeModel::iterator& row);
  virtual void on_adddel_user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd);
  virtual void on_adddel_user_added(const Gtk::TreeModel::iterator& row);
  virtual void on_adddel_user_reordered_columns();

  virtual void on_adddel_user_requested_layout();

  virtual void on_adddel_user_changed(const Gtk::TreeModel::iterator& row, guint col);

  virtual void on_record_added(const Gnome::Gda::Value& primary_key_value); //Not a signal handler. To be overridden.

  virtual bool get_field_column_index(const Glib::ustring& field_name, guint& index) const;
  
  //Member widgers:
  mutable DbAddDel_WithButtons m_AddDel; //mutable because its get_ methods aren't const.

  bool m_has_one_or_more_records;

  type_signal_user_requested_details m_signal_user_requested_details;
};

#endif
