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

#include "config.h" // GLOM_ENABLE_CLIENT_ONLY

#include "box_data_portal.h"

namespace Glom
{

class Dialog_Layout_List_Related;

class Box_Data_List_Related : public Box_Data_Portal
{
public: 
  Box_Data_List_Related();

  /**
   * @param portal: The full portal details
   */
  virtual bool init_db_details(const sharedptr<const LayoutItem_Portal>& portal, bool show_title = true);

  /** Use this if no portal is yet defined, so the user can use the context menu to define a portal.
   */
  virtual bool init_db_details(const Glib::ustring& parent_table, bool show_title = true);

protected:
  virtual bool fill_from_database(); //Override.

  //Signal handlers:
  void on_adddel_record_changed();
  void on_adddel_user_requested_edit(const Gtk::TreeModel::iterator& row);
  void on_adddel_user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd);
  void on_adddel_user_reordered_columns();

  void on_adddel_script_button_clicked(const sharedptr<const LayoutItem_Button>& layout_item, const Gtk::TreeModel::iterator& row);
  bool on_script_button_idle(const sharedptr<const LayoutItem_Button>& layout_item, const Gnome::Gda::Value& primary_key);

  void on_adddel_record_added(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& primary_key_value);
    
#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_adddel_user_requested_layout();
#endif // !GLOM_ENABLE_CLIENT_ONLY
    
#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual void on_dialog_layout_hide(); //override.
#endif // !GLOM_ENABLE_CLIENT_ONLY 
  
  //Implementations of pure virtual methods from Base_DB_Table_Data:
  virtual sharedptr<Field> get_field_primary_key() const; //TODO: Already in base class?
  virtual Gnome::Gda::Value get_primary_key_value_selected() const;
  virtual void set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value);
  virtual Gnome::Gda::Value get_primary_key_value(const Gtk::TreeModel::iterator& row) const;
 
  //Overrides of functions from Box_Data:
  virtual void create_layout(); //override
  virtual Document::type_list_layout_groups create_layout_get_layout();
  void create_layout_add_group(const sharedptr<LayoutGroup>& layout_group);
    
  virtual void enable_buttons(); //override

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual Dialog_Layout* create_layout_dialog() const; // override.
  virtual void prepare_layout_dialog(Dialog_Layout* dialog); // override.
#endif // !GLOM_ENABLE_CLIENT_ONLY
    
    
  //Member widgets:
  mutable DbAddDel_WithButtons m_AddDel; //mutable because its get_ methods aren't const.
};

} //namespace Glom

#endif
