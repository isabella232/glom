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

#ifndef GLOM_UTILITY_WIDGETS_PLACEHOLDER_H
#define GLOM_UTILITY_WIDGETS_PLACEHOLDER_H

#include <gtkmm.h>
#include <libglademm.h>

/**
  *@author Murray Cumming
  */


class PlaceHolder : public Gtk::VBox
{
public:
  PlaceHolder();
  PlaceHolder(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~PlaceHolder();

  virtual void add(Gtk::Widget& child);
  virtual void remove();

  virtual Gtk::Widget* get_child();
  virtual const Gtk::Widget* get_child() const;  

protected:
  Gtk::Widget* m_pChild;
};

#endif //GLOM_UTILITY_WIDGETS_PLACEHOLDER_H
