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

#include "flowtable.h"
#include "layoutwidgetbase.h"
#include <iostream>
#include <gdkmm/window.h>
#include <glom/utils_ui.h>

namespace Glom
{

FlowTable::FlowTable()
:
  m_design_mode(false)
{
}

FlowTable::~FlowTable()
{
}

void FlowTable::set_design_mode(bool value)
{
  m_design_mode = value;

  queue_draw(); //because this changes how the widget would be drawn.
}

void FlowTable::add(Gtk::Widget& first, Gtk::Widget& second, bool expand_second)
{
  Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, get_horizontal_spacing()));

  hbox->pack_start(first, Gtk::PACK_SHRINK);
  hbox->pack_start(second, expand_second ? Gtk::PACK_EXPAND_WIDGET : Gtk::PACK_SHRINK);
  hbox->show();

  hbox->set_halign(Gtk::ALIGN_FILL);
  append_child(*hbox);
}

void FlowTable::add(Gtk::Widget& first, bool expand)
{
  first.set_halign(expand ? Gtk::ALIGN_FILL : Gtk::ALIGN_START);
  append_child(first);
}

void FlowTable::insert_before(Gtk::Widget& /* first */, Gtk::Widget& /* before */, bool /* expand */)
{
  std::cerr << G_STRFUNC << ": Unimplemented." << std::endl;
  //FlowTableItem item(&first, this);
  //TODO: insert_before(item, before, expand);
}

void FlowTable::insert_before(Gtk::Widget& /* first */, Gtk::Widget& /* second */, Gtk::Widget& /* before */, bool /* expand_second */)
{
  std::cerr << G_STRFUNC << ": Unimplemented." << std::endl;
  //FlowTableItem item(&first, &second, this);
  //TODO: insert_before(item, before, expand_second);
}

void FlowTable::remove_all()
{
  typedef std::vector<Widget*> type_children;
  type_children children = get_children();
  while(!children.empty())
  {
    Gtk::Widget* widget = children[0];
    remove(*widget);
    children = get_children();
  }
}

bool FlowTable::get_column_for_first_widget(const Gtk::Widget& first, guint& column) const
{
  //Initialize output parameter:
  column = 0;

  if(get_lines() == 0)
    return false;

  typedef std::vector<const Widget*> type_children;
  type_children children = get_children();
  for(type_children::const_iterator iter = children.begin(); iter != children.end(); ++iter)
  {
    const Gtk::Widget* widget = *iter;
    if(!widget)
      continue;

    const Gtk::Widget* child = 0;

    if(widget == &first) //It must be a single item.
      child = widget;
    else
    {
      const Gtk::HBox* hbox = dynamic_cast<const Gtk::HBox*>(widget);
      if(hbox) //The first and second widgets are inside an HBox
      {
        type_children box_children = hbox->get_children();
        if(!box_children.empty())
          child = box_children[0]; //TODO: Is this definitely the left-most one?
      }

      if(child)
      {
        int width_min = 0;
        int width_natural = 0;
        child->get_preferred_width(width_min, width_natural);
        column = get_child_line(*child, width_natural);

        return true;
      }
    }
  }

  return false;
}

} //namespace Glom
