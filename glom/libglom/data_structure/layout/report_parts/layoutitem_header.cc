/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
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
 
#include <libglom/data_structure/layout/report_parts/layoutitem_header.h>
#include <glibmm/i18n.h>

namespace Glom
{

LayoutItem_Header::LayoutItem_Header()
{
}

LayoutItem_Header::LayoutItem_Header(const LayoutItem_Header& src)
: LayoutGroup(src)
{
}

LayoutItem* LayoutItem_Header::clone() const
{
  return new LayoutItem_Header(*this);
}


LayoutItem_Header& LayoutItem_Header::operator=(const LayoutItem_Header& src)
{
  LayoutGroup::operator=(src);

  return *this;
}

Glib::ustring LayoutItem_Header::get_part_type_name() const
{
  //Translators: This is the name of a UI element (a layout part name).
  return _("Header");
}

Glib::ustring LayoutItem_Header::get_report_part_id() const
{
  return "header";
}

} //namespace Glom

