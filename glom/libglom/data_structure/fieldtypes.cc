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

#include <libglom/data_structure/fieldtypes.h>
#include <iostream> //For debug output
#include <libgda/gda-util.h> // For gda_g_type_to_string
#include <libglom/connectionpool.h>

namespace Glom
{

FieldTypes::FieldTypes(const Glib::RefPtr<Gnome::Gda::Connection>& gda_connection)
{
  // These are documented here:
  // http://library.gnome.org/devel/libgda-4.0/3.99/connection.html#GdaConnectionMetaTypeHead
  enum GlomGdaDataModelTypesColumns
  {
    DATAMODEL_FIELDS_COL_NAME = 0,
    DATAMODEL_FIELDS_COL_GTYPE = 1,
    DATAMODEL_FIELDS_COL_COMMENTS = 2,
    DATAMODEL_FIELDS_COL_SYNONYMS = 3
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
      std::cerr << G_STRFUNC << ": Couldn't get datamodel" << std::endl;

    if(data_model_tables && (data_model_tables->get_n_columns() == 0))
    {
      std::cerr << G_STRFUNC << ": get_meta_store_data(Gnome::Gda::CONNECTION_META_TYPES) failed." << std::endl;
    }
    else if(data_model_tables)
    {
      const int rows = data_model_tables->get_n_rows();
      if(!rows)
      {
        //This happens with our developer user when sharing is activated, and with other extra users.
        //TODO: Find out why.
        std::cout << G_STRFUNC << ": no rows from CONNECTION_META_TYPES. Using default type mappings." << std::endl;
      }

      for(int i = 0; i < rows; ++i)
      {
        const Gnome::Gda::Value value_name = data_model_tables->get_value_at(DATAMODEL_FIELDS_COL_NAME, i);

        //Get the types's string representation:
        Glib::ustring schema_type_string;
        if(value_name.get_value_type() == G_TYPE_STRING)
          schema_type_string = value_name.get_string();
        
        if(!schema_type_string.empty())
        {
          const Gnome::Gda::Value value_gdatype = data_model_tables->get_value_at(DATAMODEL_FIELDS_COL_GTYPE, i);
          if(value_gdatype.get_value_type() == G_TYPE_STRING)
          {
            Glib::ustring type_string = value_gdatype.get_string();
            const GType gdatype = gda_g_type_from_string(type_string.c_str());

            //std::cout << "debug: schema_type_string=" << schema_type_string << ", gda type=" << gdatype << "(" << g_type_name(gdatype) << ")" << std::endl;

            //Save it for later:
            const Glib::ustring gdatypestring = gda_g_type_to_string(gdatype); // TODO: What is this actually used for?
            //std::cout << G_STRFUNC << ": m_mapSchemaStringsToGdaTypes[\"" << schema_type_string << "\"] = " << gdatypestring << ";" << std::endl;
            m_mapSchemaStringsToGdaTypes[schema_type_string] = gdatype;

            
            //std::cout << "schema type: " << schema_type_string << " = gdatype " << (guint)gdatype << "(" << gdatypestring << ")" << std::endl;
            
            m_mapGdaTypesToSchemaStrings[gdatype] = schema_type_string; //We save it twice, to just to make searching easier, without using a predicate.
            
            //g_warning("debug: schema type: %s = gdatype %d", schema_type_string.c_str(), gdatype);
          }
        }
          
      }
    }
  }

  //Use some default mappings if we could not get them from the database server.
  //For instance, this can happen if the user does not have read access.
  if(m_mapSchemaStringsToGdaTypes.empty())
  {
    fill_with_default_data();
  }

  m_mapFallbackTypes[GDA_TYPE_BINARY] = GDA_TYPE_BLOB;
  m_mapFallbackTypes[GDA_TYPE_NUMERIC] = G_TYPE_DOUBLE;
  m_mapFallbackTypes[GDA_TYPE_TIME] = G_TYPE_STRING;
  m_mapFallbackTypes[G_TYPE_DATE] = G_TYPE_STRING;
}

FieldTypes::~FieldTypes()
{
}

void FieldTypes::fill_with_default_data()
{
  //This is based on the values normally retrieved from the database server,
  //in the constructor.
  //This is appropriate for PostgreSQL, but SQLite should never need these defaults anyway.
  //TODO: Make something like libgda's static postgres_name_to_g_type() method public?
  m_mapSchemaStringsToGdaTypes["abstime"] = G_TYPE_INT;
  m_mapSchemaStringsToGdaTypes["bit"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["bool"] = G_TYPE_BOOLEAN;
  m_mapSchemaStringsToGdaTypes["bpchar"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["bytea"] = GDA_TYPE_BINARY;
  m_mapSchemaStringsToGdaTypes["char"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["cidr"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["circle"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["date"] = G_TYPE_DATE;
  m_mapSchemaStringsToGdaTypes["float4"] = G_TYPE_FLOAT;
  m_mapSchemaStringsToGdaTypes["float8"] = G_TYPE_DOUBLE;
  m_mapSchemaStringsToGdaTypes["gtsvector"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["inet"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["int2"] = GDA_TYPE_SHORT;
  m_mapSchemaStringsToGdaTypes["int4"] = G_TYPE_INT;
  m_mapSchemaStringsToGdaTypes["int8"] = G_TYPE_INT64;
  m_mapSchemaStringsToGdaTypes["interval"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["macaddr"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["money"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["numeric"] = GDA_TYPE_NUMERIC;
  m_mapSchemaStringsToGdaTypes["path"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["pg_node_tree"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["polygon"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["regconfig"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["regdictionary"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["reltime"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["text"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["time"] = GDA_TYPE_TIME;
  m_mapSchemaStringsToGdaTypes["timestamp"] = GDA_TYPE_TIMESTAMP;
  m_mapSchemaStringsToGdaTypes["timestamptz"] = GDA_TYPE_TIMESTAMP;
  m_mapSchemaStringsToGdaTypes["timetz"] = GDA_TYPE_TIME;
  m_mapSchemaStringsToGdaTypes["tinterval"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["tsquery"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["tsvector"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["txid_snapshot"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["uuid"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["varbit"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["varchar"] = G_TYPE_STRING;
  m_mapSchemaStringsToGdaTypes["xml"] = G_TYPE_STRING;

  //Fill the reverse map too:
  for(type_mapSchemaStringsToGdaTypes::const_iterator iter = m_mapSchemaStringsToGdaTypes.begin();
    iter != m_mapSchemaStringsToGdaTypes.end(); ++iter)
  {
    const Glib::ustring str = iter->first;
    const GType gtype = iter->second;
    m_mapGdaTypesToSchemaStrings[gtype] = str;
  }
}

guint FieldTypes::get_types_count() const
{
/*
  if(!m_mapSchemaStringsToGdaTypes.empty())
  {
    const type_mapSchemaStringsToGdaTypes::const_iterator iter = m_mapSchemaStringsToGdaTypes.begin();
    const Glib::ustring schema_type_string = iter->first;
    const GType gdatype = iter->second;
    std::cout << G_STRFUNC << ": debug: schema_type_string=" << schema_type_string << ", gdatype=" << g_type_name(gdatype) << std::endl;
  }
*/

  return m_mapSchemaStringsToGdaTypes.size();
}

Glib::ustring FieldTypes::get_string_name_for_gdavaluetype(GType field_type) const
{
  //Special-case gchararray (G_TYPE_STRING) because Gda reports this GType for several 
  //postgres field types (xml, inet, tinterval, etc),
  //though we only care about varchar:
  if(field_type == G_TYPE_STRING)
    return "varchar";

  type_mapGdaTypesToSchemaStrings::const_iterator iterFind = m_mapGdaTypesToSchemaStrings.find(field_type);
  if(iterFind == m_mapGdaTypesToSchemaStrings.end())
  {
    type_mapFallbackTypes::const_iterator iterFallback = m_mapFallbackTypes.find(field_type);
    if(iterFallback != m_mapFallbackTypes.end())
      return get_string_name_for_gdavaluetype(iterFallback->second);

    g_warning("FieldTypes::get_string_name_for_gdavaluetype(): returning unknowntype for field_type=%ld (%s)", static_cast<long>(field_type), g_type_name(field_type));

    g_warning("  possible types are: ");
    for(type_mapGdaTypesToSchemaStrings::const_iterator iter = m_mapGdaTypesToSchemaStrings.begin(); iter != m_mapGdaTypesToSchemaStrings.end(); ++iter)
    {
      g_warning("    gdatype=%ld (%s), sqltype=%s", static_cast<long>(iter->first), g_type_name(iter->first), iter->second.c_str());
    }
    
    return "unknowntype";
  }
  else
    return iterFind->second;
}

GType FieldTypes::get_fallback_type_for_gdavaluetype(GType field_type) const
{
  type_mapFallbackTypes::const_iterator iter = m_mapFallbackTypes.find(field_type);
  if(iter == m_mapFallbackTypes.end()) return G_TYPE_NONE;
  return iter->second;
}

} //namespace Glom


