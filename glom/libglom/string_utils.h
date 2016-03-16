//
// Created by murrayc on 3/16/16.
//
/* Glom
 *
 * Copyright (C) 2001-20016 Murray Cumming
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

#include <glibmm/ustring.h>
#include <vector>

#ifndef GLOM_STRING_UTILS_H
#define GLOM_STRING_UTILS_H

namespace Glom {

namespace Utils {

Glib::ustring trim_whitespace(const Glib::ustring &text);

Glib::ustring string_replace(const Glib::ustring& src, const Glib::ustring& search_for, const Glib::ustring& replace_with);

/** Remove any characters that may not be in XML even when escaped.
 */
Glib::ustring string_clean_for_xml(const Glib::ustring& src);

/** Guess an appropriate identifier name based on a human-readable title
 */
Glib::ustring create_name_from_title(const Glib::ustring& title);

Glib::ustring string_escape_underscores(const Glib::ustring& text);

Glib::ustring string_trim(const Glib::ustring& str, const Glib::ustring& to_remove);

Glib::ustring string_remove_suffix(const Glib::ustring& str, const Glib::ustring& suffix, bool case_sensitive = true);

/** Create an appropriate title for an ID string.
 * For instance, date_of_birth would become Date Of Birth.
 */
Glib::ustring title_from_string(const Glib::ustring& text);

/** Get a decimal text representation of the number,
 * in the current locale.
 */
Glib::ustring string_from_decimal(guint decimal);

typedef std::vector<Glib::ustring> type_vec_strings;
type_vec_strings string_separate(const Glib::ustring& str, const Glib::ustring& separator, bool ignore_quoted_separator = false);

} //namespace Utils

} //namespace Glom

#endif //GLOM_STRING_UTILS_H
