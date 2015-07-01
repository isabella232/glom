/* gtkmm example Copyright (C) 2006 gtkmm development team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "printoperation_relationshipsoverview.h"
#include <iostream>

namespace Glom
{

PrintOperationRelationshipsOverview::PrintOperationRelationshipsOverview()
: m_canvas(0)
{
}

PrintOperationRelationshipsOverview::~PrintOperationRelationshipsOverview()
{
}

Glib::RefPtr<PrintOperationRelationshipsOverview> PrintOperationRelationshipsOverview::create()
{
  return Glib::RefPtr<PrintOperationRelationshipsOverview>(new PrintOperationRelationshipsOverview());
}

void PrintOperationRelationshipsOverview::on_begin_print(
        const Glib::RefPtr<Gtk::PrintContext>& /* print_context */)
{
  set_n_pages(1);
}

void PrintOperationRelationshipsOverview::on_draw_page(
        const Glib::RefPtr<Gtk::PrintContext>& print_context, int /* page_nr */)
{
  if(!m_canvas)
    return;

  //Get a Cairo Context, which is used as a drawing board:
  Cairo::RefPtr<Cairo::Context> cairo_context = print_context->get_cairo_context();
  
  //Set a drawing scale (before drawing) so that the cairo context fits on the page:
  const auto print_height = print_context->get_height();
  const auto print_width = print_context->get_width();
  //std::cout << "print_height=" << print_height << ", print_width=" << print_width << std::endl;

  //TODO: Get the total size of the drawn objects instead of the bounds (which includes extra whitespace): 
  double canvas_left = 0;
  double canvas_top = 0;
  double canvas_right = 0;
  double canvas_bottom = 0;

  Glib::RefPtr<Goocanvas::Item> root_item = m_canvas->get_root_item();
  if(!root_item)
    return;

  const auto bounds = root_item->get_bounds();
  canvas_left = bounds.get_x1();
  canvas_right = bounds.get_x2();
  canvas_top = bounds.get_y1();
  canvas_bottom = bounds.get_y2();

  std::cout << "canvas_left=" << canvas_left << ", canvas_top=" << canvas_top << ", canvas_right=" << canvas_right << ", canvas_bottom=" << canvas_bottom << std::endl;

  const double canvas_height = (canvas_bottom - canvas_top);
  const double canvas_width = (canvas_right - canvas_left);
  std::cout << "canvas_height=" << canvas_height << ", canvas_width=" << canvas_width << std::endl;

  double scale_x = 1.0;
  double scale_y = 1.0;
  if(canvas_width)
    scale_x = print_width / canvas_width;
  if(canvas_height)
    scale_y = print_height / canvas_height;

  std::cout << "scale_x=" << scale_x << ", scale_y=" << scale_y << std::endl;
  scale_x = std::min(scale_x, scale_y);
  scale_y = scale_x;

  cairo_context->scale(scale_x, scale_y);

  //Render the canvas onto the cairo context:
  if(m_canvas)
    m_canvas->render(cairo_context);
}

void PrintOperationRelationshipsOverview::set_canvas(Goocanvas::Canvas* canvas)
{
  m_canvas = canvas;
}

} //namespace Glom


