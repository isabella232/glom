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

#ifndef GLOM_WINDOW_BOXHOLDER_H
#define GLOM_WINDOW_BOXHOLDER_H

#include <glom/box_withbuttons.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>

#ifdef GLOM_ENABLE_MAEMO
#include <hildonmm/stackable-window.h>
#endif

namespace Glom
{

/** A window that can hold a Box_WithButtons.
 */
class Window_BoxHolder :
#ifndef GLOM_ENABLE_MAEMO
  public Gtk::Window
#else
  public Hildon::StackableWindow
#endif
{
public: 
  Window_BoxHolder(Box_WithButtons* pBox, const Glib::ustring& title = Glib::ustring());
  virtual ~Window_BoxHolder();

private:

  //Signal handlers:
  void on_box_cancelled();
};

} //namespace Glom

#endif
