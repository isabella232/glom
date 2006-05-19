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

#ifndef BOX_DATA_LIST_RELATED_H
#define BOX_DATA_LIST_RELATED_H

#include "box_data_list.h"
#include "../utility_widgets/layoutwidgetbase.h"

class Dialog_Layout_List_Related;

class Box_Data_List_Related : 
  public Box_Data_List,
  public LayoutWidgetBase
{
public: 
  Box_Data_List_Related();
  virtual ~Box_Data_List_Related();

  /**
   * @param portal: The full portal details
   */
  virtual bool init_db_details(const sharedptr<const LayoutItem_Portal>& portal, bool show_title = true);

  /**
   * @param foreign_key_value: The value that should be found in this table.
   * @param from_table_primary_key_value The primary key of the parent record's table, used to associate new related records.
   */
  virtual bool refresh_data_from_database_with_foreign_key(const Gnome::Gda::Value& foreign_key_value);

  virtual sharedptr<Relationship> get_relationship() const;
  virtual sharedptr<const Field> get_key_field() const;

  virtual void show_layout_dialog();

  sigc::signal<void, Gnome::Gda::Value> signal_record_added;

  bool get_has_suitable_record_to_view_details() const;
  void get_suitable_record_to_view_details(const Gnome::Gda::Value& primary_key_value, Glib::ustring& table_name, Gnome::Gda::Value& table_primary_key_value);

  ///@param relationship_name
  typedef sigc::signal<void, const Glib::ustring&> type_signal_record_changed;
  type_signal_record_changed signal_record_changed();

protected:
  virtual bool fill_from_database(); //Override.
  virtual type_vecLayoutFields get_fields_to_show() const; //override

  sharedptr<const LayoutItem_Field> get_field_is_from_non_hidden_related_record() const;
  sharedptr<const LayoutItem_Field> get_field_identifies_non_hidden_related_record() const;

  virtual void on_adddel_user_requested_add();
  virtual void on_adddel_user_changed(const Gtk::TreeModel::iterator& row, guint col);
  virtual void on_adddel_user_added(const Gtk::TreeModel::iterator& row, guint col_with_first_value); //Override.
  virtual void on_record_added(const Gnome::Gda::Value& primary_key_value, const Gtk::TreeModel::iterator& row); //Override. Not a signal handler.
  virtual void on_record_deleted(const Gnome::Gda::Value& primary_key_value); //override.
  virtual void on_dialog_layout_hide(); //override.

  virtual void enable_buttons(); //override

protected:
  Gtk::Frame m_Frame;
  Gtk::Alignment m_Alignment;
  Gtk::Label m_Label;
  Dialog_Layout_List_Related* m_pDialogLayoutRelated;

  sharedptr<LayoutItem_Portal> m_portal;
  sharedptr<Field> m_key_field;
  Gnome::Gda::Value m_key_value;

  type_signal_record_changed m_signal_record_changed;
};

#endif
