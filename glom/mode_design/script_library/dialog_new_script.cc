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

#include "dialog_new_script.h"

namespace Glom
{

const char* Dialog_NewScript::glade_id("dialog_new_library_script");
const bool Dialog_NewScript::glade_developer(true);

Dialog_NewScript::Dialog_NewScript(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_entry_name(nullptr)
{
  builder->get_widget("entry_name", m_entry_name);

  //m_entry_name->signal_changed().connect( sigc::mem_fun(*this, &Dialog_NewScript::on_entry_name_changed) );
}

} //namespace Glom
