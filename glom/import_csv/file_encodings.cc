/* Glom
 *
 * Copyright (C) 2009 Openismus GmbH
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

//#include "config.h" //For ISO_CODES_PREFIX.

#include <glom/import_csv/file_encodings.h>
#include <libglom/algorithms_utils.h>
#include <glibmm/i18n.h>

namespace Glom
{

namespace FileEncodings
{

Encoding::Encoding(const char* name, const char* charset)
: m_name(name), m_charset(charset)
{
}

Glib::ustring Encoding::get_charset() const
{
  if(m_charset)
    return m_charset;
  else
    return Glib::ustring();
}

Glib::ustring Encoding::get_name() const
{
  if(m_name)
    return m_name;
  else
    return Glib::ustring();
}

static type_list_encodings list_encodings;

static void add_encoding(const gchar* name, const gchar* encoding)
{
  list_encodings.push_back(Encoding(name, encoding));
}

type_list_encodings get_list_of_encodings()
{
  if(!list_encodings.empty())
    return list_encodings;

  //TODO: Can we get this from anywhere else, such as iso-codes? murrayc
  //TODO: Make this generally more efficient.
  add_encoding(_("Unicode"), "UTF-8");
  add_encoding(_("Unicode"), "UTF-16");
  add_encoding(_("Unicode"), "UTF-16BE");
  add_encoding(_("Unicode"), "UTF-16LE");
  add_encoding(_("Unicode"), "UTF-32");
  add_encoding(_("Unicode"), "UTF-7");
  add_encoding(_("Unicode"), "UCS-2");
  add_encoding(_("Unicode"), "UCS-4");
  add_encoding(0, 0); // This just adds a separator in the combo box
  add_encoding(_("Western"), "ISO-8859-1");
  add_encoding(_("Central European"), "ISO-8859-2");
  add_encoding(_("South European"), "ISO-8859-3");
  add_encoding(_("Baltic"), "ISO-8859-4");
  add_encoding(_("Cyrillic"), "ISO-8859-5");
  add_encoding(_("Arabic"), "ISO-8859-6");
  add_encoding(_("Greek"), "ISO-8859-7");
  add_encoding(_("Hebrew Visual"), "ISO-8859-8");
  add_encoding(_("Hebrew"), "ISO-8859-8-I");
  add_encoding(_("Turkish"), "ISO-8859-9");
  add_encoding(_("Nordic"), "ISO-8859-10");
  add_encoding(_("Baltic"), "ISO-8859-13");
  add_encoding(_("Celtic"), "ISO-8859-14");
  add_encoding(_("Western"), "ISO-8859-15");
  add_encoding(_("Romanian"), "ISO-8859-16");
  add_encoding(0, 0); // This just adds a separator in the combo box
  add_encoding(_("Central European"), "WINDOWS-1250");
  add_encoding(_("Cyrillic"), "WINDOWS-1251");
  add_encoding(_("Western"), "WINDOWS-1252");
  add_encoding(_("Greek"), "WINDOWS-1253");
  add_encoding(_("Turkish"), "WINDOWS-1254");
  add_encoding(_("Hebrew"), "WINDOWS-1255");
  add_encoding(_("Arabic"), "WINDOWS-1256");
  add_encoding(_("Baltic"), "WINDOWS-1257");
  add_encoding(_("Vietnamese"), "WINDOWS-1258");

  return list_encodings;
}

Glib::ustring get_name_of_charset(const Glib::ustring& charset)
{
  //Make sure that the list is full:
  get_list_of_encodings();

  const auto iter =
    Utils::find_if(list_encodings,
      [&charset] (const Encoding& encoding) {
        return encoding.get_charset() == charset;
    });

  if(iter != list_encodings.end())
    return iter->get_name();
  else
    return Glib::ustring();
}

} //namespace FileEncodings;

} //namespace Glom
