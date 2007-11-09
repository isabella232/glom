/* Glom
 *
 * Copyright (C) 2007 Murray Cumming
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

#include "action_layout_item.h"

namespace Glom
{

Action_LayoutItem::Action_LayoutItem(const Glib::ustring& name, const Gtk::StockID& stock_id, const Glib::ustring& label, const Glib::ustring& tooltip)
: Gtk::Action(name, stock_id, label, tooltip)
{
}

Glib::RefPtr<Action_LayoutItem> Action_LayoutItem::create(const Glib::ustring& name, const Gtk::StockID& stock_id, const Glib::ustring& label, const Glib::ustring& tooltip)
{
  return Glib::RefPtr<Action_LayoutItem>(new Action_LayoutItem(name, stock_id, label, tooltip));
}

void Action_LayoutItem::set_layout_item_type(enumItems type)
{
  m_layout_item_type = type;
}

Action_LayoutItem::enumItems Action_LayoutItem::get_layout_item_type() const
{
  return m_layout_item_type;
}

} //namespace Glom


