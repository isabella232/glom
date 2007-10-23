/* Glom
 *
 * Copyright (C) 2006 Murray Cumming
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

#include "layoutitem_text.h"
#include <glibmm/i18n.h>

namespace Glom
{

LayoutItem_Text::LayoutItem_Text()
{
  m_translatable_item_type = TRANSLATABLE_TYPE_TEXTOBJECT;
  m_text = sharedptr<TranslatableItem>::create();
}

LayoutItem_Text::LayoutItem_Text(const LayoutItem_Text& src)
: LayoutItem(src),
  m_text(src.m_text)
{
}

LayoutItem_Text::~LayoutItem_Text()
{
}

LayoutItem* LayoutItem_Text::clone() const
{
  return new LayoutItem_Text(*this);
}

bool LayoutItem_Text::operator==(const LayoutItem_Text& src) const
{
  bool result = LayoutItem::operator==(src) && 
                (*m_text == *(src.m_text));

  return result;
}

//Avoid using this, for performance:
LayoutItem_Text& LayoutItem_Text::operator=(const LayoutItem_Text& src)
{
  LayoutItem::operator=(src);

  m_text = src.m_text;

  return *this;
}

Glib::ustring LayoutItem_Text::get_part_type_name() const
{
  return _("Text");
}

Glib::ustring LayoutItem_Text::get_report_part_id() const
{
  return "field"; //We reuse this for this node.
}

Glib::ustring LayoutItem_Text::get_text() const
{
  return m_text->get_title();
}

void LayoutItem_Text::set_text(const Glib::ustring& text)
{
  m_text->set_title(text);
}


} //namespace Glom

