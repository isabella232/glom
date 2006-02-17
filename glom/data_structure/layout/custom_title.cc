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
 
#include "custom_title.h"
#include <glibmm/i18n.h>

CustomTitle::CustomTitle()
{
  m_translatable_item_type = TRANSLATABLE_TYPE_CUSTOM_TITLE;
}

CustomTitle::CustomTitle(const CustomTitle& src)
: TranslatableItem(src)
{
}

CustomTitle::~CustomTitle()
{
}

bool CustomTitle::operator==(const CustomTitle& src) const
{
  bool result = TranslatableItem::operator==(src);

  return result;
}

//Avoid using this, for performance:
CustomTitle& CustomTitle::operator=(const CustomTitle& src)
{
  TranslatableItem::operator=(src);

  return *this;
}

