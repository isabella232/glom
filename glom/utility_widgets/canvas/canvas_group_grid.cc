/* Glom
 *
 * Copyright (C) 2007-2011 Murray Cumming
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

#include "canvas_group_grid.h"
#include "canvas_line_movable.h"
#include <goocanvasmm/canvas.h>
#include <cmath>
#include <iostream>

namespace Glom
{

const double LINE_WIDTH = 0.5f;

CanvasGroupGrid::CanvasGroupGrid()
: m_grid_gap(0.0),
  m_grid_sensitivity(5.0)
{
  m_grid_rules_group = Goocanvas::Group::create();
  add_child(m_grid_rules_group);

  //Create the temp rule and hide it by default:
  m_temp_rule = create_rule_line(0, true); //Arbitrary defaults.
  m_temp_rule->property_visibility() = Goocanvas::ITEM_INVISIBLE;
  add_child(m_temp_rule);
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

double CanvasGroupGrid::snap_position_rules(const type_vec_doubles& rules, double a) const
{
  double result = a;

  for(const auto& rule_a : rules)
  {
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
  const auto rules = get_vertical_rules();
  return snap_position_rules(rules, x);
}

double CanvasGroupGrid::snap_position_rules_y(double y) const
{
  const auto rules = get_horizontal_rules();
  return snap_position_rules(rules, y);
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
  double offset_x = 0;
  double offset_y = 0;
  double offset_x_min = 0;
  double offset_y_min = 0;

  if(m_grid_gap)
  {
    //Try snapping to the grid:
    double temp_x = snap_position_grid(x);
    double temp_y = snap_position_grid(y);

    offset_x = temp_x - x;
    offset_y = temp_y - y;

    //Use the smallest offset, preferring some offset to no offset:
    if(offset_x)
      offset_x_min = offset_x;

    if(offset_y)
      offset_y_min = offset_y;
  }

  //Try snapping to the rules, if any:
  double temp_x = snap_position_rules_x(x);
  double temp_y = snap_position_rules_y(y);

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

Glib::RefPtr<CanvasLineMovable> CanvasGroupGrid::create_rule_line(double pos, bool horizontal)
{
  double left = 0.0;
  double top = 0.0;
  double right = 0.0;
  double bottom = 0.0;
  auto canvas = get_canvas();
  if(canvas)
    canvas->get_bounds(left, top, right, bottom);

  auto line = CanvasLineMovable::create();

  if(horizontal)
  {
    double data[4] = {left, pos, right, pos};
    Goocanvas::Points points(2, data);
    line->property_points() = points ;
  }
  else
  {
    double data[4] = {pos, top, pos, bottom};
    Goocanvas::Points points(2, data);
    line->property_points() = points ;
  }

  line->property_line_width() = LINE_WIDTH;
  line->property_stroke_color() = "green";
  line->set_hover_color("red"); //So the user knows when he can click to drag.

  if(horizontal)
    line->set_movement_allowed(true, false);
  else
    line->set_movement_allowed(false, true);

  return line;
}

void CanvasGroupGrid::add_vertical_rule(double x)
{
  auto line = create_rule_line(x, false);
  m_rules_x.emplace_back(line);
  m_grid_rules_group->add_child(line);
}

void CanvasGroupGrid::add_horizontal_rule(double y)
{
  auto line = create_rule_line(y, true);
  m_rules_y.emplace_back(line);
  m_grid_rules_group->add_child(line);
}

void CanvasGroupGrid::remove_rules()
{
  m_rules_x.clear();
  m_rules_y.clear();

  while(m_grid_rules_group && m_grid_rules_group->get_n_children())
    m_grid_rules_group->remove_child(0);
}

CanvasGroupGrid::type_vec_doubles CanvasGroupGrid::get_horizontal_rules() const
{
  type_vec_doubles result;
  for(const auto& line : m_rules_y)
  {
    if(!line)
      continue;

    double x = 0;
    double y = 0;
    line->get_xy(x, y);
    result.emplace_back(y);
  }

  return result;
}

CanvasGroupGrid::type_vec_doubles CanvasGroupGrid::get_vertical_rules() const
{
  type_vec_doubles result;
  for(const auto& line : m_rules_x)
  {
    if(!line)
      continue;

    double x = 0;
    double y = 0;
    line->get_xy(x, y);
    result.emplace_back(x);
  }

  return result;
}

void CanvasGroupGrid::set_grid_gap(double gap)
{
  m_grid_gap = gap;

  create_grid_lines();
}

void CanvasGroupGrid::update_grid_for_new_size()
{
  create_grid_lines();
}

void CanvasGroupGrid::remove_grid()
{
  m_grid_gap = 0.0;

  create_grid_lines();
}

void CanvasGroupGrid::create_grid_lines()
{
  //Remove any existing lines:
  if(m_grid_lines)
  {
    m_grid_lines->remove();
    m_grid_lines.reset(); //Null the RefPtr.
  }

  //Fill the parent canvas with lines:
  double left = 0.0;
  double top = 0.0;
  double right = 0.0;
  double bottom = 0.0;
  auto canvas = get_canvas();
  if(canvas)
    canvas->get_bounds(left, top, right, bottom);

  const double width = right - left;
  const double height = bottom - top;

  //Vertical and horizontal grid lines:
  if(m_grid_gap > 0) //0 steps cause a crash in older versions of goocanvas.
  {
    m_grid_lines = Goocanvas::Grid::create(0, 0, width, height, m_grid_gap, m_grid_gap);
    m_grid_lines->property_horz_grid_line_width() = LINE_WIDTH;
    m_grid_lines->property_vert_grid_line_width() = LINE_WIDTH;
    m_grid_lines->property_horz_grid_line_color() = "light blue";
    m_grid_lines->property_vert_grid_line_color() = "light blue";
    add_child(m_grid_lines);
  }

  //Make sure that the grid is below the rules, so that the rules are visible:
  if(m_grid_lines && m_grid_rules_group)
    m_grid_lines->lower(m_grid_rules_group);
}

void CanvasGroupGrid::set_rules_visibility(bool visible)
{
  if(m_grid_rules_group)
    m_grid_rules_group->property_visibility() = (visible ? Goocanvas::ITEM_VISIBLE : Goocanvas::ITEM_INVISIBLE);
  else
    std::cerr << G_STRFUNC << ": m_grid_rules_group was null.\n";

  //Make sure that the gris is below the rules, so that the rules are visible:
  if(m_grid_lines && m_grid_rules_group)
    m_grid_lines->lower(m_grid_rules_group);
}

void CanvasGroupGrid::show_temp_rule(double x, double y, bool show)
{
  m_temp_rule->property_visibility() = (show ? Goocanvas::ITEM_VISIBLE : Goocanvas::ITEM_INVISIBLE);

  double left = 0.0;
  double top = 0.0;
  double right = 0.0;
  double bottom = 0.0;
  auto canvas = get_canvas();
  if(canvas)
    canvas->get_bounds(left, top, right, bottom);

  const bool horizontal = (y == 0);

  if(horizontal)
  {
    double points_coordinates[] = {x, top, x , bottom};
    Goocanvas::Points points(2, points_coordinates);
    m_temp_rule->property_points() = points;
  }
  else
  {
    double points_coordinates[] = {left, y, right , y};
    Goocanvas::Points points(2, points_coordinates);
    m_temp_rule->property_points() = points;
  }

  m_temp_rule->raise();
}


} //namespace Glom

