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

#include "canvas_group_dbtable.h"
#include "glom/utility_widgets/canvas/canvas_rect_movable.h"
#include "glom/utility_widgets/canvas/canvas_line_movable.h"
#include "glom/utility_widgets/canvas/canvas_text_movable.h"
#include <glom/appwindow.h>
#include <goocanvasmm/canvas.h>
#include <goocanvasmm/rect.h>
#include <goocanvasmm/polyline.h>
#include <goocanvasmm/text.h>
#include <math.h>
#include <iostream>
#include <goocanvasrect.h>
#include <goocanvaspolyline.h>
#include <goocanvastext.h>

namespace Glom
{

double CanvasGroupDbTable::m_table_width = 200; //TODO: Calculate based on the title text's width.
double margin = 5.0;

CanvasGroupDbTable::CanvasGroupDbTable(const Glib::ustring& table_name, const Glib::ustring& table_title, const Document::type_vec_fields& fields, double x, double y)
: m_table_height(0) 
{
  m_table_name = table_name;

  //double max_table_height = 0;
  
  const double field_height = 20;
  m_table_height = field_height * (fields.size() + 1);

  
  Glib::RefPtr<CanvasRectMovable> rect = CanvasRectMovable::create(x, y, m_table_width, m_table_height);
  rect->property_line_width() = 2.0;
  rect->property_radius_x() = 4.0,
  rect->property_radius_y() = 4.0;
  rect->property_stroke_color() = "black";
  rect->property_fill_color() = "white";
  rect->set_movement_allowed(false, false); //Move only as part of the parent group.
  add_child(rect);

  const Glib::ustring title = "<b>" + table_title + "</b>";
  Glib::RefPtr<CanvasTextMovable> text = CanvasTextMovable::create(title,
    x + margin, y + margin, m_table_width - margin*2,
    Goocanvas::ANCHOR_NORTH_WEST);
  text->property_font() = "Sans 12"; //TODO: Let the user specify this.
  text->property_use_markup() = true;
  text->set_movement_allowed(false, false); //Move only as part of the parent group.
  add_child(text);

  Glib::RefPtr<CanvasLineMovable> line = CanvasLineMovable::create();
  double points_coordinates[] = {x, y + field_height, x + m_table_width, y + field_height};
  Goocanvas::Points points(2, points_coordinates);
  line->property_points() = points;
  line->property_stroke_color() = "black";
  line->property_line_width() = 1.0;
  line->set_movement_allowed(false, false); //Move only as part of the parent group.
  add_child(line);


  //Add the table's fields:
  double field_y = field_height;
  for(const auto& field : fields)
  {
    //Show the primary key as bold:
    Glib::ustring title_field;
    if(field->get_primary_key())
      title_field = "<u>" + item_get_title_or_name(field) + "</u>";
    else
      title_field = item_get_title_or_name(field);

    Glib::RefPtr<CanvasTextMovable> text_item = CanvasTextMovable::create(title_field, 
      x + margin, y + margin + field_y, m_table_width - margin*2,
      Goocanvas::ANCHOR_NORTH_WEST);
    text_item->property_font() = "Sans 12"; //TODO: Let the user specify this.
    text_item->property_use_markup() = true;
    text_item->set_movement_allowed(false, false); //Move only as part of the parent group.
    add_child(text_item);
    
    //Remember the postion for later, for drawing relationships lines:
    m_map_fields_y[field->get_name()] = field_y;

    field_y += field_height;
  }
}

CanvasGroupDbTable::~CanvasGroupDbTable()
{
}

Glib::RefPtr<CanvasGroupDbTable> CanvasGroupDbTable::create(const Glib::ustring& table_name, const Glib::ustring& table_title, const Document::type_vec_fields& fields, double x, double y)
{
  return Glib::RefPtr<CanvasGroupDbTable>(new CanvasGroupDbTable(table_name, table_title, fields, x, y));
}

double CanvasGroupDbTable::get_table_height() const
{
  return m_table_height;
}

double CanvasGroupDbTable::get_table_width() const
{
  return m_table_width;
}

double CanvasGroupDbTable::get_field_y(const Glib::ustring& field_name) const
{
  type_map_fields_y::const_iterator iterFind = m_map_fields_y.find(field_name);
  if(iterFind !=  m_map_fields_y.end())
    return iterFind->second + 10.0; //Added an offset so that lines point approximately to the middle of the text.
  else
    return 0;
}

Glib::ustring CanvasGroupDbTable::get_table_name() const
{
  return m_table_name;
}






} //namespace Glom

