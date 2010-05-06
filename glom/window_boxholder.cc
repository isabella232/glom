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
 
#include <glom/window_boxholder.h>
#include <glom/utils_ui.h>

namespace Glom
{

Window_BoxHolder::Window_BoxHolder(Box_WithButtons* pBox, const Glib::ustring& title)
{
  g_assert(pBox);

  if(!title.empty())
    set_title(title);

  //Set default position:
  set_position(Gtk::WIN_POS_CENTER_ON_PARENT);

#ifndef GLOM_ENABLE_MAEMO

#else
  //Maemo has wide border margins:
  set_border_width(HILDON_MARGIN_DOUBLE);
#endif

  pBox->signal_cancelled.connect(sigc::mem_fun(*this, &Window_BoxHolder::on_box_cancelled));

  #ifndef GLOM_ENABLE_MAEMO
  set_border_width(Utils::DEFAULT_SPACING_SMALL);
  add(*pBox);
  #else
  //Maemo has wide borders, but not on the right-hand side when there is a scrollbar:
  set_border_width(0);
  add(m_alignment);
  m_alignment.set_padding(HILDON_MARGIN_DOUBLE, HILDON_MARGIN_DOUBLE, HILDON_MARGIN_DOUBLE, 0);
  m_alignment.show();
  m_alignment.add(*pBox);
  #endif

  pBox->show();

  //Set the default button, if there is one:
  Gtk::Widget* default_button = pBox->get_default_button();
  if(default_button)
    set_default(*default_button);
}

Window_BoxHolder::~Window_BoxHolder()
{
}

void Window_BoxHolder::on_box_cancelled()
{
  hide();
}

} //namespace Glom
