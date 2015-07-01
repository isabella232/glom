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

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_FOOTER_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_FOOTER_H

#include <libglom/data_structure/layout/layoutgroup.h>
#include <libglom/data_structure/field.h>

namespace Glom
{

/** 
 */
class LayoutItem_Footer
: public LayoutGroup
{
public:

  LayoutItem_Footer();
  LayoutItem_Footer(const LayoutItem_Footer& src);
  LayoutItem_Footer(LayoutItem_Footer&& src) = delete;
  LayoutItem_Footer& operator=(const LayoutItem_Footer& src);
  LayoutItem_Footer& operator=(LayoutItem_Footer&& src) = delete;
  virtual ~LayoutItem_Footer();

  virtual LayoutItem* clone() const;

  virtual Glib::ustring get_part_type_name() const;
  virtual Glib::ustring get_report_part_id() const;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_FOOTER_H



