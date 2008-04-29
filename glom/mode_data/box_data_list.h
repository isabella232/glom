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

#include "config.h" // GLOM_ENABLE_CLIENT_ONLY

#include "box_data_manyrecords.h"
#include "../utility_widgets/db_adddel/db_adddel_withbuttons.h"

namespace Glom
{

class Box_Data_List : public Box_Data_ManyRecords
{
public: 
  Box_Data_List();
  virtual ~Box_Data_List();

  void refresh_data_from_database_blank();

  Gnome::Gda::Value get_primary_key_value_first() const;

  //Implementations of pure virtual methods from Base_DB_Table_Data:
  virtual Gnome::Gda::Value get_primary_key_value_selected() const;
  virtual void set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value);

  virtual Gnome::Gda::Value get_entered_field_data(const sharedptr<const LayoutItem_Field>& field) const;
  virtual void set_entered_field_data(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value);
  virtual void set_entered_field_data(const Gtk::TreeModel::iterator& row, const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value);

  bool get_showing_multiple_records() const;

  void set_read_only(bool read_only = true);

  //For instance, change "Open" to "Select" when used to select an ID.
  void set_open_button_title(const Glib::ustring& title);

  ///Highlight and scroll to the specified record, with primary key value @primary_key_value.
  void set_primary_key_value_selected(const Gnome::Gda::Value& primary_key_value);


  //Signal Handlers:
  virtual void on_details_nav_first();
  virtual void on_details_nav_previous();
  virtual void on_details_nav_next();
  virtual void on_details_nav_last();
  virtual void on_details_record_deleted(const Gnome::Gda::Value& primary_key_value);

  void get_record_counts(gulong& total, gulong& found) const;

  #ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual void on_dialog_layout_hide(); //override
  #endif //GLOM_ENABLE_CLIENT_ONLY


protected:

  //Implementations of pure virtual methods from Base_DB_Table_Data:
  virtual Gnome::Gda::Value get_primary_key_value(const Gtk::TreeModel::iterator& row) const;

  //Overrides of functions from Box_Data:
  virtual void create_layout(); //override
  virtual Document_Glom::type_list_layout_groups create_layout_get_layout();
  void create_layout_add_group(const sharedptr<LayoutGroup>& layout_group);
    
  virtual bool fill_from_database(); //override.
  virtual void enable_buttons();

  virtual sharedptr<Field> get_field_primary_key() const;

  //Signal handlers:
  void on_adddel_user_requested_edit(const Gtk::TreeModel::iterator& row);
  void on_adddel_user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd);
  void on_adddel_user_reordered_columns();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_adddel_user_requested_layout();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  void on_adddel_script_button_clicked(const sharedptr<const LayoutItem_Button>& layout_item, const Gtk::TreeModel::iterator& row);
  virtual bool on_script_button_idle(const Gnome::Gda::Value& primary_key_value);

  virtual void print_layout();
  virtual void print_layout_group(xmlpp::Element* node_parent, const sharedptr<const LayoutGroup>& group);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual Dialog_Layout* create_layout_dialog() const; // override.
  virtual void prepare_layout_dialog(Dialog_Layout* dialog); // override.
#endif // !GLOM_ENABLE_CLIENT_ONLY

  //Member widgets:
  mutable DbAddDel_WithButtons m_AddDel; //mutable because its get_ methods aren't const.

  bool m_has_one_or_more_records;
  bool m_read_only;

  bool m_reset_column_widths; //create_layout() sets these to 0 when this is set.
};

} //namespace Glom

#endif
