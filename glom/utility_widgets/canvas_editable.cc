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

namespace Glom
{

CanvasEditable::CanvasEditable()
: m_dragging(false),
  m_drag_x(0.0), m_drag_y(0.0)
{
}

CanvasEditable::~CanvasEditable()
{
}

void CanvasEditable::add_item(const Glib::RefPtr<Goocanvas::Item>& item, bool resizable)
{
  if(!item)
    return;

  ItemInfo info;
  info.m_resizable = resizable;

  Glib::RefPtr<Goocanvas::Item> root = get_root_item();

  Glib::RefPtr<Goocanvas::Group> root_group = Glib::RefPtr<Goocanvas::Group>::cast_dynamic(root);
  if(root_group)
  {
    if(resizable)
    {
      Glib::RefPtr<CanvasGroupResizable> resizable = CanvasGroupResizable::create();
      root_group->add_child(resizable); //We must do this before calling set_child(), so that set_child() can discover the bounds.
      resizable->set_child(item); //Puts draggable corners and edges around it. 
    }
    else
      root_group->add_child(item);
  }

  item->signal_motion_notify_event().connect(sigc::mem_fun(*this, &CanvasEditable::on_item_motion_notify_event));
  item->signal_button_press_event().connect(sigc::mem_fun(*this, &CanvasEditable::on_item_button_press_event));
  item->signal_button_release_event().connect(sigc::mem_fun(*this, &CanvasEditable::on_item_button_release_event));

  m_map_item_info[item] = info;
}

void CanvasEditable::remove_all_items()
{
  Glib::RefPtr<Goocanvas::Item> root = get_root_item();
  while(root && root->get_n_children())
      root->remove_child(0);
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

bool CanvasEditable::on_item_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event)
{
  //printf("%s\n", __FUNCTION__);
  pointer_ungrab(target, event->time);
  m_dragging = false;


  return true;
}

bool CanvasEditable::on_item_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event)
{
  printf("%s: button=%d\n", __FUNCTION__, event->button);
  switch(event->button)
  {
    case 1:
    {
      Glib::RefPtr<Goocanvas::Item> item = get_parent_container_or_self(target);
      
      item->raise();
    
      m_drag_x = event->x;
      m_drag_y = event->y;
    
      Gdk::Cursor fleur(Gdk::FLEUR);
      pointer_grab(item, Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_RELEASE_MASK,
                  fleur,
                  event->time);
      m_dragging = true;
      break;
    }
    case 3:
    {
      //This method is virtual:
      on_show_context_menu(event->button, event->time);
      break;
    }

    default:
      break;
  }
  
  return true;
}

bool CanvasEditable::on_item_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event)
{
  //printf("%s\n", __FUNCTION__);

  Glib::RefPtr<Goocanvas::Item> item = get_parent_container_or_self(target);
  
  if(item && m_dragging && (event->state & Gdk::BUTTON1_MASK))
  {
    double new_x = event->x;
    double new_y = event->y;

    item->translate(new_x - m_drag_x, new_y - m_drag_y);
  }

  return true;
}

void CanvasEditable::on_show_context_menu(guint button, guint32 activate_time)
{
  printf("%s\n", __FUNCTION__);
}


} //namespace Glom

