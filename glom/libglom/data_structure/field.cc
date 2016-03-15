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

#include <libglom/algorithms_utils.h>
#include <libglom/data_structure/field.h>
#include <libglom/connectionpool.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/utils.h>
#include <libgda/gda-blob-op.h>
#include <glibmm/i18n.h>

#include <iostream>

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
: m_glom_type(glom_field_type::INVALID),
  m_field_info(Gnome::Gda::Column::create()),
  m_visible(true),
  m_primary_key(false),
  m_unique_key(false)
{
  m_translatable_item_type = enumTranslatableItemType::FIELD;
}

Field::Field(const Field& src)
: TranslatableItem(src)
{
  //TODO_Performance: Implement this properly, without the extra copy.
  operator=(src);
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
  auto old_type = m_glom_type;

  // Set glom type from fieldinfo if it represents a different type.
  m_glom_type = fieldtype;

  // Reset default value if the default value type does not match the new
  // glom type.
  // TODO: Should we try to convert the default value to the new type here?
  if(fieldtype != old_type)
    set_default_value(Gnome::Gda::Value()); //TODO: Try to convert it, maybe by rendering and parsing it.
}

Glib::RefPtr<Gnome::Gda::Column> Field::get_field_info()
{
  return m_field_info;
}

Glib::RefPtr<const Gnome::Gda::Column> Field::get_field_info() const
{
  return m_field_info;
}

static std::shared_ptr<const FieldTypes> get_field_types()
{
  auto connection_pool = ConnectionPool::get_instance();
  if(!connection_pool)
    return nullptr;

  return connection_pool->get_field_types();
}

void Field::set_field_info(const Glib::RefPtr<Gnome::Gda::Column>& fieldinfo)
{
  m_field_info = fieldinfo;

  const auto glom_type = get_glom_type();
  const auto new_type = fieldinfo->get_g_type();
  if( (glom_type == glom_field_type::INVALID) &&
    (new_type == GDA_TYPE_NULL)) //GDA_TYPE_NULL is the default for GdaColumn.
  {
    //Don't bother with any of the following checks.
    return;
  }

  // Also take fallback types into account as fieldinfo might originate from
  // the database system directly.
  GType cur_type = G_TYPE_NONE;
  if(glom_type != glom_field_type::INVALID)
  {
    cur_type = get_gda_type_for_glom_type(glom_type);

    const auto field_types = get_field_types();

    if(field_types)
    {
      while(cur_type != new_type && cur_type != G_TYPE_NONE)
        cur_type = field_types->get_fallback_type_for_gdavaluetype(cur_type);
    }
  }

  if(cur_type == G_TYPE_NONE)
    set_glom_type( get_glom_type_for_gda_type(fieldinfo->get_g_type()) );

  const auto value = get_default_value();
  if(!value.is_null())
  {
    // Check that the default value is consistent with the field's type
    if(!value.is_null() && value.get_value_type() != get_gda_data_type_with_fallback(value))
    {
      std::cerr << G_STRFUNC << ": New field's default value type (" << g_type_name(value.get_value_type()) << " does not match field type (" << g_type_name(get_gda_type_for_glom_type(get_glom_type())) << "). Resetting default value.\n";
      m_field_info->set_default_value(Gnome::Gda::Value());
    }
  }
}

GType Field::get_gda_data_type_with_fallback(const Gnome::Gda::Value& value)
{
  auto cur_type = get_gda_type_for_glom_type(get_glom_type());
  const auto field_types = get_field_types();

  // Take into account that value might be one of the fallback types
  if(field_types)
  {
    while(cur_type != value.get_value_type() && cur_type != G_TYPE_NONE)
      cur_type = field_types->get_fallback_type_for_gdavaluetype(cur_type);
  }

  return cur_type;
}

std::shared_ptr<Relationship> Field::get_lookup_relationship() const
{
  return m_lookup_relationship;
}

void Field::set_lookup_relationship(const std::shared_ptr<Relationship>& relationship)
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

Glib::ustring Field::sql(const Gnome::Gda::Value& value, const Glib::RefPtr<Gnome::Gda::Connection>& connection) const
{
  //std::cout << ": glom_type=" << get_glom_type() << std::endl;

  if(value.is_null() && (get_glom_type() == glom_field_type::TEXT))
  {
    return "''"; //We want to ignore the concept of NULL strings, and deal only with empty strings.
  }


  //Use libgda's DataHandler to get the string for SQL:
  Glib::RefPtr<const Gnome::Gda::ServerProvider> provider = connection->get_provider();
  if(!provider)
  {
    std::cerr << G_STRFUNC << ": The ServerProvider was null.\n";
    return Glib::ustring();
  } 

  const auto gda_type = get_gda_type_for_glom_type(m_glom_type);
  Glib::RefPtr<const Gnome::Gda::DataHandler> datahandler = 
    provider->get_data_handler_g_type(connection, gda_type);
  if(datahandler)
  {
    //This is documented as adding the necessary quotes.
    return datahandler->get_sql_from_value(value);
  }
  else
  {
    std::cerr << G_STRFUNC << ": The DataHandler was null.\n";
    return Glib::ustring();
  }
}

#define GLOM_QUOTE_FOR_FILE_FORMAT "\""

Glib::ustring Field::to_file_format(const Gnome::Gda::Value& value) const
{
  return to_file_format(value, m_glom_type);
}

Glib::ustring Field::to_file_format(const Gnome::Gda::Value& value, glom_field_type glom_type)
{
  //Handle glom_field_type::IMAGE specially:
  if(glom_type == glom_field_type::IMAGE)
  {
    if(!value.gobj())
      return Glib::ustring();
 
    gchar* str = nullptr;
    const auto value_type = value.get_value_type();
    if(value_type == GDA_TYPE_BINARY)
    {
      const auto gdabinary = gda_value_get_binary(value.gobj());
      if(!gdabinary)
        return Glib::ustring();
      else
      {
        auto base64 = g_base64_encode(gdabinary->data, gdabinary->binary_length);
        if(!base64)
          return Glib::ustring();
        else
        {
          str = base64;
        }
      }
    }
    else if(value_type == GDA_TYPE_BLOB)
    {
      const auto gdablob = gda_value_get_blob(value.gobj());

      if(!gdablob || !gdablob->op)
        return Glib::ustring();
      else
      {
        if(!gda_blob_op_read_all(gdablob->op, const_cast<GdaBlob*>(gdablob)))
        {
          return Glib::ustring();
        }
        else
        {
          const auto gdabinary = &(gdablob->data);
          auto base64 = g_base64_encode(gdabinary->data, gdabinary->binary_length);
          if(!base64)
            return Glib::ustring();
          else
          {
            str = base64;
          }
        }
      }
    }

    if(!str)
      return Glib::ustring();

    auto result = Glib::ustring(Glib::make_unique_ptr_gfree(str).get());

    //Correction for text representations of image (binary) data:
    //Avoid arbitrary newlines in this text.
    //See libgda bug: https://bugzilla.gnome.org/show_bug.cgi?id=597390
    //though that libgda behaviour will not be changed.
    result = Utils::string_replace(result, "\n", "\\012");
    result = Utils::string_replace(result, "\r", "\\015");
    return Utils::string_replace(result, "\"", "\\042");
  }
  
  NumericFormat format_ignored; //Because we use ISO format.
  const auto result = Conversions::get_text_for_gda_value(glom_type, value, std::locale::classic() /* SQL uses the C locale */, format_ignored, true /* ISO standard */);
  
  //Escape " as "", as specified by the CSV RFC:
  return Utils::string_replace(result, GLOM_QUOTE_FOR_FILE_FORMAT, GLOM_QUOTE_FOR_FILE_FORMAT GLOM_QUOTE_FOR_FILE_FORMAT);
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

Gnome::Gda::Value Field::from_file_format(const Glib::ustring& str, glom_field_type glom_type, bool& success, bool old_image_format)
{
  success = true;

  //Unescape "" to ", because to_file_format() escaped ", as specified by the CSV RFC:
  Glib::ustring string_unescaped;
  if(glom_type == glom_field_type::IMAGE)
  {
    string_unescaped = str; //binary data does not have quote characters so we do not bother to escape or unescape it.
  }
  else
  {
    string_unescaped = 
      Utils::string_replace(str, GLOM_QUOTE_FOR_FILE_FORMAT GLOM_QUOTE_FOR_FILE_FORMAT, GLOM_QUOTE_FOR_FILE_FORMAT);
  }


  if(glom_type == glom_field_type::IMAGE)
  {
    if(string_unescaped.empty())
      return Gnome::Gda::Value();

    if(old_image_format)
    {
      //We used the GDA format before:
      auto gdabinary = gda_string_to_binary(string_unescaped.c_str());
      if(!success || !gdabinary)
        return Gnome::Gda::Value();
      else
      {
        Gnome::Gda::Value value;
        value_reinit(value.gobj(), GDA_TYPE_BINARY);
        gda_value_take_binary(value.gobj(), gdabinary);
        return value;
      }
    } else {
      //What we use now in new files:

      auto gdabinary = g_new(GdaBinary, 1);
      gsize len = 0;
      gdabinary->data = g_base64_decode(string_unescaped.c_str(), &len);
      gdabinary->binary_length = len;

      Gnome::Gda::Value value;
      value_reinit(value.gobj(), GDA_TYPE_BINARY);
      gda_value_take_binary(value.gobj(), gdabinary);
      return value;
    }
  }
  else
  {
    NumericFormat format_ignored; //Because we use ISO format.
    return Conversions::parse_value(glom_type, string_unescaped, format_ignored, success, true);
  }
}

Glib::ustring Field::sql_find(const Gnome::Gda::Value& value, const Glib::RefPtr<Gnome::Gda::Connection>& connection) const
{
  switch(get_glom_type())
  {
    case(glom_field_type::TEXT):
    {
      //% means 0 or more characters.
      
      if(value.is_null())
        return "''"; //We want to ignore the concept of NULL strings, and deal only with empty strings.
      else
        return ("%" + value.to_string() + "%"); //TODO: Escape it before adding "%", but then prevent SqlBuilder from re-escaping it.
    }
    case(glom_field_type::DATE):
    case(glom_field_type::TIME):
    case(glom_field_type::NUMERIC):
    case(glom_field_type::BOOLEAN):
    default:
    {
      return sql(value, connection);
    }
  }
}

Gnome::Gda::SqlOperatorType Field::sql_find_operator() const
{
  switch(get_glom_type())
  {
    case(glom_field_type::TEXT):
    {
      auto connection_pool = ConnectionPool::get_instance();
      if(connection_pool && connection_pool->get_backend())
        return connection_pool->get_string_find_operator();
      else
        return Gnome::Gda::SQL_OPERATOR_TYPE_LIKE; // Default
      break;
    }
    case(glom_field_type::DATE):
    case(glom_field_type::TIME):
    case(glom_field_type::NUMERIC):
    case(glom_field_type::BOOLEAN):
    default:
    {
      return Gnome::Gda::SQL_OPERATOR_TYPE_EQ;
    }
  }

  return Gnome::Gda::SQL_OPERATOR_TYPE_EQ;
}


Glib::ustring Field::get_name() const noexcept
{
  return m_field_info->get_name();
}

void Field::set_name(const Glib::ustring& value) noexcept
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
  // TODO: Allow setting a NULL default value when glom type is invalid?

  // Verify that the value matches the type of the field.
  if(value.is_null() || value.get_value_type() == get_gda_data_type_with_fallback(value))
    m_field_info->set_default_value(value);
  else
    std::cerr << G_STRFUNC << ": Cannot set incompatible default value: Default value has type " << g_type_name(value.get_value_type()) << ", but field has type " << g_type_name(get_gda_type_for_glom_type(get_glom_type())) << std::endl;
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

    auto pConnectionPool = ConnectionPool::get_instance();
    if(pConnectionPool)
    {
      const auto field_types = pConnectionPool->get_field_types();
      if(field_types)
      {
        const auto fieldType = m_field_info->get_g_type();
        strType = field_types->get_string_name_for_gdavaluetype(fieldType);
      }
      else
      {
        std::cerr << G_STRFUNC << ": get_field_types() returned null\n";
      }
    }

    if(strType == "unknowntype")
    {
      std::cerr << G_STRFUNC << ": returning unknowntype for field name=" << get_name() << ", glom_type=" << Utils::to_utype(get_glom_type()) << ", gda_type=" << (int)m_field_info->get_g_type() << std::endl;
    }

    return strType;
  }
}

Glib::ustring Field::get_gda_type_name() const
{
  //return g_type_name( get_gda_type_for_glom_type(m_glom_type) );
  return g_type_name( m_field_info->get_g_type());
}

/// Ignores any part of FieldAttributes that libgda does not properly fill.
bool Field::field_info_from_database_is_equal(const Glib::RefPtr<const Gnome::Gda::Column>& field)
{
  auto temp = m_field_info->copy();

  temp->set_auto_increment( field->get_auto_increment() ); //Don't compare this, because the data is incorrect when libgda reads it from the database.

  const auto value = field->get_default_value();
  temp->set_default_value(value); //Don't compare this, because the data is incorrect when libgda reads it from the database.

  //TODO_gda: temp->set_primary_key( field->get_primary_key() ); //Don't compare this, because the data is incorrect when libgda reads it from the database.

  return temp->equal(field); 
}

Field::glom_field_type Field::get_glom_type_for_gda_type(GType gda_type)
{
  init_map();

  Field::glom_field_type result = glom_field_type::INVALID;

  //Get the glom type used for this gda type:
  {
    auto iterFind = m_map_gda_type_to_glom_type.find(gda_type);
    if(iterFind != m_map_gda_type_to_glom_type.end())
      result = iterFind->second;
    else
    {
      std::cerr << G_STRFUNC << ": Unhandled GType: " << g_type_name(gda_type) << std::endl;
    }
  }

  return result;
}

GType Field::get_gda_type_for_glom_type(Field::glom_field_type glom_type)
{
  init_map();

  //Get the ideal gda type used for that glom type;
  auto iterFind = m_map_glom_type_to_gda_type.find(glom_type);
  GType ideal_gda_type = G_TYPE_NONE;
  if(iterFind != m_map_glom_type_to_gda_type.end())
    ideal_gda_type = iterFind->second;

  if(ideal_gda_type == G_TYPE_NONE)
  {
    std::cerr << G_STRFUNC << ": Returning G_TYPE_NONE for glom_type=" << Utils::to_utype(glom_type) << std::endl;
  }

  //std::cout << "debug: " << G_STRFUNC << ": returning: " << g_type_name(ideal_gda_type) << std::endl;
  
  return ideal_gda_type;
}

//static:
void Field::init_map()
{
  if(!m_maps_inited)
  {
    //Fill maps.

    //Ideals:
    m_map_gda_type_to_glom_type[GDA_TYPE_NUMERIC] = glom_field_type::NUMERIC;
    m_map_gda_type_to_glom_type[G_TYPE_INT] = glom_field_type::NUMERIC; //Only for "serial" (auto-increment) fields.
    m_map_gda_type_to_glom_type[G_TYPE_STRING] = glom_field_type::TEXT;
    m_map_gda_type_to_glom_type[GDA_TYPE_TIME] = glom_field_type::TIME;
    m_map_gda_type_to_glom_type[G_TYPE_DATE] = glom_field_type::DATE;
    m_map_gda_type_to_glom_type[G_TYPE_BOOLEAN] = glom_field_type::BOOLEAN;
    m_map_gda_type_to_glom_type[GDA_TYPE_BINARY] = glom_field_type::IMAGE;

    //SQLite can return a GdaBlob though it can take a GdaBinary:
    m_map_gda_type_to_glom_type[GDA_TYPE_BLOB] = glom_field_type::IMAGE;

    //Extra conversions for GTypes that can be returned by glom_pygda_value_from_pyobject():
    m_map_gda_type_to_glom_type[G_TYPE_DOUBLE] = glom_field_type::NUMERIC;
    //TODO? m_map_gda_type_to_glom_type[GDA_TYPE_TIME] = ;
    //TODO? m_map_gda_type_to_glom_type[GDA_TYPE_TIMESTAMP] = ;

    m_map_glom_type_to_gda_type[glom_field_type::NUMERIC] = GDA_TYPE_NUMERIC;
    m_map_glom_type_to_gda_type[glom_field_type::TEXT] = G_TYPE_STRING;
    m_map_glom_type_to_gda_type[glom_field_type::TIME] = GDA_TYPE_TIME;
    m_map_glom_type_to_gda_type[glom_field_type::DATE] = G_TYPE_DATE;
    m_map_glom_type_to_gda_type[glom_field_type::BOOLEAN] = G_TYPE_BOOLEAN;
    m_map_glom_type_to_gda_type[glom_field_type::IMAGE] = GDA_TYPE_BINARY;

    // Translators: This means an unknown or unnacceptable value type in a database.
    m_map_type_names_ui[glom_field_type::INVALID] = _("Invalid");
    
    // Translators: This means a numeric value type in a database.
    m_map_type_names_ui[glom_field_type::NUMERIC] = _("Number");
    
    // Translators: This means a text/string value type in a database.
    m_map_type_names_ui[glom_field_type::TEXT] = _("Text");
    
    // Translators: This means a time value type in a database.
    m_map_type_names_ui[glom_field_type::TIME] = _("Time");
    
    // Translators: This means a time value type in a database.
    m_map_type_names_ui[glom_field_type::DATE] = _("Date");
    
    // Translators: This means a true/false value type in a database.
    m_map_type_names_ui[glom_field_type::BOOLEAN] = _("Boolean");
    
    // Translators: This means a picture value type in a database.
    m_map_type_names_ui[glom_field_type::IMAGE] = _("Image");

    //Non-translated names used for the document:
    m_map_type_names[glom_field_type::INVALID] = "Invalid";
    m_map_type_names[glom_field_type::NUMERIC] = "Number";
    m_map_type_names[glom_field_type::TEXT] = "Text";
    m_map_type_names[glom_field_type::TIME] = "Time";
    m_map_type_names[glom_field_type::DATE] = "Date";
    m_map_type_names[glom_field_type::BOOLEAN] = "Boolean";
    m_map_type_names[glom_field_type::IMAGE] = "Image";


    //Conversions:
    //These are the conversions know to be supported by the used version of postgres
    m_map_conversions.clear();

    type_list_conversion_targets list_conversions( {
      Field::glom_field_type::BOOLEAN,
      Field::glom_field_type::TEXT} );
    //to_date(numeric) was supported in 8.2 but not in 8.3: list_conversions.emplace_back(Field::glom_field_type::DATE);
    //to_timestamp(numeric) was supported in 8.2 but not in 8.3: list_conversions.emplace_back(Field::glom_field_type::TIME);
    m_map_conversions[Field::glom_field_type::NUMERIC] = list_conversions;

    //Text:
    list_conversions = {
      Field::glom_field_type::BOOLEAN,
      Field::glom_field_type::NUMERIC,
      Field::glom_field_type::DATE,
      Field::glom_field_type::TIME};
    m_map_conversions[Field::glom_field_type::TEXT] = list_conversions;

    //Boolean:
    list_conversions = {
      Field::glom_field_type::TEXT,
      Field::glom_field_type::NUMERIC};
    //to_timestamp(numeric) was supported in 8.2 but not in 8.3: list_conversions.emplace_back(Field::glom_field_type::DATE);
    //to_timestamp(numeric) was supported in 8.2 but not in 8.3: list_conversions.emplace_back(Field::glom_field_type::TIME);
    m_map_conversions[Field::glom_field_type::BOOLEAN] = list_conversions;

    //Date:
    list_conversions = {
      Field::glom_field_type::TEXT};
    //to_number(textcat()) was supported in 8.2 but not in 8.3: list_conversions.emplace_back(Field::glom_field_type::NUMERIC);
    //to_number(textcat()) was supported in 8.2 but not in 8.3: list_conversions.emplace_back(Field::glom_field_type::BOOLEAN);
    m_map_conversions[Field::glom_field_type::DATE] = list_conversions;

    //Time:
    list_conversions = {
      Field::glom_field_type::TEXT,
      Field::glom_field_type::NUMERIC,
      Field::glom_field_type::BOOLEAN};
    m_map_conversions[Field::glom_field_type::TIME] = list_conversions;


    m_maps_inited = true;
  }
}

//static:
bool Field::get_conversion_possible(glom_field_type field_type_src, glom_field_type field_type_dest)
{
  auto iterFind = m_map_conversions.find(field_type_src);
  if(iterFind != m_map_conversions.end())
  {
    const auto list_conversions = iterFind->second;
    if(Utils::find_exists(list_conversions, field_type_dest))
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
  auto iter = result.find(glom_field_type::INVALID);
  if(iter != result.end())
    result.erase(iter);

  return result;
}

//static:
Glib::ustring Field::get_type_name_ui(glom_field_type glom_type)
{
  Glib::ustring result = "Invalid";

  auto iterFind = m_map_type_names_ui.find(glom_type);
  if(iterFind != m_map_type_names_ui.end())
    result = iterFind->second;

  return result;
}

//TODO: Performance, though this is a small map.
//static:
Field::glom_field_type Field::get_type_for_ui_name(const Glib::ustring& glom_type)
{
  glom_field_type result = glom_field_type::INVALID;

  for(const auto& the_pair : m_map_type_names_ui)
  {
    if(the_pair.second == glom_type)
    {
      result = the_pair.first;
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
  const auto count = m_calculation.size();
  const Glib::ustring prefix = "record.related[\"";
  const auto prefix_size = prefix.size();

  while(index < count)
  {
    auto pos_find = m_calculation.find(prefix, index);
    if(pos_find != Glib::ustring::npos)
    {
      auto pos_find_end = m_calculation.find("\"]", pos_find);
      if(pos_find_end  != Glib::ustring::npos)
      {
        Glib::ustring::size_type pos_start = pos_find + prefix_size;
        const auto field_name = m_calculation.substr(pos_start, pos_find_end - pos_start);
        result.emplace_back(field_name);
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
