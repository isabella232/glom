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

#ifndef BOX_DATA_DETAILS_H
#define BOX_DATA_DETAILS_H

#include "box_data.h"
#include "box_data_list_related.h"
#include "../utility_widgets/placeholder.h"
#include "../utility_widgets/flowtablewithfields.h"

/**
  *@author Murray Cumming
  */

class Box_Data_Details : public Box_Data
{
public: 
  Box_Data_Details(bool bWithNavButtons = true);
  virtual ~Box_Data_Details();
  
  virtual void init_db_details(const Glib::ustring& strDatabaseName, const Glib::ustring& strTableName, const Glib::ustring& strPrimaryKeyValue);
  virtual void init_db_details(const Glib::ustring& strPrimaryKeyValue);
  
  virtual Glib::ustring get_primary_key_value() const; //Actual primary key value of this record.
  virtual Glib::ustring get_primary_key_value_selected(); //Value in the primary key's cell.
    
  virtual Field get_Entered_Field(guint index) const;
  
  //Signals:
  
  typedef sigc::signal<void> type_signal_void;
  type_signal_void signal_nav_first();
  type_signal_void signal_nav_prev();
  type_signal_void signal_nav_next();
  type_signal_void signal_nav_last();
  
  typedef sigc::signal<void, Glib::ustring> type_signal_record_deleted; //arg is PrimaryKey.
  type_signal_record_deleted signal_record_deleted();
  
  typedef sigc::signal<void, Glib::ustring, Glib::ustring> type_signal_user_requested_related_details; //args are TableName, PrimaryKey.
  type_signal_user_requested_related_details signal_user_requested_related_details();
   
protected:
  virtual void fill_from_database(); //override.
  virtual void fill_related();

  //Signal handlers:
  virtual void on_button_new();
  virtual void on_button_del();

  virtual void on_button_nav_first();
  virtual void on_button_nav_prev();
  virtual void on_button_nav_next();
  virtual void on_button_nav_last();

  //Signal handler: The last 2 args are bind-ed.
  virtual void on_related_record_added(Glib::ustring strKeyValue, Glib::ustring strFromKeyName);

  //Signal handler: The last arg is bind-ed.
  virtual void on_related_user_requested_details(Glib::ustring strKeyValue, Glib::ustring strTableName);

  virtual void on_flowtable_field_edited(Glib::ustring id);

   
  Glib::ustring m_strPrimaryKeyValue;

  //Member widgets:
  Gtk::VPaned m_Paned;
  Gtk::HBox m_HBox;
  Gtk::Button m_Button_New;
  Gtk::Button m_Button_Del;

  Gtk::Frame m_Frame_Related;
  Gtk::Alignment m_Alignment_Related;
  Gtk::Label m_Label_Related;
  Gtk::Notebook m_Notebook_Related;

  Gtk::Button m_Button_Nav_First;
  Gtk::Button m_Button_Nav_Prev;
  Gtk::Button m_Button_Nav_Next;
  Gtk::Button m_Button_Nav_Last;
  FlowTableWithFields m_FlowTable;

  guint m_ColumnName, m_ColumnValue;
  bool m_bDoNotRefreshRelated;
  bool m_ignore_signals;

  type_signal_user_requested_related_details m_signal_user_requested_related_details;

  type_signal_void m_signal_nav_first;
  type_signal_void m_signal_nav_prev;
  type_signal_void m_signal_nav_next;
  type_signal_void m_signal_nav_last;

  type_signal_record_deleted m_signal_record_deleted;
};

#endif //BOX_DATA_DETAILS_H
