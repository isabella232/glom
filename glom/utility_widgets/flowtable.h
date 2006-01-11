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


class FlowTable : public Gtk::Container
{
public: 
  FlowTable();
  virtual ~FlowTable();

  typedef Gtk::Container type_base;

  virtual void add(Gtk::Widget& first, Gtk::Widget& second, bool expand_second = false);
  virtual void add(Gtk::Widget& first, bool expand = false); //override

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

  int get_column_height(guint start_widget, guint widget_count, int& total_width) const;

  /** 
   * @result The height when the children are arranged optimally (so that the height is minimum).
   */
  int get_minimum_column_height(guint start_widget, guint columns_count, int& total_width) const;

  class FlowTableItem
  {
  public:
    FlowTableItem();

    Gtk::Widget* m_first;
    Gtk::Widget* m_second;
    bool m_expand_first;
    bool m_expand_second;

    //Cache the positions, so we can use them in on_expose_event:
    Gtk::Allocation m_first_allocation;
    Gtk::Allocation m_second_allocation;
  };

  typedef std::vector<FlowTableItem> type_vecChildren;

  int get_item_requested_height(const FlowTableItem& item) const;
  void get_item_requested_width(const FlowTableItem& item, int& first, int& second) const;
  void get_item_max_width_requested(guint start, guint height, guint& first_max_width, guint& second_max_width, guint& singles_max_width, bool& is_last_column) const; //TODO: maybe combine this with code in get_minimum_column_height().

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

#endif //GLOM_UTILITYWIDGETS_FLOWTABLE_H
