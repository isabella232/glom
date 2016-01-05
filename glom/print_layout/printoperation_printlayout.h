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

#ifndef GLOM_PRINT_OPERATION_PRINT_LAYOUT_H
#define GLOM_PRINT_OPERATION_PRINT_LAYOUT_H

#include <glom/print_layout/canvas_print_layout.h>
#include <gtkmm/printoperation.h>
#include <vector>

namespace Glom
{

//We derive our own class from PrintOperation,
//so we can put the actual print implementation here.
class PrintOperationPrintLayout : public Gtk::PrintOperation
{
public:
  static Glib::RefPtr<PrintOperationPrintLayout> create();

  void set_canvas(Canvas_PrintLayout* canvas);

private:
  PrintOperationPrintLayout();

  //PrintOperation default signal handler overrides:
  bool on_paginate(const Glib::RefPtr<Gtk::PrintContext>& context) override; //Comment this out if GTK+ bug #345345 has not been fixed yet.
  void on_begin_print(const Glib::RefPtr<Gtk::PrintContext>& context) override;
  void on_draw_page(const Glib::RefPtr<Gtk::PrintContext>& context, int page_nr) override;

  //Not owned by this instance:
  Canvas_PrintLayout* m_canvas;
};

} //namespace Glom


#endif // GLOM_PRINT_OPERATION_PRINT_LAYOUT_H
