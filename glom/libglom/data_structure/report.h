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

#ifndef GLOM_DATASTRUCTURE_REPORT_H
#define GLOM_DATASTRUCTURE_REPORT_H

#include <libglom/data_structure/translatable_item.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_groupby.h>
#include <glibmm/ustring.h>

namespace Glom
{

class Report : public TranslatableItem
{
public:
  Report();
  Report(const Report& src) = default;
  Report(Report&& src) = delete;
  Report& operator=(const Report& src) = default;
  Report& operator=(Report&& src) = delete;

  bool get_show_table_title() const;
  void set_show_table_title(bool show_table_title = true);

  std::shared_ptr<LayoutGroup> get_layout_group();
  std::shared_ptr<const LayoutGroup> get_layout_group() const;

private:
  std::shared_ptr<LayoutGroup> m_layout_group;
  bool m_show_table_title;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_REPORT_H



