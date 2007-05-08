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


#ifndef GLOM_DATASTRUCTURE_FIELDTYPES_H
#define GLOM_DATASTRUCTURE_FIELDTYPES_H

#include <libgdamm.h>

namespace Glom
{

/* This class maps the SQL type names (seen in libgda schema datamodels, and in SQL) to the Gda Types.
 */
class FieldTypes
{
public:
  FieldTypes(const Glib::RefPtr<Gnome::Gda::Connection>& gda_connection);
  virtual ~FieldTypes();

  GType get_gdavalue_for_schema_type_string(const Glib::ustring& schema_type_string) const;
  Glib::ustring get_string_name_for_gdavaluetype(GType field_type) const;
  
protected:
  typedef std::map<Glib::ustring, GType> type_mapSchemaStringsToGdaTypes;
  type_mapSchemaStringsToGdaTypes m_mapSchemaStringsToGdaTypes;

  //Duplicate information, to make searching easier:
  typedef std::map<GType, Glib::ustring> type_mapGdaTypesToSchemaStrings;
  type_mapGdaTypesToSchemaStrings m_mapGdaTypesToSchemaStrings;
};

} //namespace Glom

#endif  //GLOM_DATASTRUCTURE_FIELDTYPES_H

