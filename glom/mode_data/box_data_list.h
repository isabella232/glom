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

#ifndef GLOM_MODE_DATA_BOX_DATA_LIST_H
#define GLOM_MODE_DATA_BOX_DATA_LIST_H

#include "config.h" // GLOM_ENABLE_CLIENT_ONLY

#include "box_data_manyrecords.h"
#include <glom/mode_data/db_adddel/db_adddel_withbuttons.h>

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
  Gnome::Gda::Value get_primary_key_value_selected() const override;
  void set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value) override;

  Gnome::Gda::Value get_entered_field_data(const std::shared_ptr<const LayoutItem_Field>& field) const override;
  void set_entered_field_data(const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value) override;
  void set_entered_field_data(const Gtk::TreeModel::iterator& row, const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value) override;

  Gtk::TreeModel::iterator get_row_selected() override;

  bool get_showing_multiple_records() const;

  void set_read_only(bool read_only = true);

  //For instance, change "Open" to "Select" when used to select an ID.
  void set_open_button_title(const Glib::ustring& title);

  ///Highlight and scroll to the specified record, with primary key value @primary_key_value.
  void set_primary_key_value_selected(const Gnome::Gda::Value& primary_key_value) override;


  //Signal Handlers:
  void on_details_nav_first();
  void on_details_nav_previous();
  void on_details_nav_next();
  void on_details_nav_last();
  void on_details_record_deleted(const Gnome::Gda::Value& primary_key_value);

  void get_record_counts(gulong& total, gulong& found) const;

  #ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_dialog_layout_hide() override;
  #endif //GLOM_ENABLE_CLIENT_ONLY


protected:

  //Implementations of pure virtual methods from Base_DB_Table_Data:
  Gnome::Gda::Value get_primary_key_value(const Gtk::TreeModel::iterator& row) const override;

  //Overrides of functions from Box_Data:
  void create_layout() override;
  Document::type_list_layout_groups create_layout_get_layout();

  bool fill_from_database() override;
  virtual void enable_buttons();

  std::shared_ptr<Field> get_field_primary_key() const override;

  //Signal handlers:
  void on_adddel_user_requested_edit(const Gtk::TreeModel::iterator& row);
  void on_adddel_user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd);
  void on_adddel_user_sort_clause_changed();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_adddel_user_requested_layout();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  void on_adddel_script_button_clicked(const std::shared_ptr<const LayoutItem_Button>& layout_item, const Gtk::TreeModel::iterator& row);
  bool on_script_button_idle(const std::shared_ptr<const LayoutItem_Button>& layout_item, const Gnome::Gda::Value& primary_key);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  Dialog_Layout* create_layout_dialog() const override;
  void prepare_layout_dialog(Dialog_Layout* dialog) override;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  //Member widgets:
  mutable DbAddDel_WithButtons m_AddDel; //mutable because its get_ methods aren't const.

  bool m_read_only;
};

} //namespace Glom

#endif // GLOM_MODE_DATA_BOX_DATA_LIST_H
