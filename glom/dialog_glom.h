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

#ifndef DIALOG_GLOM_H
#define DIALOG_GLOM_H

#include <glom/box_withbuttons.h>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>

namespace Glom
{

//TODO: Use a Window instead of a Dialog?
/** A window that can hold a Box_WithButtons.
 */
class Dialog_Glom :
  public Gtk::Dialog
{
public: 
  Dialog_Glom(Box_WithButtons* pBox, const Glib::ustring& title = Glib::ustring());
  virtual ~Dialog_Glom();

//TODO: Make this private and non-virtual?
  //Signal handlers:
  virtual void on_box_cancelled();

protected:

  //Member widgets:
  Box_WithButtons* m_pBox;
};

} //namespace Glom

#endif
