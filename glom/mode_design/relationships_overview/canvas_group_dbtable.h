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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_GROUP_DBTABLE_H
#define GLOM_UTILITY_WIDGETS_CANVAS_GROUP_DBTABLE_H

#include "glom/utility_widgets/canvas/canvas_group_movable.h"
#include <goocanvasmm/polyline.h>
#include <libglom/document/document.h>

namespace Glom
{

class CanvasGroupDbTable : public CanvasGroupMovable
{
private:
  CanvasGroupDbTable(const Glib::ustring& table_name, const Glib::ustring& table_title, const Document::type_vec_fields& fields, double x = 0.0, double y = 0.0);
  virtual ~CanvasGroupDbTable();

public:
  static Glib::RefPtr<CanvasGroupDbTable> create(const Glib::ustring& table_name, const Glib::ustring& table_title, const Document::type_vec_fields& fields, double x = 0.0, double y = 0.0);
 
  //TODO: Use bounds instead?
  double get_table_height() const;
  double get_table_width() const;

  Glib::ustring get_table_name() const;
  double get_field_y(const Glib::ustring& field_name) const;

private:
   Glib::ustring m_table_name;
   double m_table_height;
   static double m_table_width;

   typedef std::map <Glib::ustring, double> type_map_fields_y;
   type_map_fields_y m_map_fields_y;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_GROUP_DBTABLE_H

