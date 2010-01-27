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

#include <libglom/data_structure/print_layout.h>
#include <gtk/gtkpagesetup.h> //For gtk_page_setup_copy().

namespace Glom
{

PrintLayout::PrintLayout()
: m_show_table_title(true)
{
  m_translatable_item_type = TRANSLATABLE_TYPE_PRINT_LAYOUT;
  m_layout_group = sharedptr<LayoutGroup>::create();
}

PrintLayout::PrintLayout(const PrintLayout& src)
: TranslatableItem(src),
  m_layout_group(src.m_layout_group),
  m_show_table_title(src.m_show_table_title)
{
  m_page_setup = src.m_page_setup;
}

PrintLayout& PrintLayout::operator=(const PrintLayout& src)
{
  TranslatableItem::operator=(src);

  m_layout_group = src.m_layout_group;
  m_show_table_title = src.m_show_table_title;
  m_page_setup = src.m_page_setup;
  
  return *this;
}

bool PrintLayout::get_show_table_title() const
{
  return m_show_table_title;
}

void PrintLayout::set_show_table_title(bool show_table_title)
{
  m_show_table_title = show_table_title;
}

void PrintLayout::set_page_setup(const std::string& page_setup)
{
  m_page_setup = page_setup;
}

std::string PrintLayout::get_page_setup() const
{
  return m_page_setup;
}

} //namespace Glom

