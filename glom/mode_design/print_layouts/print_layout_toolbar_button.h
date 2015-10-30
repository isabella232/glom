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
 

#ifndef GLOM_PRINT_LAYOUT_TOOLBAR_BUTTON_H
#define GLOM_PRINT_LAYOUT_TOOLBAR_BUTTON_H

#include <gtkmm/toolbutton.h>
#include <gtkmm/image.h>
#include <string>

//#include "layoutwidgetbase.h"

namespace Glom
{

class PrintLayoutToolbarButton : public Gtk::ToolButton
{
public:

  //TODO: Use LayoutWidgetBase::enumType m_type instead (and just use LayoutToolbarButton?)
  enum class enumItems
  {
    INVALID,
    FIELD,
    TEXT,
    IMAGE,
    PORTAL,
    LINE_HORIZONTAL,
    LINE_VERTICAL
  };

  PrintLayoutToolbarButton(const std::string& icon_name, enumItems type, const Glib::ustring& title, const Glib::ustring& tooltip);
  virtual ~PrintLayoutToolbarButton();

  static enumItems get_item_type_from_selection_data(const Glib::RefPtr<Gdk::DragContext>& drag_context, const Gtk::SelectionData& selection_data);

private:

  //TODO: What is this for? murrayc.
  // We need an unique identifier for drag & drop! jhs
  static const gchar* get_target()
  {
    return "flowtable";
  };
  
private:
  enumItems m_type;
};

}
#endif //GLOM_PRINT_LAYOUT_TOOLBAR_BUTTON_H

