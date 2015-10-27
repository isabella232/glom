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

#ifndef GLOM_BOX_DATA_LIST_FIND_H
#define GLOM_BOX_DATA_LIST_FIND_H

#include "../mode_data/box_data_list.h"

namespace Glom
{

class Box_Data_List_Find : public Box_Data_List
{
public: 
  Box_Data_List_Find();
  virtual ~Box_Data_List_Find();

  virtual bool init_db_details(const Glib::ustring& table_name, const Glib::ustring& layout_platform);

  virtual Gtk::Widget* get_default_button() override;

private:

  bool fill_from_database() override;
  void create_layout() override;

  //Member widgets:
  Gtk::Box m_HBox;
};

} //namespace Glom

#endif // GLOM_BOX_DATA_LIST_FIND_H
