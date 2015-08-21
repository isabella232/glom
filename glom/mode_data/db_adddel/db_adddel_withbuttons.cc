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

#include "db_adddel_withbuttons.h"
#include <glom/utils_ui.h>
#include <gtkmm/buttonbox.h>
#include <glibmm/i18n.h>

namespace Glom
{

DbAddDel_WithButtons::DbAddDel_WithButtons()
: m_ButtonBox(Gtk::ORIENTATION_HORIZONTAL),
  m_Button_Del(_("_Delete"), true),
  m_Button_Edit(_("_Open"), true),
  m_Button_Add(_("_Add"), true)
{
  m_ButtonBox.set_layout(Gtk::BUTTONBOX_END);
  m_ButtonBox.set_spacing(static_cast<int>(UiUtils::DefaultSpacings::SMALL));

  setup_buttons();
  pack_start(m_ButtonBox, Gtk::PACK_SHRINK);

  //Link buttons to handlers:

  m_Button_Add.signal_clicked().connect(sigc::mem_fun(*this, &DbAddDel_WithButtons::on_button_add));
  m_ButtonBox.pack_end(m_Button_Add, Gtk::PACK_SHRINK);

  m_Button_Del.signal_clicked().connect(sigc::mem_fun(*this, &DbAddDel_WithButtons::on_button_del));
  m_Button_Edit.signal_clicked().connect(sigc::mem_fun(*this, &DbAddDel_WithButtons::on_button_edit));

  m_ButtonBox.pack_end(m_Button_Del, Gtk::PACK_SHRINK);
  m_ButtonBox.pack_end(m_Button_Edit, Gtk::PACK_SHRINK);

  setup_buttons();
}

DbAddDel_WithButtons::~DbAddDel_WithButtons()
{
}

void DbAddDel_WithButtons::on_button_add()
{
  on_MenuPopup_activate_Add();
}

void DbAddDel_WithButtons::on_button_del()
{
  on_MenuPopup_activate_Delete();
}

void DbAddDel_WithButtons::on_button_edit()
{
  do_user_requested_edit();
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
  setup_menu(this);
}

void DbAddDel_WithButtons::setup_buttons()
{
  const auto allow_edit = get_allow_user_actions() && get_allow_view_details();
  const auto allow_del = get_allow_user_actions() && m_allow_delete;
  const auto allow_add = get_allow_user_actions() && m_allow_add;

  m_Button_Add.show();
  m_Button_Add.set_property("visible", allow_add);

  m_Button_Edit.show();
  m_Button_Edit.set_property("visible", allow_edit);

  if(!m_open_button_title.empty())
    m_Button_Edit.set_label(m_open_button_title);

  m_Button_Del.show();
  m_Button_Del.set_property("visible", allow_del);

  m_ButtonBox.show();
}

void DbAddDel_WithButtons::show_all_vfunc()
{
  //Call the base class:
  Gtk::Box::show_all_vfunc();

  //Hide some stuff:
  setup_buttons();
}

void DbAddDel_WithButtons::set_allow_view_details(bool val)
{
  DbAddDel::set_allow_view_details(val);

  setup_buttons();
}

void DbAddDel_WithButtons::on_selection_changed(bool selection)
{
  m_Button_Edit.set_sensitive(selection);
  m_Button_Del.set_sensitive(selection);
  
  DbAddDel::on_selection_changed(selection);
}

} //namespace Glom
