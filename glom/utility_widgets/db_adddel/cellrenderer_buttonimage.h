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

#ifndef GLOM_CELLRENDERER_BUTTONIMAGE_H
#define GLOM_CELLRENDERER_BUTTONIMAGE_H

#include <gtkmm/cellrendererpixbuf.h>
#include <gtkmm/treepath.h>
#include <gtkmm/treemodel.h>
#include <gtk/gtkcellrenderer.h>

namespace Glom
{

class GlomCellRenderer_ButtonImage : public Gtk::CellRendererPixbuf
{
public: 
  GlomCellRenderer_ButtonImage();
  virtual ~GlomCellRenderer_ButtonImage();

  typedef sigc::signal<void, const Gtk::TreeModel::Path&> type_signal_clicked;
  type_signal_clicked signal_clicked();
  
private:
#ifndef GLIBMM_VFUNCS_ENABLED
  static gboolean activate_impl(GtkCellRenderer* cell, GdkEvent* event, GtkWidget* widget, const gchar* path, GdkRectangle* background_area, GdkRectangle* cell_area, GtkCellRendererState flags);
#endif

  virtual bool activate_vfunc(GdkEvent* event, Gtk::Widget& widget, const Glib::ustring& path, const Gdk::Rectangle& background_area, const Gdk::Rectangle& cell_area, Gtk::CellRendererState flags);

  type_signal_clicked m_signal_clicked;
};

} //namespace Glom

#endif //GLOM_CELLRENDERER_BUTTONIMAGE_H
