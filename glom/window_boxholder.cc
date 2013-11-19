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

  pBox->signal_cancelled.connect(sigc::mem_fun(*this, &Window_BoxHolder::on_box_cancelled));

  set_border_width(UiUtils::DEFAULT_SPACING_LARGE);
  add(*pBox);

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
