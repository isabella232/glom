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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_MODE_DESIGN_ISO_CODES_H
#define GLOM_MODE_DESIGN_ISO_CODES_H

#include <libglom/data_structure/field.h>
#include <libglom/data_structure/numeric_format.h>

namespace Glom
{

namespace IsoCodes
{

class Currency
{
public:
  Glib::ustring m_symbol;
  Glib::ustring m_name;
};

typedef std::vector<Currency> type_list_currencies;
type_list_currencies get_list_of_currency_symbols();

class Locale
{
public:
  Glib::ustring m_identifier;
  Glib::ustring m_name;
};

typedef std::vector<Locale> type_list_locales;
type_list_locales get_list_of_locales();

Glib::ustring get_locale_name(const Glib::ustring& locale_id);

} //namespace IsoCodes

} //namespace Glom

#endif //GLOM_MODE_DESIGN_ISO_CODES_H

