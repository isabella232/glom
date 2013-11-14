/* Glom
 *
 * Copyright (C) 2012 Openismus GmbH
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

#include <libglom/init.h>
#include "glom/mode_design/iso_codes.h"
#include <iostream>

static bool currencies_contains(const Glom::IsoCodes::type_list_currencies& container, const Glib::ustring& name)
{
  for(Glom::IsoCodes::type_list_currencies::const_iterator iter = container.begin(); iter != container.end(); ++iter)
  {
    const Glom::IsoCodes::Currency& item = *iter;
    if(item.m_symbol == name)
     return true;
  }

  return false;
}

static bool locales_contains(const Glom::IsoCodes::type_list_locales& container, const Glib::ustring& name)
{
  for(Glom::IsoCodes::type_list_locales::const_iterator iter = container.begin(); iter != container.end(); ++iter)
  {
    const Glom::IsoCodes::Locale& item = *iter;
    if(item.m_identifier == name)
     return true;
  }

  return false;
}

int main()
{
  Glom::libglom_init();

  Glom::IsoCodes::type_list_currencies currencies =
    Glom::IsoCodes::get_list_of_currency_symbols();
  g_assert(!currencies.empty());
  g_assert( currencies_contains(currencies, "EUR") );

  Glom::IsoCodes::type_list_locales locales =
    Glom::IsoCodes::get_list_of_locales();
  g_assert(!locales.empty());
  g_assert( locales_contains(locales, "de") );
  g_assert( locales_contains(locales, "de_DE") );
  g_assert( locales_contains(locales, "de_AT") );
  g_assert( locales_contains(locales, "en") );
  g_assert( locales_contains(locales, "en_GB") );

  g_assert( Glom::IsoCodes::get_locale_name("de") == "German" );
  g_assert( Glom::IsoCodes::get_locale_name("de_AT") == "German (Austria)" );

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
