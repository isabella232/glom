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

#include <glom/mode_find/box_data_list_find.h>
#include <glom/utils_ui.h>
#include <glibmm/i18n.h>

namespace Glom
{

Box_Data_List_Find::Box_Data_List_Find()
: m_HBox(Gtk::ORIENTATION_HORIZONTAL, Utils::to_utype(Glom::UiUtils::DefaultSpacings::SMALL))
{
  //m_strHint = _("Enter the search criteria and click [Find]\n Glom will then change to Data mode to display the results.");

  m_HBox.pack_end(m_Button_Find, Gtk::PACK_SHRINK);
  pack_start(m_HBox, Gtk::PACK_SHRINK);

  //A signal handler is connected in the Box_Data base class.
  m_Button_Find.set_can_default();

  //Prevent the widget from trying to add or change records:
  m_AddDel.set_find_mode();

  show_all_children();
}

void Box_Data_List_Find::create_layout()
{
  Box_Data_List::create_layout();
}

bool Box_Data_List_Find::fill_from_database()
{
  BusyCursor busy_cursor(get_app_window());

  const auto result = Base_DB_Table_Data::fill_from_database();
  if(!result)
    return false;

  //Field Names:
  create_layout();

  m_FieldsShown = get_fields_to_show();

  return result;
}


Gtk::Widget* Box_Data_List_Find::get_default_button() //override
{
  return &m_Button_Find;
}

bool Box_Data_List_Find::init_db_details(const Glib::ustring& table_name, const Glib::ustring& layout_platform)
{
  FoundSet found_set;
  found_set.m_table_name = table_name;
  return Box_Data_List::init_db_details(found_set, layout_platform);
}

} //namespace Glom
