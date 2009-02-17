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
#include <glom/libglom/connectionpool.h>
#include "glomconversions.h"
#include <glom/libglom/utils.h>
#include <glibmm/i18n.h>

namespace Glom
{

//Initialize static data:
Field::type_map_gda_type_to_glom_type Field::m_map_gda_type_to_glom_type;
Field::type_map_glom_type_to_gda_type Field::m_map_glom_type_to_gda_type;
Field::type_map_type_names Field::m_map_type_names;
Field::type_map_type_names Field::m_map_type_names_ui;
Field::type_map_conversions Field::m_map_conversions;
bool Field::m_maps_inited = false;


Field::Field()
: m_glom_type(TYPE_INVALID),
  m_field_info(Gnome::Gda::Column::create()),
  m_visible(true),
  m_primary_key(false),
  m_unique_key(false)
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
  m_field_info = src.m_field_info->copy();

  m_lookup_relationship = src.m_lookup_relationship;
  m_strLookupField = src.m_strLookupField;

  m_calculation = src.m_calculation;

  m_visible = src.m_visible;

  m_primary_key = src.m_primary_key;
  m_unique_key = src.m_unique_key;

  m_default_formatting = src.m_default_formatting;

  return *this;
}

bool Field::operator==(const Field& src) const
{
  return TranslatableItem::operator==(src)
         && (m_field_info->equal(src.m_field_info))
         && (m_glom_type == src.m_glom_type)
         && (m_lookup_relationship == src.m_lookup_relationship)
         && (m_strLookupField == src.m_strLookupField)
         && (m_calculation == src.m_calculation)
         && (m_visible == src.m_visible)
         && (m_primary_key == src.m_primary_key)
         && (m_unique_key == src.m_unique_key)
         && (m_default_formatting == src.m_default_formatting);
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

Glib::RefPtr<Gnome::Gda::Column> Field::get_field_info()
{
  return m_field_info;
}

Glib::RefPtr<const Gnome::Gda::Column> Field::get_field_info() const
{
  return m_field_info;
}

void Field::set_field_info(const Glib::RefPtr<Gnome::Gda::Column>& fieldinfo)
{
  m_field_info = fieldinfo;

  // Set glom type from fieldinfo if it represents a different type.
  // Also take fallback types into account as fieldinfo might originate from
  // the database system directly.
  GType new_type = fieldinfo->get_g_type();

  GType cur_type = G_TYPE_NONE;
  if(get_glom_type() != TYPE_INVALID)
  {
    cur_type = get_gda_type_for_glom_type(get_glom_type());

    const FieldTypes* pFieldTypes = NULL;

    ConnectionPool* pConnectionPool = ConnectionPool::get_instance();
    if(pConnectionPool)
      pFieldTypes = pConnectionPool->get_field_types();

    if(pFieldTypes)
    {
      while(cur_type != new_type && cur_type != G_TYPE_NONE)
        cur_type = pFieldTypes->get_fallback_type_for_gdavaluetype(cur_type);
    }
  }

  if(cur_type == G_TYPE_NONE)
    set_glom_type( get_glom_type_for_gda_type(fieldinfo->get_g_type()) );
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
//TODO: Now that we use libgda-3.0, actually use that equivalent.

#define SQL_STR_DOUBLE(ch) ((ch) == '\'' || (ch) == '\\')

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
		if(SQL_STR_DOUBLE(*source))
			*target++ = *source;

		*target++ = *source++;
		remaining--;
	}

	/* Write the terminating NUL character. */
	*target = '\0';

	return target - to;
}

} //anonymous

/// Escape text, including text that is the result of get_escaped_binary_data().
static std::string glom_escape_text(const std::string& src)
{
  if(src.empty())
    return "''"; //We want to ignore the concept of NULL strings, and deal only with empty strings.
  else
  {
    const size_t len = src.size();
    char* to = (char*)malloc(sizeof(char) * 2 * (len + 1)); //recommended size for to.
    //TODO: Use gda_data_handler_get_sql_from_value() instead.
    //See gda_server_provider_get_data_handler_g_type().
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
      escaped = Utils::string_replace(escaped, ";", "\\073");

      return ("'" + escaped + "'"); //Add single-quotes. Actually escape it 
      //std::cout << "glom_escape_text: escaped and quoted: " << str << std::endl;
    }
  }
}

Glib::ustring Field::sql(const Gnome::Gda::Value& value, sql_format format) const
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
        str = value.get_string();
        str = glom_escape_text(str);
      }

      break;
    }
    case(TYPE_DATE):
    case(TYPE_TIME):
    {
      NumericFormat format_ignored; //Because we use ISO format.
      str = Conversions::get_text_for_gda_value(m_glom_type, value, std::locale() /* SQL uses the C locale */, format_ignored, true /* ISO standard */);
      if(str != "NULL")
         str = "'" + str + "'"; //Add single-quotes.

      break;
    }
    case(TYPE_NUMERIC):
    {
      str =  Conversions::get_text_for_gda_value(m_glom_type, value, std::locale() /* SQL uses the C locale */); //No quotes for numbers.
      break;
    }
    case(TYPE_BOOLEAN):
    {
      if(G_VALUE_TYPE(value.gobj()) == G_TYPE_BOOLEAN)
        str = (value.get_boolean() ? "TRUE" : "FALSE" );
      else
        str = "FALSE";

      break;
    }
    case(TYPE_IMAGE):
    {
      std::cerr << "Field::sql(): Field type TYPE_IMAGE is not supported. A SQL parameter should be used instead." << std::endl;
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

Glib::ustring Field::sql(const Gnome::Gda::Value& value) const
{
  sql_format format = ConnectionPool::get_instance()->get_sql_format();
  return sql(value, format);
}

Glib::ustring Field::to_file_format(const Gnome::Gda::Value& value) const
{
  return to_file_format(value, m_glom_type);
}

Glib::ustring Field::to_file_format(const Gnome::Gda::Value& value, glom_field_type glom_type)
{
  if(glom_type == TYPE_IMAGE)
  {
    if(!value.gobj() || (value.get_value_type() != GDA_TYPE_BINARY))
      return Glib::ustring();
 
    const GdaBinary* gdabinary = gda_value_get_binary(value.gobj());
    if(!gdabinary)
      return Glib::ustring();
    else
    {
      gchar* str = gda_binary_to_string(gdabinary, 0);
      return (str) ? Glib::ustring(Glib::ScopedPtr<char>(str).get())
        : Glib::ustring();
    }
  }
  
  NumericFormat format_ignored; //Because we use ISO format.
  return Conversions::get_text_for_gda_value(glom_type, value, std::locale() /* SQL uses the C locale */, format_ignored, true /* ISO standard */);
}

namespace
{
  // Changes the type of a given value
  void value_reinit(GValue* value, GType g_type)
  {
    if(G_IS_VALUE(value) && G_VALUE_TYPE(value) != g_type)
      g_value_unset(value);

    if(!G_IS_VALUE(value))
      g_value_init(value, g_type);
  }
}

Gnome::Gda::Value Field::from_file_format(const Glib::ustring& str, bool& success) const
{
  return from_file_format(str, m_glom_type, success);
}

Gnome::Gda::Value Field::from_file_format(const Glib::ustring& str, glom_field_type glom_type, bool& success)
{
  success = true;
  
  if(glom_type == TYPE_IMAGE)
  {
    GdaBinary* gdabinary = (GdaBinary*)g_malloc0(sizeof(GdaBinary));
    success = gda_string_to_binary(str.c_str(), gdabinary);
    if(!success)
      return Gnome::Gda::Value();
    else
    {
      Gnome::Gda::Value value;
      value_reinit(value.gobj(), GDA_TYPE_BINARY);
      gda_value_take_binary(value.gobj(), gdabinary);
      return value; 
    }
  }
  else
  {
    NumericFormat format_ignored; //Because we use ISO format.
    return Conversions::parse_value(glom_type, str, format_ignored, success, true);
  }
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
  return m_field_info->get_name();
}

void Field::set_name(const Glib::ustring& value)
{
  m_field_info->set_name(value);
}

bool Field::get_auto_increment() const
{
  return m_field_info->get_auto_increment();
}

void Field::set_auto_increment(bool val)
{
  m_field_info->set_auto_increment(val);
}

bool Field::get_primary_key() const
{
  //TODO_gda: return m_field_info->get_primary_key();
  return m_primary_key;
}

void Field::set_primary_key(bool val)
{
  m_primary_key = val;
  //TODO_gda: m_field_info->set_primary_key(val);
}

bool Field::get_unique_key() const
{
  //TODO_gda: return m_field_info->get_unique_key();
  return m_unique_key;
}

void Field::set_unique_key(bool val)
{
  //TODO_gda: m_field_info->set_unique_key(val);
  m_unique_key = val;
}

Gnome::Gda::Value Field::get_default_value() const
{
  return m_field_info->get_default_value();
}

void Field::set_default_value(const Gnome::Gda::Value& value)
{
  m_field_info->set_default_value(value);
}

Glib::ustring Field::get_sql_type() const
{
  if(false) //See get_next_auto_increment_value() //m_field_info->get_auto_increment())
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
        const GType fieldType = m_field_info->get_g_type();
        strType = pFieldTypes->get_string_name_for_gdavaluetype(fieldType);
      }
    }

    if(strType == "unknowntype")
    {
      g_warning("Field::get_sql_type(): returning unknowntype for field name=%s , glom_type=%d, gda_type=%d", get_name().c_str(), get_glom_type(), (int)m_field_info->get_g_type());
    }

    return strType;
  }
}

Glib::ustring Field::get_gda_type_name() const
{
  //return g_type_name( get_gda_type_for_glom_type(m_glom_type) );
  return g_type_name( m_field_info->get_g_type());
}

Glib::RefPtr<Gnome::Gda::Holder> Field::get_holder(const Gnome::Gda::Value& value, const Glib::ustring& name) const
{
  const Glib::ustring real_name = name.empty() ? get_name() : name;

  const GType gtype = value.get_value_type();

  const GType field_type = m_field_info->get_g_type();
  if(m_field_info->get_g_type() != gtype)
  {
    // This currently happens with SQLite for pictures. The value type is
    // GdaBinary, but the field type is GdaBlob. This is not an optimal
    // solution this way. We should maybe check fallback types here, or
    // investigate why the field type is not GdaBinary as well.
    std::cout << "DEBUG: Field::get_holder(): Field type " << g_type_name(field_type) << " and value type " << g_type_name(gtype) << " don't match." << std::endl;
  }

  Glib::RefPtr<Gnome::Gda::Holder> holder = Gnome::Gda::Holder::create(gtype, real_name);
  holder->set_value_as_value(value);
  return holder;
}

Glib::ustring Field::get_gda_holder_string(const Glib::ustring& name) const
{
  const Glib::ustring real_name = name.empty() ? get_name() : name;
  return "##" + real_name + "::" + get_gda_type_name();
}

/// Ignores any part of FieldAttributes that libgda does not properly fill.
bool Field::field_info_from_database_is_equal(const Glib::RefPtr<const Gnome::Gda::Column>& field)
{
  Glib::RefPtr<Gnome::Gda::Column> temp = m_field_info->copy();

  temp->set_auto_increment( field->get_auto_increment() ); //Don't compare this, because the data is incorrect when libgda reads it from the database.

  Gnome::Gda::Value value = field->get_default_value();
  temp->set_default_value(value); //Don't compare this, because the data is incorrect when libgda reads it from the database.

  //TODO_gda: temp->set_primary_key( field->get_primary_key() ); //Don't compare this, because the data is incorrect when libgda reads it from the database.

  return temp->equal(field); 
}

Field::glom_field_type Field::get_glom_type_for_gda_type(GType gda_type)
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
      // g_warning("FieldType::FieldType(GType gda_type): Invalid gda type: %d",  gda_type);
    }
  }

  return result;
}

GType Field::get_gda_type_for_glom_type(Field::glom_field_type glom_type)
{
  init_map();

  //Get the ideal gda type used for that glom type;
  type_map_glom_type_to_gda_type::iterator iterFind = m_map_glom_type_to_gda_type.find(glom_type);
  GType ideal_gda_type = G_TYPE_NONE;
  if(iterFind != m_map_glom_type_to_gda_type.end())
    ideal_gda_type = iterFind->second;

  if(ideal_gda_type == G_TYPE_NONE)
  {
    g_warning("Field::get_gda_type_for_glom_type(): Returning G_TYPE_NONE for glom_type=%d", glom_type);
  }

  //std::cout << "Field::get_gda_type_for_glom_type(): returning: " << g_type_name(ideal_gda_type) << std::endl;
  
  return ideal_gda_type;
}

//static:
void Field::init_map()
{
  if(!m_maps_inited)
  {
    //Fill maps.

    //Ideals:
    m_map_gda_type_to_glom_type[GDA_TYPE_NUMERIC] = TYPE_NUMERIC;
    m_map_gda_type_to_glom_type[G_TYPE_INT] = TYPE_NUMERIC; //Only for "serial" (auto-increment) fields.
    m_map_gda_type_to_glom_type[G_TYPE_STRING] = TYPE_TEXT;
    m_map_gda_type_to_glom_type[GDA_TYPE_TIME] = TYPE_TIME;
    m_map_gda_type_to_glom_type[G_TYPE_DATE] = TYPE_DATE;
    m_map_gda_type_to_glom_type[G_TYPE_BOOLEAN] = TYPE_BOOLEAN;
    m_map_gda_type_to_glom_type[GDA_TYPE_BINARY] = TYPE_IMAGE;

    m_map_glom_type_to_gda_type[TYPE_NUMERIC] = GDA_TYPE_NUMERIC;
    m_map_glom_type_to_gda_type[TYPE_TEXT] = G_TYPE_STRING;
    m_map_glom_type_to_gda_type[TYPE_TIME] = GDA_TYPE_TIME;
    m_map_glom_type_to_gda_type[TYPE_DATE] = G_TYPE_DATE;
    m_map_glom_type_to_gda_type[TYPE_BOOLEAN] = G_TYPE_BOOLEAN;
    m_map_glom_type_to_gda_type[TYPE_IMAGE] = GDA_TYPE_BINARY;

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
    //to_date(numeric) was supported in 8.2 but not in 8.3: list_conversions.push_back(Field::TYPE_DATE);
    //to_timestamp(numeric) was supported in 8.2 but not in 8.3: list_conversions.push_back(Field::TYPE_TIME);
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
    //to_timestamp(numeric) was supported in 8.2 but not in 8.3: list_conversions.push_back(Field::TYPE_DATE);
    //to_timestamp(numeric) was supported in 8.2 but not in 8.3: list_conversions.push_back(Field::TYPE_TIME);
    m_map_conversions[Field::TYPE_BOOLEAN] = list_conversions;

    //Date:
    list_conversions.clear();
    list_conversions.push_back(Field::TYPE_TEXT);
    //to_number(textcat()) was supported in 8.2 but not in 8.3: list_conversions.push_back(Field::TYPE_NUMERIC);
    //to_number(textcat()) was supported in 8.2 but not in 8.3: list_conversions.push_back(Field::TYPE_BOOLEAN);
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

  type_map_type_names::iterator iterFind = m_map_type_names_ui.find(glom_type);
  if(iterFind != m_map_type_names_ui.end())
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

} //namespace Glom
