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
 
#include "layoutitem_field.h"

LayoutItem_Field::LayoutItem_Field()
{
}

LayoutItem_Field::LayoutItem_Field(const LayoutItem_Field& src)
: LayoutItem(src),
  m_field(src.m_field)
{
}

LayoutItem_Field::~LayoutItem_Field()
{
}

LayoutItem* LayoutItem_Field::clone() const
{
  return new LayoutItem_Field(*this);
}


LayoutItem_Field& LayoutItem_Field::operator=(const LayoutItem_Field& src)
{
  LayoutItem::operator=(src);

  m_field = src.m_field;
  
  return *this;
}


