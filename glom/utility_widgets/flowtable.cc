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
#include <libglom/algorithms_utils.h>
#include <iostream>
#include <gtkmm/eventbox.h>
#include <gdkmm/window.h>
#include <glom/utils_ui.h>

namespace Glom
{

FlowTable::FlowTable()
: Glib::ObjectBase("flowtable"),
  Gtk::WidgetCustomDraw(),
  m_design_mode(false)
{
  //Default to disabling drag and drop:
  set_drag_enabled(EGG_DRAG_DISABLED);
  set_drop_enabled(false);
}

FlowTable::~FlowTable()
{
  while(!m_list_hboxes.empty())
  {
    auto iter = m_list_hboxes.begin();
    auto hbox = *iter;
    delete_and_forget_hbox(hbox);
  }
}

const Gtk::Box* FlowTable::get_parent_hbox(const Gtk::Widget* first) const
{
  if(!Utils::find_exists(m_list_first_widgets, first))
  {
    std::cerr << G_STRFUNC << ": first was not a first widget. first=" << first << std::endl;
    return nullptr; //It has no Box parent because it is not even a first widget.
  }

  for(const auto& hbox : m_list_hboxes)
  {
    if(!hbox)
      continue;

    //Check if it has the widget as one of its children:
    const auto box_children = hbox->get_children();
    if(box_children.empty())
      continue;

    if(Utils::find_exists(box_children, first))
      return hbox;
  }

  return nullptr;
}

void FlowTable::delete_and_forget_hbox(Gtk::Box* hbox)
{
  //Remove its children because the API hides the fact that they are inside the Box.
  //Otherwise they could even be deleted by the Box.
  auto children = hbox->get_children();
  while(!children.empty())
  {
    auto widget = children[0];
    hbox->remove(*widget);
    children = hbox->get_children();
  }

  //This check does not work because EggSpreadTableDnD adds an intermediate GtkEventBox:
  //if(hbox->get_parent() == this)

  //Check that it is in our list of hboxes:
  const auto iter = Utils::find(m_list_hboxes, hbox);
  if(iter == m_list_hboxes.end())
  {
    std::cerr << G_STRFUNC << ": hbox=" << hbox << " is not in our list of hboxes.\n";
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
    //std::cerr << G_STRFUNC << ": hbox=" << hbox << " has no parent. Not removing from SpreadTableDnd\n";
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
  insert(&first, nullptr /* second */, -1, expand);
}

void FlowTable::insert(Gtk::Widget* first, Gtk::Widget* second, int index, bool expand)
{
  if(first && second)
  {
    auto hbox = new Gtk::Box(Gtk::Orientation::HORIZONTAL, get_horizontal_spacing());
    m_list_hboxes.emplace_back(hbox); //So we can delete it whenever necessary.

    hbox->pack_start(*first, Gtk::PackOptions::SHRINK);
    hbox->pack_start(*second, expand ? Gtk::PackOptions::EXPAND_WIDGET : Gtk::PackOptions::SHRINK);
    hbox->show();

    hbox->set_halign(Gtk::Align::FILL);
    Egg::SpreadTableDnd::insert_child(*hbox, index);
    //std::cout << "DEBUG: inserted hbox=" << hbox << " for first=" << first << std::endl;

    m_list_first_widgets.emplace_back(first);
  }
  else if(first)
  {
    first->set_halign(expand ? Gtk::Align::FILL : Gtk::Align::START);
    Egg::SpreadTableDnd::append_child(*first);
    //std::cout << "DEBUG: inserted first=" << first << std::endl;
    m_list_first_widgets.emplace_back(first);
  }
  else
  {
    std::cerr << G_STRFUNC << ": first was null\n";
  }
}

void FlowTable::remove_all()
{
  for(const auto& item : m_list_first_widgets)
  {
    auto first_widget = const_cast<Gtk::Widget*>(item);

    if(first_widget)
      remove(*first_widget);
  }
  m_list_first_widgets.clear();

  //We can't use get_children() because EggSpreadTableDnd does not allow that,
  //because it handles children differently via its specific API.
  /*
  for(const auto& widget : get_children())
  {
    remove(*widget);
  }
  */
}

void FlowTable::remove(Gtk::Widget& first)
{
  //std::cout << G_STRFUNC << ": debug: remove() first=" << &first << std::endl;

  //Handle widgets that were added to an Box:
  auto parent = const_cast<Gtk::Box*>(get_parent_hbox(&first));
  if(parent)
  {
    //std::cout << "  debug: hbox=" << parent << std::endl;

    delete_and_forget_hbox(parent);
    return;
  }

  Egg::SpreadTableDnd::remove_child(first);
}

bool FlowTable::get_column_for_first_widget(const Gtk::Widget& first, guint& column) const
{
  //Initialize output parameter:
  column = 0;

  if(get_lines() == 0)
    return false;

  //Discover actual child widget that was added to the EggSpreadTable,
  //so we can use it again to call EggSpreadTable::get_child_line():
  const Gtk::Widget* child = nullptr;

  //Check that it is really a child widget:
  if(!Utils::find_exists(m_list_first_widgets, &first))
    return false; //It is not a first widget.

  child = &first;

  //Check if it was added to an Box:
  const auto hbox = get_parent_hbox(child);
  if(hbox)
    child = hbox;

  if(!child)
    return false;

  int width = 0;
  int width_min = 0;
  int width_natural = 0;
  int baseline_min = 0;
  int baseline_natural = 0;
  child->measure(Gtk::Orientation::HORIZONTAL, width, width_min, width_natural, baseline_min, baseline_natural);
  //std::cout << G_STRFUNC << ": Calling get_child_line() with child=" << child << ", for first=" << &first << std::endl;

  //Get the internal parent GtkEventBox, if any,
  //though we need a derived get_child_line() to do this automatically:
  const auto parent = child->get_parent();
  if(dynamic_cast<const Gtk::EventBox*>(parent))
     child = parent;

  column = get_child_line(*child, width_natural);

  return true;
}

bool FlowTable::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  // TODO ? const auto result = Egg::SpreadTableDnd::on_draw(cr);
  if(!m_design_mode)
    return true;

  cr->set_line_width(1);
  cr->set_line_cap(Cairo::Context::LineCap::SQUARE);
  cr->set_line_join(Cairo::Context::LineJoin::MITER);
  std::vector<double> dashes;
  dashes.emplace_back(10);
  cr->set_dash(dashes, 0);

  //Draw lines based on the allocations of the "first" widgets:
  //This is a very rough interpretation of the column/item borders,
  //but it is better than nothing.
  //TODO: Add API to EggSpreadTable for this?
  for(const auto& widget : m_list_first_widgets)
  {
    //std::cout << G_STRFUNC << ": widget: " << widget << std::endl;

    if(!widget)
      continue;

    const auto allocation = widget->get_allocation();
    const auto x = allocation.get_x();
    const auto y = allocation.get_y();
    //std::cout << G_STRFUNC << ": x: " << x << ", y: " << y << std::endl;

    int real_x = 0;
    int real_y = 0;
    auto unconst = const_cast<Gtk::Widget*>(widget);
    unconst->translate_coordinates(*this, x, y, real_x, real_y);
    //std::cout << G_STRFUNC << ": real_x: " << real_x << ", real_y: " << real_y << std::endl;

    cr->move_to(real_x, real_y);
    cr->line_to(real_x + allocation.get_width(), real_y);
    cr->stroke();

    //cr->move_to(real_x, real_y + allocation.get_height());
    //cr->line_to(real_x + allocation.get_width(), real_y + allocation.get_height());
    //cr->stroke();
  }

  return true;
}

} //namespace Glom
