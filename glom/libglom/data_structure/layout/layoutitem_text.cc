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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include <libglom/data_structure/layout/layoutitem_text.h>
#include <glibmm/i18n.h>

namespace Glom
{

LayoutItem_Text::LayoutItem_Text()
{
  m_translatable_item_type = TRANSLATABLE_TYPE_TEXTOBJECT;
  m_text = sharedptr<StaticText>::create(); //TODO: Why use a smartpointer?
}

LayoutItem_Text::LayoutItem_Text(const LayoutItem_Text& src)
: LayoutItem_WithFormatting(src)
{
  //Copy the underlying TranslatableItem, not the sharedptr to it:
  const StaticText& src_item = *(src.m_text);
  m_text = sharedptr<StaticText>(new StaticText(src_item));
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
  bool result = LayoutItem_WithFormatting::operator==(src) && 
                (*m_text == *(src.m_text));

  return result;
}

//Avoid using this, for performance:
LayoutItem_Text& LayoutItem_Text::operator=(const LayoutItem_Text& src)
{
  LayoutItem_WithFormatting::operator=(src);

  //Copy the underlying TranslatableItem, not the shardptr to it:
  const StaticText& src_item = *(src.m_text);
  m_text = sharedptr<StaticText>(new StaticText(src_item));

  return *this;
}

Glib::ustring LayoutItem_Text::get_part_type_name() const
{
  //Translators: This is the name of a UI element (a layout part name).
  return _("Text");
}

Glib::ustring LayoutItem_Text::get_report_part_id() const
{
  return "field"; //We reuse this for this node.
}

Glib::ustring LayoutItem_Text::get_text(const Glib::ustring& locale) const
{
  return m_text->get_title(locale);
}

void LayoutItem_Text::set_text(const Glib::ustring& text, const Glib::ustring& locale)
{
  m_text->set_title(text, locale);
}

void LayoutItem_Text::set_text_original(const Glib::ustring& text)
{
  m_text->set_title_original(text);
}

} //namespace Glom

