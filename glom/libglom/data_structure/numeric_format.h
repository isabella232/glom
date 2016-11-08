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

#ifndef GLOM_DATA_STRUCTURE_NUMERIC_FORMAT_H
#define GLOM_DATA_STRUCTURE_NUMERIC_FORMAT_H

#include <glibmm/ustring.h>

namespace Glom
{

class NumericFormat
{
public:
  NumericFormat();
  NumericFormat(const NumericFormat& src) = default;
  NumericFormat(NumericFormat&& src) = default;

  NumericFormat& operator=(const NumericFormat& src) = default;
  NumericFormat& operator=(NumericFormat&& src) = default;

  bool operator==(const NumericFormat& src) const;
  bool operator!=(const NumericFormat& src) const;

  /** The foreground color to use for negative values, if
   * m_alt_foreground_color_for_negatives is true.
   * @returns the foreground color, in a format recognised by XParseColor
   */
  static Glib::ustring get_alternative_color_for_negatives();

  /** Get the number of significant figures we should allow to be shown until
   * we show the awkward e syntax. This should not be used if
   * m_decimal_places_restricted is true.
   * @returns the number of significant figures to show
   */
  static guint get_default_precision();

  /** String to use as the currency symbol. When the symbol is shown in the UI,
   * a space is appended to the string, and the result is prepended to the
   * data from the database. Be aware that the string supplied by the Glom
   * document might have no representation in the current user's locale.
   */
  Glib::ustring m_currency_symbol;

  /** Setting this to false would override the locale, if it used a 1000s
   * separator.
   */
  bool m_use_thousands_separator;

  /** Whether to restrict numeric precision. If true, a fixed precision is set
   * according to m_decimal_places. If false, the maximum precision is used.
   * However, the chosen fixed precision might exceed the maximum precision.
   */
  bool m_decimal_places_restricted;

  /** The number of decimal places to show, although it is only used if
   * m_decimal_places_restricted is false.
   */
  guint m_decimal_places;

  /** Whether to use an alternative foreground color for negative values. */
  bool m_alt_foreground_color_for_negatives;
};

} //namespace Glom

#endif //GLOM_DATA_STRUCTURE_NUMERIC_FORMAT_H
