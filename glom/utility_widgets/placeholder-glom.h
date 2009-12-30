/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * glom
 * Copyright (C) Johannes Schmid 2007 <jhs@gnome.org>
 * 
 * glom is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * glom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with glom.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include <gtkmm.h>
#include "layoutwidgetbase.h"
//#include <libglom/data_structure/layout/layoutitem_button.h>
//#include <gtkmm/builder.h>

#ifndef GLOM_UTILITY_WIDGETS_PLACEHOLDER_GLOM_H_
#define GLOM_UTILITY_WIDGETS_PLACEHOLDER_GLOM_H_

namespace Glom
{

class PlaceholderGlom: 
  public Gtk::Frame,
  public LayoutWidgetBase
{
public:
  //explicit PlaceholderGlom(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  explicit PlaceholderGlom();
  virtual ~PlaceholderGlom();

private:
  virtual App_Glom* get_application();
};

} // namespace Glom

#endif // GLOM_UTILITY_WIDGETS_PLACEHOLDER_GLOM_H_

