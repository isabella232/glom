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

#ifndef GLOM_DB_ADDDEL_WITHBUTTONS_H
#define GLOM_DB_ADDDEL_WITHBUTTONS_H

#include "db_adddel.h"
#include <gtkmm/buttonbox.h>

#ifdef GLOM_ENABLE_MAEMO
#include <hildonmm/button.h>
#endif

namespace Glom
{

class DbAddDel_WithButtons : public DbAddDel
{
public: 
  DbAddDel_WithButtons();
  virtual ~DbAddDel_WithButtons();

  virtual void set_allow_add(bool val = true); //override
  virtual void set_allow_delete(bool val = true); //override
  virtual void set_allow_user_actions(bool bVal = true); //override

  ///Whether each row should have a button, to request edit.
  virtual void set_allow_view_details(bool val = true);

private:
  void setup_buttons();
  
  #ifndef GLOM_ENABLE_MAEMO
  void on_button_add();
  void on_button_del();
  void on_button_edit();
  #endif //GLOM_ENABLE_MAEMO

  virtual void on_selection_changed(bool selection);

  virtual void show_all_vfunc();

  //member widgets:
  Gtk::HButtonBox m_ButtonBox;
  
#ifndef GLOM_ENABLE_MAEMO
  typedef Gtk::Button type_button;

  //We don't need these buttons on Maemo because the user can 
  //- click to got to details.
  //- go to the details and then del.
  type_button m_Button_Del;
  type_button m_Button_Edit;
#else
  typedef Hildon::Button type_button;
#endif

  type_button m_Button_Add;
};

} //namespace Glom

#endif //GLOM_DB_ADDDEL_WITHBUTTONS_H
