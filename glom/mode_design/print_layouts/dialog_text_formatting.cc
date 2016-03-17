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

#include "dialog_text_formatting.h"
#include <glom/glade_utils.h>

namespace Glom
{

const char* Dialog_TextFormatting::glade_id("window_text_format");
const bool Dialog_TextFormatting::glade_developer(true);

Dialog_TextFormatting::Dialog_TextFormatting(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Window(cobject),
  m_box_formatting_placeholder(nullptr),
  m_box_formatting(nullptr)
{
  Gtk::Button* button_close = nullptr;
  builder->get_widget("button_close",  button_close);
  if(button_close)
  {
    button_close->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_TextFormatting::on_button_close) );
  }

  //Formatting:
  //Get the place to put the Formatting stuff:
  builder->get_widget("box_formatting_placeholder", m_box_formatting_placeholder);
 
  Utils::box_pack_start_glade_child_widget_derived_with_warning(m_box_formatting_placeholder, m_box_formatting);
  if(m_box_formatting)
  {
    add_view(m_box_formatting);
    m_box_formatting->set_is_for_non_editable();
  }

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
