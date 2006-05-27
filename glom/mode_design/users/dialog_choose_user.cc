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
 
#include "dialog_choose_user.h"

namespace Glom
{

Dialog_ChooseUser::Dialog_ChooseUser(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_combo_name(0)
{
  refGlade->get_widget_derived("combo_user_name", m_combo_name);
}

Dialog_ChooseUser::~Dialog_ChooseUser()
{
}

void Dialog_ChooseUser::set_user_list(const type_vecStrings& users)
{
  for(type_vecStrings::const_iterator iter = users.begin(); iter != users.end(); ++iter)
  {
    m_combo_name->append_text(*iter);
  }

  m_combo_name->set_first_active();
}

Glib::ustring Dialog_ChooseUser::get_user() const
{
  return m_combo_name->get_active_text();
}

} //namespace Glom
