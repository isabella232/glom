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

#include "dialog_design.h"
#include "../box_db_table.h"
//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_Design::Dialog_Design(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Window(cobject),
  m_label_table(0),
  m_label_frame(0)
{
  Gtk::Button* button_close = 0;
  refGlade->get_widget("button_close",  button_close);
  button_close->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Design::on_button_close) );

  refGlade->get_widget("label_table_name", m_label_table);
  refGlade->get_widget("label_frame_title", m_label_frame);

  set_modal(); //We don't want people to edit the main window while we are changing structure.

  show_all_children();
}

Dialog_Design::~Dialog_Design()
{
}

bool Dialog_Design::init_db_details(const Glib::ustring& table_name)
{
  if(get_document())
  {
    Glib::ustring table_label = _("None selected");

    //Show the table title (if any) and name:
     Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
     if(document)
     {
       Glib::ustring table_title = document->get_table_title(table_name);
       if(table_title.empty())
         table_label = table_name;
       else
         table_label = table_title + " (" + table_name + ")";
     }

    m_label_table->set_text(table_label);
  }
  else
  {
    g_warning("no document");
  }

  return true;
}

void Dialog_Design::on_button_close()
{
  hide();
}

} //namespace Glom
