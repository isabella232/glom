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

#include <libglom/libglom_config.h>
#include <libglom/data_structure/layout/layoutitem_placeholder.h>
#include <glibmm/i18n-lib.h>

namespace Glom
{

LayoutItem_Placeholder::LayoutItem_Placeholder()
{

}

LayoutItem_Placeholder::LayoutItem_Placeholder(const LayoutItem_Placeholder& src) :
  LayoutItem(src)
{

}

LayoutItem* LayoutItem_Placeholder::clone() const
{
  return new LayoutItem_Placeholder(*this);
}

bool LayoutItem_Placeholder::operator==(const LayoutItem_Placeholder* src) const
{
  return LayoutItem::operator==(*src);
}

Glib::ustring LayoutItem_Placeholder::get_part_type_name() const
{
  //Translators: This is the name of a UI element (a layout part name).
  return _("Placeholder");
}

Glib::ustring LayoutItem_Placeholder::get_report_part_id() const
{
  return "placeholder";
}

}
