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

#ifndef GLOM_DATASTRUCTURE_TRANSLATABLE_ITEM_H
#define GLOM_DATASTRUCTURE_TRANSLATABLE_ITEM_H

#include <glibmm/ustring.h>
#include <map>
#include <libglom/sharedptr.h>

namespace Glom
{

///TranslatableItem have a map of translation strings - one string for each locale.
class TranslatableItem
{
public:
  TranslatableItem();
  TranslatableItem(const TranslatableItem& src);
  TranslatableItem(TranslatableItem&& src);
  virtual ~TranslatableItem();

  TranslatableItem& operator=(const TranslatableItem& src);
  TranslatableItem& operator=(TranslatableItem&& src);

  bool operator==(const TranslatableItem& src) const;
  bool operator!=(const TranslatableItem& src) const;

  /** Set the  non-translated identifier name.
   */
  virtual void set_name(const Glib::ustring& name);

  /** Get the non-translated identifier name.
   */
  virtual Glib::ustring get_name() const;

  bool get_name_not_empty() const; //For performance.

  virtual Glib::ustring get_title_or_name(const Glib::ustring& locale) const;

  /** Get the title's translation for the specified locale, falling back to the
   * original text if there is no translation.
   *
   * See also get_title_translation() and get_title_original(), which (optionally)
   * do not use fallbacks.
   *
   * @param locale The locale whose title text should be returned. If this is empty then the original text will be returned.
   * @result The text of the title.
   */
  virtual Glib::ustring get_title(const Glib::ustring& locale) const;

  //This is virtual so that ChoiceValue can override it.
  /** Get the title's original (non-translated, usually English) text.
   */
  virtual Glib::ustring get_title_original() const;

  /** Get the title's translation for the specified @a locale, optionally
   * falling back to a locale of the same language, and then falling back to 
   * the original.
   * Calling this with the current locale is the same as calling get_title_original().
   */
  Glib::ustring get_title_translation(const Glib::ustring& locale, bool fallback = true) const;
  

  /** Set the title's translation for the specified locale.
   * @param title The text of the title.
   * @param locale The locale whose title text should be set. If this is empty then the original text will be set.
   */
  void set_title(const Glib::ustring& title, const Glib::ustring& locale);

  /** Set the title's original (non-translated, usually English) text.
   * This is the same as calling set_title() with an empty locale parameter.
   */
  void set_title_original(const Glib::ustring& title);


  /// Clear the original title and any translations of the title.
  void clear_title_in_all_locales();

  typedef std::map<Glib::ustring, Glib::ustring> type_map_locale_to_translations;

  bool get_has_translations() const;

  enum enumTranslatableItemType
  {
     TRANSLATABLE_TYPE_INVALID,
     TRANSLATABLE_TYPE_FIELD,
     TRANSLATABLE_TYPE_RELATIONSHIP,
     TRANSLATABLE_TYPE_LAYOUT_ITEM,
     TRANSLATABLE_TYPE_CUSTOM_TITLE,
     TRANSLATABLE_TYPE_PRINT_LAYOUT,
     TRANSLATABLE_TYPE_REPORT,
     TRANSLATABLE_TYPE_TABLE,
     TRANSLATABLE_TYPE_BUTTON,
     TRANSLATABLE_TYPE_TEXTOBJECT, //This has a TRANSLATABLE_TYPE_STATIC_TEXT child.
     TRANSLATABLE_TYPE_IMAGEOBJECT,
     TRANSLATABLE_TYPE_CHOICEVALUE,
     TRANSLATABLE_TYPE_DATABASE_TITLE,
     TRANSLATABLE_TYPE_STATIC_TEXT
   };

  enumTranslatableItemType get_translatable_item_type() const;

  //Direct access, for performance:
  const type_map_locale_to_translations& _get_translations_map() const;

  static Glib::ustring get_translatable_type_name(enumTranslatableItemType item_type);

  /** The non-translated name is used for the context in gettext .po files.
   */
  static Glib::ustring get_translatable_type_name_nontranslated(enumTranslatableItemType item_type);

private:

  /** Get the locale used as the source language.
   * This is the language of the title that is used when there are no translations.
   */
  static Glib::ustring get_original_locale();

  void set_title_translation(const Glib::ustring& locale, const Glib::ustring& translation);


protected:
  enumTranslatableItemType m_translatable_item_type;

private:
  Glib::ustring m_name; //Non-translated identifier;
  Glib::ustring m_title; //The original, untranslated (usually-English) title.
  type_map_locale_to_translations m_map_translations;
};

template <class T_object>
Glib::ustring glom_get_sharedptr_name(const std::shared_ptr<T_object>& item)
{
  if(item)
    return item->get_name();
  else
    return Glib::ustring();
}

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_TRANSLATABLE_ITEM_H

