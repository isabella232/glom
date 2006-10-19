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

#include "layoutitem_button.h"
#include <glibmm/i18n.h>

namespace Glom
{

LayoutItem_Button::LayoutItem_Button()
{
  m_translatable_item_type = TRANSLATABLE_TYPE_BUTTON;
}

LayoutItem_Button::LayoutItem_Button(const LayoutItem_Button& src)
: LayoutItem(src),
  m_script(src.m_script)
{
}

LayoutItem_Button::~LayoutItem_Button()
{
}

LayoutItem* LayoutItem_Button::clone() const
{
  return new LayoutItem_Button(*this);
}

bool LayoutItem_Button::operator==(const LayoutItem_Button& src) const
{
  bool result = LayoutItem::operator==(src) && 
                (m_script == src.m_script);

  return result;
}

//Avoid using this, for performance:
LayoutItem_Button& LayoutItem_Button::operator=(const LayoutItem_Button& src)
{
  LayoutItem::operator=(src);

  m_script = src.m_script;

  return *this;
}

Glib::ustring LayoutItem_Button::get_part_type_name() const
{
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
