/* Glom
 *
 * Copyright (C) 2001-2016 Murray Cumming
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

#ifndef GLOM_FILE_UTILS_H
#define GLOM_FILE_UTILS_H

#include <giomm/file.h>
#include <libgdamm/value.h>

namespace Glom
{

namespace Utils
{

bool file_exists(const Glib::ustring& uri);

bool file_exists(const Glib::RefPtr <Gio::File>& file);


Glib::ustring create_local_image_uri(const Gnome::Gda::Value& value);

/** Delete a file, if it exists.
 * See also delete_directory().
 */
bool delete_file(const std::string& uri);

/** Get a URI with the extension (any extension, not just .glom) removed.
 */
Glib::ustring get_file_uri_without_extension(const Glib::ustring& uri);

/** Get a filepath with the extension (any extension, not just .glom) removed.
 */
std::string get_file_path_without_extension(const std::string& filepath);


std::string get_temp_file_path(const std::string& prefix = std::string(), const std::string& extension = std::string());
Glib::ustring get_temp_file_uri(const std::string& prefix = std::string(), const std::string& extension = std::string());

/** This actually creates the directory.
 */
std::string get_temp_directory_path(const std::string& prefix = std::string());

/** This actually creates the directory.
 */
Glib::ustring get_temp_directory_uri(const std::string& prefix = std::string());

} //namespace Utils

} //namespace Glom

#endif //GLOM_FILE_UTILS_H
