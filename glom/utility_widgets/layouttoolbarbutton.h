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
 

#ifndef GLOM_UTILITY_WIDGETS_LAYOUTTOOLBARBUTTON_H
#define GLOM_UTILITY_WIDGETS_LAYOUTTOOLBARBUTTON_H

#include <gtkmm/toolbutton.h>
#include <gtkmm/image.h>
#include <string>

#include "layoutwidgetbase.h"

namespace Glom
{

class LayoutToolbarButton : public Gtk::ToolButton
{
public:
  LayoutToolbarButton(const std::string& icon_name, LayoutWidgetBase::enumType type, const Glib::ustring& title, const Glib::ustring& tooltip);

private:

  //TODO: What is this for? murrayc.
  // We need an unique identifier for drag & drop! jhs
  static const gchar* get_target()
  {
    return "glom_print_layout_palette";
  };
  
private:
  LayoutWidgetBase::enumType m_type;
};

}
#endif //GLOM_UTILITY_WIDGETS_LAYOUTTOOLBARBUTTON_H
