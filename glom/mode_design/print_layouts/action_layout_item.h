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

#ifndef GLOM_PRINT_LAYOUTS_ACTION_LAYOUT_ITEM_H
#define GLOM_PRINT_LAYOUTS_ACTION_LAYOUT_ITEM_H

#include <gtkmm/action.h>

namespace Glom
{

//This derived Gtk::Action allows us to associate extra information with each action:
class Action_LayoutItem : public Gtk::Action
{
protected:
  explicit Action_LayoutItem(const Glib::ustring& name, const Gtk::StockID& stock_id = Gtk::StockID(), const Glib::ustring& label = Glib::ustring(), const Glib::ustring& tooltip = Glib::ustring());

public:
  static Glib::RefPtr<Action_LayoutItem> create(const Glib::ustring& name, const Gtk::StockID& stock_id = Gtk::StockID(), const Glib::ustring& label = Glib::ustring(), const Glib::ustring& tooltip = Glib::ustring());

  enum enumItems
  {
    ITEM_INVALID,
    ITEM_FIELD,
    ITEM_TEXT,
    ITEM_IMAGE,
    ITEM_PORTAL,
    ITEM_LINE_HORIZONTAL,
    ITEM_LINE_VERTICAL
  };

  void set_layout_item_type(enumItems type);
  enumItems get_layout_item_type() const;

protected:
  enumItems m_layout_item_type;
};

} //namespace Glom


#endif //GLOM_PRINT_LAYOUTS_ACTION_LAYOUT_ITEM_H
