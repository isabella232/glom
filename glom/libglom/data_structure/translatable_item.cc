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

#include <libglom/data_structure/translatable_item.h>
#include <libglom/utils.h>
#include <glibmm/i18n.h>
#include <iostream>

namespace Glom
{

TranslatableItem::TranslatableItem()
: m_translatable_item_type(TRANSLATABLE_TYPE_INVALID)
{
}

TranslatableItem::TranslatableItem(const TranslatableItem& src)
: m_translatable_item_type(src.m_translatable_item_type),
  m_name(src.m_name),
  m_title(src.m_title),
  m_map_translations(src.m_map_translations)
{
}

TranslatableItem::TranslatableItem(TranslatableItem&& src)
: m_translatable_item_type(std::move(src.m_translatable_item_type)),
  m_name(std::move(src.m_name)),
  m_title(std::move(src.m_title)),
  m_map_translations(std::move(src.m_map_translations))
{
}

TranslatableItem::~TranslatableItem()
{
}

TranslatableItem& TranslatableItem::operator=(const TranslatableItem& src)
{
  m_name = src.m_name;
  m_title = src.m_title;
  m_translatable_item_type = src.m_translatable_item_type;
  m_map_translations = src.m_map_translations;

  return *this;
}

TranslatableItem& TranslatableItem::operator=(TranslatableItem&& src)
{
  m_name = std::move(src.m_name);
  m_title = std::move(src.m_title);
  m_translatable_item_type = std::move(src.m_translatable_item_type);
  m_map_translations = std::move(src.m_map_translations);

  return *this;
}

bool TranslatableItem::operator==(const TranslatableItem& src) const
{
  bool bResult = (m_name == src.m_name)
                 && (m_title == src.m_title)
                 && (m_translatable_item_type == src.m_translatable_item_type)
                 && (m_map_translations == src.m_map_translations);

  return bResult;
}

bool TranslatableItem::operator!=(const TranslatableItem& src) const
{
  return !(operator==(src));
}

void TranslatableItem::set_title_translation(const Glib::ustring& locale, const Glib::ustring& translation)
{
  if(translation.empty())
  {
    //Remove it from the map, to save space:
    auto iterFind = m_map_translations.find(locale);
    if(iterFind != m_map_translations.end())
      m_map_translations.erase(iterFind);
  }
  else
    m_map_translations[locale] = translation;
}

Glib::ustring TranslatableItem::get_title_translation(const Glib::ustring& locale, bool fallback) const
{
  auto iterFind = m_map_translations.find(locale);
  if(iterFind != m_map_translations.end())
    return iterFind->second;

  if(!fallback)
    return Glib::ustring();

  if(m_map_translations.empty())
    return get_title_original();

  //Return the first translation from a locale with the same language, if any.
  //TODO_Performance: This is slow.
  //Note that this would, for instance, give en_GB translations before en_US translations, if there are no en_AU translations.
  const auto locale_language_id = Utils::locale_language_id(locale);
  for(const auto& the_pair : m_map_translations)
  {
    const auto locale_id = the_pair.first;
    if(Utils::locale_language_id(locale_id) == locale_language_id)
    {
      if(!(the_pair.second.empty()))
        return the_pair.second;
    }
  }

  //Fall back to the original title:
  const auto title_original = get_title_original();
  if(!title_original.empty())
    return title_original;

  //Fall back to first translation, if any.
  //This would be quite unusual.
  auto iter = m_map_translations.begin();
  if(iter != m_map_translations.end())
  {
    //std::cout << "debug: TranslatableItem::get_title() falling back to the first translation: locale=" << iter->first << std::endl;
    return iter->second;
  }

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


Glib::ustring TranslatableItem::get_title(const Glib::ustring& locale) const
{
  if(!locale.empty())
  {
    const auto translated_title = get_title_translation(locale, true /* fallback */);
    if(!translated_title.empty())
      return translated_title;
  }

  return get_title_original();
}


Glib::ustring TranslatableItem::get_title_original() const
{
  return m_title;
}

void TranslatableItem::set_title(const Glib::ustring& title, const Glib::ustring& locale)
{
  if(locale.empty())
  {
    set_title_original(title);
    return;
  }

  set_title_translation(locale, title);
}

void TranslatableItem::set_title_original(const Glib::ustring& title)
{
  m_title = title;
}

//TODO: Make this virtual and handle it in ChoiceValue?
void TranslatableItem::clear_title_in_all_locales()
{
  m_title.clear();
  
  for(const auto& the_pair : m_map_translations)
  {
    auto translation = the_pair.second;
    translation.clear();
  }
}

TranslatableItem::enumTranslatableItemType TranslatableItem::get_translatable_item_type() const
{
  return m_translatable_item_type;
}

Glib::ustring TranslatableItem::get_translatable_type_name_nontranslated(enumTranslatableItemType item_type)
{
  //TODO: Is there an easier way to do this, without duplicating code?

  if(item_type == TRANSLATABLE_TYPE_FIELD)
    return "Field";
  else if(item_type == TRANSLATABLE_TYPE_CUSTOM_TITLE)
    return "Custom Title";
  else if(item_type == TRANSLATABLE_TYPE_RELATIONSHIP)
    return "Relationship";
  else if(item_type == TRANSLATABLE_TYPE_RELATIONSHIP)
    return "Layout Item";
  else if(item_type == TRANSLATABLE_TYPE_PRINT_LAYOUT)
    return "Print Layout";
  else if(item_type == TRANSLATABLE_TYPE_REPORT)
    return "Report";
  else if(item_type == TRANSLATABLE_TYPE_TABLE)
    return "Table";
  else if(item_type == TRANSLATABLE_TYPE_LAYOUT_ITEM)
    return "Layout Group";
  else if(item_type == TRANSLATABLE_TYPE_BUTTON)
    return "Button";
  else if(item_type == TRANSLATABLE_TYPE_TEXTOBJECT)
    return "Text Item";
  else if(item_type == TRANSLATABLE_TYPE_IMAGEOBJECT)
    return "Image";
  else if(item_type == TRANSLATABLE_TYPE_CHOICEVALUE)
    return "Field Choice";
  else if(item_type == TRANSLATABLE_TYPE_DATABASE_TITLE)
    return "Database Title";
  else if(item_type == TRANSLATABLE_TYPE_STATIC_TEXT)
    return "Text";
  else
    return "Unknown";
}

Glib::ustring TranslatableItem::get_translatable_type_name(enumTranslatableItemType item_type)
{
  if(item_type == TRANSLATABLE_TYPE_FIELD)
    return _("Field");
  else if(item_type == TRANSLATABLE_TYPE_CUSTOM_TITLE)
    return _("Custom Title");
  else if(item_type == TRANSLATABLE_TYPE_RELATIONSHIP)
    return _("Relationship");
  else if(item_type == TRANSLATABLE_TYPE_RELATIONSHIP)
    return _("Layout Item");
  else if(item_type == TRANSLATABLE_TYPE_PRINT_LAYOUT)
    return _("Print Layout");
  else if(item_type == TRANSLATABLE_TYPE_REPORT)
    return _("Report");
  else if(item_type == TRANSLATABLE_TYPE_TABLE)
    return _("Table");
  else if(item_type == TRANSLATABLE_TYPE_LAYOUT_ITEM)
    return _("Layout Group");
  else if(item_type == TRANSLATABLE_TYPE_BUTTON)
    return _("Button");
  else if(item_type == TRANSLATABLE_TYPE_TEXTOBJECT)
    return _("Text Item");
  else if(item_type == TRANSLATABLE_TYPE_IMAGEOBJECT)
    return _("Image");
  else if(item_type == TRANSLATABLE_TYPE_CHOICEVALUE)
    return _("Field Choice");
  else if(item_type == TRANSLATABLE_TYPE_DATABASE_TITLE)
    return _("Database Title");
  else if(item_type == TRANSLATABLE_TYPE_STATIC_TEXT)
    return _("Text");
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

Glib::ustring TranslatableItem::get_title_or_name(const Glib::ustring& locale) const
{
  const auto title = get_title(locale);
  if(title.empty())
    return get_name();
  else
    return title;
}

} //namespace Glom
