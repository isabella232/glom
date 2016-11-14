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

#include <libglom/data_structure/numeric_format.h>

namespace Glom
{

NumericFormat::NumericFormat()
: m_use_thousands_separator(true), //A sensible default.
  m_decimal_places_restricted(false),
  m_decimal_places(2), //A sensible default.
  m_alt_foreground_color_for_negatives(false)
{
}

bool NumericFormat::operator==(const NumericFormat& src) const
{
  return (m_currency_symbol == src.m_currency_symbol) &&
         (m_use_thousands_separator == src.m_use_thousands_separator) &&
         (m_decimal_places_restricted == src.m_decimal_places_restricted) &&
         (m_decimal_places == src.m_decimal_places) &&
         (m_alt_foreground_color_for_negatives == src.m_alt_foreground_color_for_negatives);
}

bool NumericFormat::operator!=(const NumericFormat& src) const
{
  return !(operator==(src));
}

guint NumericFormat::get_default_precision()
{
  return 15;
}

Glib::ustring NumericFormat::get_alternative_color_for_negatives()
{
  return "#ffff00000000"; //red
}


} //namespace Glom
