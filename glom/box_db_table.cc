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

#include "box_db_table.h"
#include <sstream>

Box_DB_Table::Box_DB_Table()
{
}

Box_DB_Table::Box_DB_Table(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Box_DB(cobject, refGlade)
{
}

Box_DB_Table::~Box_DB_Table()
{
}

Glib::ustring Box_DB_Table::get_table_name()
{
  return m_strTableName;
}

void Box_DB_Table::init_db_details(const Glib::ustring& strDatabaseName, const Glib::ustring& strTableName, const Glib::ustring& strWhereClause /* = "" */)
{
  m_strDatabaseName = strDatabaseName;
  m_strTableName = strTableName;
  m_strWhereClause = strWhereClause;

  fill_fields();
  fill_from_database();
}

Glib::ustring Box_DB_Table::get_PrimaryKey_Name()
{
  Glib::ustring strResult;

  guint uiCol = 0;
  bool bPresent = get_field_primary_key(uiCol);

  if(bPresent)
  {
    strResult = m_Fields[uiCol].get_name();
  }
  else
  {
    std::cout << "Box_DB_Table::get_PrimaryKey_Name(): not found in " << m_Fields.size() << " fields." << std::endl;
  }

  return strResult;
}

void Box_DB_Table::fill_fields()
{
/*
  try
  {
    Bakery::BusyCursor(*get_app_window());

    sharedptr<SharedConnection> sharedconnection = connect_to_server();
    if(sharedconnection)
    {
      Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

      bool bKeepTrying = true;
      bool bTriedNew = false;
      while(bKeepTrying)
      {
        //Don't use WHERE clause for list of fields.
        Glib::ustring strWhereClausePiece;
        //if(m_strWhereClause.size())
          //strWhereClausePiece = " WHERE " + m_strWhereClause;

        //TODO_port: We should probably just get the schema here:
        Gnome::Gda::Command command("SELECT * FROM " + m_strTableName + strWhereClausePiece + " LIMIT 1"); //LIMIT 1 means we get only the first row.
        //std::cout << query.str() << std::endl;

        Glib::RefPtr<Gnome::Gda::DataModel> result = connection->execute_single_command(command);
        m_Fields = get_fields_for_datamodel(result);

        //g_warning("Box_DB_Table::fill_fields(): size=%d", m_Fields.size());

        if(m_Fields.size() == 0)
        {
          //No records were returned.
          if(bTriedNew)
          {
            bKeepTrying = false; //Still can't get fields.
          }
          else
          {
            //Add 1 record in order to get the field info.
            //We can only get the fields with records.
            //TODO_port: This is probably not true with libgdamm.
            record_new();
            //We can only get the fields with records.
            bTriedNew = true; //Only try this once.
          }
        }
        else
        {
          bKeepTrying = false; //Succeeded.
        }
      }
    }
  }
  catch(std::exception& ex)
  {
    handle_error(ex);
  }
*/
}

bool Box_DB_Table::record_delete(const Glib::ustring& strPrimaryKeyValue)
{
  if(strPrimaryKeyValue.size())
  {
    return Query_execute( "DELETE FROM " + m_strTableName + " WHERE " + get_table_name() + "." + get_PrimaryKey_Name() + " = " + strPrimaryKeyValue );
  }
  else
  {
    return false; //no primary key
  }
}

Glib::RefPtr<Gnome::Gda::DataModel> Box_DB_Table::record_new(Glib::ustring strPrimaryKeyValue /* = "" */)
{
  if(strPrimaryKeyValue.size() == 0)
  {
    strPrimaryKeyValue = "0"; //To auto-increment.
  }

  return Query_execute( "INSERT INTO " + m_strTableName + " (" + get_PrimaryKey_Name() + ") VALUES (" + strPrimaryKeyValue + ")" );
}

guint Box_DB_Table::get_Entered_Field_count() const
{
  return m_Fields.size();
}

Field Box_DB_Table::get_Entered_Field(guint index) const
{
  //Override this to use Field::set_data() too.
  Field fieldResult;

  //Glom-specific information:
  const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
  if(pDoc)
  {
    const Glib::ustring& strFieldName = m_Fields[index].get_name();
    pDoc->get_field(m_strTableName, strFieldName, fieldResult);
  }

  //DB field definition:
  if(index < get_Entered_Field_count())
  {
    fieldResult = m_Fields[index];
  }

  return fieldResult;
}

bool Box_DB_Table::get_field(const Glib::ustring& name, Field& field) const
{
  type_vecFields::const_iterator iterFind = std::find_if( m_Fields.begin(), m_Fields.end(), predicate_FieldHasName<Field>(name) );
  if(iterFind != m_Fields.end()) //If it was found:
  {
    field = *iterFind;
    return true;
  }
  else
  {
    return false; //not found.
  }
}

bool Box_DB_Table::get_field_primary_key(guint& field_column) const
{
  return get_field_primary_key(m_Fields, field_column);
}

//static:
bool Box_DB_Table::get_field_primary_key(const type_vecFields& fields, guint& field_column)
{
  //Initialize input parameter:
  field_column = 0;
  
  //TODO_performance: Cache the primary key?
  guint col = 0;
  guint cols_count = fields.size();
  while(col < cols_count)
  {
    if(fields[col].get_field_info().get_primary_key())
    {
      field_column = col;
      return true;
    }
    else
    {
      ++col;
    }
  }

  return false; //Not found.
}

bool Box_DB_Table::get_field_index(const Glib::ustring& name, guint& field_index) const
{
  //Initialize input parameter:
  field_index = 0;

  //TODO_performance: Cache the primary key?
  guint col = 0;
  guint cols_count = m_Fields.size();
  while(col < cols_count)
  {
    if(m_Fields[col].get_name() == name)
    {
      field_index = col;
      return true;
    }
    else
      ++col;
  }

  return false; //Not found.
}

unsigned long Box_DB_Table::get_last_auto_increment_value(const Glib::RefPtr<Gnome::Gda::DataModel>& data_model, const Glib::ustring /* field_name */)
{
  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();
    Glib::ustring id = connection->get_last_insert_id(data_model);

    //Convert it to a numeric type:
    std::stringstream stream;
    stream << id;
    unsigned long id_numeric = 0;
    stream >> id_numeric;

    return id_numeric;
  }
  else
    return 0;
}

//static
Box_DB_Table::type_map_valuetypes Box_DB_Table::get_field_type_names()
{
  //TODO: Performance: Cache this map, and/or pass it by reference:
  type_map_valuetypes mapTypes;
  
  mapTypes[Gnome::Gda::VALUE_TYPE_NULL] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_NULL);
  mapTypes[Gnome::Gda::VALUE_TYPE_BIGINT] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_BIGINT);
  mapTypes[Gnome::Gda::VALUE_TYPE_BIGUINT] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_BIGUINT);
  mapTypes[Gnome::Gda::VALUE_TYPE_BINARY] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_BINARY);
  mapTypes[Gnome::Gda::VALUE_TYPE_BOOLEAN] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_BOOLEAN);
  mapTypes[Gnome::Gda::VALUE_TYPE_DATE] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_DATE);
  mapTypes[Gnome::Gda::VALUE_TYPE_DOUBLE] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_DOUBLE);
  mapTypes[Gnome::Gda::VALUE_TYPE_GEOMETRIC_POINT] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_GEOMETRIC_POINT);
  mapTypes[Gnome::Gda::VALUE_TYPE_GOBJECT] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_GOBJECT);
  mapTypes[Gnome::Gda::VALUE_TYPE_INTEGER] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_INTEGER);
  mapTypes[Gnome::Gda::VALUE_TYPE_LIST] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_LIST);
  mapTypes[Gnome::Gda::VALUE_TYPE_MONEY] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_MONEY);
  mapTypes[Gnome::Gda::VALUE_TYPE_NUMERIC] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_NUMERIC);
  mapTypes[Gnome::Gda::VALUE_TYPE_SINGLE] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_SINGLE);
  mapTypes[Gnome::Gda::VALUE_TYPE_SMALLINT] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_SMALLINT);
  mapTypes[Gnome::Gda::VALUE_TYPE_SMALLUINT] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_SMALLUINT);
  mapTypes[Gnome::Gda::VALUE_TYPE_STRING] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_STRING);
  mapTypes[Gnome::Gda::VALUE_TYPE_TIME] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_TIME);
  mapTypes[Gnome::Gda::VALUE_TYPE_TIMESTAMP] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_TIMESTAMP);
  mapTypes[Gnome::Gda::VALUE_TYPE_TINYINT] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_TINYINT);
  mapTypes[Gnome::Gda::VALUE_TYPE_TINYUINT] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_TINYUINT);
  mapTypes[Gnome::Gda::VALUE_TYPE_TYPE] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_TYPE);
  mapTypes[Gnome::Gda::VALUE_TYPE_UINTEGER] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_UINTEGER);
  mapTypes[Gnome::Gda::VALUE_TYPE_UNKNOWN] = Gnome::Gda::Value::type_to_string(Gnome::Gda::VALUE_TYPE_UNKNOWN);

  return mapTypes;
}

Field Box_DB_Table::get_fields_for_table_one_field(const Glib::ustring& table_name, const Glib::ustring& field_name) const
{
  Field result;

  type_vecFields fields =  get_fields_for_table(table_name);
  type_vecFields::iterator iter = std::find_if(fields.begin(), fields.end(), predicate_FieldHasName<Field>(field_name));
  if(iter != fields.end()) //TODO: Handle error?
    result = *iter;  

  return result;
}

Box_DB_Table::type_vecFields Box_DB_Table::get_fields_for_table(const Glib::ustring& table_name) const
{
  //Get field definitions from the database, because not all field information is stored in the document (for instance, whether it is a primary key)
  type_vecFields fieldsDatabase = get_fields_for_table_from_database(table_name);

  const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
  if(!pDoc)
    return fieldsDatabase; //This should never happen.
  else
  {
    type_vecFields fieldsDocument = pDoc->get_table_fields(m_strTableName);

    //Look at each field in the database:
    for(type_vecFields::iterator iter = fieldsDocument.begin(); iter != fieldsDocument.end(); ++iter)
    {
      Glib::ustring field_name = iter->get_name();

      //Get the field info from the database:
      //This is in the document as well, but it _might_ have changed.
      type_vecFields::const_iterator iterFindDatabase = std::find_if(fieldsDatabase.begin(), fieldsDatabase.end(), predicate_FieldHasName<Field>(field_name));

      //TODO: What shall we do about fields that don't exist in the database anymore?
      if(iterFindDatabase != fieldsDatabase.end() )
      {
        Gnome::Gda::FieldAttributes field_info_document = iter->get_field_info();
         
        //Update the Field information that _might_ have changed in the database.
        Gnome::Gda::FieldAttributes field_info = iterFindDatabase->get_field_info();

        //libgda does not tell us whether the field is auto_incremented, so we need to get that from the document.
        field_info.set_auto_increment( field_info_document.get_auto_increment() );

        //libgda does not tell us whether the field is auto_incremented, so we need to get that from the document.
        field_info.set_primary_key( field_info_document.get_primary_key() );
        
        //libgda does yet tell us correct default_value information so we need to get that from the document.
        field_info.set_default_value( field_info_document.get_default_value() );

        iter->set_field_info(field_info);
      }
    }

    //Add any fields that are in the database, but not in the document:
    for(type_vecFields::iterator iter = fieldsDatabase.begin(); iter != fieldsDatabase.end(); ++iter)
    {
      Glib::ustring field_name = iter->get_name();

       //Look in the result so far:
       type_vecFields::const_iterator iterFindDatabase = std::find_if(fieldsDocument.begin(), fieldsDocument.end(), predicate_FieldHasName<Field>(field_name));

       //Add it if it is not there:
       if(iterFindDatabase == fieldsDocument.end() )
         fieldsDocument.push_back(*iter);
    }

    return fieldsDocument;
  }
   
}

Box_DB_Table::type_vecFields Box_DB_Table::get_fields_for_table_from_database(const Glib::ustring& table_name)
{
  type_vecFields result;

  // These are documented here:
  // http://www.gnome-db.org/docs/libgda/libgda-provider-class.html#LIBGDA-PROVIDER-GET-SCHEMA
  enum GlomGdaDataModelFieldColumns
  {
    DATAMODEL_FIELDS_COL_NAME = 0,
    DATAMODEL_FIELDS_COL_TYPE = 1,
    DATAMODEL_FIELDS_COL_SIZE = 2,
    DATAMODEL_FIELDS_COL_SCALE = 3,
    DATAMODEL_FIELDS_COL_NOTNULL = 4,
    DATAMODEL_FIELDS_COL_PRIMARYKEY = 5,
    DATAMODEL_FIELDS_COL_UNIQUEINDEX = 6,
    DATAMODEL_FIELDS_COL_REFERENCES = 7,
    DATAMODEL_FIELDS_COL_DEFAULTVALUE = 8,
  };

  Bakery::BusyCursor(*get_app_window());

  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

    Gnome::Gda::Parameter param_table_name("name", table_name);
    Gnome::Gda::ParameterList param_list;
    param_list.add_parameter(param_table_name);

    Glib::RefPtr<Gnome::Gda::DataModel> data_model_fields = connection->get_schema(Gnome::Gda::CONNECTION_SCHEMA_FIELDS, param_list);
    if(data_model_fields && (data_model_fields->get_n_columns() == 0))
    {
      std::cerr << "Box_DB_Table_Definition::fill_fields(): libgda reported 0 fields for the table." << std::endl;
    }
    else if(data_model_fields)
    {
      guint row = 0;
      guint rows_count = data_model_fields->get_n_rows();
      while(row < rows_count)
      {
        Gnome::Gda::FieldAttributes field_info;

        //Get the field name:
        Gnome::Gda::Value value_name = data_model_fields->get_value_at(DATAMODEL_FIELDS_COL_NAME, row);
        if(value_name.get_value_type() ==  Gnome::Gda::VALUE_TYPE_STRING)
          field_info.set_name( value_name.get_string() );

        //Get the field type:
        //This is a string representation of the type, so we need to discover the Gnome::Gda::ValueType for it:
        Gnome::Gda::Value value_fieldtype = data_model_fields->get_value_at(DATAMODEL_FIELDS_COL_TYPE, row);
        if(value_fieldtype.get_value_type() ==  Gnome::Gda::VALUE_TYPE_STRING)
        {
           Glib::ustring schema_type_string = value_fieldtype.get_string();
           if(!schema_type_string.empty())
           {
             ConnectionPool* connection_pool = ConnectionPool::get_instance();
             const FieldTypes* pFieldTypes = connection_pool->get_field_types();
             if(pFieldTypes)
             {
               Gnome::Gda::ValueType gdatype = pFieldTypes->get_gdavalue_for_schema_type_string(schema_type_string);
               field_info.set_gdatype(gdatype);
             }
           }
        }

        //Get the default value:
        /* libgda does not return this correctly yet. TODO: check bug http://bugzilla.gnome.org/show_bug.cgi?id=143576
        Gnome::Gda::Value value_defaultvalue = data_model_fields->get_value_at(DATAMODEL_FIELDS_COL_DEFAULTVALUE, row);
        if(value_defaultvalue.get_value_type() ==  Gnome::Gda::VALUE_TYPE_STRING)
          field_info.set_default_value(value_defaultvalue);
        */

        //Get whether it can be null:
        Gnome::Gda::Value value_notnull = data_model_fields->get_value_at(DATAMODEL_FIELDS_COL_NOTNULL, row);
        if(value_notnull.get_value_type() ==  Gnome::Gda::VALUE_TYPE_BOOLEAN)
          field_info.set_allow_null(value_notnull.get_bool());


        //Get whether it is a primary key:
        Gnome::Gda::Value value_primarykey = data_model_fields->get_value_at(DATAMODEL_FIELDS_COL_PRIMARYKEY, row);
        if(value_primarykey.get_value_type() ==  Gnome::Gda::VALUE_TYPE_BOOLEAN)
          field_info.set_primary_key( value_primarykey.get_bool() );

        //Get whether it is unique
        Gnome::Gda::Value value_unique = data_model_fields->get_value_at(DATAMODEL_FIELDS_COL_UNIQUEINDEX, row);
        if(value_unique.get_value_type() ==  Gnome::Gda::VALUE_TYPE_BOOLEAN)
          field_info.set_unique_key( value_unique.get_bool() );

        //Get whether it is autoincrements
        /* libgda does not provide this yet.
        Gnome::Gda::Value value_autoincrement = data_model_fields->get_value_at(DATAMODEL_FIELDS_COL_AUTOINCREMENT, row);
        if(value_autoincrement.get_value_type() ==  Gnome::Gda::VALUE_TYPE_BOOLEAN)
          field_info.set_auto_increment( value_autoincrement.get_bool() );
        */

        Field field; //TODO: Get glom-specific information from the document?
        field.set_field_info(field_info);
        result.push_back(field);

        ++row;
      }
    }
  }
  
  return result;
}

//static:
Box_DB_Table::type_vecFields Box_DB_Table::get_fields_for_datamodel(const Glib::RefPtr<Gnome::Gda::DataModel>& data_model)
{
  type_vecFields result;

  int columns =  data_model->get_n_columns();
  for(int i = 0; i < columns; ++i)
  {
    Gnome::Gda::FieldAttributes fieldinfo = data_model->describe_column(i);
    Field field;
    field.set_field_info(fieldinfo); //TODO: Get glom-specific information from document?
    result.push_back( field );
  }

  return result;
}

Glib::ustring Box_DB_Table::postgres_get_field_definition_for_sql(const Gnome::Gda::FieldAttributes& field_info)
{
  Glib::ustring strResult;

  //Type
  Glib::ustring strType = "unknowntype";
  
  //Postgres has a special "serial" datatype. (MySQL uses a numeric type, and has an extra "AUTO_INCREMENT" command)
  if(false) //disabled for now - see generate_next_auto_increment() //field_info.get_auto_increment())
  {
    strType = "serial";
  }
  else
  {
    ConnectionPool* pConnectionPool = ConnectionPool::get_instance();
    if(pConnectionPool)
    {
      const FieldTypes* pFieldTypes = pConnectionPool->get_field_types();
      if(pFieldTypes)
      {
        const Gnome::Gda::ValueType fieldType = field_info.get_gdatype();
        strType = pFieldTypes->get_string_name_for_gdavaluetype(fieldType);
      }
    }
  }
  
  strResult += strType;
  
   /*
  //Optinal type details: (M, D), UNSIGNED
  FieldType::enumTypeOptionals optionals = fieldType.get_TypeOptionals();
  if(optionals != FieldType::TYPE_OPTIONALS_None)
  {
  Glib::ustring strOptionals;

  char pchM[10] = {0};
  sprintf(pchM, "%d", fieldType.get_MaxLength());
    Glib::ustring strM(pchM);


  if(optionals == FieldType::TYPE_OPTIONALS_M_D)
  {
    if( (fieldType.get_MaxLength() != 0) && ( fieldType.get_DecimalsCount() != 0) ) //0 here means use default, so don't specify.
    {
      char pchD[10] = {0};
      sprintf(pchD, "%d", fieldType.get_DecimalsCount());
      Glib::ustring strD(pchD);

      strOptionals = "(" + strM + "," + strD + ")";
    }

  }
  else if(optionals == FieldType::TYPE_OPTIONALS_M_S)
  {
    if( fieldType.get_MaxLength() != 0 ) //0 here means use default, so don't specify.
      strOptionals = "(" + strM + ")";

      if(!(fieldType.get_Signed()))
        strOptionals += " UNSIGNED";
  }
  else if(optionals == FieldType::TYPE_OPTIONALS_M)
  {
      if( fieldType.get_MaxLength() != 0 ) //0 here means use default, so don't specify.
      strOptionals = "(" + strM + ")";
  }

    if(strOptionals.size())
      strResult += (" " + strOptionals);
  }
  */

  //Unique:
  if(field_info.get_unique_key())
    strResult += " UNIQUE";
      
  /* Posgres needs us to do this separately
  //Not Null:
  if(!(field_info.get_allow_null()))
    strResult += " NOT NULL";
  */
    
  //Default:
  Gnome::Gda::Value valueDefault = field_info.get_default_value();
  const Glib::ustring& strDefault =  valueDefault.to_string();
  if(strDefault.size() && strDefault != "NULL")
    strResult += " DEFAULT " + strDefault; //TODO: Quote/Escape it if necessary.
      
  //Primary Key:
  if(field_info.get_primary_key())
    strResult += " PRIMARY KEY";
      
  return strResult;
}



