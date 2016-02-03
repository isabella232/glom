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

#include "dialog_design.h"
#include "../box_db_table.h"
#include <glom/appwindow.h>
//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_Design::Dialog_Design(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Window(cobject),
  m_label_table(nullptr)
{
  Gtk::Button* button_close = nullptr;
  builder->get_widget("button_close",  button_close);
  button_close->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Design::on_button_close) );

  builder->get_widget("label_table_name", m_label_table);

  set_modal(); //We don't want people to edit the main window while we are changing structure.

  show_all_children();
}

bool Dialog_Design::init_db_details(const Glib::ustring& table_name)
{
  if(get_document())
  {
    Glib::ustring table_label = _("None selected");

    //Show the table title (if any) and name:
     auto document = std::dynamic_pointer_cast<Document>(get_document());
     if(document)
     {
       Glib::ustring table_title = document->get_table_title(table_name, AppWindow::get_current_locale());
       if(table_title.empty())
         table_label = table_name;
       else
         table_label = table_title + " (" + table_name + ')';
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
