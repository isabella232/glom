/* Glom
 *
 * Copyright (C) 2009 Murray Cumming
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

#include "layoutitem_withformatting.h"
#include <glibmm/i18n.h>

namespace Glom
{

LayoutItem_WithFormatting::LayoutItem_WithFormatting()
{
}

LayoutItem_WithFormatting::LayoutItem_WithFormatting(const LayoutItem_WithFormatting& src)
: LayoutItem(src),
  m_formatting(src.m_formatting)
{
}

LayoutItem_WithFormatting::~LayoutItem_WithFormatting()
{
}

bool LayoutItem_WithFormatting::operator==(const LayoutItem_WithFormatting& src) const
{
  bool result = LayoutItem::operator==(src) && 
                (m_formatting == src.m_formatting);

  return result;
}

//Avoid using this, for performance:
LayoutItem_WithFormatting& LayoutItem_WithFormatting::operator=(const LayoutItem_WithFormatting& src)
{
  LayoutItem::operator=(src);

  m_formatting = src.m_formatting;

  return *this;
}

const Formatting& LayoutItem_WithFormatting::get_formatting_used() const
{
  return m_formatting;
}

Formatting::HorizontalAlignment LayoutItem_WithFormatting::get_formatting_used_horizontal_alignment(bool /* for_details_view */) const
{
  const Formatting& format = get_formatting_used();
  Formatting::HorizontalAlignment alignment = 
    format.get_horizontal_alignment();
  
  if(alignment == Formatting::HORIZONTAL_ALIGNMENT_AUTO)
    alignment = Formatting::HORIZONTAL_ALIGNMENT_LEFT;
  
  return alignment;
}


} //namespace Glom

