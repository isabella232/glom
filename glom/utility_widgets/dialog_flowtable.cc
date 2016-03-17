/* Glom
 *
 * Copyright (C) 2008 Johannes Schmid
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


#include "dialog_flowtable.h"
#include <glom/appwindow.h>

namespace Glom
{

const char* Dialog_FlowTable::glade_id("dialog_flowtable");
const bool Dialog_FlowTable::glade_developer(true);
		
Dialog_FlowTable::Dialog_FlowTable(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_entry_title(nullptr),
  m_spin_columns(nullptr),
  m_flowtable(nullptr)
{
  builder->get_widget("entry_title",  m_entry_title);
  builder->get_widget("spin_columns",  m_spin_columns);
  
  //Set the adjustment details, to avoid a useless 0-to-0 range and a 0 incremenet.
  //We don't do this the Glade file because GtkBuilder wouldn't find the
  //associated adjustment object unless we specified it explictly:
  //See http://bugzilla.gnome.org/show_bug.cgi?id=575714
  m_spin_columns->set_range(0, 10);
  m_spin_columns->set_increments(1, 2);
  m_spin_columns->set_value(3); //A sensible default.

  show_all_children();
}

void Dialog_FlowTable::set_flowtable(FlowTableWithFields* flowtable)
{
  m_flowtable = flowtable;
  m_layoutgroup = std::dynamic_pointer_cast<LayoutGroup>(flowtable->get_layout_item());
  m_entry_title->set_text(item_get_title(m_layoutgroup));
  m_spin_columns->set_value(m_layoutgroup->get_columns_count());
}

Glib::ustring Dialog_FlowTable::get_title()
{
  return m_entry_title->get_text();
}

gint Dialog_FlowTable::get_columns_count() const
{
  return m_spin_columns->get_value_as_int();
}

} //namespace Glom





