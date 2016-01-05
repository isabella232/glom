/* Glom
 *
 * Copyright (C) 2012 Murray Cumming
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
 
#include <libglom/data_structure/layout/static_text.h>
#include <glibmm/i18n.h>

namespace Glom
{

StaticText::StaticText()
{
  m_translatable_item_type = enumTranslatableItemType::STATIC_TEXT;
}

StaticText::StaticText(const StaticText& src)
: TranslatableItem(src)
{
}

bool StaticText::operator==(const StaticText& src) const
{
  const auto result = TranslatableItem::operator==(src);

  return result;
}

//Avoid using this, for performance:
StaticText& StaticText::operator=(const StaticText& src)
{
  TranslatableItem::operator=(src);
  return *this;
}

} //namespace Glom
