/* Glom
 *
 * Copyright (C) 2001-2005 Murray Cumming
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

#include "cellrenderer_button.h"
#include <gtkmm/stock.h>

namespace Glom
{

GlomCellRenderer_Button::GlomCellRenderer_Button()
{
  const Gtk::StockID stock_id = Gtk::Stock::OPEN;
  property_stock_id() = stock_id.get_string();

  property_mode() = Gtk::CELL_RENDERER_MODE_ACTIVATABLE; //So that it calls activate_vfunc().
}

GlomCellRenderer_Button::~GlomCellRenderer_Button()
{}

GlomCellRenderer_Button::type_signal_clicked GlomCellRenderer_Button::signal_clicked()
{
  return m_signal_clicked;
}

bool GlomCellRenderer_Button::activate_vfunc(GdkEvent* event, Gtk::Widget& widget, const Glib::ustring& path, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, Gtk::CellRendererState flags)
{
  //TODO: It would be nice to depress this like a real button.

  //Call base class:
  bool result = CellRendererPixbuf::activate_vfunc(event, widget, path, background_area, cell_area, flags);

  m_signal_clicked.emit( Gtk::TreeModel::Path(path) );

  return result;
}


} //namespace Glom


