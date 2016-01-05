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
 *   The Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor
 *   Boston, MA  02110-1301, USA.
 */

#ifndef GLOM_LAYOUTITEM_PLACEHOLDER_H
#define GLOM_LAYOUTITEM_PLACEHOLDER_H

#include <libglom/data_structure/layout/layoutitem.h>

namespace Glom
{

class LayoutItem_Placeholder: public LayoutItem 
{
public:
  LayoutItem_Placeholder();
  
  LayoutItem_Placeholder(const LayoutItem_Placeholder& src);
  LayoutItem_Placeholder(LayoutItem_Placeholder&& src) = delete;

  //TODO: Add operator=().
  
  /** Create a new copied instance.
  * This allows us to deep-copy a list of LayoutItems.
  */
  LayoutItem* clone() const override;
  
  Glib::ustring get_part_type_name() const override;
  Glib::ustring get_report_part_id() const override;
  
  bool operator==(const LayoutItem_Placeholder* src) const;
};

}

#endif // GLOM_LAYOUTITEM_PLACEHOLDER_H
