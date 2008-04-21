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

protected:
  virtual bool fill_from_database(); //Override.
  virtual type_vecLayoutFields get_fields_to_show() const; //override

  virtual void on_adddel_user_requested_add();
  virtual void on_adddel_user_changed(const Gtk::TreeModel::iterator& row, guint col);
  virtual void on_adddel_user_added(const Gtk::TreeModel::iterator& row, guint col_with_first_value); //Override.
  virtual void on_record_added(const Gnome::Gda::Value& primary_key_value, const Gtk::TreeModel::iterator& row); //Override. Not a signal handler.
  virtual void on_record_deleted(const Gnome::Gda::Value& primary_key_value); //override.

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual void on_dialog_layout_hide(); //override.
#endif // !GLOM_ENABLE_CLIENT_ONLY

  virtual void enable_buttons(); //override

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual Dialog_Layout* create_layout_dialog() const; // override.
  virtual void prepare_layout_dialog(Dialog_Layout* dialog); // override.
#endif // !GLOM_ENABLE_CLIENT_ONLY
};

} //namespace Glom

#endif
