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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_LINE_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_LINE_H

#include <libglom/data_structure/layout/layoutitem.h>
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
  LayoutItem_Line(LayoutItem_Line&& src) = delete;
  LayoutItem_Line& operator=(const LayoutItem_Line& src);
  LayoutItem_Line& operator=(LayoutItem_Line&& src) = delete;
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
  
  double get_line_width() const;
  void set_line_width(double line_width);
  
  /** Get the line color in CSS3 format.
   */
  Glib::ustring get_line_color() const;

  /** Set the line color in CSS3 format.
   */
  void set_line_color(const Glib::ustring& color);

private:

  double m_start_x, m_start_y, m_end_x, m_end_y;
  double m_line_width;
  Glib::ustring m_color;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_LINE_H



