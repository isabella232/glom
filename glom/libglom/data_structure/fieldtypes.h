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


#ifndef GLOM_DATASTRUCTURE_FIELDTYPES_H
#define GLOM_DATASTRUCTURE_FIELDTYPES_H

#include <libgdamm/connection.h>
#include <unordered_map>

namespace Glom
{

/* This class maps the SQL type names (seen in libgda schema datamodels, and in SQL) to the Gda Types.
 */
class FieldTypes
{
public:
  FieldTypes(const Glib::RefPtr<Gnome::Gda::Connection>& gda_connection);

  Glib::ustring get_string_name_for_gdavaluetype(GType field_type) const;

  GType get_fallback_type_for_gdavaluetype(GType field_type) const;

  guint get_types_count() const;

private:
  /** Use some default mappings,
   * if, for some reason, we cannot get it from the database server at runtime.
   */
  void fill_with_default_data();

  //Duplicate information, to make searching easier:
  typedef std::unordered_map<GType, Glib::ustring> type_mapGdaTypesToSchemaStrings;
  type_mapGdaTypesToSchemaStrings m_mapGdaTypesToSchemaStrings;

  //Fallback types used if the database system does not support a type natively
  typedef std::unordered_map<GType, GType> type_mapFallbackTypes;
  type_mapFallbackTypes m_mapFallbackTypes;
};

} //namespace Glom

#endif  //GLOM_DATASTRUCTURE_FIELDTYPES_H

