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
   * @param Relationship: The relationship used by the parent table to get rows from this table.
   */
  virtual void init_db_details(const Relationship& relationship);

  /**
   * @param foreign_key_value: The value that should be found in this table.
   * @param from_table_primary_key_value The primary key of the parent record's table, used to associate new related records.
   */
  virtual void refresh_db_details(const Gnome::Gda::Value& foreign_key_value, const Gnome::Gda::Value& from_table_primary_key_value);

  virtual Relationship get_relationship() const;
  virtual Field get_key_field() const;

  virtual void show_layout_dialog();

  sigc::signal<void, Gnome::Gda::Value> signal_record_added;

protected:
  virtual void fill_from_database(); //Override.
  virtual type_vecFields get_fields_to_show() const; //override

  virtual void on_adddel_user_added(const Gtk::TreeModel::iterator& row); //Override.
  virtual void on_record_added(const Gnome::Gda::Value& primary_key_value); //Override. Not a signal handler.
  virtual void on_dialog_layout_hide(); //override.

  virtual void enable_buttons();

protected:
  Gtk::Frame m_Frame;
  Gtk::Alignment m_Alignment;
  Gtk::Label m_Label;
  Dialog_Layout_List_Related* m_pDialogLayoutRelated;

  Relationship m_relationship; //The relationship of the parent table to this one.
  Field m_key_field;
  Gnome::Gda::Value m_key_value;
};

#endif
