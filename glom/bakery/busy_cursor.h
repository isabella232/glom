/*
 * Copyright 2000 Murray Cumming
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef GLOM_BAKERY_UTILITIES_BUSYCURSOR_H
#define GLOM_BAKERY_UTILITIES_BUSYCURSOR_H

#include <gtkmm/window.h>
#include <gdkmm/cursor.h>
#include <map>

namespace Glom
{

/** Changes the cursor for as long as this instance lives.
 * For instance, put it at the start of code in a { and } block.
 */
class BusyCursor
{
public:
  /** Associate a busy cursor with the window, for the lifetime of this object.
   */
  explicit BusyCursor(Gtk::Window& window, Gdk::CursorType cursor_type = Gdk::WATCH);

  /**  Associate a busy cursor with the window, for the lifetime of this object, if window is not 0.
   */
  explicit BusyCursor(Gtk::Window* window, Gdk::CursorType cursor_type = Gdk::WATCH);

  virtual ~BusyCursor();

private:

  void force_gui_update();

  Glib::RefPtr<Gdk::Cursor> m_cursor;
  Gtk::Window* m_window;
  Glib::RefPtr<Gdk::Window> m_gdk_window;

  typedef std::map<Gtk::Window*, Glib::RefPtr<Gdk::Cursor> > type_map_cursors;
  static type_map_cursors m_map_cursors;
  Glib::RefPtr<Gdk::Cursor> m_old_cursor;
};

} //namespace Glom

#endif //BAKERY_UTILITIES_BUSYCURSOR_H
