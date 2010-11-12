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
  while(!m_list_hboxes.empty())
  {
    type_list_hboxes::iterator iter = m_list_hboxes.begin();
    Gtk::HBox* hbox = *iter;
    delete_and_forget_hbox(hbox);
  }
}

Gtk::HBox* FlowTable::get_parent_hbox(Gtk::Widget* first)
{
  typedef std::vector<Widget*> type_children;
  type_children children = get_children();
  for(type_children::iterator iter = children.begin(); iter != children.end(); ++iter)
  {
    Gtk::Widget* widget = *iter;
    if(!widget)
      continue;

    if(widget == first) //It must be a single item.
      return 0; //It has no HBox parent.
    else
    {
      Gtk::HBox* hbox = dynamic_cast<Gtk::HBox*>(widget);
      if(hbox) //The first and second widgets are inside an HBox
      {
        //Check that it is one of our special HBoxes,
        //though this check should not be neccesary:
        const type_list_hboxes::const_iterator iter_boxes =
          std::find(m_list_hboxes.begin(), m_list_hboxes.end(), hbox);
        if(iter_boxes != m_list_hboxes.end())
        {
          const type_children box_children = hbox->get_children();
          if(!box_children.empty())
          {
            const Gtk::Widget* child_widget = box_children[0]; //TODO: Is this definitely the left-most one?
            if(child_widget == first)
              return hbox;
          }
        }
      }
    }
  }

  return 0;
}

void FlowTable::delete_and_forget_hbox(Gtk::HBox* hbox)
{
  //Remove its children because the API hides the fact that they are inside the HBox.
  //Otherwise they could even be deleted by the HBox.
  typedef std::vector<Widget*> type_children;
  type_children children = hbox->get_children();
  while(!children.empty())
  {
    Gtk::Widget* widget = children[0];
    hbox->remove(*widget);
    children = hbox->get_children();
  }

  if(hbox->get_parent() == this)
  {
    Egg::SpreadTable::remove(*hbox);
  }
  else
  {
    std::cerr << G_STRFUNC << ": hbox is not a direct child." << std::endl;
  }

  //Delete and forget it:
  for(type_list_hboxes::iterator iter = m_list_hboxes.begin(); iter != m_list_hboxes.end(); ++iter)
  {
    Gtk::HBox* this_hbox = *iter;
    if(hbox == this_hbox)
    {
      delete hbox; //TODO: This causes a warning during gtk_container_remove(), though we have already removed it: sys:1: Warning: g_object_ref: assertion `object->ref_count > 0' failed
      m_list_hboxes.erase(iter);
      return;
    }
  }
}

void FlowTable::set_design_mode(bool value)
{
  m_design_mode = value;

  queue_draw(); //because this changes how the widget would be drawn.
}

void FlowTable::add(Gtk::Widget& first, Gtk::Widget& second, bool expand_second)
{
  insert(&first, &second, -1, expand_second);
}

void FlowTable::add(Gtk::Widget& first, bool expand)
{
  insert(&first, 0 /* second */, -1, expand);
}

void FlowTable::insert(Gtk::Widget* first, Gtk::Widget* second, int index, bool expand)
{
  if(first && second)
  {
    Gtk::HBox* hbox = new Gtk::HBox(false, get_horizontal_spacing());
    m_list_hboxes.push_back(hbox); //So we can delete it whenever necessary.

    hbox->pack_start(*first, Gtk::PACK_SHRINK);
    hbox->pack_start(*second, expand ? Gtk::PACK_EXPAND_WIDGET : Gtk::PACK_SHRINK);
    hbox->show();

    hbox->set_halign(Gtk::ALIGN_FILL);
    insert_child(*hbox, index);
  }
  else if(first)
  {
    first->set_halign(expand ? Gtk::ALIGN_FILL : Gtk::ALIGN_START);
    append_child(*first);
  }
  else
  {
    std::cerr << G_STRFUNC << ": first was null" << std::endl;
  }
}

int FlowTable::get_child_index(const Gtk::Widget& first) const
{
  int index = 0;

  typedef std::vector<const Widget*> type_children;
  const type_children children = get_children();
  for(type_children::const_iterator iter = children.begin(); iter != children.end(); ++iter)
  {
    const Gtk::Widget* widget = *iter;
    if(!widget)
      continue;

    if(widget == &first) //It must be a single item.
      break;
    else
    {
      const Gtk::HBox* hbox = dynamic_cast<const Gtk::HBox*>(widget);
      if(hbox) //The first and second widgets are inside an HBox
      {
        const type_children box_children = hbox->get_children();
        if(!box_children.empty())
        {
          const Gtk::Widget* child_widget = box_children[0]; //TODO: Is this definitely the left-most one?
          if(child_widget == &first)
            break;
        }
      }
    }

    ++index;
  }

  return index;
}

void FlowTable::insert_before(Gtk::Widget& first, Gtk::Widget& before, bool expand)
{
  const int index = get_child_index(before);
  insert(&first, 0 /* second */, index - 1, expand);
}

void FlowTable::insert_before(Gtk::Widget& first, Gtk::Widget& second, Gtk::Widget& before, bool expand_second)
{
  const int index = get_child_index(before);
  insert(&first, &second, index - 1, expand_second);
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

void FlowTable::remove(Gtk::Widget& first)
{
  //Handle widgets that were added to an HBox:
  Gtk::HBox* parent = get_parent_hbox(&first);
  if(parent)
  {
    delete_and_forget_hbox(parent);
    return;
  }

  Egg::SpreadTable::remove(first);
}

bool FlowTable::get_column_for_first_widget(const Gtk::Widget& first, guint& column) const
{
  //Initialize output parameter:
  column = 0;

  if(get_lines() == 0)
    return false;

  typedef std::vector<const Widget*> type_children;
  const type_children children = get_children();
  for(type_children::const_iterator iter = children.begin(); iter != children.end(); ++iter)
  {
    const Gtk::Widget* widget = *iter;
    if(!widget)
      continue;

    //Get the widget that EggSpreadTable thinks of as the child:
    const Gtk::Widget* child = 0;

    if(widget == &first) //It must be a single item.
      child = widget;
    else
    {
      const Gtk::HBox* hbox = dynamic_cast<const Gtk::HBox*>(widget);
      if(hbox) //The first and second widgets are inside an HBox
      {
        const type_children box_children = hbox->get_children();
        if(!box_children.empty())
        {
          const Gtk::Widget* child_widget = box_children[0]; //TODO: Is this definitely the left-most one?
          if(child_widget == &first)
            child = hbox;
        }
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
