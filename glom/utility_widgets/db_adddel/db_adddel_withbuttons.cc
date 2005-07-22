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

#include "db_adddel_withbuttons.h"
//#include <libgnome/gnome-i18n.h>

DbAddDel_WithButtons::DbAddDel_WithButtons()
: m_HBox(false, 6),
  m_Button_Add(Gtk::Stock::ADD),
  m_Button_Del(Gtk::Stock::DELETE),
  m_Button_Edit(Gtk::Stock::OPEN)
{
  m_HBox.set_spacing(6);
  //m_Button_Add.set_border_width(6);
  //m_Button_Del.set_border_width(6);
  //m_Button_Edit.set_border_width(6);

  setup_buttons();
  pack_start(m_HBox, Gtk::PACK_SHRINK);

  //Link buttons to handlers:
  m_Button_Add.signal_clicked().connect(sigc::mem_fun(*this, &DbAddDel_WithButtons::on_button_add));
  m_Button_Del.signal_clicked().connect(sigc::mem_fun(*this, &DbAddDel_WithButtons::on_button_del));
  m_Button_Edit.signal_clicked().connect(sigc::mem_fun(*this, &DbAddDel_WithButtons::on_button_edit));

  m_HBox.pack_end(m_Button_Edit, Gtk::PACK_SHRINK);
  m_HBox.pack_end(m_Button_Del, Gtk::PACK_SHRINK);
  m_HBox.pack_end(m_Button_Add, Gtk::PACK_SHRINK);

  setup_buttons();
}

DbAddDel_WithButtons::~DbAddDel_WithButtons()
{
}

void DbAddDel_WithButtons::on_button_add()
{
  if(m_auto_add)
  {
    Gtk::TreeModel::iterator iter = get_item_placeholder();
    if(iter)
    {
      guint first_visible = get_count_hidden_system_columns();
      select_item(iter, first_visible, true /* start_editing */);
    }
  }
  else
  {
    signal_user_requested_add().emit(); //Let the client code add the row explicitly, if it wants.
  }
}

void DbAddDel_WithButtons::on_button_del()
{
  on_MenuPopup_activate_Delete();
}

void DbAddDel_WithButtons::on_button_edit()
{
  on_MenuPopup_activate_Edit();
}

void DbAddDel_WithButtons::set_allow_add(bool val)
{
  DbAddDel::set_allow_add(val);

  setup_buttons();
}

void DbAddDel_WithButtons::set_allow_delete(bool val)
{
  DbAddDel::set_allow_delete(val);

  setup_buttons();
}

void DbAddDel_WithButtons::set_allow_user_actions(bool bVal)
{
  DbAddDel::set_allow_user_actions(bVal);

  setup_buttons();

  //Recreate popup menu with correct items:
  setup_menu();
}

void DbAddDel_WithButtons::setup_buttons()
{
  const bool allow_edit = get_allow_user_actions() && get_allow_view_details();
  const bool allow_del = get_allow_user_actions() && m_allow_delete;
  const bool allow_add = get_allow_user_actions() && m_allow_add;
  
  //g_warning("DbAddDel_WithButtons::setup_buttons(): allow_edit=%d", allow_edit);
  m_Button_Edit.property_visible() = allow_edit;
  m_Button_Del.property_visible() = allow_del;  
  m_Button_Add.property_visible() = allow_add;
}


