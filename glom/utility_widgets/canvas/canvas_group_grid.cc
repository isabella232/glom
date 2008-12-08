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

#include "canvas_group_grid.h"
#include <goocanvasmm/canvas.h>
#include <math.h>
#include <iostream>
#include <goocanvasgroup.h>

namespace Glom
{


CanvasGroupGrid::CanvasGroupGrid()
: m_grid_gap(0.0),
  m_grid_sensitivity(5.0)
{
  m_grid_rules_group = Goocanvas::Group::create();
  add_child(m_grid_rules_group);
}

CanvasGroupGrid::~CanvasGroupGrid()
{
}

Glib::RefPtr<CanvasGroupGrid> CanvasGroupGrid::create()
{
  return Glib::RefPtr<CanvasGroupGrid>(new CanvasGroupGrid());
}

inline void division_and_remainder(double a, double b, double& whole, double& remainder)
{
  if(b == 0)
  {
    whole = 0;
    remainder = 0;
    return;
  }

  remainder = fmod(a, b);
  whole = (int)(a / b);
}

bool CanvasGroupGrid::is_close(double a, double b) const
{
  return (std::abs((long)(a - b)) < m_grid_sensitivity);

}

double CanvasGroupGrid::snap_position_rules(const type_vec_double& rules, double a) const
{
  double result = a;

  for(type_vec_double::const_iterator iter = rules.begin(); iter != rules.end(); ++iter)
  {
    const double rule_a = *iter;
    if(is_close(a, rule_a))
    {
      if(result == a) //Prefer some snap to no snap
        result = rule_a; 
      else if(std::abs((long)(a - rule_a)) < std::abs((long)(a - result))) //Use the closest one.
        result = rule_a;
    }
  }


  return result;
}

double CanvasGroupGrid::snap_position_rules_x(double x) const
{
  return snap_position_rules(m_rules_x, x);
}

double CanvasGroupGrid::snap_position_rules_y(double y) const
{
  return snap_position_rules(m_rules_y, y);
}

double CanvasGroupGrid::snap_position_grid(double a) const
{
  double result = a;

  if(m_grid_gap)
  {
    /* Get closest horizontal grid line: */
    double grid_line_num_before = 0;
    double distance_after_grid_line_before = 0;
    division_and_remainder(a, m_grid_gap, grid_line_num_before, distance_after_grid_line_before);
    //printf("grid_line_num_before=%f, distance_after_grid_line_before=%f\n", grid_line_num_before, distance_after_grid_line_before);
    
    if(is_close(0, distance_after_grid_line_before))
    {
      //Snap to the grid line:
      result = grid_line_num_before * m_grid_gap; 
    }
    else
    {
      const double distance_to_next_grid_line = m_grid_gap - distance_after_grid_line_before;
      if(is_close(m_grid_gap, distance_to_next_grid_line))
      {
        //Snap to the grid line:
        result = (grid_line_num_before + 1) * m_grid_gap; 
      }
    }
  }

  return result;
}

void CanvasGroupGrid::snap_position(double& x, double& y) const
{
  //printf("%s: x=%f, y=%f\n", __FUNCTION__, x, y);
  if(m_grid_gap)
  {
    double offset_x_min = 0;
    double offset_y_min = 0;

    //Try snapping to the grid:
    double temp_x = snap_position_grid(x);
    double temp_y = snap_position_grid(y);
   
    double offset_x = temp_x - x;
    double offset_y = temp_y - y;

    //Use the smallest offset, preferring some offset to no offset:
    if(offset_x)
      offset_x_min = offset_x;

    if(offset_y)
      offset_y_min = offset_y;

    //Try snapping to the rules:
    temp_x = snap_position_rules_x(x);
    temp_y = snap_position_rules_y(y);

    offset_x = temp_x - x;
    offset_y = temp_y - y;

    //Use the smallest offset, preferring some offset to no offset:
    if(offset_x && ((std::abs((long)offset_x) < std::abs((long)offset_x_min)) || !offset_x_min))
      offset_x_min = offset_x;

    if(offset_y && ((std::abs((long)offset_y) < std::abs((long)offset_y_min)) || !offset_y_min))
      offset_y_min = offset_y;

    x += offset_x_min;
    y += offset_y_min;
  }
}

Glib::RefPtr<Goocanvas::Polyline> CanvasGroupGrid::create_grid_or_rule_line(double x1, double y1, double x2, double y2, bool is_rule)
{
  Glib::RefPtr<Goocanvas::Polyline> line = Goocanvas::Polyline::create(x1, y1, x2, y2);
  line->property_line_width() = 1.0f;

  if(is_rule)
    line->property_stroke_color() = "green";
  else
    line->property_stroke_color() = "gray";

  return line;
}

void CanvasGroupGrid::add_vertical_rule(double x)
{
  m_rules_x.push_back(x);
}

void CanvasGroupGrid::add_horizontal_rule(double y)
{
  m_rules_y.push_back(y);
}

void CanvasGroupGrid::set_grid_gap(double gap)
{
  m_grid_gap = gap;

  create_lines();
}

void CanvasGroupGrid::remove_grid()
{
  m_grid_gap = 0.0;

  create_lines();
}

void CanvasGroupGrid::create_lines()
{
  //Remove any existing lines:
  if(m_grid_lines)
  {
    m_grid_lines->remove();
    m_grid_lines.clear(); //Null the RefPtr.
  }

  while(m_grid_rules_group && m_grid_rules_group->get_n_children())
    m_grid_rules_group->remove_child(0);

  //Fill the parent canvas with lines:
  double left, top, right, bottom = 0.0;
  Goocanvas::Canvas* canvas = get_canvas();
  if(canvas)
    canvas->get_bounds(left, top, right, bottom);
 
  const double width = right - left;
  const double height = bottom - top;
  
  //Vertical and horizontal grid lines:
  if(m_grid_gap > 0) //0 steps cause a crash in older versions of goocanvas.
  {
    m_grid_lines = Goocanvas::Grid::create(0, 0, width, height, m_grid_gap, m_grid_gap);
    m_grid_lines->property_horz_grid_line_width() = 1.0f;
    m_grid_lines->property_vert_grid_line_width() = 1.0f;
    m_grid_lines->property_horz_grid_line_color() = "gray";
    m_grid_lines->property_vert_grid_line_color() = "gray";
    add_child(m_grid_lines);
  }

  //Vertical rules:
  for(CanvasGroupGrid::type_vec_double::const_iterator iter = m_rules_x.begin(); iter != m_rules_x.end(); ++iter)
  {
    const double x = *iter;
    Glib::RefPtr<Goocanvas::Polyline> line = create_grid_or_rule_line(x, top, x, bottom, true);
    m_grid_rules_group->add_child(line);
  }

  //Horizontal rules:
  for(CanvasGroupGrid::type_vec_double::const_iterator iter = m_rules_y.begin(); iter != m_rules_y.end(); ++iter)
  {
    const double y = *iter;
    Glib::RefPtr<Goocanvas::Polyline> line = create_grid_or_rule_line(left, y, right, y, true);
    m_grid_rules_group->add_child(line);
  }
}



} //namespace Glom

