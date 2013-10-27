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

#include "flowtable.h"
#include "layoutwidgetbase.h"
#include <iostream>
#include <gtkmm/eventbox.h>
#include <gdkmm/window.h>
#include <glom/utils_ui.h>

namespace Glom
{

FlowTable::FlowTable()
: m_design_mode(false)
{
  //Default to disabling drag and drop:
  set_drag_enabled(EGG_DRAG_DISABLED);
  set_drop_enabled(false);
}

FlowTable::~FlowTable()
{
  while(!m_list_hboxes.empty())
  {
    type_list_hboxes::iterator iter = m_list_hboxes.begin();
    Gtk::Box* hbox = *iter;
    delete_and_forget_hbox(hbox);
  }
}

const Gtk::Box* FlowTable::get_parent_hbox(const Gtk::Widget* first) const
{
  const type_const_list_widgets::const_iterator iter_find = 
    std::find(m_list_first_widgets.begin(), m_list_first_widgets.end(), first);
  if(iter_find == m_list_first_widgets.end())
  {
    std::cerr << G_STRFUNC << ": first was not a first widget. first=" << first << std::endl;
    return 0; //It has no Box parent because it is not even a first widget.
  }
  
  for(type_list_hboxes::const_iterator iter = m_list_hboxes.begin(); iter != m_list_hboxes.end(); ++iter)
  {
    const Gtk::Box* hbox = *iter;
    if(!hbox)
      continue;

    //Check if it has the widget as one of its children:
    typedef std::vector<const Gtk::Widget*> type_children;
    const type_children box_children = hbox->get_children();
    if(box_children.empty())
      continue;

    const type_children::const_iterator iter_find = 
      std::find(box_children.begin(), box_children.end(), first);
    if(iter_find != box_children.end())
      return hbox;
  }
  return 0;
}

void FlowTable::delete_and_forget_hbox(Gtk::Box* hbox)
{
  //Remove its children because the API hides the fact that they are inside the Box.
  //Otherwise they could even be deleted by the Box.
  typedef std::vector<Widget*> type_children;
  type_children children = hbox->get_children();
  while(!children.empty())
  {
    Gtk::Widget* widget = children[0];
    hbox->remove(*widget);
    children = hbox->get_children();
  }

  //This check does not work because EggSpreadTableDnD adds an intermediate GtkEventBox:
  //if(hbox->get_parent() == this)
  
  //Check that it is in our list of hboxes:
  const type_list_hboxes::iterator iter = std::find(
    m_list_hboxes.begin(), m_list_hboxes.end(), hbox);
  if(iter == m_list_hboxes.end())
  {
    std::cerr << G_STRFUNC << ": hbox=" << hbox << " is not in our list of hboxes." << std::endl;
    return;
  }
  
  //Check that it has a parent,
  //as a sanity check:
  Gtk::Widget* parent= hbox->get_parent();
  if(parent)
  {
    Egg::SpreadTableDnd::remove_child(*hbox);
  }
  else
  {
    //TODO: Fix this leak.
    //std::cerr << G_STRFUNC << ": hbox=" << hbox << " has no parent. Not removing from SpreadTableDnd" << std::endl;
  }

  //Delete and forget it:
  delete hbox; //TODO: This causes a warning during gtk_container_remove(), though we have already removed it: sys:1: Warning: g_object_ref: assertion `object->ref_count > 0' failed
  m_list_hboxes.erase(iter);
}

void FlowTable::set_design_mode(bool value)
{
  m_design_mode = value;

  queue_draw(); //because this changes how the widget would be drawn.
}

void FlowTable::add_widgets(Gtk::Widget& first, Gtk::Widget& second, bool expand_second)
{
  insert(&first, &second, -1, expand_second);
}

void FlowTable::add_widgets(Gtk::Widget& first, bool expand)
{
  insert(&first, 0 /* second */, -1, expand);
}

void FlowTable::insert(Gtk::Widget* first, Gtk::Widget* second, int index, bool expand)
{
  if(first && second)
  {
    Gtk::Box* hbox = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, get_horizontal_spacing());
    m_list_hboxes.push_back(hbox); //So we can delete it whenever necessary.

    hbox->pack_start(*first, Gtk::PACK_SHRINK);
    hbox->pack_start(*second, expand ? Gtk::PACK_EXPAND_WIDGET : Gtk::PACK_SHRINK);
    hbox->show();

    hbox->set_halign(Gtk::ALIGN_FILL);
    Egg::SpreadTableDnd::insert_child(*hbox, index);
    //std::cout << "DEBUG: inserted hbox=" << hbox << " for first=" << first << std::endl;

    m_list_first_widgets.push_back(first);
  }
  else if(first)
  {
    first->set_halign(expand ? Gtk::ALIGN_FILL : Gtk::ALIGN_START);
    Egg::SpreadTableDnd::append_child(*first);
    //std::cout << "DEBUG: inserted first=" << first << std::endl;
    m_list_first_widgets.push_back(first);
  }
  else
  {
    std::cerr << G_STRFUNC << ": first was null" << std::endl;
  }
}

void FlowTable::remove_all()
{
  for(type_const_list_widgets::const_iterator iter = m_list_first_widgets.begin(); iter != m_list_first_widgets.end(); ++iter)
  {
    Gtk::Widget* first_widget = const_cast<Gtk::Widget*>(*iter);
    
    if(first_widget)
      remove(*first_widget);
  }
  m_list_first_widgets.clear();

  //We can't use get_children() because EggSpreadTableDnd does not allow that,
  //because it handles children differently via its specific API.
  /*
  typedef std::vector<Widget*> type_children;
  const type_children children = get_children();
  for(type_children::iterator iter = children.begin(); iter != children.end(); ++iter)
  {
    Gtk::Widget* widget = *iter;
    remove(*widget);
  }
  */
}

void FlowTable::remove(Gtk::Widget& first)
{
  //std::cout << G_STRFUNC << ": debug: remove() first=" << &first << std::endl;
  
  //Handle widgets that were added to an Box:
  Gtk::Box* parent = const_cast<Gtk::Box*>(get_parent_hbox(&first));
  if(parent)
  {
    //std::cout << "  debug: hbox=" << parent << std::endl;
 
    delete_and_forget_hbox(parent);
    return;
  }

  Egg::SpreadTableDnd::remove_child(first);
}

FlowTable::type_const_list_widgets FlowTable::get_first_child_widgets() const
{
  return m_list_first_widgets;
}
  
bool FlowTable::get_column_for_first_widget(const Gtk::Widget& first, guint& column) const
{
  //Initialize output parameter:
  column = 0;

  if(get_lines() == 0)
    return false;

  //Discover actual child widget that was added to the EggSpreadTable,
  //so we can use it again to call EggSpreadTable::get_child_line():
  const Gtk::Widget* child = 0;
      
  //Check that it is really a child widget:
  const type_const_list_widgets::const_iterator iter_find = 
    std::find(m_list_first_widgets.begin(), m_list_first_widgets.end(), &first);
  if(iter_find == m_list_first_widgets.end())
    return false; //It is not a first widget.
    
  child = &first;
  
  //Check if it was added to an Box:
  const Gtk::Box* hbox = get_parent_hbox(child);
  if(hbox)
    child = hbox;
    
  if(!child)
    return false;

  int width_min = 0;
  int width_natural = 0;
  child->get_preferred_width(width_min, width_natural);
  //std::cout << G_STRFUNC << ": Calling get_child_line() with child=" << child << ", for first=" << &first << std::endl;
  
  //Get the internal parent GtkEventBox, if any,
  //though we need a derived get_child_line() to do this automatically:
  const Gtk::Widget* parent = child->get_parent();
  if(dynamic_cast<const Gtk::EventBox*>(parent))
     child = parent;
     
  column = get_child_line(*child, width_natural);

  return true;
}

bool FlowTable::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  const bool result = Egg::SpreadTableDnd::on_draw(cr);
  if(!m_design_mode)
    return result;

  cr->set_line_width(1);
  cr->set_line_cap(Cairo::LINE_CAP_SQUARE);
  cr->set_line_join(Cairo::LINE_JOIN_MITER);
  std::vector<double> dashes;
  dashes.push_back(10);
  cr->set_dash(dashes, 0);

  //Draw lines based on the allocations of the "first" widgets:
  //This is a very rough interpretation of the column/item borders,
  //but it is better than nothing.
  //TODO: Add API to EggSpreadTable for this?
  for(type_const_list_widgets::iterator iter = m_list_first_widgets.begin(); iter != m_list_first_widgets.end(); ++iter)
  {
    const Gtk::Widget* widget = *iter;
    //std::cout << G_STRFUNC << ": widget: " << widget << std::endl;

    if(!widget)
      continue;

    const Gtk::Allocation allocation = widget->get_allocation();
    const int x = allocation.get_x();
    const int y = allocation.get_y();
    //std::cout << G_STRFUNC << ": x: " << x << ", y: " << y << std::endl;

    int real_x = 0;
    int real_y = 0;
    Gtk::Widget* unconst = const_cast<Gtk::Widget*>(widget);
    unconst->translate_coordinates(*this, x, y, real_x, real_y);
    //std::cout << G_STRFUNC << ": real_x: " << real_x << ", real_y: " << real_y << std::endl;

    cr->move_to(real_x, real_y);
    cr->line_to(real_x + allocation.get_width(), real_y);
    cr->stroke();

    //cr->move_to(real_x, real_y + allocation.get_height());
    //cr->line_to(real_x + allocation.get_width(), real_y + allocation.get_height());
    //cr->stroke();
  }

  return result;
}

} //namespace Glom
