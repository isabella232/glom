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

#include "dialog_user.h"
#include <glom/frame_glom.h> //For Frame_Glom::show_ok_dialog().
#include <libglom/privs.h>
//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_User::glade_id("dialog_user");
const bool Dialog_User::glade_developer(true);

Dialog_User::Dialog_User(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject)
{
  //builder->get_widget("label_table_name", m_label_table_name);

  builder->get_widget("entry_user", m_entry_user);
  m_entry_user->set_max_length(Privs::MAX_ROLE_SIZE);
  builder->get_widget_derived("combobox_group", m_combo_group);

  builder->get_widget("entry_password", m_entry_password);
  m_entry_password->set_max_length(Privs::MAX_ROLE_SIZE); //Let's assume that this has a similar (undocumented in PostgreSQL) max size as the user.
  builder->get_widget("entry_password_confirm", m_entry_password_confirm);
  m_entry_password_confirm->set_max_length(Privs::MAX_ROLE_SIZE);
}

bool Dialog_User::check_password()
{
  if(m_entry_password->get_text() != m_entry_password_confirm->get_text())
  {
    Frame_Glom::show_ok_dialog(_("Passwords Do Not Match"), _("The entered password does not match the entered password confirmation. Please try again."), *this, Gtk::MESSAGE_ERROR);
    return false;
  }
  else if(m_entry_password->get_text().empty())
  {
    Frame_Glom::show_ok_dialog(_("Password Is Empty"), _("Please enter a password for this user."), *this, Gtk::MESSAGE_ERROR);
    return false;
  }
  else
    return true;
}

} //namespace Glom











