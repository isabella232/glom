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

#include "dialog_new_group.h"
#include <libglom/privs.h>

namespace Glom
{

const char* Dialog_NewGroup::glade_id("dialog_new_group");
const bool Dialog_NewGroup::glade_developer(true);

Dialog_NewGroup::Dialog_NewGroup(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_entry_name(nullptr)
{
  builder->get_widget("entry_group_name", m_entry_name);
  m_entry_name->set_max_length(Privs::MAX_ROLE_SIZE);

  //m_entry_name->signal_changed().connect( sigc::mem_fun(*this, &Dialog_NewGroup::on_entry_name_changed) );
}

} //namespace Glom
