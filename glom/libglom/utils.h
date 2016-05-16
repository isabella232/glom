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

#ifndef GLOM_UTILS_H
#define GLOM_UTILS_H

#include <libglom/data_structure/field.h>
#include <libglom/data_structure/numeric_format.h>
#include <libglom/document/document.h>
#include <libglom/data_structure/layout/layoutitem_field.h>
#include <libglom/algorithms_utils.h>

#include <giomm/file.h>

namespace Glom
{

namespace Utils
{

/** Get just the first part of a locale, such as de_DE,
 * ignoring, for instance, .UTF-8 or \@euro at the end.
 */
Glib::ustring locale_simplify(const Glib::ustring& locale_id);

/** Get just the language ID part of a locale, such as de from "de_DE",
 */
Glib::ustring locale_language_id(const Glib::ustring& locale_id);


/** @returns true if the script is OK, or
 * false if the script uses pygtk2, which would cause a crash,
 * because Glom itself uses GTK+ 3.
 */
bool script_check_for_pygtk2(const Glib::ustring& script);

/**
 * This is simpler than catching the exception from Gio::Resource::get_info_global().
 *
 * @returns true if the GResource exists.
 */
bool get_resource_exists(const std::string& resource_path);

template<typename E>
constexpr typename std::underlying_type<E>::type
to_utype(E enumerator) noexcept
{
  return static_cast<typename std::underlying_type<E>::type>(enumerator);
}

} //namespace Utils

} //namespace Glom

#endif //GLOM_UTILS_H
