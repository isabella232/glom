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

#ifndef GLOM_DATASTRUCTURE_HAS_TITLE_SINGULAR_H
#define GLOM_DATASTRUCTURE_HAS_TITLE_SINGULAR_H

#include <libglom/data_structure/translatable_item.h>

namespace Glom
{

/** HasTitleSingular instances may have a (translated) singular form of their title.
 * For instance, "Album" instead of "Albums".
 * This is useful in some generated UI strings.
 */
class HasTitleSingular
{
public:
  HasTitleSingular();
  HasTitleSingular(const HasTitleSingular& src) = default;
  HasTitleSingular(HasTitleSingular&& src) = delete;
  virtual ~HasTitleSingular();

  HasTitleSingular& operator=(const HasTitleSingular& src) = default;
  HasTitleSingular& operator=(HasTitleSingular&& src) = delete;

  bool operator==(const HasTitleSingular& src) const;
  bool operator!=(const HasTitleSingular& src) const;

  /** Get the (translation of the) singular form of the title, in the current locale,
   * if specified.
   */
  Glib::ustring get_title_singular(const Glib::ustring& locale) const;

  /** Get the title's original (non-translated, usually English) text.
   */
  Glib::ustring get_title_singular_original() const;

  /** Get the (translation of the) singular form of the title, in the current locale,
   * if specified, falling back to the non-singular title, and
   * then falling back to the table name.
   */
  Glib::ustring get_title_singular_with_fallback(const Glib::ustring& locale) const;

  /** Set the singular title's translation for the current locale.
   */
  void set_title_singular(const Glib::ustring& title, const Glib::ustring& locale);

  /** For instance, "Customer" if the table is titled "Customers".
   * This is useful in some UI strings.
   */
  std::shared_ptr<TranslatableItem> m_title_singular;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_HAS_TITLE_SINGULAR_H

