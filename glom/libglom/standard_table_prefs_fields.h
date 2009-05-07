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


#ifndef GLOM_STANDARD_TABLE_PREFS_FIELDS_H
#define GLOM_STANDARD_TABLE_PREFS_FIELDS_H

namespace Glom
{

//Define these to avoid entering the string literals repeatedly.
#define GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "glom_system_preferences"
#define GLOM_STANDARD_TABLE_PREFS_FIELD_ID "system_prefs_id"
#define GLOM_STANDARD_TABLE_PREFS_FIELD_NAME "name"
#define GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_NAME "org_name"
#define GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_LOGO "org_logo"
#define GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET "org_address_street"
#define GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET2 "org_address_street2"
#define GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_TOWN "org_address_town"
#define GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTY "org_address_county"
#define GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTRY "org_address_country"
#define GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_POSTCODE "org_address_postcode"

#define GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME "glom_system_autoincrements"
#define GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_ID "system_autoincrements_id"
#define GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_TABLE_NAME "table_name"
#define GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_FIELD_NAME "field_name"
#define GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_NEXT_VALUE "next_value" //Numeric

#define GLOM_STANDARD_FIELD_LOCK "glom_lock" //Text. In every table. Not used yet.

#define GLOM_STANDARD_DEFAULT_FIELD_CREATION_DATE "creation_date" //Date. In every table. Not used yet.
#define GLOM_STANDARD_DEFAULT_FIELD_CREATION_TIME "creation_time" //Time. In every table. Not used yet.
#define GLOM_STANDARD_DEFAULT_FIELD_CREATION_USER "creation_user" //Text. In every table. Not used yet.
#define GLOM_STANDARD_DEFAULT_FIELD_MODIFICATION_DATE "modification_date" //Text. In every table. Not used yet.
#define GLOM_STANDARD_DEFAULT_FIELD_MODIFICATION_TIME "modification_time" //Time. In every table. Not used yet.
#define GLOM_STANDARD_DEFAULT_FIELD_MODIFICATION_USER "modification_user" //Text. In every table. Not used yet.

} //namespace Glom

#endif //GLOM_STANDARD_TABLE_PREFS_FIELDS_H

