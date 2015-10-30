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

#ifndef GLOM_UTILITYWIDGETS_FLOWTABLE_H
#define GLOM_UTILITYWIDGETS_FLOWTABLE_H

#include <glom/utility_widgets/eggspreadtablemm/eggspreadtabledndmm.h>
#include "layoutwidgetbase.h"
#include <gtkmm/box.h>

namespace Glom
{

class FlowTable : public Egg::SpreadTableDnd
{
public:
  FlowTable();
  virtual ~FlowTable();

  typedef Gtk::Container type_base;

  void add_widgets(Gtk::Widget& first, Gtk::Widget& second, bool expand_second = false);
  void add_widgets(Gtk::Widget& first, bool expand = false);
 
  /** Show extra UI that is useful in RAD tools:
   */
  virtual void set_design_mode(bool value = true);

  void remove(Gtk::Widget& first);
  void remove_all();

protected:

  /** Get the column in which the specified "first" widget is placed.
   * result false if the widget is not one of the "first" widgets, or
   * if has not yet been placed in a column, because the size has not yet been requested.
   */
  bool get_column_for_first_widget(const Gtk::Widget& first, guint& column) const;

  void insert(Gtk::Widget* first, Gtk::Widget* second, int index, bool expand);

  typedef std::list<const Gtk::Widget*> type_const_list_widgets;

  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

private:

  const Gtk::Box* get_parent_hbox(const Gtk::Widget* first) const;
  void delete_and_forget_hbox(Gtk::Box* hbox);

  bool m_design_mode;

  //For drawing:
  Glib::RefPtr<Gdk::Window> m_refGdkWindow;

  //We remember the Boxes so we can delete them when the are no longer used.
  typedef std::list<Gtk::Box*> type_list_hboxes;
  type_list_hboxes m_list_hboxes;

  type_const_list_widgets m_list_first_widgets;
};

} //namespace Glom

#endif //GLOM_UTILITYWIDGETS_FLOWTABLE_H
