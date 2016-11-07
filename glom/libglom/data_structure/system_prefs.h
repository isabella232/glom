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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_DATASTRUCTURE_SYSTEMPREFS_H
#define GLOM_DATASTRUCTURE_SYSTEMPREFS_H

#include <glibmm/ustring.h>
#include <libgdamm/value.h>

namespace Glom
{

class SystemPrefs
{
public:
  SystemPrefs();
  SystemPrefs(const SystemPrefs& src) = default;
  SystemPrefs(SystemPrefs&& src);

  SystemPrefs& operator=(const SystemPrefs& src) = default;
  SystemPrefs& operator=(SystemPrefs&& src);

  bool operator==(const SystemPrefs& src) const;
  bool operator!=(const SystemPrefs& src) const;


  //TODO: Add getters and setters:
  Glib::ustring m_name, m_org_name,
    m_org_address_street, m_org_address_street2, m_org_address_town,
    m_org_address_county, m_org_address_country, m_org_address_postcode;
  Gnome::Gda::Value m_org_logo; //enumType::IMAGE.
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_SYSTEMPREFS_H



