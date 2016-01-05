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
 
#include <libglom/data_structure/layout/custom_title.h>
#include <glibmm/i18n.h>

namespace Glom
{

CustomTitle::CustomTitle()
: m_use_custom_title(false)
{
  m_translatable_item_type = enumTranslatableItemType::CUSTOM_TITLE;
}

CustomTitle::CustomTitle(const CustomTitle& src)
: TranslatableItem(src),
  m_use_custom_title(src.m_use_custom_title)
{
}

bool CustomTitle::operator==(const CustomTitle& src) const
{
  const auto result = TranslatableItem::operator==(src) &&
                      (m_use_custom_title == src.m_use_custom_title);

  return result;
}

//Avoid using this, for performance:
CustomTitle& CustomTitle::operator=(const CustomTitle& src)
{
  TranslatableItem::operator=(src);

  m_use_custom_title = src.m_use_custom_title;

  return *this;
}


bool CustomTitle::get_use_custom_title() const
{
  return m_use_custom_title;
}

void CustomTitle::set_use_custom_title(bool use_custom_title)
{
  m_use_custom_title = use_custom_title;
}

} //namespace Glom
