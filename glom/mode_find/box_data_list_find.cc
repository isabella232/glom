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

#include "box_data_list_find.h"
#include <libintl.h>

Box_Data_List_Find::Box_Data_List_Find()
: m_HBox(false, 6)
{
  m_strHint = gettext("Enter the search criteria and click [Find]\n Glom will then change to Data mode to display the results.");

  m_HBox.pack_end(m_Button_Find, Gtk::PACK_EXPAND_WIDGET);
  pack_start(m_HBox, Gtk::PACK_EXPAND_WIDGET);

  show_all();
}

Box_Data_List_Find::~Box_Data_List_Find()
{
}


void Box_Data_List_Find::fill_from_database()
{
  Bakery::BusyCursor(*get_app_window());

  Box_DB_Table::fill_from_database();

  m_AddDel.remove_all();

  //Field Names:
  fill_column_titles();

  m_AddDel.add_item("find");
}
