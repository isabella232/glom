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

Field::Field()
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
  m_FieldType = src.m_FieldType;
  m_field_info = src.m_field_info;
  m_strData = src.m_strData;

  m_strTitle = src.m_strTitle;
  m_strLookupRelationship = src.m_strLookupRelationship;
  m_strLookupField = src.m_strLookupField;

  return *this;
}

bool Field::operator==(const Field& src) const
{
  bool bResult = (m_field_info == src.m_field_info);
  bResult = bResult && (m_FieldType == src.m_FieldType);
  bResult = bResult && (m_strData == src.m_strData);

  bResult = bResult && (m_strTitle == src.m_strTitle);
  bResult = bResult && (m_strLookupRelationship == src.m_strLookupRelationship);
  bResult = bResult && (m_strLookupField == src.m_strLookupField);

  return bResult;
}

bool Field::operator!=(const Field& src) const
{
  return !(operator==(src));
}

FieldType Field::get_field_type() const
{
  return m_FieldType;
}

void Field::set_field_type(const FieldType& fieldtype)
{
  m_FieldType = fieldtype;
}

Gnome::Gda::FieldAttributes Field::get_field_info() const
{
  return m_field_info;
}

void Field::set_field_info(const Gnome::Gda::FieldAttributes& fieldInfo)
{
  m_field_info = fieldInfo;

  //TODO: Maybe just do this in get_field_type()?
  set_field_type( FieldType(fieldInfo.get_gdatype()) );
}

Glib::ustring Field::get_data() const
{
  return m_strData;
}

void Field::set_data(const Glib::ustring& strData)
{
  m_strData = strData;
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
  m_strLookupField = strRelationship;
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
  if(m_strTitle.size())
  {
    return m_strTitle;
  }
  else
  {
    return m_field_info.get_name();
  }
}

//static:
Glib::ustring Field::sql(const Gnome::Gda::Value& value)
{
  Glib::ustring str = value.to_string();
  if(value.get_value_type() == Gnome::Gda::VALUE_TYPE_STRING)
  {
     str = "'" + str + "'";
  }
  else
  {
    if(str.empty())
      str = "NULL";
  }
     
  return str; //TODO_port: Actually escape it.
}

Glib::ustring Field::sql(const Glib::ustring& str) const
{
//TODO: This method should not be used in future = sql(Value) would be more type-safe.

g_warning("get_field_type().get_glom_type(): %d", get_field_type().get_glom_type());
  FieldType::enumTypes type = get_field_type().get_glom_type();
  if( (type == FieldType::TYPE_TEXT) || (type == FieldType::TYPE_DATE) || (type == FieldType::TYPE_TIME) ) //TODO: We really need to think about locales and canonical formats for dates and times.
    return "'" + str + "'"; //TODO: special characters?
  else
  {
    if(str.empty())
      return "NULL";
    else
     return str;
  }
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

Glib::ustring Field::get_default_value_as_string() const
{
  Glib::ustring result;
  Gnome::Gda::Value value = m_field_info.get_default_value();

  if(value.is_null())
    result = "";
  else
  {
    if(value.get_value_type() == Gnome::Gda::VALUE_TYPE_STRING)
    {
       result = value.get_string();
       g_warning("resut as string: %s", result.c_str());
    }
    else
      result = value.to_string();
  }

  return result;
}

Glib::ustring  Field::value_to_string(const Gnome::Gda::Value& value) const
{
  if(value.is_null())
    return "";
  else
    return value.to_string();
}


  
