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

#include <libglom/data_structure/translatable_item.h>
#include <libglom/utils.h>
#include <glibmm/i18n.h>
#include <iostream>

namespace Glom
{

Glib::ustring TranslatableItem::m_current_locale;
Glib::ustring TranslatableItem::m_original_locale;

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
    type_map_locale_to_translations::iterator iterFind = m_map_translations.find(locale);
    if(iterFind != m_map_translations.end())
      m_map_translations.erase(iterFind);
  }
  else
    m_map_translations[locale] = translation;
}

Glib::ustring TranslatableItem::get_title_translation(const Glib::ustring& locale, bool fallback) const
{
  type_map_locale_to_translations::const_iterator iterFind = m_map_translations.find(locale);
  if(iterFind != m_map_translations.end())
    return iterFind->second;
 
  //The original is not in m_map_translations,
  //but we want to handle that locale too:
  if(locale == m_original_locale)
    return get_title_original();

  if(!fallback)
    return Glib::ustring();

  if(m_map_translations.empty())
    return get_title_original();

  //Return the first translation from a locale with the same language, if any.
  //TODO_Performance: This is slow.
  //Note that this would, for instance, give en_GB translations before en_US translations, if there are no en_AU translations.
  const Glib::ustring locale_language_id = Utils::locale_language_id(locale);
  for(type_map_locale_to_translations::const_iterator iter = m_map_translations.begin(); iter != m_map_translations.end(); ++iter)
  {
    const Glib::ustring& locale_id = iter->first;
    if(Utils::locale_language_id(locale_id) == locale_language_id)
    {
      if(!(iter->second.empty()))
        return iter->second;
    }
  }

  //Fall back to the original title:
  if(!m_title.empty())
    return m_title;

  //Fall back to first translation, if any.
  //This would be quite unusual.
  type_map_locale_to_translations::const_iterator iter = m_map_translations.begin();
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


Glib::ustring TranslatableItem::get_title() const
{
  if(get_current_locale_not_original()) //Avoid this code if we don't need translations.
  {
    const Glib::ustring current_locale_id = get_current_locale();
    const Glib::ustring translated_title = get_title_translation(current_locale_id);
    if(!translated_title.empty())
      return translated_title;
  }

  return m_title;
}


Glib::ustring TranslatableItem::get_title_original() const
{
  return m_title;
}

void TranslatableItem::set_title(const Glib::ustring& title)
{
  if(get_current_locale_not_original()) //Avoid this code if we don't need translations.
  {
    //std::cout << "TranslatableItem::set_title() setting translation: " << title;
    const Glib::ustring the_locale = get_current_locale();
    if(the_locale.empty())
      set_title_original(title);
    else
    {
      set_title_translation(the_locale, title);
    }
  }
  else
  {
    set_title_original(title);
  }
}

void TranslatableItem::set_title_original(const Glib::ustring& title)
{
  m_title = title;
}

void TranslatableItem::clear_title_in_all_locales()
{
  m_title.clear();
  
  for(type_map_locale_to_translations::iterator iter = m_map_translations.begin(); iter != m_map_translations.end(); ++iter)
  {
    Glib::ustring& translation = iter->second;
    translation.clear();
  }
}

Glib::ustring TranslatableItem::get_current_locale()
{
  if(m_current_locale.empty())
  {
    const char* cLocale = setlocale(LC_ALL, 0); //Passing NULL means query, instead of set.
    if(cLocale)
    {
      //std::cout << "debug1: " << G_STRFUNC << ": locale=" << cLocale << std::endl;
      m_current_locale = Utils::locale_simplify(cLocale);
      //std::cout << "debug2: " << G_STRFUNC << ": m_current_locale=" << m_current_locale << std::endl;
    }
    else
      m_current_locale = 'C';
  }

  return m_current_locale;
}

void TranslatableItem::set_current_locale(const Glib::ustring& locale)
{
  if(locale.empty())
    return;

  m_current_locale = locale;
}

void TranslatableItem::set_original_locale(const Glib::ustring& locale)
{
  if(locale.empty())
    return;

  m_original_locale = locale;
}


Glib::ustring TranslatableItem::get_original_locale()
{
  if(m_original_locale.empty())
    m_original_locale = "en_US"; //"en_US.UTF-8";

  return m_original_locale; 
}

bool TranslatableItem::get_current_locale_not_original()
{
  if(m_original_locale.empty())
    get_original_locale();

  if(m_current_locale.empty())
    get_current_locale();

  return m_original_locale != m_current_locale;
}

TranslatableItem::enumTranslatableItemType TranslatableItem::get_translatable_item_type()
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
  else if(item_type == TRANSLATABLE_TYPE_CUSTOM_TITLE)
    return "Field Title";
  else if(item_type == TRANSLATABLE_TYPE_BUTTON)
    return "Button";
  else if(item_type == TRANSLATABLE_TYPE_TEXTOBJECT)
    return "Text";
 else if(item_type == TRANSLATABLE_TYPE_IMAGEOBJECT)
    return "Image";
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
  else if(item_type == TRANSLATABLE_TYPE_CUSTOM_TITLE)
    return _("Field Title");
  else if(item_type == TRANSLATABLE_TYPE_BUTTON)
    return _("Button");
  else if(item_type == TRANSLATABLE_TYPE_TEXTOBJECT)
    return _("Text");
 else if(item_type == TRANSLATABLE_TYPE_IMAGEOBJECT)
    return _("Image");
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

} //namespace Glom
