/* Glom
 *
 * Copyright (C) 2001-2006 Murray Cumming
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

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_LINE_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_LINE_H

#include "layoutitem.h"
#include <libgdamm/value.h>

namespace Glom
{

/// This is only used on print layouts.
class LayoutItem_Line 
 : public LayoutItem
{
public:
  LayoutItem_Line();
  LayoutItem_Line(const LayoutItem_Line& src);
  LayoutItem_Line& operator=(const LayoutItem_Line& src);
  virtual ~LayoutItem_Line();

  virtual LayoutItem* clone() const;

  bool operator==(const LayoutItem_Line& src) const;

  virtual Glib::ustring get_part_type_name() const;
  virtual Glib::ustring get_report_part_id() const;

  /** Get the coordinates.
   */
  void get_coordinates(double& start_x, double& start_y, double& end_x, double& end_y) const;

  /** Set the coordinates.
   */
  void set_coordinates(double start_x, double start_y, double end_x, double end_y);

private:

  double m_start_x, m_start_y, m_end_x, m_end_y;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_LINE_H



