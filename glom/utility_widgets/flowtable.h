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

#ifndef GLOM_UTILITYWIDGETS_FLOWTABLE_H
#define GLOM_UTILITYWIDGETS_FLOWTABLE_H

#include <gtkmm.h>
#include "layoutwidgetbase.h"

namespace Glom
{

class FlowTable : public Gtk::Container
{
public: 
  FlowTable();
  virtual ~FlowTable();

  typedef Gtk::Container type_base;

  virtual void add(Gtk::Widget& first, Gtk::Widget& second, bool expand_second = false);
  virtual void add(Gtk::Widget& first, bool expand = false); //override
  void insert_before(Gtk::Widget& first, Gtk::Widget& second, Gtk::Widget& before, bool expand_second);
  void insert_before(Gtk::Widget& first, Gtk::Widget& before, bool expand_second);

	virtual void remove(Gtk::Widget& first); //override

  void set_columns_count(guint value);

  /** Sets the padding to put between the child widgets.
   */
  virtual void set_padding(guint padding);

  /** Show extra UI that is useful in RAD tools:
   */
  virtual void set_design_mode(bool value = true);

  void remove_all();


protected:
#ifndef GLIBMM_VFUNCS_ENABLED
  // These are the hand-coded C vfunc implementations in case the
  // corresponding glibmm API has been disabled
  static void glom_forall_impl(GtkContainer* container, gboolean include_internals, GtkCallback callback, gpointer callback_data);
  static GType glom_child_type_impl(GtkContainer* container);
#endif
#ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
  // These are the hand-coded C default signal handlers in case the
  // corresponding glibmm API has been disabled
  static void glom_size_request_impl(GtkWidget* widget, GtkRequisition* requisition);
  static void glom_size_allocate_impl(GtkWidget* widget, GtkAllocation* allocation);
  static void glom_add_impl(GtkContainer* container, GtkWidget* widget);
  static void glom_remove_impl(GtkContainer* container, GtkWidget* widget);

  static void glom_realize_impl(GtkWidget* widget);
  static void glom_unrealize_impl(GtkWidget* widget);
  static gboolean glom_expose_event_impl(GtkWidget* widget, GdkEventExpose* event);
#endif
  //Overrides:

  //Handle child widgets:
  virtual void on_size_request(Gtk::Requisition* requisition);
  virtual void on_size_allocate(Gtk::Allocation& allocation);
  virtual GtkType child_type_vfunc() const;
  virtual void on_add(Gtk::Widget* child);
  virtual void forall_vfunc(gboolean include_internals, GtkCallback callback, gpointer callback_data);
  virtual void on_remove(Gtk::Widget* child);

  //Do extra drawing:
  virtual void on_realize();
  virtual void on_unrealize();
  virtual bool on_expose_event(GdkEventExpose* event);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //DND stuff:
  virtual bool on_drag_motion(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, guint time);
  virtual void on_drag_leave(const Glib::RefPtr<Gdk::DragContext>& drag_context, guint time);
  virtual void on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& drag_context, int, int, const Gtk::SelectionData& selection_data, guint, guint time);
  virtual void on_dnd_add_placeholder(LayoutWidgetBase* above) = 0;
  virtual void on_dnd_remove_placeholder() = 0;
  
  // Methods for the different layout object
  virtual void on_dnd_add_layout_item_field (LayoutWidgetBase* above) = 0;
  virtual void on_dnd_add_layout_group(LayoutWidgetBase* above) = 0;
  virtual void on_dnd_add_layout_item_button (LayoutWidgetBase* above) = 0;
  virtual void on_dnd_add_layout_item_text (LayoutWidgetBase* above) = 0;

#endif // !GLOM_ENABLE_CLIENT_ONLY

  int get_column_height(guint start_widget, guint widget_count, int& total_width) const;

  /** 
   * @result The height when the children are arranged optimally (so that the height is minimum).
   */
  int get_minimum_column_height(guint start_widget, guint columns_count, int& total_width) const;

  class FlowTableItem
  {
  public:
    FlowTableItem();
    FlowTableItem(Gtk::Widget* first, Gtk::Widget* second);

    Gtk::Widget* m_first;
    Gtk::Widget* m_second;
    bool m_expand_first_full;
    bool m_expand_second;

    //Cache the positions, so we can use them in on_expose_event:
    Gtk::Allocation m_first_allocation;
    Gtk::Allocation m_second_allocation;
  };
	
  typedef std::vector<FlowTableItem> type_vecChildren;
  void insert_before(FlowTableItem& item, Gtk::Widget& before);
	
  int get_item_requested_height(const FlowTableItem& item) const;
  void get_item_requested_width(const FlowTableItem& item, int& first, int& second) const;
  void get_item_max_width_requested(guint start, guint height, guint& first_max_width, guint& second_max_width, guint& singles_max_width, bool& is_last_column) const; //TODO: maybe combine this with code in get_minimum_column_height().
  
#ifndef GLOM_ENABLE_CLIENT_ONLY
  FlowTableItem* dnd_get_item(int x, int y);
  void change_dnd_status(bool active = true);
  LayoutWidgetBase* dnd_find_datawidget();
  FlowTableItem* m_current_dnd_item;
	
  bool m_dnd_in_progress;
#endif // !GLOM_ENABLE_CLIENT_ONLY
  
  enum
  {
    BELOW = -1
  };
  
  bool child_is_visible(const Gtk::Widget* widget) const;

  Gtk::Allocation assign_child(Gtk::Widget* widget, int x, int y);
  Gtk::Allocation assign_child(Gtk::Widget* widget, int x, int y, int width, int height);

  type_vecChildren m_children;
  guint m_columns_count;
  guint m_padding;
  bool m_design_mode;

  //Lines to draw in on_expose_event:
  typedef std::pair<Gdk::Point, Gdk::Point> type_line;
  typedef std::vector<type_line> type_vecLines;
  type_vecLines m_lines_vertical;
  type_vecLines m_lines_horizontal;

  //For drawing:
  Glib::RefPtr<Gdk::Window> m_refGdkWindow;
  Glib::RefPtr<Gdk::GC> m_refGC;
};

} //namespace Glom

#endif //GLOM_UTILITYWIDGETS_FLOWTABLE_H
