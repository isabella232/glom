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

#include "placeholder.h"

namespace Glom
{

PlaceHolder::PlaceHolder()
: Gtk::Box(Gtk::ORIENTATION_VERTICAL),
  m_pChild(nullptr)
{
}

PlaceHolder::PlaceHolder(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& /*builder*/)
: Gtk::Box(cobject),
  m_pChild(nullptr)
{
}

void PlaceHolder::add(Gtk::Widget& child)
{
  remove();
  pack_start(child);
  m_pChild = &child;
}

void PlaceHolder::remove()
{
  if(m_pChild)
  {
    Gtk::Box::remove(*m_pChild);
    m_pChild = nullptr;
  }
}


Gtk::Widget *PlaceHolder::get_child()
{
  return m_pChild;
}

const Gtk::Widget* PlaceHolder::get_child() const
{
  return m_pChild;
}

} //namespace Glom

