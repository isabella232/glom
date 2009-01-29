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
  m_data = src.m_data;

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
         && (m_data == src.m_data)
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

#define ISFIRSTOCTDIGIT(CH) ((CH) >= '0' && (CH) <= '3')
#define ISOCTDIGIT(CH) ((CH) >= '0' && (CH) <= '7')
#define OCTVAL(CH) ((CH) - '0')

/// Unescape text that was escaped by the above function. Only unescapes
/// quoted strings. Nonquoted strings are returned without being modified.
static std::string glom_unescape_text(const std::string& str)
{
  std::string::const_iterator iter = str.begin();
  if(iter == str.end()) return str;  // Empty string
  if(*iter != '\'') return str; // Non-quoted
  ++ iter;

  std::string result;

  while(iter != str.end())
  {
    // End here if this is the terminating quotation character, or unescape
    // '' to '.
    if(*iter == '\'')
    {
      ++ iter;
      if(iter == str.end() || *iter != '\'') break;
      result += '\'';
      ++ iter;
    }
    /* double quotes are not escaped, so don't unescape them. */
#if 0
    // Unescape "" to ".
    else if(*iter == '\"')
    {
      ++ iter;
      if(iter == str.end()) break;
      result += '\"';
      ++ iter;
    }
#endif
    // Escape sequence beginning with backslash.
    else if(*iter == '\\')
    {
      ++ iter;
      if(iter == str.end()) break;

      // Escaped backslash
      if(*iter == '\\')
      {
        result += '\\';
        ++ iter;
      }
      // Some octal representation
      else if(ISFIRSTOCTDIGIT(*iter))
      {
        unsigned char byte = OCTVAL(*iter);

        ++ iter;
        if(iter != str.end() && ISOCTDIGIT(*iter))
        {
          byte = (byte << 3) | OCTVAL(*iter);
          ++ iter;
          if(iter != str.end() && ISOCTDIGIT(*iter))
          {
            byte = (byte << 3) | OCTVAL(*iter);
            ++ iter;
          }
        }

        result += byte;
      }
    }
    else
    {
      // Take char as is
      result += *iter;
      ++ iter;
    }
  }

  return result;
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
      if(value.get_value_type() == GDA_TYPE_BINARY)
      {
        long buffer_length;
        const guchar* buffer = value.get_binary(buffer_length);
        if(buffer && buffer_length > 0)
        {
          if(format == SQL_FORMAT_POSTGRES)
          {
            //Get the escaped text that represents the binary data:
            const std::string escaped_binary_data = Conversions::escape_binary_data_postgres((guint8*)buffer, buffer_length);
            //Now escape that text (to convert \ to \\, for instance):
            //The E prefix indicates ""escape" string constants, which are an extension to the SQL standard"
            //Otherwise, we get a warning when using large escaped strings:
            if(format == SQL_FORMAT_POSTGRES)
              str = "E" + glom_escape_text(escaped_binary_data) /* has quotes */ + "::bytea";
          }
          else
          {
            const std::string escaped_binary_data = Conversions::escape_binary_data_sqlite((guint8*)buffer, buffer_length);
            str = "x'" + escaped_binary_data + "'";
          }
        }
      }
      else
      {
        g_warning("Field::sql(): glom_type is TYPE_IMAGE but gda type is not VALUE_TYPE_BINARY");
      }

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

Gnome::Gda::Value Field::from_sql(const Glib::ustring& str, sql_format format, bool& success) const
{
  success = true;
  switch(m_glom_type)
  {
  case TYPE_TEXT:
    {
      return Gnome::Gda::Value(glom_unescape_text(str));
    }
  case TYPE_DATE:
  case TYPE_TIME:
    {
      if(str == "NULL") return Gnome::Gda::Value();
      Glib::ustring unescaped = glom_unescape_text(str);

      NumericFormat format_ignored; //Because we use ISO format.
      return Conversions::parse_value(m_glom_type, unescaped, format_ignored, success, true);
    }
  case TYPE_NUMERIC:
    {
      //No quotes for numbers.
      NumericFormat format_ignored; //Because we use ISO format.
      return Conversions::parse_value(m_glom_type, str, format_ignored, success, true);
    }
  case TYPE_BOOLEAN:
    {
      if(str.lowercase() == "true")
        return Gnome::Gda::Value(true);
      return Gnome::Gda::Value(false);
    }
  case TYPE_IMAGE:
    {
      if(str == "NULL") return Gnome::Gda::Value();

      // We store the data into the format E'some escaped data'::bytea, and we
      // now expect it in exactly that format now.
      // We operate on the raw std::string since access into the Glib::ustring
      // is expensive, and we only check for ASCII stuff anyway.
      const std::string& raw = str.raw();

      Glib::ustring unescaped;
      switch(format)
      {
      case SQL_FORMAT_POSTGRES:
        if(raw.length() >= 10 &&
           raw.compare(0, 2, "E'") == 0 && raw.compare(raw.length() - 8, 8, "'::bytea") == 0)
        {
          unescaped = glom_unescape_text(raw.substr(1, raw.length() - 8));
	}
	// The E can be omitted, and it is in some example files, such as
	// example_smallbusiness.glom. We need to parse this to convert the
	// data into sqlite format, when the small bussiness example is loaded
	// into a sqlite database.
	else if(raw.length() >= 9 &&
	        raw.compare(0, 1, "'") == 0 && raw.compare(raw.length() - 8, 8, "'::bytea") == 0)
	{
          unescaped = glom_unescape_text(raw.substr(0, raw.length() - 7));
        }

        if(!unescaped.empty())
        {
          gsize length;
          guint8* binary_data = Conversions::unescape_binary_data_postgres(unescaped, length);
          if(binary_data)
          {
            Gnome::Gda::Value value;
            value.set(binary_data, length);
            g_free(binary_data);
            return value;
          }
        }

        success = false;
        return Gnome::Gda::Value();
      case SQL_FORMAT_SQLITE:
        if(raw.length() >= 3 &&
           (raw.compare(0, 2, "x'") == 0 || raw.compare(0, 2, "X'")) &&
           raw.compare(raw.length() - 1, 1, "'") == 0)
        {
          gsize length;
          guint8* binary_data = Conversions::unescape_binary_data_sqlite(raw.substr(2, raw.length()), length);
          if(binary_data)
          {
            Gnome::Gda::Value value;
            value.set(binary_data, length);
            g_free(binary_data);
            return value;
          }
        }

        success = false;
        return Gnome::Gda::Value();
      default:
        g_assert_not_reached();
        break;
      }
    }
  default:
    g_assert_not_reached();
    break;
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

GType Field::get_gda_g_type() const
{
  // TODO: Can't we just do this here? armin.
  // return get_field_info()->get_g_type();

  switch(m_glom_type)
  {
    case TYPE_NUMERIC:
      return GDA_TYPE_NUMERIC;
    case TYPE_TEXT:
      return G_TYPE_STRING;
    case TYPE_DATE:
      return G_TYPE_DATE;
    case TYPE_TIME:
      return GDA_TYPE_TIME;
    case TYPE_BOOLEAN:
      return G_TYPE_BOOLEAN;
    case TYPE_IMAGE:
      return GDA_TYPE_BINARY;
    default:
      g_assert_not_reached();
  }
}

Glib::ustring Field::get_gda_type() const
{
  return g_type_name(get_gda_g_type());
}

Glib::RefPtr<Gnome::Gda::Holder> Field::get_holder(const Glib::ustring& name) const
{
  Glib::ustring real_name = name.empty() ? get_name() : name;
  Glib::RefPtr<Gnome::Gda::Holder> holder = Gnome::Gda::Holder::create(get_gda_g_type(),
                                                                       real_name);
  holder->set_value_as_value(get_data());
  return holder;
}

Glib::RefPtr<Gnome::Gda::Holder> Field::get_holder(const Gnome::Gda::Value& value, const Glib::ustring& name) const
{
  Glib::RefPtr<Gnome::Gda::Holder> holder = get_holder(name);
  holder->set_value_as_value(value);
  return holder;
}

Glib::ustring Field::get_gda_holder_string(const Glib::ustring& name) const
{
  Glib::ustring real_name = name.empty() ? get_name() : name;
  return "##" + real_name + "::" + get_gda_type();
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
