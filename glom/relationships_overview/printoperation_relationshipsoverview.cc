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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "printoperation_relationshipsoverview.h"

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
        const Glib::RefPtr<Gtk::PrintContext>& print_context)
{
  set_n_pages(1);
}

void PrintOperationRelationshipsOverview::on_draw_page(
        const Glib::RefPtr<Gtk::PrintContext>& print_context, int page_nr)
{
  //Get a Cairo Context, which is used as a drawing board:
  Cairo::RefPtr<Cairo::Context> cairo_context = print_context->get_cairo_context();
  
  //Render the canvas onto the cairo context:
  if(m_canvas)
    m_canvas->render(cairo_context);

  //Scale the cairo context down so that it fits on the page:
  const double print_height = print_context->get_height();
  const double print_width = print_context->get_width();
  //TODO: What are the dimensions of the cairo context (the area where there are drawings)?
  
  double scale_x = 0.5;
  double scale_y = 0.5;

  //TODO: Doesn't work:
  cairo_context->scale(scale_x, scale_y);
}

void PrintOperationRelationshipsOverview::set_canvas(Goocanvas::Canvas* canvas)
{
  m_canvas = canvas;
}

