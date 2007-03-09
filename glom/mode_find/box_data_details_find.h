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

#ifndef BOX_DATA_DETAILS_FIND_H
#define BOX_DATA_DETAILS_FIND_H

#include "../mode_data/box_data_details.h"

namespace Glom
{

class Box_Data_Details_Find : public Box_Data_Details
{
public: 
  Box_Data_Details_Find();
  virtual ~Box_Data_Details_Find();

  virtual bool init_db_details(const Glib::ustring& table_name);

  virtual Gtk::Widget* get_default_button(); //override

protected:

  virtual bool fill_from_database(); //override.
  virtual void fill_related(); //override.

  virtual void on_flowtable_field_edited(const sharedptr<const LayoutItem_Field>& id, const Gnome::Gda::Value& value);

};

} //namespace Glom

#endif
