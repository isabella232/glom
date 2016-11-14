/* Glom
 *
 * Copyright (C) 2009  Openismus GmbH
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

#ifndef GLOM_IMPORT_CSV_FILE_ENCODINGS_H
#define GLOM_IMPORT_CSV_FILE_ENCODINGS_H

#include <glibmm/ustring.h>
#include <vector>

namespace Glom
{

namespace FileEncodings
{

class Encoding
{
public:
  Encoding(const char* name, const char* charset);

  Glib::ustring get_charset() const;
  Glib::ustring get_name() const;

private:
  const char* m_name;
  const char* m_charset;
};

typedef std::vector<Encoding> type_list_encodings;

/** Get a list of file encodings to offer to the user.
 */
type_list_encodings get_list_of_encodings();

/** Discover the human-readable name (such as "Western") of a charset
 * (such as "ISO-8859-1")
 */
Glib::ustring get_name_of_charset(const Glib::ustring& charset);

} //namespace FileEncodings

} //namespace Glom

#endif //GLOM_IMPORT_CSV_FILE_ENCODINGS_H

