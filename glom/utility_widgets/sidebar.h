/* Glom
 *
 * Copyright (C) 2007 Johannes Schmid <johannes.schmid@openismus.com>
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

#ifndef GLOM_UTILITY_WIDGETS_SIDEBAR_H
#define GLOM_UTILITY_WIDGETS_SIDEBAR_H

#include <gtkmm/window.h>
#include <gtkmm/handlebox.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>

namespace Glom
{

class SideBar : public Gtk::HandleBox
{
public:
  SideBar();
  ~SideBar();
    
  void add_button(Gtk::Button& button);
  void remove_button(Gtk::Button& button);
  
private:
  Gtk::VBox m_box;
};

} //namespace Glom

#endif // GLOM_UTILITY_WIDGETS_SIDEBAR_H
