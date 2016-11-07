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

#ifndef GLOM_DATASTRUCTURE_PRINT_LAYOUT_H
#define GLOM_DATASTRUCTURE_PRINT_LAYOUT_H

#include <libglom/data_structure/translatable_item.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_groupby.h>
#include <glibmm/ustring.h>

namespace Glom
{

class PrintLayout : public TranslatableItem
{
public:
  PrintLayout();
  PrintLayout(const PrintLayout& src) = default;
  PrintLayout(PrintLayout&& src) = delete;
  PrintLayout& operator=(const PrintLayout& src) = default;
  PrintLayout& operator=(PrintLayout&& src) = delete;

  bool get_show_table_title() const;
  void set_show_table_title(bool show_table_title = true);

  std::shared_ptr<LayoutGroup> get_layout_group();
  std::shared_ptr<const LayoutGroup> get_layout_group() const;

  /** Sets the Page Setup as it would be created by a Gtk::PageSetup.
   */
  void set_page_setup(const std::string& page_setup);

  /** Returns the Page Setup as it would be created by a Gtk::PageSetup.
   */
  std::string get_page_setup() const;

  void set_page_count(guint count);
  guint get_page_count() const;

  bool get_show_grid() const;
  void set_show_grid(bool show_grid = true);

  bool get_show_rules() const;
  void set_show_rules(bool show_rules = true);

  bool get_show_outlines() const;
  void set_show_outlines(bool show_outlines = true);

  typedef std::vector<double> type_vec_doubles;

  /** Get the y positions of the horizontal rule lines.
   */
  type_vec_doubles get_horizontal_rules() const;
  void set_horizontal_rules(const type_vec_doubles& rules);

  /** Get the x positions of the vertical rule lines.
   */
  type_vec_doubles get_vertical_rules() const;
  void set_vertical_rules(const type_vec_doubles& rules);

private:
  std::shared_ptr<LayoutGroup> m_layout_group;
  bool m_show_table_title;

  bool m_show_grid;
  bool m_show_rules;
  bool m_show_outlines;

  std::string m_page_setup;
  guint m_page_count;

  type_vec_doubles m_horizontal_rules;
  type_vec_doubles m_vertical_rules;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_PRINT_LAYOUT_H



