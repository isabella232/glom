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

#ifndef GLOM_DB_ADDDEL_WITHBUTTONS_H
#define GLOM_DB_ADDDEL_WITHBUTTONS_H

#include "db_adddel.h"
#include <gtkmm/buttonbox.h>

namespace Glom
{

class DbAddDel_WithButtons : public DbAddDel
{
public: 
  DbAddDel_WithButtons();

  void set_allow_add(bool val = true) override;
  void set_allow_delete(bool val = true) override;
  void set_allow_user_actions(bool value = true) override;

  ///Whether each row should have a button, to request edit.
  void set_allow_view_details(bool val = true) override;

private:
  void setup_buttons();
  
  void on_button_add();
  void on_button_del();
  void on_button_edit();
  void on_selection_changed(bool selection) override;

  void show_all_vfunc() override;

  //member widgets:
  Gtk::ButtonBox m_ButtonBox;
  
  typedef Gtk::Button type_button;

  type_button m_Button_Del;
  type_button m_Button_Edit;

  type_button m_Button_Add;
};

} //namespace Glom

#endif //GLOM_DB_ADDDEL_WITHBUTTONS_H
