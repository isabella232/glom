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
#include "data_structure/glomconversions.h"
#include "glom_python.h"
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

void Box_DB_Table::init_db_details(const Glib::ustring& strTableName)
{
  m_strTableName = strTableName;

  fill_fields();
  fill_from_database();
}

void Box_DB_Table::refresh_db_details()
{
  fill_from_database();
}


void Box_DB_Table::fill_fields()
{

}

Gnome::Gda::Value Box_DB_Table::get_entered_field_data(const LayoutItem_Field& /* field */) const
{
  //Override this to use Field::set_data() too.

  return Gnome::Gda::Value(); //null
}

void Box_DB_Table::set_entered_field_data(const LayoutItem_Field& /* field */, const Gnome::Gda::Value& /* value */)
{
  //Override this.
}

bool Box_DB_Table::get_field_primary_key_for_table(const Glib::ustring table_name, Field& field) const
{
  const Document_Glom* document = get_document();
  if(document)
  {
    //TODO_Performance:
    Document_Glom::type_vecFields fields = document->get_table_fields(table_name);
    for(Document_Glom::type_vecFields::iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      if(iter->get_field_info().get_primary_key())
      {
        field = *iter;
        return true;
      }
    }
  }

  return false;
}

/*
bool Box_DB_Table::get_field_primary_key(Field& field) const
{
 return get_field_primary_key(m_Fields, field);
}
*/


/*
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
*/

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

bool Box_DB_Table::get_fields_for_table_one_field(const Glib::ustring& table_name, const Glib::ustring& field_name, Field& field) const
{
  //Initialize output parameter:
  Field result = Field();

  type_vecFields fields = get_fields_for_table(table_name);
  type_vecFields::iterator iter = std::find_if(fields.begin(), fields.end(), predicate_FieldHasName<Field>(field_name));
  if(iter != fields.end()) //TODO: Handle error?
  {
    field = *iter;
    return true;
  }

  return false;
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
  Field::enumTypeOptionals optionals = fieldType.get_TypeOptionals();
  if(optionals != Field::TYPE_OPTIONALS_None)
  {
  Glib::ustring strOptionals;

  char pchM[10] = {0};
  sprintf(pchM, "%d", fieldType.get_MaxLength());
    Glib::ustring strM(pchM);


  if(optionals == Field::TYPE_OPTIONALS_M_D)
  {
    if( (fieldType.get_MaxLength() != 0) && ( fieldType.get_DecimalsCount() != 0) ) //0 here means use default, so don't specify.
    {
      char pchD[10] = {0};
      sprintf(pchD, "%d", fieldType.get_DecimalsCount());
      Glib::ustring strD(pchD);

      strOptionals = "(" + strM + "," + strD + ")";
    }

  }
  else if(optionals == Field::TYPE_OPTIONALS_M_S)
  {
    if( fieldType.get_MaxLength() != 0 ) //0 here means use default, so don't specify.
      strOptionals = "(" + strM + ")";

      if(!(fieldType.get_Signed()))
        strOptionals += " UNSIGNED";
  }
  else if(optionals == Field::TYPE_OPTIONALS_M)
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
  if(!strDefault.empty() && strDefault != "NULL")
    strResult += " DEFAULT " + strDefault; //TODO: Quote/Escape it if necessary.
      
  //Primary Key:
  if(field_info.get_primary_key())
    strResult += " PRIMARY KEY";
      
  return strResult;
}



