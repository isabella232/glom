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

#include "combochoices.h"
#include <libglom/data_structure/glomconversions.h>
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl


namespace Glom
{

namespace DataWidgetChildren
{

ComboChoices::ComboChoices()
: m_with_second(false)
{
  init();
}

ComboChoices::ComboChoices(const sharedptr<LayoutItem_Field>& field_second)
: m_with_second(true),
  m_layoutitem_second(field_second)
{
  init();
}

void ComboChoices::init()
{
}

ComboChoices::~ComboChoices()
{
}

} //namespace DataWidetChildren
} //namespace Glom
