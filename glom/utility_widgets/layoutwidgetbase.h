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

#ifndef GLOM_MODE_DATA_LAYOUT_WIDGET_BASE_H
#define GLOM_MODE_DATA_LAYOUT_WIDGET_BASE_H

#include "../data_structure/layout/layoutitem.h"
#include <sigc++/sigc++.h>

class LayoutWidgetBase
{
public: 
  LayoutWidgetBase();
  virtual ~LayoutWidgetBase();

  ///Takes ownership.
  void set_layout_item(LayoutItem* layout_item);

  //The caller should call clone().
  const LayoutItem* get_layout_item() const;
  LayoutItem* get_layout_item();

  typedef sigc::signal<void> type_signal_layout_changed;
  type_signal_layout_changed signal_layout_changed();

protected:
  LayoutItem* m_pLayoutItem;

  type_signal_layout_changed m_signal_layout_changed;

};

#endif //GLOM_MODE_DATA_LAYOUT_WIDGET_BASE_H
