/* Glom
 *
 * Copyright (C) 2007, 2008 Openismus GmbH
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
 
#include "print_layout_toolbar_button.h"
#include <gtk/gtktoolpalette.h>
#include <iostream>

namespace
{

std::string get_icon_path(const std::string& filename)
{
#ifdef G_OS_WIN32
  gchar* basepath = g_win32_get_package_installation_directory_of_module(0);
  const std::string result = Glib::build_filename(Glib::build_filename(basepath,
      "share" G_DIR_SEPARATOR_S "glom" G_DIR_SEPARATOR_S "pixmaps"), filename);
  g_free(basepath);
  return result;
#else
  return Glib::build_filename(GLOM_PKGDATADIR G_DIR_SEPARATOR_S "pixmaps", filename);
#endif
}

} //anonymous namespace


namespace Glom
{

PrintLayoutToolbarButton::PrintLayoutToolbarButton(const std::string& icon_name, enumItems type,
                                         const Glib::ustring& title, const Glib::ustring& tooltip)
: Gtk::ToolButton( *(Gtk::manage (new Gtk::Image(get_icon_path(icon_name)))) )
{
  m_type = type;
  g_object_set_data(G_OBJECT(gobj()), "glom-type", GINT_TO_POINTER(type));

  std::list<Gtk::TargetEntry> targetentries;
  targetentries.push_back(Gtk::TargetEntry(get_target()));

  drag_source_set(targetentries, Gdk::MODIFIER_MASK, 
                  Gdk::ACTION_COPY | Gdk::ACTION_MOVE);
  set_tooltip_text(tooltip);
  set_label(title);
}

PrintLayoutToolbarButton::~PrintLayoutToolbarButton()
{
}

PrintLayoutToolbarButton::enumItems PrintLayoutToolbarButton::get_item_type_from_selection_data(const Glib::RefPtr<Gdk::DragContext>& drag_context, const Gtk::SelectionData& selection_data)
{
  PrintLayoutToolbarButton::enumItems result = ITEM_INVALID;

  //Put this code in the toolbar class:
  Gtk::Widget* palette = drag_get_source_widget(drag_context);
  while(palette && !GTK_IS_TOOL_PALETTE (palette->gobj()))
    palette = palette->get_parent();
  
  if(!palette)
    return result;

  GtkWidget* tool_item = gtk_tool_palette_get_drag_item(GTK_TOOL_PALETTE (palette->gobj()), selection_data.gobj());
  if(!tool_item)
    return result;

  result = static_cast<PrintLayoutToolbarButton::enumItems>(GPOINTER_TO_INT(g_object_get_data(G_OBJECT(tool_item), "glom-type")));
  return result;
}

void PrintLayoutToolbarButton::on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>&, 
                                  Gtk::SelectionData& selection_data, guint, guint)
{
  selection_data.set(8, (guint8*)(&m_type), 4);
}

void PrintLayoutToolbarButton::on_drag_begin(const Glib::RefPtr<Gdk::DragContext>& drag_context)
{
  drag_context->set_icon(dynamic_cast<Gtk::Image*>(get_icon_widget())->get_pixbuf(), 0, 0);
}

} // namespace Glom

