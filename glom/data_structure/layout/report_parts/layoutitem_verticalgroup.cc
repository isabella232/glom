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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
 
#include "layoutitem_verticalgroup.h"
#include <glibmm/i18n.h>

LayoutItem_VerticalGroup::LayoutItem_VerticalGroup()
{
}

LayoutItem_VerticalGroup::LayoutItem_VerticalGroup(const LayoutItem_VerticalGroup& src)
: LayoutGroup(src)
{
}

LayoutItem_VerticalGroup::~LayoutItem_VerticalGroup()
{
}

LayoutItem* LayoutItem_VerticalGroup::clone() const
{
  return new LayoutItem_VerticalGroup(*this);
}


LayoutItem_VerticalGroup& LayoutItem_VerticalGroup::operator=(const LayoutItem_VerticalGroup& src)
{
  LayoutGroup::operator=(src);

  return *this;
}

Glib::ustring LayoutItem_VerticalGroup::get_part_type_name() const
{
  return _("Vertical Group");
}

