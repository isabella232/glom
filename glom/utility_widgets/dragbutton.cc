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
 
#include "dragbutton.h"

namespace Glom
{
  
DragButton::DragButton(Gtk::Image& image, const Glib::ustring& id)
{
	set_image (image);
  m_id = id;
  std::list<Gtk::TargetEntry> targetentries;
  targetentries.push_back(Gtk::TargetEntry(get_target()));
  drag_source_set(targetentries);
}

DragButton::~DragButton()
{

}

void DragButton::on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>&, 
                                  Gtk::SelectionData& selection_data, guint, guint)
{
  selection_data.set(get_target(), m_id);
}

void DragButton::on_drag_begin(const Glib::RefPtr<Gdk::DragContext>& drag_context)
{
	drag_context->set_icon (dynamic_cast<Gtk::Image*>(get_image())->get_pixbuf(), 0, 0);
}

} // namespace Glom
