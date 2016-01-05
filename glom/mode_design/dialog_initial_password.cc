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

#include <glom/mode_design/dialog_initial_password.h>
#include <glom/frame_glom.h> //For Frame_Glom::show_ok_dialog
#include <glom/glade_utils.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_InitialPassword::glade_id("dialog_initial_password");
const bool Dialog_InitialPassword::glade_developer(true);

Dialog_InitialPassword::Dialog_InitialPassword(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  Base_DB(),
  m_entry_user(nullptr),
  m_entry_password(nullptr)
{
  builder->get_widget("entry_user", m_entry_user);
  builder->get_widget("entry_password", m_entry_password);
  builder->get_widget("entry_password_confirm", m_entry_password_confirm);
}

Glib::ustring Dialog_InitialPassword::get_user() const
{
  return m_entry_user->get_text();
}

Glib::ustring Dialog_InitialPassword::get_password() const
{
  return m_entry_password->get_text();
}

void Dialog_InitialPassword::load_from_document()
{
  auto document = get_document();
  if(document)
  {
    Glib::ustring user = document->get_connection_user(); //TODO: Offer a drop-down list of users.

    if(user.empty())
    {
      //Default to the UNIX user name, which is often the same as the Postgres user name:
      const auto pchUser = getenv("USER"); 
      if(pchUser)
        user = pchUser;
    }

    m_entry_user->set_text(user);
  }
  else
    std::cerr << G_STRFUNC << ": no document" << std::endl;

}

bool Dialog_InitialPassword::check_password()
{
  if(m_entry_user->get_text().empty())
  {
    Frame_Glom::show_ok_dialog(_("Username Is Empty"), _("Please enter a login name for the new user."), *this, Gtk::MESSAGE_ERROR);
    return false;
  }
  else if(m_entry_password->get_text() != m_entry_password_confirm->get_text())
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
