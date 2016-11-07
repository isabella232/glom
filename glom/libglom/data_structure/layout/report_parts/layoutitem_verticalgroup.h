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

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_VERTICALGROUP_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_VERTICALGROUP_H

#include <libglom/data_structure/layout/layoutgroup.h>
#include <libglom/data_structure/field.h>

namespace Glom
{

/** The child items are arranged vertically in a row on a report.
 */
class LayoutItem_VerticalGroup
: public LayoutGroup
{
public:

  LayoutItem_VerticalGroup();
  LayoutItem_VerticalGroup(const LayoutItem_VerticalGroup& src) = default;
  LayoutItem_VerticalGroup(LayoutItem_VerticalGroup&& src) = delete;
  LayoutItem_VerticalGroup& operator=(const LayoutItem_VerticalGroup& src) = default;
  LayoutItem_VerticalGroup& operator=(LayoutItem_VerticalGroup&& src) = delete;

  LayoutItem* clone() const override;

  Glib::ustring get_part_type_name() const override;
  Glib::ustring get_report_part_id() const override;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_VERTICALGROUP_H



