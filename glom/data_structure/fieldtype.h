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


#ifndef GLOM_DATASTRUCTURE_FIELDTYPE_H
#define GLOM_DATASTRUCTURE_FIELDTYPE_H

#include <libgdamm.h>

//FieldType info, such as Name, Title, definitions, and, sometimes, contents.
class FieldType
{
public:
  enum enumTypes
  {
    TYPE_INVALID,
    TYPE_NUMERIC,
    TYPE_TEXT,
    TYPE_DATE,
    TYPE_TIME
  };
  
  FieldType();
  FieldType(enumTypes glom_field_type);
  FieldType(Gnome::Gda::ValueType gda_type);
  FieldType(Glib::ustring underlying_type, bool& underlying_change_required);
  FieldType(const FieldType& src);
  virtual ~FieldType();

  FieldType& operator=(const FieldType& src);

  bool operator==(const FieldType& src) const;
  bool operator!=(const FieldType& src) const;

  Gnome::Gda::ValueType get_gda_type() const;
  Glib::ustring get_underlying_type() const;
  enumTypes get_glom_type() const;

  typedef std::map<enumTypes, Glib::ustring> type_map_type_names;
  static type_map_type_names get_type_names();
  static type_map_type_names get_usable_type_names();
  
  static Glib::ustring get_type_name(enumTypes glom_type);
  static enumTypes get_type_for_name(const Glib::ustring& glom_type);
                                                        
protected:
  static void init_map();

  enumTypes m_glom_type;
  //Gnome::Gda::ValueType m_gda_type;
  Glib::ustring m_underlying_type;

  //The glom type to be used for the gda type:
  typedef std::map<Gnome::Gda::ValueType, enumTypes> type_map_gda_type_to_glom_type;
  static type_map_gda_type_to_glom_type m_map_gda_type_to_glom_type;

  //The gda type to be used for the glom type:
  typedef std::map<enumTypes, Gnome::Gda::ValueType> type_map_glom_type_to_gda_type;
  static type_map_glom_type_to_gda_type m_map_glom_type_to_gda_type;

  static type_map_type_names m_map_type_names;
  static bool m_maps_inited;

};

#endif  //GLOM_DATASTRUCTURE_FIELDTYPE_H

