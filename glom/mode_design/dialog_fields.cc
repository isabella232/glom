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

#include "dialog_fields.h"
#include "../box_db_table.h"
//#include <libgnome/gnome-i18n.h>
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>


Dialog_Fields::Dialog_Fields(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Dialog_Design(cobject, refGlade),
  m_box(0)
{
  refGlade->get_widget_derived("vbox_placeholder", m_box);

  m_label_frame->set_markup( Bakery::App_Gtk::util_bold_message(_("Field Definitions")) );


  //Fill composite view:
  add_view(m_box);

  show_all_children();
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


