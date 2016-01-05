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

#ifndef GLOM_UTILITY_WIDGETS_PLACEHOLDER_H
#define GLOM_UTILITY_WIDGETS_PLACEHOLDER_H

#include <gtkmm/box.h>
#include <gtkmm/builder.h>

namespace Glom
{

/** This is just an easy way to use a Gtk::Box as a single-item container.
 */
class PlaceHolder : public Gtk::Box
{
public:
  PlaceHolder();
  PlaceHolder(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  void add(Gtk::Widget& child) override;
  void remove();

  Gtk::Widget* get_child();
  const Gtk::Widget* get_child() const;  

private:
  Gtk::Widget* m_pChild;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_PLACEHOLDER_H
