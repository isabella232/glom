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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */
 
#include "print_layout_toolbar_button.h"
#include <glom/utils_ui.h>
#include <libglom/utils.h>
#include <gtkmm/toolpalette.h>
#include <iostream>

namespace Glom
{

PrintLayoutToolbarButton::PrintLayoutToolbarButton(const std::string& icon_name, enumItems type,
                                         const Glib::ustring& title, const Glib::ustring& tooltip)
: Gtk::ToolButton()
{
  Gtk::Image* image = Gtk::manage (new Gtk::Image());

  const auto resource_path = UiUtils::get_icon_path(icon_name);
  if(!Utils::get_resource_exists(resource_path))
  {
    std::cerr << G_STRFUNC << ": icon resource not found: " << resource_path << std::endl;
  }

  image->set_from_resource(resource_path);
  set_icon_widget(*image);


  m_type = type;
  g_object_set_data(G_OBJECT(gobj()), "glom-type", GINT_TO_POINTER(type));

  std::vector<Gtk::TargetEntry> targetentries;
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
  PrintLayoutToolbarButton::enumItems result = enumItems::INVALID;

  //Put this code in the toolbar class:
  Gtk::Widget* palette_candidate = drag_get_source_widget(drag_context);
  Gtk::ToolPalette* palette = dynamic_cast<Gtk::ToolPalette*>(palette_candidate);
  while(palette_candidate && !palette) {
    palette_candidate = palette_candidate->get_parent();
    palette = dynamic_cast<Gtk::ToolPalette*>(palette_candidate);
  }
  
  if(!palette)
    return result;

  Gtk::Widget* tool_item = palette->get_drag_item(selection_data);
  if(!tool_item)
    return result;

  result = static_cast<PrintLayoutToolbarButton::enumItems>(GPOINTER_TO_INT(tool_item->get_data("glom-type")));
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

