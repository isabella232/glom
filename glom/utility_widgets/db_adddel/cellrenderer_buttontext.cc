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

#include "cellrenderer_buttontext.h"
#include <gtk/gtkcellrenderertext.h>

#ifndef GLIBMM_VFUNCS_ENABLED
namespace
{
  GtkCellRendererTextClass* parent_class;
}
#endif

namespace Glom
{

#ifndef GLIBMM_VFUNCS_ENABLED
  gboolean GlomCellRenderer_ButtonText::activate_impl(GtkCellRenderer* cell, GdkEvent* event, GtkWidget* widget, const gchar* path, GdkRectangle* background_area, GdkRectangle* cell_area, GtkCellRendererState flags)
  {
    Glib::ObjectBase* const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)cell));
    if(obj_base)
    {
      GlomCellRenderer_ButtonText* const obj = dynamic_cast<GlomCellRenderer_ButtonText*>(obj_base);
      if(obj)
      {
	return obj->activate_vfunc(event, *Glib::wrap(widget), path, Glib::wrap(background_area), Glib::wrap(cell_area), static_cast<Gtk::CellRendererState>(flags));
      }
    }
    else if(GTK_CELL_RENDERER_CLASS(parent_class)->activate)
      return GTK_CELL_RENDERER_CLASS(parent_class)->activate(cell, event, widget, path, background_area, cell_area, flags);
    else
      return FALSE;
  }
#endif
} // namespace Glom

namespace Glom
{

GlomCellRenderer_ButtonText::GlomCellRenderer_ButtonText():
  Glib::ObjectBase("GlomCellRenderer_ButtonText") // Create a new GType for us
{
#ifndef GLIBMM_VFUNCS_ENABLED
  if(parent_class == NULL)
  {
    GtkCellRendererClass* klass = GTK_CELL_RENDERER_GET_CLASS(gobj());
    klass->activate = &GlomCellRenderer_ButtonText::activate_impl;
    parent_class = GTK_CELL_RENDERER_TEXT_CLASS(g_type_class_peek_parent(klass));
  }
#endif

  //const Gtk::StockID stock_id = Gtk::Stock::OPEN; //A default.
  //property_stock_id() = stock_id.get_string();

  set_property("mode", Gtk::CELL_RENDERER_MODE_ACTIVATABLE); //So that it calls activate_vfunc().
}

GlomCellRenderer_ButtonText::~GlomCellRenderer_ButtonText()
{}

GlomCellRenderer_ButtonText::type_signal_clicked GlomCellRenderer_ButtonText::signal_clicked()
{
  return m_signal_clicked;
}

bool GlomCellRenderer_ButtonText::activate_vfunc(GdkEvent* event, Gtk::Widget& widget, const Glib::ustring& path, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, Gtk::CellRendererState flags)
{
  //TODO: It would be nice to depress this like a real button.

  //Call base class:
  bool result = false;
#ifdef GLIBMM_VFUNCS_ENABLED
  CellRendererText::activate_vfunc(event, widget, path, background_area, cell_area, flags);
#else
  if(GTK_CELL_RENDERER_CLASS(parent_class)->activate)
    result = GTK_CELL_RENDERER_CLASS(parent_class)->activate(GTK_CELL_RENDERER(gobj()), event, widget.gobj(), path.c_str(), const_cast<GdkRectangle*>(background_area.gobj()), const_cast<GdkRectangle*>(cell_area.gobj()), static_cast<GtkCellRendererState>(flags));
#endif

  m_signal_clicked.emit( Gtk::TreeModel::Path(path) );

  return result;
}

} //namespace Glom


