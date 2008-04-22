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

#ifndef GLOM_UTILITYWIDGETS_FLOWTABLE_DND_H
#define GLOM_UTILITYWIDGETS_FLOWTABLE_DND_H

#include <gtkmm.h>
#include "flowtable.h"

#ifdef GLOM_ENABLE_CLIENT_ONLY
#error FlowTableDnd does not work in Client-only mode
#endif

namespace Glom
{

class FlowTableDnd : public FlowTable
{
public:
  FlowTableDnd();
  ~FlowTableDnd();
  
protected:
  virtual bool on_drag_motion(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, guint time);
  virtual void on_drag_leave(const Glib::RefPtr<Gdk::DragContext>& drag_context, guint time);
  virtual void on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& drag_context, int, int, const Gtk::SelectionData& selection_data, guint, guint time);
    
  virtual bool on_child_drag_motion(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, guint time, Gtk::Widget* child);
  virtual void on_child_drag_leave(const Glib::RefPtr<Gdk::DragContext>& drag_context, guint time);
  virtual void on_child_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& drag_context, int, int, 
                                           const Gtk::SelectionData& selection_data, guint, guint time, Gtk::Widget* child);  
  virtual void on_child_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& drag_context, 
                                           const Gtk::SelectionData& selection_data, guint, guint time, Gtk::Widget* child);  

  virtual void start_dnd (Gtk::Widget& child);
  virtual void stop_dnd (Gtk::Widget& child);

  virtual void set_design_mode(bool value = true);
    
  // Methods for the different layout object
  virtual void on_dnd_add_layout_item_field (LayoutWidgetBase* above) = 0;
  virtual void on_dnd_add_layout_group(LayoutWidgetBase* above) = 0;
  virtual void on_dnd_add_layout_item_button (LayoutWidgetBase* above) = 0;
  virtual void on_dnd_add_layout_item_text (LayoutWidgetBase* above) = 0;
  virtual void on_dnd_add_layout_notebook (LayoutWidgetBase* above) = 0;
  
  virtual void on_dnd_add_placeholder(LayoutWidgetBase* above) = 0;
  virtual void on_dnd_remove_placeholder() = 0;    
  
  void dnd_remove_placeholder_idle();
  bool dnd_remove_placeholder_real();
  
  FlowTableItem* dnd_item_at_position(int x, int y);
  LayoutWidgetBase* dnd_datawidget_from_item();
    
private:
  FlowTableItem* m_current_dnd_item;
};

} // namespace Glom

#endif // GLOM_UTILITYWIDGETS_FLOWTABLE_DND_H
