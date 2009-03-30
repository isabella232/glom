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

#ifndef GLOM_DATASTRUCTURE_PRINT_LAYOUT_H
#define GLOM_DATASTRUCTURE_PRINT_LAYOUT_H

#include "translatable_item.h"
#include "layout/report_parts/layoutitem_groupby.h"
#include <glibmm/ustring.h>

namespace Glom
{

class PrintLayout : public TranslatableItem
{
public:
  PrintLayout();
  PrintLayout(const PrintLayout& src);
  PrintLayout& operator=(const PrintLayout& src);

  bool get_show_table_title() const;
  void set_show_table_title(bool show_table_title = true);

  sharedptr<LayoutGroup> m_layout_group;

  /** Sets the Page Setup as it would be created by a Gtk::PageSetup.
   */
  void set_page_setup(const std::string& page_setup);

  /** Returns the Page Setup as it would be created by a Gtk::PageSetup.   
   */
  std::string get_page_setup() const;

private:
  bool m_show_table_title;

  std::string m_page_setup;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_PRINT_LAYOUT_H



