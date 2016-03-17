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

#include <libglom/data_structure/fieldtypes.h>
#include <iostream> //For debug output
#include <libgda/gda-util.h> // For gda_g_type_to_string
#include <libglom/utils.h>

namespace Glom
{

FieldTypes::FieldTypes(const Glib::RefPtr<Gnome::Gda::Connection>& gda_connection)
{
  // These are documented here:
  // http://library.gnome.org/devel/libgda-4.0/3.99/connection.html#GdaConnectionMetaTypeHead
  enum class GlomGdaDataModelTypesColumns
  {
    NAME = 0,
    GTYPE = 1,
    COMMENTS = 2,
    SYNONYMS = 3
  };
  
  if(gda_connection && gda_connection->is_opened())
  {
    //Read the Types information, so that we can map the string representation of the type (returned by CONNECTION_META_FIELDS) to
    //the Gda::ValueType used by Glib::RefPtr<Gnome::Gda::Column>.
    //This first call to update_meta_store() is also necessary for other calls to get_meta_store_data() elsewhere to succeed.
    Glib::RefPtr<Gnome::Gda::DataModel> data_model_tables;
    if(true) //Already done in ConnectionPool::connect(): gda_connection->update_meta_store())
    {
      data_model_tables = gda_connection->get_meta_store_data(Gnome::Gda::CONNECTION_META_TYPES);
    }

    if(!data_model_tables)
      std::cerr << G_STRFUNC << ": Couldn't get datamodel\n";

    if(data_model_tables && (data_model_tables->get_n_columns() == 0))
    {
      std::cerr << G_STRFUNC << ": get_meta_store_data(Gnome::Gda::CONNECTION_META_TYPES) failed.\n";
    }
    else if(data_model_tables)
    {
      const auto rows = data_model_tables->get_n_rows();
      if(!rows)
      {
        //This happens with our developer user when sharing is activated, and with other extra users.
        //TODO: Find out why.
        std::cout << G_STRFUNC << ": no rows from CONNECTION_META_TYPES. Using default type mappings.\n";
      }

      for(int i = 0; i < rows; ++i)
      {
        const auto value_name = data_model_tables->get_value_at(Utils::to_utype(GlomGdaDataModelTypesColumns::NAME), i);

        //Get the types's string representation:
        Glib::ustring schema_type_string;
        if(value_name.get_value_type() == G_TYPE_STRING)
          schema_type_string = value_name.get_string();
        
        if(!schema_type_string.empty())
        {
          const auto value_gdatype = data_model_tables->get_value_at(Utils::to_utype(GlomGdaDataModelTypesColumns::GTYPE), i);
          if(value_gdatype.get_value_type() == G_TYPE_STRING)
          {
            auto type_string = value_gdatype.get_string();
            const auto gdatype = gda_g_type_from_string(type_string.c_str());

            //std::cout << "debug: schema_type_string=" << schema_type_string << ", gda type=" << gdatype << "(" << g_type_name(gdatype) << ")\n";

            //Save it for later:
            //const auto gdatypestring = gda_g_type_to_string(gdatype);
           
            //std::cout << "schema type: " << schema_type_string << " = gdatype " << (guint)gdatype << "(" << gdatypestring << ")\n";
            
            m_mapGdaTypesToSchemaStrings[gdatype] = schema_type_string; //We save it twice, to just to make searching easier, without using std::find.
            
            //g_warning("debug: schema type: %s = gdatype %d", schema_type_string.c_str(), gdatype);
          }
        }
          
      }
    }
  }

  //Use some default mappings if we could not get them from the database server.
  //For instance, this can happen if the user does not have read access.
  if(m_mapGdaTypesToSchemaStrings.empty())
  {
    fill_with_default_data();
  }

  m_mapFallbackTypes[GDA_TYPE_BINARY] = GDA_TYPE_BLOB;
  m_mapFallbackTypes[GDA_TYPE_NUMERIC] = G_TYPE_DOUBLE;
  m_mapFallbackTypes[GDA_TYPE_TIME] = G_TYPE_STRING;
  m_mapFallbackTypes[G_TYPE_DATE] = G_TYPE_STRING;
}

void FieldTypes::fill_with_default_data()
{
  //This is based on the values normally retrieved from the database server,
  //in the constructor.
  //This is appropriate for PostgreSQL, but SQLite should never need these defaults anyway.
  //TODO: Make something like libgda's static postgres_name_to_g_type() method public?
  m_mapGdaTypesToSchemaStrings[G_TYPE_INT] = "abstime";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "bit";
  m_mapGdaTypesToSchemaStrings[G_TYPE_BOOLEAN] = "bool";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "bpchar";
  m_mapGdaTypesToSchemaStrings[GDA_TYPE_BINARY] = "bytea";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "char";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "cidr";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "circle";
  m_mapGdaTypesToSchemaStrings[G_TYPE_DATE] = "date";
  m_mapGdaTypesToSchemaStrings[G_TYPE_FLOAT] = "float4";
  m_mapGdaTypesToSchemaStrings[G_TYPE_DOUBLE] = "float8";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "gtsvector";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "inet";
  m_mapGdaTypesToSchemaStrings[GDA_TYPE_SHORT] = "int2";
  m_mapGdaTypesToSchemaStrings[G_TYPE_INT] = "int4";
  m_mapGdaTypesToSchemaStrings[G_TYPE_INT64] = "int8";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "interval";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "macaddr";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "money";
  m_mapGdaTypesToSchemaStrings[GDA_TYPE_NUMERIC] = "numeric";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "path";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "pg_node_tree";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "polygon";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "regconfig";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "regdictionary";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "reltime";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "text";
  m_mapGdaTypesToSchemaStrings[GDA_TYPE_TIME] = "time";
  m_mapGdaTypesToSchemaStrings[GDA_TYPE_TIMESTAMP] = "timestamp";
  m_mapGdaTypesToSchemaStrings[GDA_TYPE_TIMESTAMP] = "timestamptz";
  m_mapGdaTypesToSchemaStrings[GDA_TYPE_TIME] = "timetz";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "tinterval";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "tsquery";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "tsvector";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "txid_snapshot";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "uuid";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "varbit";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "varchar";
  m_mapGdaTypesToSchemaStrings[G_TYPE_STRING] = "xml";
}

guint FieldTypes::get_types_count() const
{
  return m_mapGdaTypesToSchemaStrings.size();
}

Glib::ustring FieldTypes::get_string_name_for_gdavaluetype(GType field_type) const
{
  //Special-case gchararray (G_TYPE_STRING) because Gda reports this GType for several 
  //postgres field types (xml, inet, tinterval, etc),
  //though we only care about varchar:
  if(field_type == G_TYPE_STRING)
    return "varchar";

  auto iterFind = m_mapGdaTypesToSchemaStrings.find(field_type);
  if(iterFind == m_mapGdaTypesToSchemaStrings.end())
  {
    auto iterFallback = m_mapFallbackTypes.find(field_type);
    if(iterFallback != m_mapFallbackTypes.end())
      return get_string_name_for_gdavaluetype(iterFallback->second);

    std::cerr << G_STRFUNC << ": returning unknowntype for field_type=" << field_type << " (" << g_type_name(field_type) << ")\n";

    std::cerr << G_STRFUNC << ":   possible types are: \n";
    for(const auto& item : m_mapGdaTypesToSchemaStrings)
    {
      std::cerr << G_STRFUNC << ":     gdatype=" << item.first << " (" << g_type_name(item.first) << "), sqltype=" << item.second << std::endl;
    }
    
    return "unknowntype";
  }
  else
    return iterFind->second;
}

GType FieldTypes::get_fallback_type_for_gdavaluetype(GType field_type) const
{
  auto iter = m_mapFallbackTypes.find(field_type);
  if(iter == m_mapFallbackTypes.end()) return G_TYPE_NONE;
  return iter->second;
}

} //namespace Glom


