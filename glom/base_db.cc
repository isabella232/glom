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

#include "base_db.h"
#include "application.h" //App_Glom.
#include "appstate.h"
//#include <libgnomeui/gnome-app-helper.h>

#include <sstream> //For stringstream

Base_DB::Base_DB()
{
  m_pDocument = 0;
}

Base_DB::~Base_DB()
{
}

void Base_DB::init_db_details()
{
  fill_from_database();
}

void Base_DB::refresh_db_details()
{
  fill_from_database();
}

void Base_DB::fill_from_database()
{
  //m_AddDel.remove_all();
}

void Base_DB::fill_end()
{
  //Call this from the end of fill_from_database() overrides.
}

//static:
sharedptr<SharedConnection> Base_DB::connect_to_server()
{
  Bakery::BusyCursor(*get_app_window());

  sharedptr<SharedConnection> result(0);

  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
  {
    result = connection_pool->connect();
  }

  return result;
}

void Base_DB::handle_error(const std::exception& ex)
{
  Gtk::MessageDialog dialog(Glib::ustring("Internal error:\n") + ex.what(), Gtk::MESSAGE_WARNING );
  dialog.run();
}

bool Base_DB::handle_error()
{
  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

    typedef std::list< Glib::RefPtr<Gnome::Gda::Error> > type_list_errors;
    type_list_errors list_errors = gda_connection->get_errors();

    if(!list_errors.empty())
    {
      Glib::ustring error_details;
      for(type_list_errors::iterator iter = list_errors.begin(); iter != list_errors.end(); ++iter)
      {
        if(iter != list_errors.begin())
          error_details += "\n"; //Add newline after each error.

        error_details += (*iter)->get_description();
        std::cerr << "Internal error: " << error_details << std::endl;
      }

      Gtk::MessageDialog dialog(Glib::ustring("Internal error:\n") + error_details, Gtk::MESSAGE_WARNING );
      dialog.run();

      return true; //There really was an error.
    }
  }

   //There was no error. libgda just did not return any data, and has no concept of an empty datamodel.
   return false;
}


Glib::RefPtr<Gnome::Gda::DataModel> Base_DB::Query_execute(const Glib::ustring& strQuery)
{
  Glib::RefPtr<Gnome::Gda::DataModel> result;

  Bakery::BusyCursor(*get_app_window());

  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

    std::cout << "Query_execute():  " << strQuery << std::endl;
    result = gda_connection->execute_single_command(strQuery);
    if(!result)
    {
      handle_error();
    }
  }

  return result;
}

void Base_DB::load_from_document()
{
  if(m_pDocument)
  {
    fill_from_database(); //virtual.

    //Call base class:
    View_Composite_Glom::load_from_document();
  }
}


Base_DB::type_vecStrings Base_DB::get_table_names()
{
  type_vecStrings result;

  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();
    Glib::RefPtr<Gnome::Gda::DataModel> data_model_tables = gda_connection->get_schema(Gnome::Gda::CONNECTION_SCHEMA_TABLES);
    if(data_model_tables && (data_model_tables->get_n_columns() == 0))
    {
      std::cerr << "Base_DB_Table::get_table_names(): libgda reported 0 tables for the database." << std::endl;
    }
    else if(data_model_tables)
    {
      int rows = data_model_tables->get_n_rows();
      for(int i = 0; i < rows; ++i)
      {
        Gnome::Gda::Value value = data_model_tables->get_value_at(0, i);

        //Get the table name:
        Glib::ustring table_name;
        if(value.get_value_type() ==  Gnome::Gda::VALUE_TYPE_STRING)
        {
          result.push_back( value.get_string() );
        }
      }
    }
  }

  return result;
}


AppState::userlevels Base_DB::get_userlevel() const
{
  const Document_Glom* document = dynamic_cast<const Document_Glom*>(get_document());
  if(document)
  {
    return document->get_userlevel();
  }
  else
  {
    g_warning("Base_DB::get_userlevel(): document not found.");
    return AppState::USERLEVEL_OPERATOR;
  }
}

void Base_DB::set_userlevel(AppState::userlevels value)
{
  Document_Glom* document = get_document();
  if(document)
  {
    document->set_userlevel(value);
  }
}

void Base_DB::on_userlevel_changed(AppState::userlevels /* userlevel */)
{
  //Override this in derived classes.
}

Glib::ustring Base_DB::util_string_from_decimal(guint decimal)
{
    std::stringstream stream;
    stream << decimal;

    Glib::ustring result;
    stream >> result;

    return result;
}

guint Base_DB::util_decimal_from_string(const Glib::ustring& str)
{
    //Convert it to a numeric type:
    std::stringstream stream;
    stream << str;
    guint id_numeric = 0;
    stream >> id_numeric;

    return id_numeric;
}

void Base_DB::set_document(Document_Glom* pDocument)
{
  View_Composite_Glom::set_document(pDocument);

  //Connect to a signal that is only on the derived document class:
  if(m_pDocument)
  {
    m_pDocument->signal_userlevel_changed().connect( sigc::mem_fun(*this, &Base_DB::on_userlevel_changed) );

    //Show the appropriate UI for the user level that is specified by this new document:
    on_userlevel_changed(m_pDocument->get_userlevel());
  }


}


//static:
Base_DB::type_vecStrings Base_DB::util_vecStrings_from_Fields(const type_vecFields& fields)
{
  //Get vector of field names, suitable for a combo box:

  type_vecStrings vecNames;
  for(type_vecFields::size_type i = 0; i < fields.size(); i++)
  {
    vecNames.push_back(fields[i].get_name());
  }

  return vecNames;
}


Base_DB::type_vecFields Base_DB::get_fields_for_table_from_database(const Glib::ustring& table_name)
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
    DATAMODEL_FIELDS_COL_DEFAULTVALUE = 8
  };

  Bakery::BusyCursor(*get_app_window());

  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(!sharedconnection)
  {
    g_warning("Base_DB::get_fields_for_table_from_database(): connection failed.");
  }
  else  
  {
    Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

    Gnome::Gda::Parameter param_table_name("name", table_name);
    Gnome::Gda::ParameterList param_list;
    param_list.add_parameter(param_table_name);

    Glib::RefPtr<Gnome::Gda::DataModel> data_model_fields = connection->get_schema(Gnome::Gda::CONNECTION_SCHEMA_FIELDS, param_list);

    if(!data_model_fields)
    {
      std::cerr << "Base_DB_Table_Definition::fill_fields(): libgda reported empty fields schema data_model for the table." << std::endl;
    } 
    else if(data_model_fields->get_n_columns() == 0)
    {
      std::cerr << "Base_DB_Table_Definition::fill_fields(): libgda reported 0 fields for the table." << std::endl;
    }
    else if(data_model_fields->get_n_rows() == 0)
    {
      g_warning("Base_DB::get_fields_for_table_from_database(): data_model_fields->get_n_rows() == 0: The table probably does not exist in the specified database.");
    }
    else
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

  if(result.empty())
  {
    g_warning("Base_DB::get_fields_for_table_from_database(): returning empty result.");
  }
  
  return result;
}

Base_DB::type_vecFields Base_DB::get_fields_for_table(const Glib::ustring& table_name) const
{
  //Get field definitions from the database:
  type_vecFields fieldsDatabase = get_fields_for_table_from_database(table_name);

  const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
  if(!pDoc)
    return fieldsDatabase; //This should never happen.
  else
  {
    type_vecFields result;

    type_vecFields fieldsDocument = pDoc->get_table_fields(table_name);
  
    //Look at each field in the database:
    for(type_vecFields::iterator iter = fieldsDocument.begin(); iter != fieldsDocument.end(); ++iter)
    {
      Glib::ustring field_name = iter->get_name();

      //Get the field info from the database:
      //This is in the document as well, but it _might_ have changed.
      type_vecFields::const_iterator iterFindDatabase = std::find_if(fieldsDatabase.begin(), fieldsDatabase.end(), predicate_FieldHasName<Field>(field_name));

      if(iterFindDatabase != fieldsDatabase.end() ) //Ignore fields that don't exist in the database anymore.
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

        result.push_back(*iter);
      }
    }
  
    //Add any fields that are in the database, but not in the document:
    for(type_vecFields::iterator iter = fieldsDatabase.begin(); iter != fieldsDatabase.end(); ++iter)
    {
      Glib::ustring field_name = iter->get_name();
      
       //Look in the result so far:
       type_vecFields::const_iterator iterFind = std::find_if(result.begin(), result.end(), predicate_FieldHasName<Field>(field_name));

       //Add it if it is not there:
       if(iterFind == result.end() )
         result.push_back(*iter);
    }

    return result;
  }

}

bool Base_DB::util_string_has_whitespace(const Glib::ustring& text)
{
 for(Glib::ustring::const_iterator iter = text.begin(); iter != text.end(); ++iter)
 {
   if(Glib::Unicode::isspace(*iter))
     return true; //White space was found.
 }

 return false; //No white space found.
}

Glib::ustring Base_DB::util_title_from_string(const Glib::ustring& text)
{
  Glib::ustring result;

  bool capitalise_next_char = true;
  for(Glib::ustring::const_iterator iter = text.begin(); iter != text.end(); ++iter)
  {
    
    const gunichar& ch = *iter;
    if(ch == '_') //Replace _ with space.
    {
      capitalise_next_char = true; //Capitalise all words.
      result += " ";
    }
    else
    {
      if(capitalise_next_char)
        result += Glib::Unicode::toupper(*iter);
      else
        result += *iter;

      capitalise_next_char = false;
    }
  }

  return result;
}

