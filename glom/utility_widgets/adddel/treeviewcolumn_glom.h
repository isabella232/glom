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

#ifndef GLOM_UTLITY_WIDGETS_ADDDEL_TREEVIEWCOLUMN_GLOM_H
#define GLOM_UTLITY_WIDGETS_ADDDEL_TREEVIEWCOLUMN_GLOM_H


#include <gtkmm/treeviewcolumn.h>

namespace Glom
{
class TreeViewColumnGlom : public Gtk::TreeViewColumn
{
public:
  TreeViewColumnGlom(const Glib::ustring& title, Gtk::CellRenderer& cell);
  virtual ~TreeViewColumnGlom();

  Glib::ustring get_column_id() const;
  void set_column_id(const Glib::ustring& value);

private:
  Glib::ustring m_column_id; 
};

} //namespace Glom

#endif //ADDDEL_TREEVIEWCOLUMN_GLOM_H
