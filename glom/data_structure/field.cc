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

#include "field.h"
#include "../connectionpool.h"
#include "glomconversions.h"
#include <glibmm/i18n.h>

//Initialize static data:
Field::type_map_gda_type_to_glom_type Field::m_map_gda_type_to_glom_type;
Field::type_map_glom_type_to_gda_type Field::m_map_glom_type_to_gda_type;
Field::type_map_type_names Field::m_map_type_names;
Field::type_map_type_names Field::m_map_type_names_ui;
bool Field::m_maps_inited = false;


Field::Field()
: m_glom_type(TYPE_INVALID),
  m_visible(true)
{
}

Field::Field(const Field& src)
{
  operator=(src);
}

Field::~Field()
{
}

Field& Field::operator=(const Field& src)
{
  m_glom_type = src.m_glom_type;
  m_field_info = src.m_field_info;
  m_data = src.m_data;

  m_strTitle = src.m_strTitle;
  m_strLookupRelationship = src.m_strLookupRelationship;
  m_strLookupField = src.m_strLookupField;

  m_calculation = src.m_calculation;

  m_visible = src.m_visible;

  return *this;
}

bool Field::operator==(const Field& src) const
{
  bool bResult = (m_field_info == src.m_field_info);
  bResult = bResult && (m_glom_type == src.m_glom_type);
  bResult = bResult && (m_data == src.m_data);

  bResult = bResult && (m_strTitle == src.m_strTitle);
  bResult = bResult && (m_strLookupRelationship == src.m_strLookupRelationship);
  bResult = bResult && (m_strLookupField == src.m_strLookupField);

  bResult = bResult && (m_calculation == src.m_calculation);

  bResult = bResult && (m_visible == src.m_visible);
  
  return bResult;
}

bool Field::operator!=(const Field& src) const
{
  return !(operator==(src));
}

Field::glom_field_type Field::get_glom_type() const
{
  return m_glom_type;
}

void Field::set_glom_type(glom_field_type fieldtype)
{
  m_glom_type = fieldtype;
}

Gnome::Gda::FieldAttributes Field::get_field_info() const
{
  return m_field_info;
}

void Field::set_field_info(const Gnome::Gda::FieldAttributes& fieldInfo)
{
  m_field_info = fieldInfo;

  //TODO: Maybe just do this in get_glom_type()?
  set_glom_type( get_glom_type_for_gda_type(fieldInfo.get_gdatype()) );
}

Gnome::Gda::Value Field::get_data() const
{
  return m_data;
}

void Field::set_data(const Gnome::Gda::Value& data)
{
  m_data = data;
}

Glib::ustring Field::get_title() const
{
  return m_strTitle;
}
void Field::set_title(const Glib::ustring& strTitle)
{
  m_strTitle = strTitle;
}

Glib::ustring Field::get_lookup_relationship() const
{
  return m_strLookupRelationship;
}

void Field::set_lookup_relationship(const Glib::ustring& strRelationship)
{
  m_strLookupRelationship = strRelationship;
}

Glib::ustring Field::get_lookup_field() const
{
  return m_strLookupField;
}

void Field::set_lookup_field(const Glib::ustring& strField)
{
  m_strLookupField = strField;
}

bool Field::get_is_lookup() const
{
  //It's a lookup if the lookup relationship and field are set:
  return ( (m_strLookupRelationship.size() > 0) && (m_strLookupField.size() > 0) );
}

Glib::ustring Field::get_title_or_name() const
{
  if(!m_strTitle.empty())
  {
    return m_strTitle;
  }
  else
  {
    return m_field_info.get_name();
  }
}

Glib::ustring Field::sql(const Gnome::Gda::Value& value) const
{
  if(value.is_null())
  {
    switch(get_glom_type())
    {
      case(TYPE_TEXT):
      {
        return "''"; //We want to ignore the concept of NULL strings, and deal only with empty strings.
        break;
      }
      case(TYPE_DATE):
      case(TYPE_TIME):
      case(TYPE_NUMERIC):
      {
        return "NULL";
        break;
      }
      default:
      {
        //Don't deal with these here.
        break;
      }
    }
  }

  Glib::ustring str;

  switch(get_glom_type())
  {
    case(TYPE_TEXT):
    {
      if(value.is_null())
        return "''"; //We want to ignore the concept of NULL strings, and deal only with empty strings.
      else
        str = "'" + value.to_string() + "'"; //Add single-quotes. Actually escape it.
    }
    case(TYPE_DATE):
    case(TYPE_TIME):
    {
      NumericFormat format_ignored; //Because we use ISO format.
      str = GlomConversions::get_text_for_gda_value(m_glom_type, value, std::locale() /* SQL uses the C locale */, format_ignored, true /* ISO standard */);
      if(str != "NULL")
         str = "'" + str + "'"; //Add single-quotes.

      break;
    }
    case(TYPE_NUMERIC):
    {
      str =  GlomConversions::get_text_for_gda_value(m_glom_type, value, std::locale() /* SQL uses the C locale */); //No quotes for numbers.
      break;
    }
    case(TYPE_BOOLEAN):
    {
      if(value.get_value_type() == Gnome::Gda::VALUE_TYPE_BOOLEAN)
        str = ( value.get_bool() ? "TRUE" : "FALSE" );
      else
        str = "FALSE";

      break;
    }
    default:
    {
      str = value.to_string();
      if(str.empty() && (m_glom_type != Field::TYPE_TEXT))
        str = "NULL"; //This has probably been handled in get_text_for_gda_value() anyway.

      break;
    }
  }

  return str;
}

Glib::ustring Field::get_name() const
{
  return m_field_info.get_name();
}

void Field::set_name(const Glib::ustring& value)
{
  m_field_info.set_name(value);
}

Glib::ustring Field::get_sql_type() const
{
  if(false) //See generate_next_auto_increment() //m_field_info.get_auto_increment())
    return "serial"; //See Postgres manual: http://www.postgresql.org/docs/7.4/interactive/datatype.html#DATATYPE-SERIAL. It's actually an integer..
  else
  {
    Glib::ustring result;

    //Type
    Glib::ustring strType = "unknowntype";

    ConnectionPool* pConnectionPool = ConnectionPool::get_instance();
    if(pConnectionPool)
    {
      const FieldTypes* pFieldTypes = pConnectionPool->get_field_types();
      if(pFieldTypes)
      {
        const Gnome::Gda::ValueType fieldType = m_field_info.get_gdatype();
        strType = pFieldTypes->get_string_name_for_gdavaluetype(fieldType);
      }
    }

    if(strType == "unknowntype")
    {
      g_warning("Field::get_sql_type(): returning unknowntype for field name=%s , glom_type=%d, gda_type=%d", get_name().c_str(), get_glom_type(), m_field_info.get_gdatype());
    }
    
    return strType;
  }
}

/// Ignores any part of FieldAttributes that libgda does not properly fill.
bool Field::field_info_from_database_is_equal(const Gnome::Gda::FieldAttributes& field)
{
  Gnome::Gda::FieldAttributes temp = m_field_info;
  
  temp.set_auto_increment( field.get_auto_increment() ); //Don't compare this, because the data is incorrect when libgda reads it from the database.
  temp.set_default_value( field.get_default_value() ); //Don't compare this, because the data is incorrect when libgda reads it from the database.
  temp.set_primary_key( field.get_primary_key() ); //Don't compare this, because the data is incorrect when libgda reads it from the database.
    
  return temp == field; 
}

Field::glom_field_type Field::get_glom_type_for_gda_type(Gnome::Gda::ValueType gda_type)
{
  init_map();

  Field::glom_field_type result = TYPE_INVALID;
  
  //Get the glom type used for this gda type:
  {
    type_map_gda_type_to_glom_type::iterator iterFind = m_map_gda_type_to_glom_type.find(gda_type);
    if(iterFind != m_map_gda_type_to_glom_type.end())
      result = iterFind->second;
    else
    {
      // g_warning("FieldType::FieldType(Gnome::Gda::ValueType gda_type): Invalid gda type: %d",  gda_type);
    }
  }

  return result;
}

Gnome::Gda::ValueType Field::get_gda_type_for_glom_type(Field::glom_field_type glom_type)
{
  init_map();

  //Get the ideal gda type used for that glom type;
  type_map_glom_type_to_gda_type::iterator iterFind = m_map_glom_type_to_gda_type.find(glom_type);
  Gnome::Gda::ValueType ideal_gda_type = Gnome::Gda::VALUE_TYPE_UNKNOWN;
  if(iterFind != m_map_glom_type_to_gda_type.end())
    ideal_gda_type = iterFind->second;

  if(ideal_gda_type == Gnome::Gda::VALUE_TYPE_UNKNOWN)
  {
    g_warning("Field::get_gda_type_for_glom_type(): Returning VALUE_TYPE_UNKNOWN for glom_type=%d", glom_type);
  }
  
  return ideal_gda_type;
}

//static:
void Field::init_map()
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
    m_map_gda_type_to_glom_type[Gnome::Gda::VALUE_TYPE_BOOLEAN] = TYPE_BOOLEAN;
    
    m_map_glom_type_to_gda_type[TYPE_NUMERIC] = Gnome::Gda::VALUE_TYPE_NUMERIC;
    m_map_glom_type_to_gda_type[TYPE_TEXT] = Gnome::Gda::VALUE_TYPE_STRING;
    m_map_glom_type_to_gda_type[TYPE_TIME] = Gnome::Gda::VALUE_TYPE_TIME;
    m_map_glom_type_to_gda_type[TYPE_DATE] = Gnome::Gda::VALUE_TYPE_DATE;
    m_map_glom_type_to_gda_type[TYPE_BOOLEAN] = Gnome::Gda::VALUE_TYPE_BOOLEAN;
    
    m_map_type_names_ui[TYPE_INVALID] = _("Invalid");
    m_map_type_names_ui[TYPE_NUMERIC] = _("Number");
    m_map_type_names_ui[TYPE_TEXT] = _("Text");
    m_map_type_names_ui[TYPE_TIME] = _("Time");
    m_map_type_names_ui[TYPE_DATE] = _("Date");    
    m_map_type_names_ui[TYPE_BOOLEAN] = _("Boolean");

    m_map_type_names[TYPE_INVALID] = "Invalid";
    m_map_type_names[TYPE_NUMERIC] = "Number";
    m_map_type_names[TYPE_TEXT] = "Text";
    m_map_type_names[TYPE_TIME] = "Time";
    m_map_type_names[TYPE_DATE] = "Date";
    m_map_type_names[TYPE_BOOLEAN] = "Boolean";
    
    m_maps_inited = true;
  }
}

//static:
Field::type_map_type_names Field::get_type_names_ui()
{
  init_map();
  return m_map_type_names_ui;
}

//static:
Field::type_map_type_names Field::get_type_names()
{
  init_map();
  return m_map_type_names;
}

//static:
Field::type_map_type_names Field::get_usable_type_names()
{
  init_map();

  type_map_type_names result =  m_map_type_names_ui;

  //Remove INVALID, because it's not something that a user can use for a field type.
  type_map_type_names::iterator iter = result.find(TYPE_INVALID);
  if(iter != result.end())
    result.erase(iter);

  return result;
}

//static:
Glib::ustring Field::get_type_name_ui(glom_field_type glom_type)
{
  Glib::ustring result = "Invalid";

  type_map_type_names::iterator iterFind = m_map_type_names.find(glom_type);
  if(iterFind != m_map_type_names.end())
    result = iterFind->second;

  return result;
}

//static:
Field::glom_field_type Field::get_type_for_ui_name(const Glib::ustring& glom_type)
{
  glom_field_type result = TYPE_INVALID;

  for(type_map_type_names::iterator iter = m_map_type_names_ui.begin(); iter != m_map_type_names_ui.end(); ++iter)
  {
    if(iter->second == glom_type)
    {
      result = iter->first;
      break;
    }
  }

  return result;
}

Glib::ustring Field::get_calculation() const
{
  return m_calculation;
}

void Field::set_calculation(const Glib::ustring& calculation)
{
  m_calculation = calculation;
}

Field::type_list_strings Field::get_calculation_fields() const
{
  //TODO: Use regex, for instance with pcre here?
  //TODO: Better?: Run the calculation on some example data, and record the touched fields? But this could not exercise every code path.
  //TODO_Performance: Just cache the result whenever m_calculation changes.

  type_list_strings result;

  Glib::ustring::size_type index = 0;
  const Glib::ustring::size_type count = m_calculation.size();
  const Glib::ustring::size_type prefix_size = 8; //"fields[\""

  while(index < count)
  {
    Glib::ustring::size_type pos_find = m_calculation.find("fields[\"", index);
    if(pos_find != Glib::ustring::npos)
    {
      Glib::ustring::size_type pos_find_end = m_calculation.find("\"]", pos_find);
      if(pos_find_end  != Glib::ustring::npos)
      {
        Glib::ustring::size_type pos_start = pos_find + prefix_size;
        const Glib::ustring field_name = m_calculation.substr(pos_start, pos_find_end - pos_start);
        result.push_back(field_name);
        index = pos_find_end + 1;
      }
    }

    ++index;
  }

  return result;
}

void Field::set_visible(bool val)
{
  m_visible = val;
}

bool Field::get_visible() const
{
  return m_visible;
}
