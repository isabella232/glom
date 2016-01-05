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

#ifndef GLOM_RELATIONSHIPS_OVERVIEW_PRINT_OPERATION_H
#define GLOM_RELATIONSHIPS_OVERVIEW_PRINT_OPERATION_H

#include <gtkmm/printoperation.h>
#include <goocanvasmm/canvas.h>
#include <vector>

namespace Glom
{

//We derive our own class from PrintOperation,
//so we can put the actual print implementation here.
class PrintOperationRelationshipsOverview : public Gtk::PrintOperation
{
public:
  static Glib::RefPtr<PrintOperationRelationshipsOverview> create();

  void set_canvas(Goocanvas::Canvas* canvas);

private:
  PrintOperationRelationshipsOverview();

  //PrintOperation default signal handler overrides:
  void on_begin_print(const Glib::RefPtr<Gtk::PrintContext>& context) override;
  void on_draw_page(const Glib::RefPtr<Gtk::PrintContext>& context, int page_nr) override;

  //Not owned by this instance:
  Goocanvas::Canvas* m_canvas;
};

} //namespace Glom

#endif // GLOM_RELATIONSHIPS_OVERVIEW_PRINT_OPERATION_H
