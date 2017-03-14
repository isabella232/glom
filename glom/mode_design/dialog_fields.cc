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

#include "dialog_fields.h"
//#include <libgnome/gnome-i18n.h>
#include <glom/utils_ui.h> //For bold_message()).

namespace Glom
{

const char* Dialog_Fields::glade_id("window_design");
const bool Dialog_Fields::glade_developer(true);

Dialog_Fields::Dialog_Fields(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Dialog_Design(cobject, builder),
  m_box(nullptr)
{
  builder->get_widget_derived("vbox_placeholder", m_box);

  //Fill composite view:
  add_view(m_box);
}

Dialog_Fields::~Dialog_Fields()
{
  remove_view(m_box);
}

bool Dialog_Fields::init_db_details(const Glib::ustring& table_name)
{
  if(m_box)
  {
    m_box->load_from_document();

    Dialog_Design::init_db_details(table_name);

    m_box->init_db_details(table_name);
  }

  return true;
}

} //namespace Glom

