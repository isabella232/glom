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

#include "printoperation_printlayout.h"
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
}

bool PrintOperationPrintLayout::on_paginate(const Glib::RefPtr<Gtk::PrintContext>& print_context)
{
  std::cout << "PrintOperationPrintLayout::on_paginate" << std::endl;

  set_n_pages(1); //on_draw_page() will be called for any new pages.

  //Call base class:
  Gtk::PrintOperation::on_paginate(print_context);

  return true; //Pagination has finished. Don't call this again.
}


void PrintOperationPrintLayout::on_draw_page(
        const Glib::RefPtr<Gtk::PrintContext>& print_context, int page_nr)
{
  if(!m_canvas)
    return;

  //Get a Cairo Context, which is used as a drawing board:
  m_canvas->hide_page_bounds();
  Cairo::RefPtr<Cairo::Context> cairo_context = print_context->get_cairo_context();


  //Render the canvas onto the cairo context:
  if(m_canvas)
    m_canvas->render(cairo_context);

  //Call base class:
  Gtk::PrintOperation::on_draw_page(print_context, page_nr);
}

void PrintOperationPrintLayout::set_canvas(Canvas_PrintLayout* canvas)
{
  m_canvas = canvas;
}

} //namespace Glom

