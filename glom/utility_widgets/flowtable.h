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

  virtual void add(Gtk::Widget& first, Gtk::Widget& second);
  virtual void add(Gtk::Widget& first); //override
  
  virtual void set_columns_count(guint value);

  /** Sets the padding to put between the child widgets.
   */
  virtual void set_padding(guint padding);

  void remove_all();

protected:

  //Overrides:
  virtual void on_size_request(Gtk::Requisition* requisition);
  virtual void on_size_allocate(Gtk::Allocation& allocation);
  virtual GtkType child_type_vfunc() const;
  virtual void on_add(Gtk::Widget* child);
  virtual void on_remove(Gtk::Widget* child);
  virtual void forall_vfunc(gboolean include_internals, GtkCallback callback, gpointer callback_data);

  
  int get_column_height(guint start_widget, guint widget_count, int& total_width);

  /** 
   * @result The height when the children are arranged optimally (so that the height is minimum).
   */
  int get_minimum_column_height(guint start_widget, guint columns_count, int& total_width);

  class FlowTableItem
  {
  public:
    Gtk::Widget* m_first;
    Gtk::Widget* m_second;
  };
  
  typedef std::vector<FlowTableItem> type_vecChildren;

  int get_item_requested_height(const FlowTableItem& item);
  void get_item_requested_width(const FlowTableItem& item, int& first, int& second);
  void get_item_max_width(guint start, int height, int& first_max_width, int& second_max_width); //TODO: maybe combine this with code in get_minimum_column_height().

  bool child_is_visible(Gtk::Widget* widget);
  void assign_child(Gtk::Widget* widget, int x, int y);
  
  type_vecChildren m_children;
  guint m_columns_count;
  guint m_padding;

};

#endif //GLOM_UTILITYWIDGETS_FLOWTABLE_H
