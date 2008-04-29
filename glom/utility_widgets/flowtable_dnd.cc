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

#include "flowtable_dnd.h"
#include "../mode_data/flowtablewithfields.h"
#include "eggtoolpalette/eggtoolpalette.h"
#include "placeholder-glom.h"
#include "dragbutton.h"
#include "entryglom.h"
#include "labelglom.h"
#include "buttonglom.h"
#include "imageglom.h"

namespace Glom
{

const std::string MOVE_TARGET = "FlowTableMoveTarget";
  
FlowTableDnd::FlowTableDnd() :
  m_current_dnd_item(0),
  m_internal_drag (false)
{
  std::list<Gtk::TargetEntry> drag_targets;
  Gtk::TargetEntry toolbar_target (egg_tool_palette_get_drag_target_item());
  Gtk::TargetEntry move_target(MOVE_TARGET);
  drag_targets.push_back(toolbar_target);
  drag_targets.push_back(move_target);
  drag_dest_set(drag_targets);
}

FlowTableDnd::~FlowTableDnd()
{
  
}

void FlowTableDnd::start_dnd (Gtk::Widget& child)
{
  if (dynamic_cast<PlaceholderGlom*>(&child) ||
      dynamic_cast<FlowTableDnd*>(&child))
    return;
  
	// Call this method recursive for all (real) children
  Gtk::Container* container = dynamic_cast<Gtk::Container*>(&child);
  if (container)
  {
    typedef Glib::ListHandle<Gtk::Widget*>::const_iterator CI;
    Glib::ListHandle<Gtk::Widget*> children = container->get_children();
    for (CI cur_child = children.begin(); cur_child != children.end();
         ++cur_child)
    {
      start_dnd (*(*cur_child));
    }
  }
  if (!(child.get_flags() & Gtk::NO_WINDOW))
  {
    // Needed to move items around
    std::list<Gtk::TargetEntry> source_targets;
    source_targets.push_back (Gtk::TargetEntry (MOVE_TARGET));
    child.drag_source_set (source_targets, Gdk::BUTTON1_MASK|Gdk::BUTTON3_MASK, 
                           Gdk::ACTION_MOVE | Gdk::ACTION_COPY);
    
    std::list<Gtk::TargetEntry> drag_targets;
    Gtk::TargetEntry toolbar_target (egg_tool_palette_get_drag_target_item());
    Gtk::TargetEntry move_target(MOVE_TARGET);
    drag_targets.push_back(toolbar_target);
    drag_targets.push_back(move_target);
    
    Glib::RefPtr<Gtk::TargetList> targets =
			child.drag_dest_get_target_list ();
    // The widget has already a default drag destination - add more targets
    if (targets)
    {
      targets->add (drag_targets);
      child.drag_dest_set_target_list (targets);
    }
    else
      child.drag_dest_set(drag_targets, Gtk::DEST_DEFAULT_ALL,
                          Gdk::ACTION_COPY | Gdk::ACTION_MOVE);
		    
    // It's important to connect this one BEFORE
    child.signal_drag_motion().connect (sigc::bind<Gtk::Widget*>(sigc::mem_fun (*this, &FlowTableDnd::on_child_drag_motion), &child),
                                        false);
    child.signal_drag_data_received().connect (sigc::bind<Gtk::Widget*>(sigc::mem_fun (*this, &FlowTableDnd::on_child_drag_data_received), &child));
    child.signal_drag_leave().connect (sigc::mem_fun (*this, &FlowTableDnd::on_child_drag_leave));

    child.signal_drag_begin().connect (sigc::bind<Gtk::Widget*>(sigc::mem_fun (*this, &FlowTableDnd::on_child_drag_begin), &child), false);
    child.signal_drag_end().connect (sigc::bind<Gtk::Widget*>(sigc::mem_fun (*this, &FlowTableDnd::on_child_drag_end), &child), false);
    child.signal_drag_data_get().connect (sigc::bind<Gtk::Widget*>(sigc::mem_fun (*this, &FlowTableDnd::on_child_drag_data_get), &child), false);
  }
}

void FlowTableDnd::stop_dnd (Gtk::Widget& child)
{
  if (dynamic_cast<PlaceholderGlom*>(&child) ||
      dynamic_cast<FlowTableDnd*>(&child))
    return;
  
	// Call this method recursive for all (real) children
  Gtk::Container* container = dynamic_cast<Gtk::Container*>(&child);
  if (container)
  {
    typedef Glib::ListHandle<Gtk::Widget*>::const_iterator CI;
    Glib::ListHandle<Gtk::Widget*> children = container->get_children();
    for (CI cur_child = children.begin(); cur_child != children.end();
         ++cur_child)
    {
      stop_dnd (*(*cur_child));
    }
  }
  if (!(child.get_flags() & Gtk::NO_WINDOW))
  {
    child.drag_source_unset();
  }
}

bool FlowTableDnd::on_drag_motion(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, guint time)
{
  // The drag position is relativ to the toplevel GdkWindow and not to
  // the widget position. We fix this here to match the widget position
  x += get_allocation().get_x();
  y += get_allocation().get_y();
  
  m_current_dnd_item = dnd_item_at_position(x, y);  
  LayoutWidgetBase* above = dnd_datawidget_from_item(0);
	
  // above might be 0 here...
  on_dnd_add_placeholder(above);
  return false;
}

void FlowTableDnd::on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& drag_context, int drag_x, int drag_y, const Gtk::SelectionData& selection_data, guint, guint time)
{
  Gtk::Widget* palette = drag_get_source_widget (drag_context);
  while (palette && !EGG_IS_TOOL_PALETTE (palette->gobj()))
    palette = palette->get_parent();
  
  on_dnd_remove_placeholder();
  LayoutWidgetBase* above = dnd_datawidget_from_item(0);
  if (palette)
  {
    GtkWidget* tool_item = egg_tool_palette_get_drag_item (EGG_TOOL_PALETTE (palette->gobj()), selection_data.gobj());
    LayoutWidgetBase::enumType type = 
      static_cast<LayoutWidgetBase::enumType>(GPOINTER_TO_INT(g_object_get_data(G_OBJECT(tool_item), "glom-type")));
    switch (type)
    {
      case LayoutWidgetBase::TYPE_FIELD:
        on_dnd_add_layout_item_field(above);
        break;
      case LayoutWidgetBase::TYPE_BUTTON:
        on_dnd_add_layout_item_button(above);
        break;
      case LayoutWidgetBase::TYPE_TEXT:
        on_dnd_add_layout_item_text(above);
        break;
      case LayoutWidgetBase::TYPE_GROUP:
        on_dnd_add_layout_group(above);
        break;
      case LayoutWidgetBase::TYPE_NOTEBOOK:
        on_dnd_add_layout_notebook(above);
        break;
      default:
        std::cerr << "Unknown drop type: " << type << std::endl;
    }
  }
  else
  {
    gpointer* data = (gpointer*)selection_data.get_data();
    LayoutWidgetBase* base = (LayoutWidgetBase*)*data;
    if (base)
    {
      sharedptr<LayoutItem> item = base->get_layout_item();
      if (item)
      {
        sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::cast_dynamic(get_layout_item());
        LayoutGroup::type_list_items items = group->m_list_items;
        if (std::find (items.begin(), items.end(), item) != items.end())
        {
          m_internal_drag = true;
          group->remove_item(item);
        }
        else
          m_internal_drag = false;
        on_dnd_add_layout_item (above, item);
        base->set_dnd_in_progress(false);
      }
    }
  }
}

void FlowTableDnd::on_drag_leave(const Glib::RefPtr<Gdk::DragContext>& drag_context, guint time)
{
  dnd_remove_placeholder_idle();
}

// Calculate the nearest FlowTableDndItem below the current drag position
FlowTable::FlowTableItem*
FlowTableDnd::dnd_item_at_position(int drag_x, int drag_y)
{ 
  int column_width;
  get_column_height (0, m_children.size(), column_width);
  int column = 0;
	
  if (column_width != 0)
    column = drag_x / column_width;
  
  for (std::vector<FlowTableItem>::iterator cur_item = m_children.begin(); cur_item != m_children.end(); 
       cur_item++)
  {
    Gdk::Rectangle rect = cur_item->m_first->get_allocation();
    if (cur_item->m_second)
    {
      Gdk::Rectangle second_rect = cur_item->m_second->get_allocation();
      rect.set_height (MAX (rect.get_height(), second_rect.get_height()));
      rect.set_width (rect.get_width() + m_padding + second_rect.get_width());
    }
    
    int cur_column = 0;
    if (column_width != 0)
      cur_column = rect.get_x() / column_width;
    
    if (cur_column != column)
    {
      continue;  
    }
    
    // Allow dragging at the end
    if (cur_item == --m_children.end())
    {
      if (drag_y > (rect.get_y() + rect.get_height() / 2) &&
          drag_y < (rect.get_y() + rect.get_height()))
      {
        return 0;
      }
    }
    
    if (drag_y < (rect.get_y() + rect.get_height()))
    {
			return &(*cur_item);
    }
  }
  
  return 0;
}

LayoutWidgetBase* FlowTableDnd::dnd_datawidget_from_item(FlowTable::FlowTableItem* item)
{
  // Test if we have a datawidget below which we want to add
  LayoutWidgetBase* above = 0;
  FlowTableItem* used_item;
  if (item)
    used_item = item;
  else
    used_item = m_current_dnd_item;
  
  if(used_item)
  {
		if(used_item->m_first)
    {
      Gtk::Alignment* alignment = dynamic_cast <Gtk::Alignment*>(used_item->m_first);
      if (alignment)
      {
        above = dynamic_cast<LayoutWidgetBase*>(alignment->get_child());
      }
    }
    if(!above && used_item->m_first)
      above = dynamic_cast<LayoutWidgetBase*>(used_item->m_first);
    if(!above && used_item->m_second)
    {
      above = dynamic_cast<LayoutWidgetBase*>(used_item->m_second);
      // Special case for labels
      if (!above)
      {       
        Gtk::Alignment* alignment = dynamic_cast <Gtk::Alignment*>(used_item->m_second);
        if (alignment)
        {
          above = dynamic_cast<LayoutWidgetBase*>(alignment->get_child());
        }
      } 
    }
  }
  return above;
}

FlowTable::FlowTableItem* FlowTableDnd::find_current_dnd_item (Gtk::Widget* child, int x, int y)
{
  FlowTableItem* item;
  type_vecChildren::iterator cur_child;
  for (cur_child = m_children.begin();
       cur_child != m_children.end(); cur_child++)
  {
    // The widget was added directly to the FlowTableDnd
    if (cur_child->m_first == child || 
        cur_child->m_second == child)
    {
      break;
    }
    // The parent of the widget was added to the FlowTableDnd
    else if (cur_child->m_first == child->get_parent() ||
             cur_child->m_second == child->get_parent())
    {
      break;
    }
    Gtk::Bin* bin = dynamic_cast<Gtk::Bin*>(cur_child->m_second);
    // The widget was added inside a Gtk::EventBox
    if (bin)
    {
      if (bin->get_child() == child ||
          bin->get_child() == child->get_parent())
        break;
    }
  }
  if (cur_child != m_children.end())
    item = &(*cur_child);
  else
    item = 0;
  
  // Allow dragging at-the-end
  if (cur_child == --m_children.end())
  {
    Gdk::Rectangle rect = child->get_allocation();
    if (y > (rect.get_y() + rect.get_height() / 2) &&
        y < (rect.get_y() + rect.get_height()))
    {
      item = 0; // means end
    }
  } 
  return item;
}

bool FlowTableDnd::on_child_drag_motion(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, guint time,
                                        Gtk::Widget* child)
{
  find_current_dnd_item(child, x, y);
  
  on_dnd_remove_placeholder ();
  
  LayoutWidgetBase* above = dnd_datawidget_from_item(0);
  
  // above might be 0 here...
  on_dnd_add_placeholder(above);
  return false;
}

void FlowTableDnd::on_child_drag_leave(const Glib::RefPtr<Gdk::DragContext>& drag_context, guint time)
{
  dnd_remove_placeholder_idle();
}

void FlowTableDnd::on_child_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, 
                                            const Gtk::SelectionData& selection_data, guint info, guint time,
                                            Gtk::Widget* child)
{
  on_drag_data_received (drag_context, x, y, selection_data, info, time);
}

void FlowTableDnd::on_child_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& drag_context, 
                                          Gtk::SelectionData& selection_data, guint, guint time, 
                                          Gtk::Widget* child)
{
  FlowTableItem* item = find_current_dnd_item (child);
  LayoutWidgetBase* base = dnd_datawidget_from_item (item);
  if (base)
  {
    gpointer data = base;
    selection_data.set ("LayoutWidgetBase*", 8, (guint8*)&data, 
                             sizeof(gpointer*));
  }
}

void FlowTableDnd::on_child_drag_begin (const Glib::RefPtr<Gdk::DragContext>& drag_context,
                          Gtk::Widget* child)
{
  FlowTableItem* item = find_current_dnd_item (child);
  if (!item)
    return;
  if (item->m_first)
    item->m_first->hide();
  if (item->m_second)
    item->m_second->hide();
  LayoutWidgetBase* base = dnd_datawidget_from_item (item);
  base->set_dnd_in_progress(); 
}

void FlowTableDnd::on_child_drag_end (const Glib::RefPtr<Gdk::DragContext>& drag_context,
                        Gtk::Widget* child)
{
  FlowTableItem* item = find_current_dnd_item (child);
  LayoutWidgetBase* base = dnd_datawidget_from_item (item);
  if (base->get_dnd_in_progress())
  {
    if (!item)
      return;
    if (item->m_first)
      item->m_first->show();
    if (item->m_second)
      item->m_second->show();  
  }
  else if (!m_internal_drag)
  {
    sharedptr<LayoutItem> item = base->get_layout_item();
    sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::cast_dynamic(get_layout_item());
    group->remove_item(item);
  }  
  signal_layout_changed().emit();
}

void FlowTableDnd::set_design_mode(bool value)
{
  FlowTable::set_design_mode(value);
  // We only want to enable drag and drop when in design mode
  if (value)
  {
    forall (sigc::mem_fun(*this, &FlowTableDnd::start_dnd));
  }
  else
  {
    forall (sigc::mem_fun(*this, &FlowTableDnd::stop_dnd));
  } 
}

/* This is a hack. The problem is that when you move the mouse down to the last
 item, the item gets the "drag-motion" signal but in the same moment, the placeholder
 is removed and the mouse pointer is no longer over the widget. Thus, it gets
 a "drag-leave" signal and it's impossible to drop an item at the end. Doing 
 the removal of the placeholder in an idle handler fixes it. */

void FlowTableDnd::dnd_remove_placeholder_idle()
{
  static sigc::connection connection;
  if (connection)
    connection.disconnect();
  Glib::signal_idle().connect( sigc::mem_fun(*this, &FlowTableDnd::dnd_remove_placeholder_real) );
}

bool FlowTableDnd::dnd_remove_placeholder_real()
{
  on_dnd_remove_placeholder();
  queue_draw();
  return false; // remove from idle source
}

} // namspace Glom
