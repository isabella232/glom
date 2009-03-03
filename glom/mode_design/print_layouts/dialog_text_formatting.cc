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

#include "dialog_text_formatting.h"
#include <glom/box_db_table.h>
#include <libglom/glade_utils.h>
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_TextFormatting::Dialog_TextFormatting(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Window(cobject),
  m_box_formatting_placeholder(0),
  m_box_formatting(0)
{
  Gtk::Button* button_close = 0;
  refGlade->get_widget("button_close",  button_close);
  button_close->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_TextFormatting::on_button_close) );

  //Formatting:
  //Get the place to put the Formatting stuff:
  refGlade->get_widget("box_formatting_placeholder", m_box_formatting_placeholder);
  
  //Get the formatting stuff:
  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXmlFormatting = Gnome::Glade::Xml::create(Utils::get_glade_file_path("glom_developer.glade"), "box_formatting");
    refXmlFormatting->get_widget_derived("box_formatting", m_box_formatting);
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  m_box_formatting_placeholder->pack_start(*m_box_formatting);
  add_view(m_box_formatting);
  m_box_formatting->set_is_for_print_layout();

  set_modal(); //We don't want people to edit the main window while we are changing structure.

  show_all_children();
}

Dialog_TextFormatting::~Dialog_TextFormatting()
{
  remove_view(m_box_formatting);
}

void Dialog_TextFormatting::on_button_close()
{
  hide();
}

} //namespace Glom
