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

#include "fieldtype.h"
#include <libintl.h>

//Initialize static data:
FieldType::type_map_gda_type_to_glom_type FieldType::m_map_gda_type_to_glom_type;
FieldType::type_map_glom_type_to_gda_type FieldType::m_map_glom_type_to_gda_type;
FieldType::type_map_type_names FieldType::m_map_type_names;
bool FieldType::m_maps_inited = false;

FieldType::FieldType()
: m_glom_type(TYPE_INVALID)
{
}

FieldType::FieldType(enumTypes glom_field_type)
: m_glom_type(glom_field_type)
{
}

FieldType::FieldType(Gnome::Gda::ValueType gda_type)
: m_glom_type(TYPE_INVALID)
{
  init_map();
  
  //Get the glom type used for this gda type:
  {
    type_map_gda_type_to_glom_type::iterator iterFind = m_map_gda_type_to_glom_type.find(gda_type);
    if(iterFind != m_map_gda_type_to_glom_type.end())
      m_glom_type = iterFind->second;
    else
    {
      // g_warning("FieldType::FieldType(Gnome::Gda::ValueType gda_type): Invalid gda type: %d",  gda_type);
    }
  }

  //Get the ideal gda type used for that glom type;
  type_map_glom_type_to_gda_type::iterator iterFind = m_map_glom_type_to_gda_type.find(m_glom_type);
  Gnome::Gda::ValueType ideal_gda_type = Gnome::Gda::VALUE_TYPE_UNKNOWN;
  if(iterFind == m_map_glom_type_to_gda_type.end())
    ideal_gda_type = iterFind->second;
}

FieldType::FieldType(Glib::ustring underlying_type, bool& underlying_change_required)
{
  //TODO.
}
    
FieldType::FieldType(const FieldType& src)
{
  operator=(src);
}

FieldType::~FieldType()
{
}

//static:
void FieldType::init_map()
{
  if(!m_maps_inited)
  {
    //Fill maps.

    //Ideals:
    m_map_gda_type_to_glom_type[Gnome::Gda::VALUE_TYPE_NUMERIC] = TYPE_NUMERIC;
    m_map_gda_type_to_glom_type[Gnome::Gda::VALUE_TYPE_INTEGER] = TYPE_NUMERIC; //Only for "serial" (auto-increment) fields.
    m_map_gda_type_to_glom_type[Gnome::Gda::VALUE_TYPE_STRING] = TYPE_TEXT;
    m_map_gda_type_to_glom_type[Gnome::Gda::VALUE_TYPE_TIME] = TYPE_TIME;
    m_map_gda_type_to_glom_type[Gnome::Gda::VALUE_TYPE_DATE] = TYPE_DATE;
    
    m_map_glom_type_to_gda_type[TYPE_NUMERIC] = Gnome::Gda::VALUE_TYPE_NUMERIC;
    m_map_glom_type_to_gda_type[TYPE_TEXT] = Gnome::Gda::VALUE_TYPE_STRING;
    m_map_glom_type_to_gda_type[TYPE_TIME] = Gnome::Gda::VALUE_TYPE_TIME;
    m_map_glom_type_to_gda_type[TYPE_DATE] = Gnome::Gda::VALUE_TYPE_DATE;

    m_map_type_names[TYPE_INVALID] = gettext("Invalid");                                        
    m_map_type_names[TYPE_NUMERIC] = gettext("Number");
    m_map_type_names[TYPE_TEXT] = gettext("Text");
    m_map_type_names[TYPE_TIME] = gettext("Time");
    m_map_type_names[TYPE_DATE] = gettext("Date");
 
    m_maps_inited = true;
  }
}

FieldType& FieldType::operator=(const FieldType& src)
{
  m_glom_type = src.m_glom_type;
  
  return *this;
}

bool FieldType::operator==(const FieldType& src) const
{
  bool result = (m_glom_type == src.m_glom_type);

  return result;
}

bool FieldType::operator!=(const FieldType& src) const
{
  return !(operator==(src));
}

Gnome::Gda::ValueType FieldType::get_gda_type() const
{
  init_map();
  
  return m_map_glom_type_to_gda_type[m_glom_type];;
}

Glib::ustring FieldType::get_underlying_type() const
{
  return m_underlying_type;
}

FieldType::enumTypes FieldType::get_glom_type() const
{  
  return m_glom_type;
}

FieldType::type_map_type_names FieldType::get_type_names()
{
  init_map();
  return m_map_type_names;
}

FieldType::type_map_type_names FieldType::get_usable_type_names()
{
  init_map();

  type_map_type_names result =  m_map_type_names;

  //Remove INVALID, because it's not something that a user can use for a field type.
  type_map_type_names::iterator iter = m_map_type_names.find(TYPE_INVALID);
  if(iter != m_map_type_names.end())
    m_map_type_names.erase(iter);
  
  return result;
}

Glib::ustring FieldType::get_type_name(enumTypes glom_type)
{
  Glib::ustring result = "Invalid";

  type_map_type_names::iterator iterFind = m_map_type_names.find(glom_type);
  if(iterFind != m_map_type_names.end())
    result = iterFind->second; 
  
  return result;
}

FieldType::enumTypes FieldType::get_type_for_name(const Glib::ustring& glom_type)
{
  enumTypes result = TYPE_INVALID;

  for(type_map_type_names::iterator iter = m_map_type_names.begin(); iter != m_map_type_names.end(); ++iter)
  {
    if(iter->second == glom_type)
    {
      result = iter->first;
      break;
    }
  }

  return result;
}



