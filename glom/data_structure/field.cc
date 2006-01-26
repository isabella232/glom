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
#include "../utils.h"
#include <glibmm/i18n.h>

//Initialize static data:
Field::type_map_gda_type_to_glom_type Field::m_map_gda_type_to_glom_type;
Field::type_map_glom_type_to_gda_type Field::m_map_glom_type_to_gda_type;
Field::type_map_type_names Field::m_map_type_names;
Field::type_map_type_names Field::m_map_type_names_ui;
Field::type_map_conversions Field::m_map_conversions;
bool Field::m_maps_inited = false;


Field::Field()
: m_glom_type(TYPE_INVALID),
  m_visible(true)
{
  m_translatable_item_type = TRANSLATABLE_TYPE_FIELD;
}

Field::Field(const Field& src)
: TranslatableItem(src)
{
  //TODO_Performance: Implement this properly, without the extra copy.
  operator=(src);
}

Field::~Field()
{
}

Field& Field::operator=(const Field& src)
{
  TranslatableItem::operator=(src);

  m_glom_type = src.m_glom_type;
  m_field_info = src.m_field_info;
  m_data = src.m_data;

  m_lookup_relationship = src.m_lookup_relationship;
  m_strLookupField = src.m_strLookupField;

  m_calculation = src.m_calculation;

  m_visible = src.m_visible;

  m_default_formatting = src.m_default_formatting;

  return *this;
}

bool Field::operator==(const Field& src) const
{
  bool bResult = TranslatableItem::operator==(src);

  bResult = (m_field_info == src.m_field_info);
  bResult = bResult && (m_glom_type == src.m_glom_type);
  bResult = bResult && (m_data == src.m_data);

  bResult = bResult && (m_lookup_relationship == src.m_lookup_relationship);
  bResult = bResult && (m_strLookupField == src.m_strLookupField);

  bResult = bResult && (m_calculation == src.m_calculation);

  bResult = bResult && (m_visible == src.m_visible);

  bResult = bResult && (m_default_formatting == src.m_default_formatting);

  return bResult;
}

bool Field::operator!=(const Field& src) const
{
  return !(operator==(src));
}

Field* Field::clone() const
{
  return new Field(*this);
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

sharedptr<Relationship> Field::get_lookup_relationship() const
{
  return m_lookup_relationship;
}

void Field::set_lookup_relationship(const sharedptr<Relationship>& relationship)
{
  m_lookup_relationship = relationship;
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
  return ( m_lookup_relationship && (!m_strLookupField.empty()) );
}

namespace { //anonymous

//A copy of PQescapeString from the Postgres source, to avoid linking with libpql directly, 
//and to use until we can use the latest libgda, which has an equivalent.

#define SQL_STR_DOUBLE(ch)	((ch) == '\'' || (ch) == '\\')

/*
 * Escaping arbitrary strings to get valid SQL literal strings.
 *
 * Replaces "\\" with "\\\\" and "'" with "''".
 *
 * length is the length of the source string.  (Note: if a terminating NUL
 * is encountered sooner, PQescapeString stops short of "length"; the behavior
 * is thus rather like strncpy.)
 *
 * For safety the buffer at "to" must be at least 2*length + 1 bytes long.
 * A terminating NUL character is added to the output string, whether the
 * input is NUL-terminated or not.
 *
 * Returns the actual length of the output (not counting the terminating NUL).
 */
size_t
Glom_PQescapeString(char *to, const char *from, size_t length)
{
	const char *source = from;
	char	   *target = to;
	size_t		remaining = length;

	while (remaining > 0 && *source != '\0')
	{
		if (SQL_STR_DOUBLE(*source))
			*target++ = *source;
		*target++ = *source++;
		remaining--;
	}

	/* Write the terminating NUL character. */
	*target = '\0';

	return target - to;
}

} //anonymous

static std::string glom_escape_text(const std::string src)
{
  if(src.empty())
    return "''"; //We want to ignore the concept of NULL strings, and deal only with empty strings.
  else
  {
    const size_t len = src.size();
    char* to = (char*)malloc(sizeof(char) * 2 * (len + 1)); //recommended size for to.
    const size_t len_escaped = Glom_PQescapeString(to, src.c_str(), len);
    if(!len_escaped)
    {
      std::cerr << "glom_escape_text(): Glom_PQescapeString() failed with text: " << src << std::endl;

      if(to)
        free(to);

      return "''";
    }
    else
    {
      std::string escaped(to, len_escaped);
      free(to);

      //Also escape any ";" characters, because these also cause problems, at least with libgda:
      //See bug #326325.
      escaped = GlomUtils::string_replace(escaped, ";", "\\073");

      return ("'" + escaped + "'"); //Add single-quotes. Actually escape it 
      //std::cout << "glom_escape_text: escaped and quoted: " << str << std::endl;
    }
  }
}

Glib::ustring Field::sql(const Gnome::Gda::Value& value) const
{
  //g_warning("Field::sql: glom_type=%d", get_glom_type());

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
      case(TYPE_IMAGE):
      {
        return "NULL";
        break;
      }
      case(TYPE_INVALID):
      {
        g_warning("Field::sql(): The field type is INVALID.");
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
      {
        str = glom_escape_text(value.get_string());
      }

      break;
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
    case(TYPE_IMAGE):
    {
      if(value.get_value_type() == Gnome::Gda::VALUE_TYPE_BINARY)
      {
        long buffer_size = 0;
        const gpointer buffer = value.get_binary(buffer_size);
        if(buffer && buffer_size)
        {
          const std::string escaped_binary_data = std::string((char*)buffer, buffer_size);
          //Now escape that text (to convert \ to \\, for instance):
          str = glom_escape_text(escaped_binary_data) /* has quotes */ + "::bytea";

          //Use this when libgda unescapes the binary data internally, as it should: 
          //str = "'" + GlomConversions::get_escaped_binary_data((guint8*)buffer, buffer_size) + "'::bytea";
        }
      }
      else
        g_warning("Field::sql(): glom_type is TYPE_IMAGE but gda type is not VALUE_TYPE_BINARY");

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

Glib::ustring Field::sql_find(const Gnome::Gda::Value& value) const
{
  switch(get_glom_type())
  {
    case(TYPE_TEXT):
    {
      //% means 0 or more characters.
      
      if(value.is_null())
        return "''"; //We want to ignore the concept of NULL strings, and deal only with empty strings.
      else
        return ("'%" + value.to_string() + "%'"); //Add single-quotes. Actually escape it.
        
      break;
    }
    case(TYPE_DATE):
    case(TYPE_TIME):
    case(TYPE_NUMERIC):
    case(TYPE_BOOLEAN):
    default:
    {
      return sql(value);
      break;
    }
  }
}

Glib::ustring Field::sql_find_operator() const
{
  switch(get_glom_type())
  {
    case(TYPE_TEXT):
    {
      return "ILIKE"; //"LIKE"; ILIKE is a postgres extension for locale-dependent case-insensitive matches.
      break;
    }
    case(TYPE_DATE):
    case(TYPE_TIME):
    case(TYPE_NUMERIC):
    case(TYPE_BOOLEAN):
    default:
    {
      return "=";
      break;
    }
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

bool Field::get_auto_increment() const
{
  return m_field_info.get_auto_increment();
}

void Field::set_auto_increment(bool val)
{
  m_field_info.set_auto_increment(val);
}

bool Field::get_primary_key() const
{
  return m_field_info.get_primary_key();
}

void Field::set_primary_key(bool val)
{
  m_field_info.set_primary_key(val);
}

bool Field::get_unique_key() const
{
  return m_field_info.get_unique_key();
}

void Field::set_unique_key(bool val)
{
  m_field_info.set_unique_key(val);
}

Gnome::Gda::Value Field::get_default_value() const
{
  return m_field_info.get_default_value();
}

void Field::set_default_value(const Gnome::Gda::Value& val)
{
  m_field_info.set_default_value(val);
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
    m_map_gda_type_to_glom_type[Gnome::Gda::VALUE_TYPE_BINARY] = TYPE_IMAGE;
        
    m_map_glom_type_to_gda_type[TYPE_NUMERIC] = Gnome::Gda::VALUE_TYPE_NUMERIC;
    m_map_glom_type_to_gda_type[TYPE_TEXT] = Gnome::Gda::VALUE_TYPE_STRING;
    m_map_glom_type_to_gda_type[TYPE_TIME] = Gnome::Gda::VALUE_TYPE_TIME;
    m_map_glom_type_to_gda_type[TYPE_DATE] = Gnome::Gda::VALUE_TYPE_DATE;
    m_map_glom_type_to_gda_type[TYPE_BOOLEAN] = Gnome::Gda::VALUE_TYPE_BOOLEAN;
    m_map_glom_type_to_gda_type[TYPE_IMAGE] = Gnome::Gda::VALUE_TYPE_BINARY;
        
    m_map_type_names_ui[TYPE_INVALID] = _("Invalid");
    m_map_type_names_ui[TYPE_NUMERIC] = _("Number");
    m_map_type_names_ui[TYPE_TEXT] = _("Text");
    m_map_type_names_ui[TYPE_TIME] = _("Time");
    m_map_type_names_ui[TYPE_DATE] = _("Date");    
    m_map_type_names_ui[TYPE_BOOLEAN] = _("Boolean");
    m_map_type_names_ui[TYPE_IMAGE] = _("Image");

    //Non-translated names used for the document:
    m_map_type_names[TYPE_INVALID] = "Invalid";
    m_map_type_names[TYPE_NUMERIC] = "Number";
    m_map_type_names[TYPE_TEXT] = "Text";
    m_map_type_names[TYPE_TIME] = "Time";
    m_map_type_names[TYPE_DATE] = "Date";
    m_map_type_names[TYPE_BOOLEAN] = "Boolean";
    m_map_type_names[TYPE_IMAGE] = "Image";
    
    
    //Conversions:
    //These are the conversions know to be supported by the used version of postgres
    m_map_conversions.clear();
  
    type_list_conversion_targets list_conversions;
  
    //Numeric:
    list_conversions.clear();
    list_conversions.push_back(Field::TYPE_BOOLEAN);
    list_conversions.push_back(Field::TYPE_TEXT);
    list_conversions.push_back(Field::TYPE_DATE);
    list_conversions.push_back(Field::TYPE_TIME);
    m_map_conversions[Field::TYPE_NUMERIC] = list_conversions;
    
    //Text:
    list_conversions.clear();
    list_conversions.push_back(Field::TYPE_BOOLEAN);
    list_conversions.push_back(Field::TYPE_NUMERIC);
    list_conversions.push_back(Field::TYPE_DATE);
    list_conversions.push_back(Field::TYPE_TIME);
    m_map_conversions[Field::TYPE_TEXT] = list_conversions;
    
    //Boolean:
    list_conversions.clear();
    list_conversions.push_back(Field::TYPE_TEXT);
    list_conversions.push_back(Field::TYPE_NUMERIC);
    list_conversions.push_back(Field::TYPE_DATE);
    list_conversions.push_back(Field::TYPE_TIME);
    m_map_conversions[Field::TYPE_BOOLEAN] = list_conversions;
    
    //Date:
    list_conversions.clear();
    list_conversions.push_back(Field::TYPE_TEXT);
    list_conversions.push_back(Field::TYPE_NUMERIC);
    list_conversions.push_back(Field::TYPE_BOOLEAN);
    m_map_conversions[Field::TYPE_DATE] = list_conversions;
    
    //Time:
    list_conversions.clear();
    list_conversions.push_back(Field::TYPE_TEXT);
    list_conversions.push_back(Field::TYPE_NUMERIC);
    list_conversions.push_back(Field::TYPE_BOOLEAN);
    m_map_conversions[Field::TYPE_TIME] = list_conversions;
    
  
    m_maps_inited = true;
  }
}

//static:
bool Field::get_conversion_possible(glom_field_type field_type_src, glom_field_type field_type_dest)
{
  type_map_conversions::const_iterator iterFind = m_map_conversions.find(field_type_src);
  if(iterFind != m_map_conversions.end())
  {
    const type_list_conversion_targets list_conversions = iterFind->second;
    type_list_conversion_targets::const_iterator iterConversionFind = std::find(list_conversions.begin(), list_conversions.end(), field_type_dest);
    if(iterConversionFind != list_conversions.end())
      return true; //Success: conversion found.
  }
  
  return false; //failed.
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

bool Field::get_has_calculation() const
{
  return !m_calculation.empty();
}

Field::type_list_strings Field::get_calculation_fields() const
{
  //TODO: Use regex, for instance with pcre here?
  //TODO: Better?: Run the calculation on some example data, and record the touched fields? But this could not exercise every code path.
  //TODO_Performance: Just cache the result whenever m_calculation changes.

  type_list_strings result;

  Glib::ustring::size_type index = 0;
  const Glib::ustring::size_type count = m_calculation.size();
  const Glib::ustring prefix = "record[\"";
  const Glib::ustring::size_type prefix_size = prefix.size();

  while(index < count)
  {
    Glib::ustring::size_type pos_find = m_calculation.find(prefix, index);
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


Field::type_list_strings Field::get_calculation_relationships() const
{
  //TODO: Use regex, for instance with pcre here?
  //TODO: Better?: Run the calculation on some example data, and record the touched fields? But this could not exercise every code path.
  //TODO_Performance: Just cache the result whenever m_calculation changes.

  type_list_strings result;

  Glib::ustring::size_type index = 0;
  const Glib::ustring::size_type count = m_calculation.size();
  const Glib::ustring prefix = "record.related[\"";
  const Glib::ustring::size_type prefix_size = prefix.size();

  while(index < count)
  {
    Glib::ustring::size_type pos_find = m_calculation.find(prefix, index);
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
