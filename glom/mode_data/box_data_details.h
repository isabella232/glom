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
//#include "box_data_list_related.h"
#include "../utility_widgets/placeholder.h"
#include "../utility_widgets/flowtablewithfields.h"

namespace Glom
{

class Box_Data_Details : public Box_Data
{
public: 
  Box_Data_Details(bool bWithNavButtons = true);
  virtual ~Box_Data_Details();

  virtual bool init_db_details(const FoundSet& found_set, const Glib::ValueBase& primary_key_value);
  virtual bool refresh_data_from_database_with_primary_key(const Glib::ValueBase& primary_key_value);
  virtual bool refresh_data_from_database_blank();

  virtual void print_layout();

  virtual Glib::ValueBase get_primary_key_value() const; //Actual primary key value of this record.
  virtual Glib::ValueBase get_primary_key_value_selected(); //Value in the primary key's cell.

  virtual void set_primary_key_value(const Gtk::TreeModel::iterator& /* row */, const Glib::ValueBase& value);

  virtual Glib::ValueBase get_entered_field_data(const sharedptr<const LayoutItem_Field>& field) const;
  virtual void set_entered_field_data(const sharedptr<const LayoutItem_Field>& field, const Glib::ValueBase& value);
  virtual void set_entered_field_data(const Gtk::TreeModel::iterator& row, const sharedptr<const LayoutItem_Field>& field, const Glib::ValueBase& value);

  //Signals:
  typedef sigc::signal<void> type_signal_void;
  type_signal_void signal_nav_first();
  type_signal_void signal_nav_prev();
  type_signal_void signal_nav_next();
  type_signal_void signal_nav_last();

  typedef sigc::signal<void, const Glib::ValueBase&> type_signal_record_deleted; //arg is PrimaryKey.   //TODO: pass by const ref?
  type_signal_record_deleted signal_record_deleted();

   /** For instance,
    * void on_requested_related_details(const Glib::ustring& table_name, Glib::ValueBase primary_key_value);
    */
  typedef sigc::signal<void, const Glib::ustring&, Glib::ValueBase> type_signal_requested_related_details;
  type_signal_requested_related_details signal_requested_related_details();

protected:
  virtual bool fill_from_database(); //override.
  virtual void create_layout();
  //virtual void fill_related();

  virtual sharedptr<Field> get_field_primary_key() const;

  void print_layout_group(xmlpp::Element* node_parent, const sharedptr<const LayoutGroup>& group);

  //Signal handlers:
  virtual void on_button_new();
  virtual void on_button_del();

  virtual void on_button_nav_first();
  virtual void on_button_nav_prev();
  virtual void on_button_nav_next();
  virtual void on_button_nav_last();

  virtual void on_userlevel_changed(AppState::userlevels user_level); //override
  virtual void on_flowtable_layout_changed();

  //Signal handler: The last 2 args are bind-ed.
  virtual void on_related_record_added(Glib::ValueBase key_value, Glib::ustring strFromKeyName);

  //Signal handler: The last arg is bind-ed.
  //virtual void on_related_user_requested_details(Glib::ValueBase key_value, Glib::ustring table_name);

  virtual void on_flowtable_field_edited(const sharedptr<const LayoutItem_Field>& id, const Glib::ValueBase& value);
  void on_flowtable_field_open_details_requested(const sharedptr<const LayoutItem_Field>& id, const Glib::ValueBase& value);
  void on_flowtable_related_record_changed(const Glib::ustring& relationship_name);
  void on_flowtable_requested_related_details(const Glib::ustring& table_name, Glib::ValueBase primary_key_value);

  void on_flowtable_script_button_clicked(const sharedptr<const LayoutItem_Button>& layout_item);

  virtual void recalculate_fields_for_related_records(const Glib::ustring& relationship_name);

  sharedptr<Field> m_field_primary_key;
  Glib::ValueBase m_primary_key_value;

  //Member widgets:
  Gtk::ScrolledWindow m_ScrolledWindow;
  Gtk::HBox m_HBox;
  Gtk::Button m_Button_New;
  Gtk::Button m_Button_Del;

  /*
  Gtk::Frame m_Frame_Related;
  Gtk::Alignment m_Alignment_Related;
  Gtk::Label m_Label_Related;
  Gtk::Notebook m_Notebook_Related;
  */

  Gtk::Button m_Button_Nav_First;
  Gtk::Button m_Button_Nav_Prev;
  Gtk::Button m_Button_Nav_Next;
  Gtk::Button m_Button_Nav_Last;
  FlowTableWithFields m_FlowTable;

  guint m_ColumnName, m_ColumnValue;
  bool m_bDoNotRefreshRelated; //Stops us from refreshing related records in response to an addition of a related record.
  bool m_ignore_signals;

  type_signal_void m_signal_nav_first;
  type_signal_void m_signal_nav_prev;
  type_signal_void m_signal_nav_next;
  type_signal_void m_signal_nav_last;

  type_signal_record_deleted m_signal_record_deleted;
  type_signal_requested_related_details m_signal_requested_related_details;
};

} //namespace Glom

#endif //BOX_DATA_DETAILS_H
