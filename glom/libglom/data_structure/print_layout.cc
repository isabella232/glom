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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include <libglom/data_structure/print_layout.h>

namespace Glom
{

PrintLayout::PrintLayout()
: m_show_table_title(true),
  m_show_grid(true),
  m_show_rules(true),
  m_show_outlines(true),
  m_page_count(1) //A sensible default
{
  m_translatable_item_type = enumTranslatableItemType::PRINT_LAYOUT;
  m_layout_group = std::make_shared<LayoutGroup>();
}

PrintLayout::PrintLayout(const PrintLayout& src)
: TranslatableItem(src),
  m_layout_group(src.m_layout_group),
  m_show_table_title(src.m_show_table_title),
  m_show_grid(src.m_show_grid),
  m_show_rules(src.m_show_rules),
  m_show_outlines(src.m_show_outlines),
  m_page_count(src.m_page_count)
{
  m_page_setup = src.m_page_setup;
  m_horizontal_rules = src.m_horizontal_rules;
  m_vertical_rules = src.m_vertical_rules;
}

PrintLayout& PrintLayout::operator=(const PrintLayout& src)
{
  TranslatableItem::operator=(src);

  m_layout_group = src.m_layout_group;
  m_show_table_title = src.m_show_table_title;
  m_show_grid = src.m_show_grid;
  m_show_rules = src.m_show_rules;
  m_show_outlines = src.m_show_outlines;
  m_page_setup = src.m_page_setup;
  m_page_count = src.m_page_count;
  m_horizontal_rules = src.m_horizontal_rules;
  m_vertical_rules = src.m_vertical_rules;
  
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

std::shared_ptr<LayoutGroup> PrintLayout::get_layout_group()
{
  return m_layout_group;
}

std::shared_ptr<const LayoutGroup> PrintLayout::get_layout_group() const
{
  return m_layout_group;
}

void PrintLayout::set_page_setup(const std::string& page_setup)
{
  m_page_setup = page_setup;
}

std::string PrintLayout::get_page_setup() const
{
  return m_page_setup;
}

void PrintLayout::set_page_count(guint count)
{
  m_page_count = count;
}

guint PrintLayout::get_page_count() const
{
  return m_page_count;
}

bool PrintLayout::get_show_grid() const
{
  return m_show_grid;
}

void PrintLayout::set_show_grid(bool show_grid)
{
  m_show_grid = show_grid;
}

bool PrintLayout::get_show_rules() const
{
  return m_show_rules;
}

void PrintLayout::set_show_rules(bool show_rules)
{
  m_show_rules = show_rules;
}

bool PrintLayout::get_show_outlines() const
{
  return m_show_outlines;
}

void PrintLayout::set_show_outlines(bool show_outlines)
{
  m_show_outlines = show_outlines;
}

PrintLayout::type_vec_doubles PrintLayout::get_horizontal_rules() const
{
  return m_horizontal_rules;
}

void PrintLayout::set_horizontal_rules(const type_vec_doubles& rules)
{
  m_horizontal_rules = rules;
}

PrintLayout::type_vec_doubles PrintLayout::get_vertical_rules() const
{
  return m_vertical_rules;
}

void PrintLayout::set_vertical_rules(const type_vec_doubles& rules)
{
  m_vertical_rules = rules;
}

} //namespace Glom

