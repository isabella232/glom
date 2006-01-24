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

#include "translatable_item.h"
#include <glibmm/i18n.h>

TranslatableItem::TranslatableItem()
: m_translatable_item_type(TRANSLATABLE_TYPE_INVALID)
{
}

TranslatableItem::TranslatableItem(const TranslatableItem& src)
: m_translatable_item_type(src.m_translatable_item_type),
  m_name(src.m_name),
  m_map_translations(src.m_map_translations)
{
}

TranslatableItem::~TranslatableItem()
{
}

TranslatableItem& TranslatableItem::operator=(const TranslatableItem& src)
{
  m_name = src.m_name;
  m_translatable_item_type = src.m_translatable_item_type;
  m_map_translations = src.m_map_translations;

  return *this;
}

bool TranslatableItem::operator==(const TranslatableItem& src) const
{
  bool bResult = (m_name == src.m_name);
  bResult == bResult &&  (m_map_translations == src.m_map_translations);
  bResult == bResult && (m_translatable_item_type == src.m_translatable_item_type);

  return bResult;
}

bool TranslatableItem::operator!=(const TranslatableItem& src) const
{
  return !(operator==(src));
}

void TranslatableItem::set_translation(const Glib::ustring& locale, const Glib::ustring& translation)
{
  m_map_translations[locale] = translation;
}

Glib::ustring TranslatableItem::get_translation(const Glib::ustring& locale) const
{
  type_map_locale_to_translations::const_iterator iterFind = m_map_translations.find(locale);
  if(iterFind != m_map_translations.end())
    return iterFind->second;
  else
    return Glib::ustring();
}

const TranslatableItem::type_map_locale_to_translations& TranslatableItem::_get_translations_map() const
{
  return m_map_translations;
}

bool TranslatableItem::get_has_translations() const
{
  return !m_map_translations.empty();
}


Glib::ustring TranslatableItem::get_title() const
{
  const Glib::ustring translated_title = get_translation(get_current_locale());
  if(!translated_title.empty())
    return translated_title;
  else if(!m_title.empty())
    return m_title; //The original, if there is no translation.
  else if(m_map_translations.empty())
  {
    return Glib::ustring();
  }
  else
  {
    //return the first translation, if any.
    //This would be quite unusual.
    type_map_locale_to_translations::const_iterator iter = m_map_translations.begin();
    return iter->second;
  }
}

Glib::ustring TranslatableItem::get_title(const Glib::ustring& locale) const
{
  return get_translation(locale);
}


Glib::ustring TranslatableItem::get_title_original() const
{
  return m_title;
}

void TranslatableItem::set_title(const Glib::ustring& title)
{
  const Glib::ustring the_locale = get_current_locale();
  if(the_locale.empty())
    set_title_original(title);
  else
    set_translation(get_current_locale(), title);
}

void TranslatableItem::set_title(const Glib::ustring& locale, Glib::ustring& title)
{
  set_translation(locale, title);
}

void TranslatableItem::set_title_original(const Glib::ustring& title)
{
  m_title = title;
}

Glib::ustring TranslatableItem::get_current_locale()
{
  return Glib::ustring();
}

TranslatableItem::enumTranslatableItemType TranslatableItem::get_translatable_item_type()
{
  return m_translatable_item_type;
}

Glib::ustring TranslatableItem::get_translatable_type_name(enumTranslatableItemType item_type)
{
  if(item_type == TRANSLATABLE_TYPE_FIELD)
    return _("Field");
  else if(item_type == TRANSLATABLE_TYPE_RELATIONSHIP)
    return _("Relationship");
  else if(item_type == TRANSLATABLE_TYPE_RELATIONSHIP)
    return _("Layout Item");
 else if(item_type == TRANSLATABLE_TYPE_REPORT)
    return _("Report");
 else if(item_type == TRANSLATABLE_TYPE_TABLE)
    return _("Table");
 else
    return _("Unknown");
}

void TranslatableItem::set_name(const Glib::ustring& name)
{
  m_name = name;
}

Glib::ustring TranslatableItem::get_name() const
{
  return m_name;
}

bool TranslatableItem::get_name_not_empty() const
{
  return !(get_name().empty());
}

Glib::ustring TranslatableItem::get_title_or_name() const
{
  const Glib::ustring title = get_title();
  if(title.empty())
    return get_name();
  else
    return title;
}

