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

#include "canvas_editable.h"
#include "canvas_group_resizable.h"
#include "canvas_rect_movable.h"
#include <math.h>
#include <iostream>

namespace Glom
{

CanvasEditable::CanvasEditable()
: m_dragging(false),
  m_drag_x(0.0), m_drag_y(0.0)
{
  m_grid = CanvasGroupGrid::create();
  add_item(m_grid);
}

CanvasEditable::~CanvasEditable()
{
}

void CanvasEditable::add_item(const Glib::RefPtr<Goocanvas::Item>& item, bool resizable)
{
  Glib::RefPtr<Goocanvas::Item> root = get_root_item();
  Glib::RefPtr<Goocanvas::Group> root_group = Glib::RefPtr<Goocanvas::Group>::cast_dynamic(root);
  if(!root_group)
    return;

  add_item(item, root_group, resizable);
}

void CanvasEditable::add_item(const Glib::RefPtr<Goocanvas::Item>& item, const Glib::RefPtr<Goocanvas::Group>& group, bool resizable)
{
  if(!group)
   return;

  std::cout << "CanvasEditable::add_item" << std::endl;

  bool added = false;

  //Add it inside a manipulatable group, if requested:
  if(resizable)
  {
    Glib::RefPtr<CanvasItemMovable> movable = Glib::RefPtr<CanvasItemMovable>::cast_dynamic(item);
    if(movable)
    {
      Glib::RefPtr<CanvasGroupResizable> resizable = CanvasGroupResizable::create();
      resizable->set_grid(m_grid);

      group->add_child(resizable); //We must do this before calling set_child(), so that set_child() can discover the bounds.
      resizable->set_child(movable); //Puts draggable corners and edges around it.
      added = true;
    }
  }

  //Or just add it directly:
  if(!added)
    group->add_child(item);


  Glib::RefPtr<CanvasItemMovable> movable = CanvasItemMovable::cast_to_movable(item);
  if(movable)
    movable->set_grid(m_grid);
}

void CanvasEditable::remove_all_items()
{
  Glib::RefPtr<Goocanvas::Item> root = get_root_item();
  Glib::RefPtr<Goocanvas::Group> root_group = Glib::RefPtr<Goocanvas::Group>::cast_dynamic(root);
  remove_all_items(root_group);
}

void CanvasEditable::remove_all_items(const Glib::RefPtr<Goocanvas::Group>& group)
{
  while(group && group->get_n_children())
      group->remove_child(0);
}


//static:
Glib::RefPtr<Goocanvas::Item> CanvasEditable::get_parent_container_or_self(const Glib::RefPtr<Goocanvas::Item>& item)
{
  return item;

  Glib::RefPtr<Goocanvas::Item> result = item; 
  while(result && !result->is_container())
    result = result->get_parent();

  return result;
}

void CanvasEditable::add_vertical_rule(double x)
{
  m_grid->add_vertical_rule(x);
}

void CanvasEditable::add_horizontal_rule(double y)
{
  m_grid->add_horizontal_rule(y);
}

void CanvasEditable::set_grid_gap(double gap)
{
  m_grid->set_grid_gap(gap);
}

void CanvasEditable::remove_grid()
{
  m_grid->remove_grid();
}

CanvasEditable::type_signal_show_context CanvasEditable::signal_show_context()
{
  return m_signal_show_context;
}

} //namespace Glom

