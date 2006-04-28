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
#include <glom/libglom/data_structure/glomconversions.h>
#include "python_embed/glom_python.h"
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
  return m_table_name;
}

bool Box_DB_Table::init_db_details(const Glib::ustring& table_name)
{
  m_table_name = table_name;

  if(!ConnectionPool::get_instance()->get_ready_to_connect())
    return false;

  return fill_from_database();
}

bool Box_DB_Table::refresh_data_from_database()
{
  if(!ConnectionPool::get_instance()->get_ready_to_connect())
    return false;

  return fill_from_database();
}

Gnome::Gda::Value Box_DB_Table::get_entered_field_data_field_only(const sharedptr<const Field>& field) const
{
  sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
  layout_item->set_full_field_details(field); 

  return get_entered_field_data(layout_item);
}

Gnome::Gda::Value Box_DB_Table::get_entered_field_data(const sharedptr<const LayoutItem_Field>& /* field */) const
{
  //Override this to use Field::set_data() too.

  return Gnome::Gda::Value(); //null
}

unsigned long Box_DB_Table::get_last_auto_increment_value(const Glib::RefPtr<Gnome::Gda::DataModel>& data_model, const Glib::ustring& /* field_name */)
{
  sharedptr<SharedConnection> sharedconnection = connect_to_server(get_app_window());
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

//static:
/*
Box_DB_Table::type_vecFields Box_DB_Table::get_fields_for_datamodel(const Glib::RefPtr<Gnome::Gda::DataModel>& data_model)
{
  type_vecFields result;

  int columns =  data_model->get_n_columns();
  for(int i = 0; i < columns; ++i)
  {
    Gnome::Gda::FieldAttributes fieldinfo = data_model->describe_column(i);
    sharedptr<Field> field(new Field());
    field->set_field_info(fieldinfo); //TODO: Get glom-specific information from document?
    result.push_back( field );
  }

  return result;
}
*/

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



