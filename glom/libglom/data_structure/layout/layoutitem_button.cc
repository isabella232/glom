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

#include <libglom/libglom_config.h>
#include <libglom/data_structure/layout/layoutitem_button.h>
#include <glibmm/i18n-lib.h>

namespace Glom
{

LayoutItem_Button::LayoutItem_Button()
{
  m_translatable_item_type = enumTranslatableItemType::BUTTON;
}

LayoutItem* LayoutItem_Button::clone() const
{
  return new LayoutItem_Button(*this);
}

bool LayoutItem_Button::operator==(const LayoutItem_Button& src) const
{
  auto result = LayoutItem_WithFormatting::operator==(src) &&
                (m_script == src.m_script);

  return result;
}

Glib::ustring LayoutItem_Button::get_part_type_name() const
{
  //Translators: This is the name of a UI element (a layout part name).
  return _("Button");
}

Glib::ustring LayoutItem_Button::get_script() const
{
  return m_script;
}

bool LayoutItem_Button::get_has_script() const
{
  return !(m_script.empty());
}

void LayoutItem_Button::set_script(const Glib::ustring& script)
{
  m_script = script;
}

} //namespace Glom
