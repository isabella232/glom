/* Glom
 *
 * Copyright (C) 2001-2005 Murray Cumming
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

#ifndef GLOM_DATASTRUCTURE_ISO_CODES_H
#define GLOM_DATASTRUCTURE_ISO_CODES_H

#include <glom/libglom/data_structure/field.h>
#include <glom/libglom/data_structure/numeric_format.h>

namespace IsoCodes
{

class Currency
{
public:
  Glib::ustring m_symbol;
  Glib::ustring m_name;
};

typedef std::list<Currency> type_list_currencies;
type_list_currencies get_list_of_currency_symbols();

class Locale
{
public:
  Glib::ustring m_identifier;
  Glib::ustring m_name;
};

typedef std::list<Locale> type_list_locales;
type_list_locales get_list_of_locales();

Glib::ustring get_locale_name(const Glib::ustring& locale_id);
}

#endif //GLOM_DATASTRUCTURE_ISO_CODES_H

