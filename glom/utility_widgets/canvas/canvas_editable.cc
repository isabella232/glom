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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "canvas_editable.h"
#include "canvas_group_resizable.h"
#include "canvas_rect_movable.h"
#include <math.h>
#include <iostream>

namespace Glom
{

CanvasEditable::CanvasEditable()
{
  m_grid = CanvasGroupGrid::create();
  add_item(m_grid);
}

void CanvasEditable::add_item(const Glib::RefPtr<Goocanvas::Item>& item, bool resizable)
{
  auto root = get_root_item();
  auto root_group = Glib::RefPtr<Goocanvas::Group>::cast_dynamic(root);
  if(!root_group)
    return;

  add_item(item, root_group, resizable);
}

void CanvasEditable::add_item(const Glib::RefPtr<Goocanvas::Item>& item, const Glib::RefPtr<Goocanvas::Group>& group, bool resizable)
{
  if(!group)
   return;

  bool added = false;

  //Add it inside a manipulatable group, if requested:
  if(resizable)
  {
    auto movable = Glib::RefPtr<CanvasItemMovable>::cast_dynamic(item);
    if(movable)
    {
      auto resizable_item = CanvasGroupResizable::create();
      resizable_item->set_grid(m_grid);

      //Specify the resizable's position, using the child's position:
      double x = 0;
      double y = 0;
      movable->get_xy(x, y);
      double width = 0;
      double height = 0;
      movable->get_width_height(width, height);
      resizable_item->set_xy(x, y);
      resizable_item->set_width_height(width, height);

      group->add_child(resizable_item);
      resizable_item->set_child(movable); //Puts draggable corners and edges around it.

      added = true;
    }
  }

  //Or just add it directly:
  if(!added)
    group->add_child(item);


  auto movable = CanvasItemMovable::cast_to_movable(item);
  if(movable)
  {
    movable->set_grid(m_grid);

    //Let this canvas item signal whenever any of its children are selected or deselected:
    movable->signal_selected().connect(
      sigc::mem_fun(*this, &CanvasEditable::on_item_selected));
  }
}

void CanvasEditable::remove_item(const Glib::RefPtr<Goocanvas::Item>& item , const Glib::RefPtr<Goocanvas::Group>& group)
{
  if(!group)
   return;

  //TODO: Remove resizable=true items via their parent item.
  item->remove();

  auto movable = Glib::RefPtr<CanvasItemMovable>::cast_dynamic(item);
  if(movable && movable->get_selected())
    m_signal_selection_changed.emit();
}

void CanvasEditable::remove_all_items()
{
  const auto some_selected = !(get_selected_items().empty());

  auto root = get_root_item();
  auto root_group = Glib::RefPtr<Goocanvas::Group>::cast_dynamic(root);

  while(root_group && root_group->get_n_children())
      root_group->remove_child(0);

  //The selection has changed because selected items have been removed:
  if(some_selected)
    m_signal_selection_changed.emit();
}

void CanvasEditable::remove_all_items(const Glib::RefPtr<Goocanvas::Group>& group)
{
 const auto some_selected = !(get_selected_items().empty());

  while(group && group->get_n_children())
      group->remove_child(0);

  //The selection has changed because selected items have been removed:
  if(some_selected)
    m_signal_selection_changed.emit();
}


void CanvasEditable::add_vertical_rule(double x)
{
  m_grid->add_vertical_rule(x);
}

void CanvasEditable::add_horizontal_rule(double y)
{
  m_grid->add_horizontal_rule(y);
}

void CanvasEditable::remove_rules()
{
  m_grid->remove_rules();
}

CanvasEditable::type_vec_doubles CanvasEditable::get_horizontal_rules() const
{
  return m_grid->get_horizontal_rules();
}

CanvasEditable::type_vec_doubles CanvasEditable::get_vertical_rules() const
{
  return m_grid->get_vertical_rules();
}

void CanvasEditable::show_temp_rule(double x, double y, bool show_rule)
{
  m_grid->show_temp_rule(x, y, show_rule);
}

void CanvasEditable::set_grid_gap(double gap)
{
  m_grid->set_grid_gap(gap);
}

void CanvasEditable::remove_grid()
{
  m_grid->remove_grid();
}

void CanvasEditable::associate_with_grid(const Glib::RefPtr<Goocanvas::Item>& item)
{
  auto movable = CanvasItemMovable::cast_to_movable(item);
  if(movable)
    movable->set_grid(m_grid);
}

CanvasEditable::type_signal_show_context CanvasEditable::signal_show_context()
{
  return m_signal_show_context;
}

CanvasEditable::type_signal_selection_changed CanvasEditable::signal_selection_changed()
{
  return m_signal_selection_changed;
}

void CanvasEditable::on_item_selected(const Glib::RefPtr<CanvasItemMovable>& item, bool group_select)
{
  const auto selected = !item->get_selected();

  if(!group_select)
  {
    //Make sure that all other items are deselected first:
    for(const auto& selected_item : get_selected_items())
    {
      if(selected_item)
        selected_item->set_selected(false);
    }
  }

  item->set_selected(!selected);

  m_signal_selection_changed.emit();
}

CanvasEditable::type_vec_items CanvasEditable::get_selected_items()
{
  //TODO: Provide a default implementation.
  return type_vec_items();
}

void CanvasEditable::set_rules_visibility(bool visible)
{
  m_grid->set_rules_visibility(visible);
}


} //namespace Glom
