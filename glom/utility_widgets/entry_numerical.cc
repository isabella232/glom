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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "entry_numerical.h"
#include <cstdio> //For ::sprintf()

Entry_Numerical::Entry_Numerical()
{
}

Entry_Numerical::~Entry_Numerical()
{
}

guint Entry_Numerical::get_value_as_guint()
{
  const Glib::ustring& strValue = get_text();
  const gchar* pchValue = strValue.c_str();

  return atoi(pchValue);
}

void Entry_Numerical::set_value(guint uiVal)
{
  gchar pchValue[10] = {0};
  sprintf(pchValue, "%d", uiVal);
  set_text(Glib::ustring(pchValue));
}
