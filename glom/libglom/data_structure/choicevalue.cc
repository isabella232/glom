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

#include <libglom/data_structure/choicevalue.h>

namespace Glom
{

ChoiceValue::ChoiceValue()
{
  m_translatable_item_type = enumTranslatableItemType::CHOICEVALUE;
}

ChoiceValue::ChoiceValue(const ChoiceValue& src)
: TranslatableItem(src)
{
  //TODO_Performance: Implement this properly, without the extra copy.
  operator=(src);
}

ChoiceValue& ChoiceValue::operator=(const ChoiceValue& src)
{
  TranslatableItem::operator=(src);

  m_value = src.m_value;

  return *this;
}

bool ChoiceValue::operator==(const ChoiceValue& src) const
{
  return TranslatableItem::operator==(src)
         && (m_value == src.m_value);
}

bool ChoiceValue::operator!=(const ChoiceValue& src) const
{
  return !(operator==(src));
}

ChoiceValue* ChoiceValue::clone() const
{
  return new ChoiceValue(*this);
}

void ChoiceValue::set_value(const Gnome::Gda::Value& value)
{
  m_value = value;
}

Gnome::Gda::Value ChoiceValue::get_value() const
{
  return m_value;
}

Glib::ustring ChoiceValue::get_title_original() const noexcept
{
  return m_value.to_string();
}

bool ChoiceValue::is_translatable() const
{
  if(m_value.get_value_type() == G_TYPE_STRING)
    return true;

  return false;
}


} //namespace Glom
