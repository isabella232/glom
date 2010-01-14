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

#ifndef GLOM_DATA_STRUCTURE_NUMERIC_FORMAT_H
#define GLOM_DATA_STRUCTURE_NUMERIC_FORMAT_H

#include "privileges.h"
#include <glibmm/ustring.h>
#include <map>

namespace Glom
{

class NumericFormat
{
public:
  NumericFormat();
  NumericFormat(const NumericFormat& src);
  ~NumericFormat();

  NumericFormat& operator=(const NumericFormat& src);

  bool operator==(const NumericFormat& src) const;
  bool operator!=(const NumericFormat& src) const;

  /** Get the number of digits (even before the decimal point) we should allow
   * to be shown until we show the awkward e syntax.
   * This should not be used if m_decimal_places_restricted is true.
   */
  static guint get_default_precision();

  Glib::ustring m_currency_symbol;
  bool m_use_thousands_separator; //Setting this to false would override the locale, if it used a 1000s separator.
  bool m_decimal_places_restricted;
  guint m_decimal_places;
};

} //namespace Glom

#endif //GLOM_DATA_STRUCTURE_NUMERIC_FORMAT_H
