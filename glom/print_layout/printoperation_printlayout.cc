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

#include <glom/print_layout/printoperation_printlayout.h>
#include <iostream>

namespace Glom
{

PrintOperationPrintLayout::PrintOperationPrintLayout()
: m_canvas(0)
{
  set_unit(Gtk::UNIT_MM);
  set_use_full_page(true); //Because we show the margins on our canvas.

  set_n_pages(1); //There is always at least one page.
}

PrintOperationPrintLayout::~PrintOperationPrintLayout()
{
}

Glib::RefPtr<PrintOperationPrintLayout> PrintOperationPrintLayout::create()
{
  return Glib::RefPtr<PrintOperationPrintLayout>(new PrintOperationPrintLayout());
}

void PrintOperationPrintLayout::on_begin_print(
        const Glib::RefPtr<Gtk::PrintContext>& print_context)
{
  //Call base class:
  Gtk::PrintOperation::on_begin_print(print_context);
  
  set_n_pages( m_canvas->get_page_count() );
  //std::cout << G_STRFUNC << ": n pages =" <<  m_canvas->get_page_count() << std::endl;
}

bool PrintOperationPrintLayout::on_paginate(const Glib::RefPtr<Gtk::PrintContext>& print_context)
{
  //std::cout << "PrintOperationPrintLayout::on_paginate" << std::endl;

  if(!m_canvas)
    return false;

  //on_draw_page() will be called for any new pages.
  set_n_pages( m_canvas->get_page_count() );
  //std::cout << G_STRFUNC << ": n pages =" <<  m_canvas->get_page_count() << std::endl;

  //Call base class:
  Gtk::PrintOperation::on_paginate(print_context);

  return true; //Pagination has finished. Don't call this again.
}


void PrintOperationPrintLayout::on_draw_page(
  const Glib::RefPtr<Gtk::PrintContext>& print_context, int page_nr)
{
  //Note that page_nr is 0-based, so the first page is page 0.

  if(!m_canvas)
    return;

  //Get a Cairo Context, which is used as a drawing board:
  m_canvas->hide_page_bounds();
  Cairo::RefPtr<Cairo::Context> cairo_context = print_context->get_cairo_context();


  //Render the canvas onto the cairo context:
  const auto bounds = m_canvas->get_page_bounds(page_nr);
  //std::cout << G_STRFUNC << ": page_nr=" << page_nr << ", bounds: x1=" << bounds.get_x1() << ", y1=" << bounds.get_y1() << ", x2=" << bounds.get_x2() << ", y2=" << bounds.get_y2() << std::endl;

  //Shift the renderer context up into the page:
  cairo_context->translate(0, - bounds.get_y1());
  
  m_canvas->render(cairo_context, bounds);

  //Call base class:
  Gtk::PrintOperation::on_draw_page(print_context, page_nr);
}

void PrintOperationPrintLayout::set_canvas(Canvas_PrintLayout* canvas)
{
  m_canvas = canvas;
}

} //namespace Glom

