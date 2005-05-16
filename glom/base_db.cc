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
#include "standard_table_prefs_fields.h"
#include "document/document_glom.h"
#include "data_structure/glomconversions.h"
#include "mode_data/dialog_choose_field.h"
//#include "dialog_layout_report.h"
#include "utils.h"
#include "data_structure/glomconversions.h"
#include "data_structure/layout/report_parts/layoutitem_summary.h"
#include "data_structure/layout/report_parts/layoutitem_fieldsummary.h"
#include <glibmm/i18n.h>
//#include <libgnomeui/gnome-app-helper.h>

#include <sstream> //For stringstream

Base_DB::Base_DB()
{
  //m_pDocument = 0;
}

Base_DB::~Base_DB()
{
}

bool Base_DB::init_db_details()
{
  return fill_from_database();
}

bool Base_DB::refresh_data_from_database()
{
  return fill_from_database();
}

bool Base_DB::fill_from_database()
{
  //m_AddDel.remove_all();
  return true;
}

void Base_DB::fill_end()
{
  //Call this from the end of fill_from_database() overrides.
}

//static:
sharedptr<SharedConnection> Base_DB::connect_to_server()
{
  Bakery::BusyCursor(*get_application());

  sharedptr<SharedConnection> result(0);

  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
  {
    result = connection_pool->connect();
  }

  return result;
}

void Base_DB::handle_error(const std::exception& ex) const
{
  Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Internal error")), true, Gtk::MESSAGE_WARNING );
  dialog.set_secondary_text(ex.what());
  //TODO: dialog.set_transient_for(*get_application());
  dialog.run();
}

bool Base_DB::handle_error() const
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

      Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Internal error")), true, Gtk::MESSAGE_WARNING );
      dialog.set_secondary_text(error_details);
      //TODO: dialog.set_transient_for(*get_application());
      dialog.run();

      return true; //There really was an error.
    }
  }

   //There was no error. libgda just did not return any data, and has no concept of an empty datamodel.
   return false;
}


Glib::RefPtr<Gnome::Gda::DataModel> Base_DB::Query_execute(const Glib::ustring& strQuery) const
{
  Glib::RefPtr<Gnome::Gda::DataModel> result;

  Bakery::BusyCursor(*get_application());

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
  else
  {
    g_warning("Base_DB::Query_execute(): No connection yet.");
  }

  return result;
}

void Base_DB::load_from_document()
{
  if(get_document())
  {
    if(ConnectionPool::get_instance()->get_ready_to_connect())
      fill_from_database(); //virtual.

    //Call base class:
    View_Composite_Glom::load_from_document();
  }
}

bool Base_DB::get_table_exists_in_database(const Glib::ustring& table_name) const
{
  //TODO_Performance

  type_vecStrings tables = get_table_names();
  type_vecStrings::const_iterator iterFind = std::find(tables.begin(), tables.end(), table_name);
  bool result = (iterFind != tables.end());

  return result;
}

Base_DB::type_vecStrings Base_DB::get_table_names() const
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
          table_name = value.get_string();

          //Ignore the pga_* tables that pgadmin adds when you use it:
          if(table_name.substr(0, 4) != "pga_")
            result.push_back( table_name );
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
  Document_Glom* document = get_document();
  if(document)
  {
    document->signal_userlevel_changed().connect( sigc::mem_fun(*this, &Base_DB::on_userlevel_changed) );

    //Show the appropriate UI for the user level that is specified by this new document:
    on_userlevel_changed(document->get_userlevel());
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

  Bakery::BusyCursor(*get_application());

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
      std::cerr << "Base_DB::get_fields_for_table_from_database(): libgda reported empty fields schema data_model for the table." << std::endl;
    } 
    else if(data_model_fields->get_n_columns() == 0)
    {
      std::cerr << "BBase_DB::get_fields_for_table_from_database(): libgda reported 0 fields for the table." << std::endl;
    }
    else if(data_model_fields->get_n_rows() == 0)
    {
      g_warning("Base_DB::get_fields_for_table_from_database(): table_name=%s, data_model_fields->get_n_rows() == 0: The table probably does not exist in the specified database.", table_name.c_str());
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
    //g_warning("Base_DB::get_fields_for_table_from_database(): returning empty result.");
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

Base_DB::type_vecStrings Base_DB::get_database_groups() const
{
  type_vecStrings result;

  Glib::ustring strQuery = "SELECT pg_group.groname FROM pg_group";
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = Query_execute(strQuery);
  if(data_model)
  {
    const int rows_count = data_model->get_n_rows();
    for(int row = 0; row < rows_count; ++row)
    {
      const Gnome::Gda::Value value = data_model->get_value_at(0, row);
      const Glib::ustring name = value.get_string();
      result.push_back(name);
    }
  }

  return result;
}

Base_DB::type_vecStrings Base_DB::get_database_users(const Glib::ustring& group_name) const
{
  type_vecStrings result;

  if(group_name.empty())
  {
    //pg_shadow contains the users. pg_users is a view of pg_shadow without the password.
    Glib::ustring strQuery = "SELECT pg_shadow.usename FROM pg_shadow";
    Glib::RefPtr<Gnome::Gda::DataModel> data_model = Query_execute(strQuery);
    if(data_model)
    {
      const int rows_count = data_model->get_n_rows();
      for(int row = 0; row < rows_count; ++row)
      {
        const Gnome::Gda::Value value = data_model->get_value_at(0, row);
        const Glib::ustring name = value.get_string();
        result.push_back(name);
      }
    }
  }
  else
  {
    Glib::ustring strQuery = "SELECT pg_group.groname, pg_group.grolist FROM pg_group WHERE pg_group.groname = '" + group_name + "'";
    Glib::RefPtr<Gnome::Gda::DataModel> data_model = Query_execute(strQuery);
    if(data_model && data_model->get_n_rows())
    {
      const int rows_count = data_model->get_n_rows();
      for(int row = 0; row < rows_count; ++row)
      {
        const Gnome::Gda::Value value = data_model->get_value_at(1, row); //Column 1 is the /* the user list.
        //pg_group is a string, formatted, bizarrely, like so: "{100, 101}".

        Glib::ustring group_list;
        if(!value.is_null())
          group_list = value.get_string();

        type_vecStrings vecUserIds = pg_list_separate(group_list);
        for(type_vecStrings::const_iterator iter = vecUserIds.begin(); iter != vecUserIds.end(); ++iter)
        {
          //TODO_Performance: Can we do this in one SQL SELECT?
          Glib::ustring strQuery = "SELECT pg_user.usename FROM pg_user WHERE pg_user.usesysid = '" + *iter + "'";
          Glib::RefPtr<Gnome::Gda::DataModel> data_model = Query_execute(strQuery);
          if(data_model)
          {
            const Gnome::Gda::Value value = data_model->get_value_at(0, 0); 
           result.push_back(value.get_string());
          }
        }

      }
    }
  }

  return result;
}

void Base_DB::set_table_privileges(const Glib::ustring& group_name, const Glib::ustring& table_name, const Privileges& privs, bool developer_privs)
{
  if(group_name.empty() || table_name.empty())
    return;

  //Change the permission in the database:

  //Build the SQL statement:

  //Grant or revoke:
  Glib::ustring strQuery = "GRANT";
  //TODO: Revoke the ones that are not specified.

  //What to grant or revoke:
  Glib::ustring strPrivilege;

  if(developer_privs)
    strPrivilege = "ALL PRIVILEGES";
  else
  {
    if(privs.m_view)
      strPrivilege += "SELECT";

    if(privs.m_edit)
    {
      if(!strPrivilege.empty())
        strPrivilege += ", ";

      strPrivilege += "UPDATE";
    }

    if(privs.m_create)
    {
      if(!strPrivilege.empty())
        strPrivilege += ", ";

      strPrivilege += "INSERT";
    }

    if(privs.m_delete)
    {
      if(!strPrivilege.empty())
        strPrivilege += ", ";

      strPrivilege += "DELETE";
    }
  }

  strQuery += " " + strPrivilege + " ON " + table_name + " ";

  //This must match the Grant or Revoke:
  strQuery += "TO";

  strQuery += " GROUP " + group_name;

  Query_execute(strQuery);
}

Privileges Base_DB::get_table_privileges(const Glib::ustring& group_name, const Glib::ustring& table_name) const
{
  Privileges result;

  if(group_name == GLOM_STANDARD_GROUP_NAME_DEVELOPER)
  {
    //Always give developers full access:
    result.m_view = true;
    result.m_edit = true;
    result.m_create = true;
    result.m_delete = true;
    return result;
  }

  //Get the permissions:
  Glib::ustring strQuery = "SELECT pg_class.relacl FROM pg_class WHERE pg_class.relname = '" + table_name + "'";
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = Query_execute(strQuery);
  if(data_model && data_model->get_n_rows())
  {
    const Gnome::Gda::Value value = data_model->get_value_at(0, 0);
    Glib::ustring access_details;
    if(!value.is_null())
      access_details = value.get_string();

    //Parse the strange postgres permissions format:
    const type_vecStrings vecItems = pg_list_separate(access_details);
    for(type_vecStrings::const_iterator iterItems = vecItems.begin(); iterItems != vecItems.end(); ++iterItems)
    {
      Glib::ustring item = *iterItems;
      item = string_trim(item, "\""); //Remove quotes from front and back.

      //Find group permissions, ignoring user permissions:
      const Glib::ustring strgroup = "group ";
      Glib::ustring::size_type posFind = item.find(strgroup);
      if(posFind == 0)
      {
        //It is a group permision:
        item = item.substr(strgroup.size());

        //Get the parts before and after the =:
        const type_vecStrings vecParts = string_separate(item, "=");
        if(vecParts.size() == 2)
        {
          const Glib::ustring this_group_name = vecParts[0];
          if(this_group_name == group_name) //Only look at permissions for the requested group.
          {
            Glib::ustring group_permissions = vecParts[1];

            //Get the part before the /user_who_granted_the_privileges:
            const type_vecStrings vecParts = string_separate(group_permissions, "/");
            if(!vecParts.empty())
              group_permissions = vecParts[0];

            //g_warning("  group=%s", group_name.c_str());
            //g_warning("  permisisons=%s", group_permissions.c_str());

            //Iterate through the characters:
            for(Glib::ustring::iterator iter = group_permissions.begin(); iter != group_permissions.end(); ++iter)
            {
              gunichar chperm = *iter;
              Glib::ustring perm(1, chperm);

              //See http://www.postgresql.org/docs/8.0/interactive/sql-grant.html
              if(perm == "r")
                result.m_view = true;
              else if (perm == "w")
                result.m_edit = true;
              else if (perm == "a")
                result.m_create = true;
              else if (perm == "d")
                result.m_delete = true;
            }
          }
        }
      }

    }
  }

  g_warning("get_table_privileges(group_name=%s, table_name=%s) returning: %d", group_name.c_str(), table_name.c_str(), result.m_create);
  return result;
}

void Base_DB::add_standard_tables() const
{
  //Name, address, etc:
  if(!get_table_exists_in_database(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME))
  {
    TableInfo prefs_table_info;
    prefs_table_info.m_name = GLOM_STANDARD_TABLE_PREFS_TABLE_NAME;
    prefs_table_info.m_hidden = true;

    Document_Glom::type_vecFields pref_fields;

    Field primary_key; //It's not used, because there's only one record, but we must have one.
    primary_key.set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ID);
    primary_key.set_glom_type(Field::TYPE_NUMERIC);
    pref_fields.push_back(primary_key);

    Field field_name;
    field_name.set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_NAME);
    field_name.set_glom_type(Field::TYPE_TEXT);
    pref_fields.push_back(field_name);

    Field field_org_name;
    field_org_name.set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_NAME);
    field_org_name.set_glom_type(Field::TYPE_TEXT);
    pref_fields.push_back(field_org_name);

    Field field_org_address_street;
    field_org_address_street.set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET);
    field_org_address_street.set_glom_type(Field::TYPE_TEXT);
    pref_fields.push_back(field_org_address_street);

    Field field_org_address_street2;
    field_org_address_street2.set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET2);
    field_org_address_street2.set_glom_type(Field::TYPE_TEXT);
    pref_fields.push_back(field_org_address_street2);

    Field field_org_address_town;
    field_org_address_town.set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_TOWN);
    field_org_address_town.set_glom_type(Field::TYPE_TEXT);
    pref_fields.push_back(field_org_address_town);

    Field field_org_address_county;
    field_org_address_county.set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTY);
    field_org_address_county.set_glom_type(Field::TYPE_TEXT);
    pref_fields.push_back(field_org_address_county);

    Field field_org_address_country;
    field_org_address_country.set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTRY);
    field_org_address_country.set_glom_type(Field::TYPE_TEXT);
    pref_fields.push_back(field_org_address_country);

    Field field_org_address_postcode;
    field_org_address_postcode.set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_POSTCODE);
    field_org_address_postcode.set_glom_type(Field::TYPE_TEXT);
    pref_fields.push_back(field_org_address_postcode);

    bool test = create_table(prefs_table_info, pref_fields);

    if(test)
    {
      //Add the single record:
      Query_execute("INSERT INTO " GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "(" GLOM_STANDARD_TABLE_PREFS_FIELD_ID ") VALUES (1)");
    }
  }

  //Auto-increment next values:
  if(!get_table_exists_in_database(GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME))
  {
    TableInfo table_info;
    table_info.m_name = GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME;
    table_info.m_hidden = true;

    Document_Glom::type_vecFields fields;

    Field primary_key; //It's not used, because there's only one record, but we must have one.
    primary_key.set_name(GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_ID);
    primary_key.set_glom_type(Field::TYPE_NUMERIC);
    fields.push_back(primary_key);

    Field field_table_name;
    field_table_name.set_name(GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_TABLE_NAME);
    field_table_name.set_glom_type(Field::TYPE_TEXT);
    fields.push_back(field_table_name);

    Field field_field_name;
    field_field_name.set_name(GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_FIELD_NAME);
    field_field_name.set_glom_type(Field::TYPE_TEXT);
    fields.push_back(field_field_name);

    Field field_next_value;
    field_next_value.set_name(GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_NEXT_VALUE);
    field_next_value.set_glom_type(Field::TYPE_TEXT);
    fields.push_back(field_next_value);

    create_table(table_info, fields);
  }
}

void Base_DB::add_standard_groups()
{
  //Add the glom_developer group if it does not exist:
  const Glib::ustring devgroup = GLOM_STANDARD_GROUP_NAME_DEVELOPER;

  const type_vecStrings vecGroups = get_database_groups();
  type_vecStrings::const_iterator iterFind = std::find(vecGroups.begin(), vecGroups.end(), devgroup);
  if(iterFind == vecGroups.end())
  {
    Query_execute("CREATE GROUP " GLOM_STANDARD_GROUP_NAME_DEVELOPER);
    Privileges priv_devs;
    priv_devs.m_view = true;
    priv_devs.m_edit = true;
    priv_devs.m_create = true;
    priv_devs.m_delete = true;

    Document_Glom::type_listTableInfo table_list = get_document()->get_tables();

    for(Document_Glom::type_listTableInfo::const_iterator iter = table_list.begin(); iter != table_list.end(); ++iter)
    {
      set_table_privileges(devgroup, iter->m_name, priv_devs, true /* developer privileges */);
    }

    //Make sure that it is in the database too:
    GroupInfo group_info;
    group_info.m_name = GLOM_STANDARD_GROUP_NAME_DEVELOPER;
    group_info.m_developer = true;
    get_document()->set_group(group_info);
  }
}

Gnome::Gda::Value Base_DB::auto_increment_insert_first_if_necessary(const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  Gnome::Gda::Value value;

  const Glib::ustring sql_query = "SELECT " GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME ".next_value FROM " GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME
   " WHERE " GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_TABLE_NAME " = '" + table_name + "' AND "
             GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_FIELD_NAME " = '" + field_name + "'";

  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = Query_execute(sql_query);
  if(!datamodel || (datamodel->get_n_rows() == 0))
  {
    //Start with zero:

    //Insert the row if it's not there.
    const Glib::ustring sql_query = "INSERT INTO " GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME " ("
      GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_TABLE_NAME ", " GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_FIELD_NAME ", " GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_NEXT_VALUE
      ") VALUES ('" + table_name + "', '" + field_name + "', 0)";

    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = Query_execute(sql_query);
    if(!datamodel)
    {
      g_warning("Base_DB::auto_increment_insert_first_if_necessary(): INSERT of new row failed.");
    }

    //GdaNumeric is a pain, so we take a short-cut:
    bool success = false;
    value = GlomConversions::parse_value(Field::TYPE_NUMERIC, "0", success, true /* iso_format */);
  }
  else
  {
    //Return the value so that a calling function does not need to do a second SELECT.
    value = datamodel->get_value_at(0, 0);
  }

  return value;
}

Gnome::Gda::Value Base_DB::get_next_auto_increment_value(const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  const Gnome::Gda::Value result = auto_increment_insert_first_if_necessary(table_name, field_name);
  long num_result = 0;
  num_result = util_decimal_from_string(result.to_string());


  //Increment the next_value:
  ++num_result;
  const Gnome::Gda::Value next_value = GlomConversions::parse_value(num_result);

  const Glib::ustring sql_query = "UPDATE " GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME " SET "
      GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_NEXT_VALUE " = " + next_value.to_string() +
      " WHERE " GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_TABLE_NAME " = '" + table_name + "' AND "
                GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_FIELD_NAME " = '" + field_name + "'";

  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = Query_execute(sql_query);
  if(!datamodel)
  {
    g_warning("Base_DB::get_next_auto_increment_value(): Increment failed.");
  }

  return result;
}


Glib::ustring Base_DB::string_trim(const Glib::ustring& str, const Glib::ustring& to_remove)
{
   Glib::ustring result = str;

   //Remove from the start:
   Glib::ustring::size_type posOpenBracket = result.find(to_remove);
   if(posOpenBracket == 0)
   {
//g_warning("string_trim: before start trim: %s", result.c_str());
      result = result.substr(to_remove.size());
//g_warning("string_trim: after start trim: %s", result.c_str());
   }

   //Remove from the end:
   Glib::ustring::size_type posCloseBracket = result.rfind(to_remove);
   if(posCloseBracket == (result.size() - to_remove.size()))
   {
     //g_warning("string_trim: before end trim: %s", result.c_str());
    result = result.substr(0, posCloseBracket);
     //g_warning("string_trim: after end trim: %s", result.c_str());
   }

  return result;
}

Base_DB::type_vecStrings Base_DB::string_separate(const Glib::ustring& str, const Glib::ustring& separator)
{
  type_vecStrings result;

  Glib::ustring unprocessed = str;
  while(!unprocessed.empty())
  {
    Glib::ustring::size_type posComma = unprocessed.find(separator);

    Glib::ustring item;
    if(posComma != Glib::ustring::npos)
    {
      item = unprocessed.substr(0, posComma);
      unprocessed = unprocessed.substr(posComma + separator.size());
    }
    else
    {
        item = unprocessed;
        unprocessed.clear();
    }

    if(!item.empty())
      result.push_back(item);

    unprocessed = string_trim(unprocessed, " ");
  }

  return result;
}

Base_DB::type_vecStrings Base_DB::pg_list_separate(const Glib::ustring& str)
{
  //Remove the first { and the last }:
  Glib::ustring without_brackets = string_trim(str, "{");
  without_brackets = string_trim(without_brackets, "}");

  //Get the comma-separated items:
  return string_separate(without_brackets, ",");
}

Glib::ustring Base_DB::get_user_visible_group_name(const Glib::ustring& group_name) const
{
  Glib::ustring result = group_name;

  //Remove the special prefix:
  const Glib::ustring prefix = "glom_";
  if(result.substr(0, prefix.size()) == prefix)
    result = result.substr(prefix.size());

  return result;
}

Base_DB::type_vecStrings Base_DB::get_groups_of_user(const Glib::ustring& user) const
{
  //TODO_Performance

  type_vecStrings result;

  //Look at each group:
  type_vecStrings groups = get_database_groups();
  for(type_vecStrings::const_iterator iter = groups.begin(); iter != groups.end(); ++iter)
  {
    //See whether the user is in this group:
    if(get_user_is_in_group(user, *iter))
    {
      //Add the group to the result:
      result.push_back(*iter);
    }
  }

  return result;
}

bool Base_DB::get_user_is_in_group(const Glib::ustring& user, const Glib::ustring& group) const
{
  const type_vecStrings users = get_database_users(group);
  type_vecStrings::const_iterator iterFind = std::find(users.begin(), users.end(), user);
  return (iterFind != users.end());
}

Privileges Base_DB::get_current_privs(const Glib::ustring& table_name) const
{
  //TODO_Performance: There's lots of database access here.
  //We could maybe replace some with the postgres has_table_* function().

  Privileges result;

  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  const Glib::ustring current_user = connection_pool->get_user();

  //Is the user in the special developers group?
  /*
  type_vecStrings developers = get_database_users(GLOM_STANDARD_GROUP_NAME_DEVELOPER);
  type_vecStrings::const_iterator iterFind = std::find(developers.begin(), developers.end(), current_user);
  if(iterFind != developers.end())
  {
    result.m_developer = true;
  }
  */

  //Get the "true" rights for any groups that the user is in:
  type_vecStrings groups = get_groups_of_user(current_user);
  for(type_vecStrings::const_iterator iter = groups.begin(); iter != groups.end(); ++iter)
  {
    Privileges privs = get_table_privileges(*iter, table_name);

    if(privs.m_view)
      result.m_view = true;

    if(privs.m_edit)
      result.m_edit = true;

    if(privs.m_create)
      result.m_create = true;

    if(privs.m_delete)
      result.m_delete = true;
  }

  return result;
}

SystemPrefs Base_DB::get_database_preferences() const
{
  //if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  //  add_standard_tables();

  SystemPrefs result;

  const Glib::ustring sql_query = "SELECT "
      GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "." GLOM_STANDARD_TABLE_PREFS_FIELD_NAME ", "
      GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "." GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_NAME ", "
      GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "." GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET ", "
      GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "." GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET2 ", "
      GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "." GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_TOWN ", "
      GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "." GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTY ", "
      GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "." GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTRY ", "
      GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "." GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_POSTCODE
      " FROM " GLOM_STANDARD_TABLE_PREFS_TABLE_NAME;

  try
  {
    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = Query_execute(sql_query);
    if(datamodel && (datamodel->get_n_rows() != 0))
    {
      result.m_name = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(0, 0));
      result.m_org_name = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(1, 0));
      result.m_org_address_street = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(2, 0));
      result.m_org_address_street2 = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(3, 0));
      result.m_org_address_town = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(4, 0));
      result.m_org_address_county = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(5, 0));
      result.m_org_address_country = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(6, 0));
      result.m_org_address_postcode = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(7, 0));      }
  }
  catch(const std::exception& ex)
  {
    std::cerr << "Base_DB::get_database_preferences(): exception: " << ex.what() << std::endl;
  }

  return result;
}

void Base_DB::set_database_preferences(const SystemPrefs& prefs)
{
  if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
    add_standard_tables();

  const Glib::ustring sql_query = "UPDATE " GLOM_STANDARD_TABLE_PREFS_TABLE_NAME " SET "
      GLOM_STANDARD_TABLE_PREFS_FIELD_NAME " = '" + prefs.m_name + "', "
      GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_NAME " = '" + prefs.m_org_name + "', "
      GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET " = '" + prefs.m_org_address_street + "', "
      GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET2 " = '" + prefs.m_org_address_street2 + "', "
      GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_TOWN " = '" + prefs.m_org_address_town + "', "
      GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTY " = '" + prefs.m_org_address_county + "', "
      GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTRY " = '" + prefs.m_org_address_country + "', "
      GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_POSTCODE " = '" + prefs.m_org_address_postcode + "'"
      " WHERE " GLOM_STANDARD_TABLE_PREFS_FIELD_ID " = 1";

    try
    {
      Glib::RefPtr<Gnome::Gda::DataModel> datamodel = Query_execute(sql_query);
    }
    catch(const std::exception& ex)
    {
      std::cerr << "Base_DB::set_database_preferences(): exception: " << ex.what() << std::endl;
    }
}

bool Base_DB::create_table(const TableInfo& table_info, const Document_Glom::type_vecFields& fields) const
{
  bool table_creation_succeeded = false;

  //Create SQL to describe all fields in this table:
  Glib::ustring sql_fields;
  for(Document_Glom::type_vecFields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
  {
    //Create SQL to describe this field:
    Field field = *iter;

    //The field has no gda type, so we set that:
    //This usually comes from the database, but that's a bit strange.
    Gnome::Gda::FieldAttributes info = field.get_field_info();
    info.set_gdatype( Field::get_gda_type_for_glom_type(field.get_glom_type()) );
    field.set_field_info(info);

    Glib::ustring sql_field_description = field.get_name() + " " + field.get_sql_type();

    if(field.get_primary_key())
      sql_field_description += " NOT NULL  PRIMARY KEY";

    //Append it:
    if(!sql_fields.empty())
      sql_fields += ", ";

    sql_fields += sql_field_description;
  }

  if(sql_fields.empty())
  {
    g_warning("Base_Db::create_table::create_table(): sql_fields is empty.");
  }

  //Actually create the table
  try
  {
    Glib::RefPtr<Gnome::Gda::DataModel> data_model = Query_execute( "CREATE TABLE \"" + table_info.get_name() + "\" (" + sql_fields + ")" );
    if(!data_model)
      table_creation_succeeded = false;
    else
      table_creation_succeeded = true;
  }
  catch(const ExceptionConnection& ex)
  {
    table_creation_succeeded = false;
  }

  return table_creation_succeeded;
}

bool Base_DB::offer_field_list(LayoutItem_Field& field, const Glib::ustring& table_name)
{
  bool result = false;

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_choose_field");

    Dialog_ChooseField* dialog = 0;
    refXml->get_widget_derived("dialog_choose_field", dialog);

    if(dialog)
    {
      dialog->set_document(get_document(), table_name, field);
      //TODO: dialog->set_transient_for(*get_app_window());
      int response = dialog->run();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen field:
        result = dialog->get_field_chosen(field);
      }

      delete dialog;
    }
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  return result;
}

void Base_DB::fill_full_field_details(const Glib::ustring& parent_table_name, LayoutItem_Field& layout_item)
{
  Glib::ustring table_name = parent_table_name;

  if(layout_item.get_has_relationship_name())
    table_name = layout_item.m_relationship.get_to_table();

  get_document()->get_field(table_name, layout_item.get_name(), layout_item.m_field);
}

Glib::ustring Base_DB::get_layout_item_table_name(const LayoutItem_Field& layout_item, const Glib::ustring& table_name)
{
  if(!layout_item.get_has_relationship_name())
    return table_name;
  else
  {
    const Glib::ustring relationship_name = layout_item.get_relationship_name();
    Relationship relationship; //TODO: We should not need to do this. It should be updated in the LayoutItem_Field already.
    Document_Glom* document = get_document();
    bool test = document->get_relationship(table_name, relationship_name, relationship);
    if(test)
     return relationship.get_to_table();
  }

  return Glib::ustring();
}

void Base_DB::report_build_summary(const Glib::ustring& table_name, xmlpp::Element& parent_node, LayoutItem_Summary& summary, const Glib::ustring& where_clause)
{
  //Add XML node:
  xmlpp::Element* nodeSummary = parent_node.add_child("summary");

  //Get fields
  GlomUtils::type_vecLayoutFields fieldsToGet;
  for(LayoutGroup::type_map_items::iterator iterChildren = summary.m_map_items.begin(); iterChildren != summary.m_map_items.end(); ++iterChildren)
  {
    LayoutItem_Field* pField = dynamic_cast<LayoutItem_Field*>(iterChildren->second);
    if(pField)
    {
      fill_full_field_details(table_name, *pField);
      fieldsToGet.push_back( sharedptr<LayoutItem_Field>( static_cast<LayoutItem_Field*>(pField->clone() ) ) );
    }
    else
    {
      LayoutItem_GroupBy* pGroupBy = dynamic_cast<LayoutItem_GroupBy*>(iterChildren->second);
      if(pGroupBy)
      {
        //Recurse, adding a sub-groupby block:
        report_build_groupby(table_name, *nodeSummary, *pGroupBy, where_clause);
      }
      else
      {
        LayoutItem_Summary* pSummary = dynamic_cast<LayoutItem_Summary*>(iterChildren->second);
        if(pSummary)
        {
          //Recurse, adding a summary block:
          report_build_summary(table_name, *nodeSummary, *pSummary, where_clause);
        }
      }
    }
  }

  if(!fieldsToGet.empty())
  {
    //Rows, with data:
    report_build_records(table_name, *nodeSummary, fieldsToGet, where_clause, Glib::ustring() /* No sort_clause because there is only one row */);
  }
}

void Base_DB::report_build_groupby(const Glib::ustring& table_name, xmlpp::Element& parent_node, LayoutItem_GroupBy& group_by, const Glib::ustring& where_clause_parent)
{
  //Get the possible heading values.
  LayoutItem_Field* field_group_by = group_by.get_field_group_by();
  if(field_group_by)
  {
    fill_full_field_details(table_name, *field_group_by);

    //Get the possible group values, ignoring repeats by using GROUP BY.
    const Glib::ustring group_field_table_name = get_layout_item_table_name(*field_group_by, table_name);
    Glib::ustring sql_query = "SELECT " + group_field_table_name + "." + field_group_by->get_name() +
      " FROM " + group_field_table_name;

    if(!where_clause_parent.empty())
      sql_query += " WHERE " + where_clause_parent;

    sql_query += " GROUP BY " + field_group_by->get_name(); //rTODO: And restrict to the current found set.

    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = Query_execute(sql_query);
    if(datamodel)
    {
      guint rows_count = datamodel->get_n_rows();
      for(guint row = 0; row < rows_count; ++row)
      {
        const Gnome::Gda::Value group_value = datamodel->get_value_at(0 /* col*/, row);

        //Add XML node:
        xmlpp::Element* nodeGroupBy = parent_node.add_child("group_by");

        nodeGroupBy->set_attribute("group_field", field_group_by->get_title_or_name());
        nodeGroupBy->set_attribute("group_value",
          GlomConversions::get_text_for_gda_value(field_group_by->m_field.get_glom_type(), group_value, field_group_by->m_numeric_format) );

        Glib::ustring where_clause = group_field_table_name + "." + field_group_by->get_name() + " = " + field_group_by->m_field.sql(group_value);
        if(!where_clause_parent.empty())
          where_clause += " AND (" + where_clause_parent + ")";

        //Secondary fields. For instance, the Contact Name, in addition to the Contact ID that we group by.
        if(!(group_by.m_group_secondary_fields.m_map_items.empty()))
        {
          xmlpp::Element* nodeSecondaryFields = nodeGroupBy->add_child("secondary_fields");

          GlomUtils::type_vecLayoutFields fieldsToGet;
          for(LayoutGroup::type_map_items::iterator iterChildren = group_by.m_group_secondary_fields.m_map_items.begin(); iterChildren != group_by.m_group_secondary_fields.m_map_items.end(); ++iterChildren)
          {
            LayoutItem_Field* pField = dynamic_cast<LayoutItem_Field*>(iterChildren->second);
            if(pField)
            {
              fill_full_field_details(table_name, *pField);
              fieldsToGet.push_back( sharedptr<LayoutItem_Field>( static_cast<LayoutItem_Field*>(pField->clone() ) ) );
            }
          }

          if(!fieldsToGet.empty())
          {
            report_build_records(table_name, *nodeSecondaryFields, fieldsToGet, where_clause, Glib::ustring(), true /* one record only */);
          }
        }


        //Get data and add child rows:
        GlomUtils::type_vecLayoutFields fieldsToGet;
        for(LayoutGroup::type_map_items::iterator iterChildren = group_by.m_map_items.begin(); iterChildren != group_by.m_map_items.end(); ++iterChildren)
        {
          LayoutItem_Field* pField = dynamic_cast<LayoutItem_Field*>(iterChildren->second);
          if(pField)
          {
            fill_full_field_details(table_name, *pField);
            fieldsToGet.push_back( sharedptr<LayoutItem_Field>( static_cast<LayoutItem_Field*>(pField->clone() ) ) );
          }
          else
          {
            LayoutItem_GroupBy* pGroupBy = dynamic_cast<LayoutItem_GroupBy*>(iterChildren->second);
            if(pGroupBy)
            {
              //Recurse, adding a sub-groupby block:
              report_build_groupby(table_name, *nodeGroupBy, *pGroupBy, where_clause);
            }
            else
            {
              LayoutItem_Summary* pSummary = dynamic_cast<LayoutItem_Summary*>(iterChildren->second);
              if(pSummary)
              {
                //Recurse, adding a summary block:
                report_build_summary(table_name, *nodeGroupBy, *pSummary, where_clause);
              }
            }
          }
        }

        if(!fieldsToGet.empty())
        {
          //Rows, with data:
          Glib::ustring sort_clause;
          if(group_by.get_field_sort_by())
            sort_clause = group_by.get_field_sort_by()->get_name(); //TODO: Deal with related fields too.

          report_build_records(table_name, *nodeGroupBy, fieldsToGet, where_clause, sort_clause);
        }
      }
    }
  }
}

void Base_DB::report_build_records(const Glib::ustring& table_name, xmlpp::Element& parent_node, const GlomUtils::type_vecLayoutFields& fieldsToGet, const Glib::ustring& where_clause, const Glib::ustring& sort_clause, bool one_record_only)
{
  if(!fieldsToGet.empty())
  {
    //Field headings:
    for(GlomUtils::type_vecLayoutFields::const_iterator iter = fieldsToGet.begin(); iter != fieldsToGet.end(); ++iter)
    {
      sharedptr<LayoutItem_Field> layout_item = *iter;
      xmlpp::Element* nodeFieldHeading = parent_node.add_child("field_heading");
      nodeFieldHeading->set_attribute("name", layout_item->get_name()); //Not really necessary, but maybe useful.
      nodeFieldHeading->set_attribute("title", layout_item->get_title_or_name());
    }

    Glib::ustring sql_query = GlomUtils::build_sql_select_with_where_clause(table_name,
      fieldsToGet,
      where_clause, sort_clause);

    //For instance, when we just want to get a name corresponding to a contact ID, and want to ignore duplicates.
    if(one_record_only)
      sql_query += "LIMIT 1";

    bool records_found = false;
    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = Query_execute(sql_query);
    if(datamodel)
    {
      guint rows_count = datamodel->get_n_rows();
      if(rows_count > 0)
        records_found = true;

      for(guint row = 0; row < rows_count; ++row)
      {
        xmlpp::Element* nodeRow = parent_node.add_child("row");

        for(guint col = 0; col < fieldsToGet.size(); ++col)
        {
          sharedptr<LayoutItem_Field> field = fieldsToGet[col];
          const Field::glom_field_type field_type = field->m_field.get_glom_type();

          xmlpp::Element* nodeField = 0;
          if(field_type == Field::TYPE_NUMERIC)
             nodeField = nodeRow->add_child("field_numeric"); //TODO: I would prefer just to add a field_type attribute to the "field" node and use an <xsl::if> instead. murrayc.
          else
             nodeField = nodeRow->add_child("field");

          nodeField->set_attribute("name", field->get_name()); //Not really necessary, but maybe useful.

          Glib::ustring text_value = GlomConversions::get_text_for_gda_value(field_type, datamodel->get_value_at(col, row), field->m_numeric_format);

          //The Postgres summary functions return NULL when summarising NULL records, but 0 is more sensible:
          if(text_value.empty() && dynamic_cast<LayoutItem_FieldSummary*>(field.obj()) && (field_type == Field::TYPE_NUMERIC))
          {
            //Use get_text_for_gda_value() instead of "0" so we get the correct numerical formatting:
            Gnome::Gda::Value value = GlomConversions::parse_value(0);
            text_value = GlomConversions::get_text_for_gda_value(field_type, value, field->m_numeric_format);
          }

          nodeField->set_attribute("value", text_value);
        }
      }

    }

    //If there are no records, show zero
    //if(!rows_found && show_null_row)
  }
}


void Base_DB::report_build(const Glib::ustring& table_name, const Report& report, const Glib::ustring& where_clause)
{
  //Create a DOM Document with the XML:
  xmlpp::DomParser dom_parser;;

  xmlpp::Document* pDocument = dom_parser.get_document();
  xmlpp::Element* nodeRoot = pDocument->get_root_node();
  if(!nodeRoot)
  {
    //Add it if it isn't there already:
    nodeRoot = pDocument->create_root_node("report_print");
  }

  Glib::ustring table_title = get_document()->get_table_title(table_name);
  if(table_title.empty())
    table_title = table_name;

  nodeRoot->set_attribute("table", table_title);


  //The groups:
  xmlpp::Element* nodeParent = nodeRoot;


  nodeRoot->set_attribute("title", report.get_title_or_name());

  GlomUtils::type_vecLayoutFields fieldsToGet_TopLevel;

  for(LayoutGroup::type_map_items::const_iterator iter = report.m_layout_group.m_map_items.begin(); iter != report.m_layout_group.m_map_items.end(); ++iter)
  {
    LayoutItem* pPart = iter->second;

    //The Group, and the details for each record in the group:
    LayoutItem_GroupBy* pGroupBy = dynamic_cast<LayoutItem_GroupBy*>(pPart);
    if(pGroupBy)
      report_build_groupby(table_name, *nodeParent, *pGroupBy, where_clause);
    else
    {
      LayoutItem_Summary* pSummary = dynamic_cast<LayoutItem_Summary*>(pPart);
      if(pSummary)
      {
        //Recurse, adding a summary block:
        report_build_summary(table_name, *nodeParent, *pSummary, where_clause);
      }
      else
      {
        LayoutItem_Field* pField = dynamic_cast<LayoutItem_Field*>(pPart);
        if(pField)
        {
          fieldsToGet_TopLevel.push_back( sharedptr<LayoutItem_Field>( static_cast<LayoutItem_Field*>(pField->clone() ) ) );
        }
      }
    }
  }

  //Add top-level records, outside of any groupby or summary, if fields have been specified:
  if(!fieldsToGet_TopLevel.empty())
  {
    xmlpp::Element* nodeGroupBy = nodeParent->add_child("ungrouped_records");
    report_build_records(table_name, *nodeGroupBy, fieldsToGet_TopLevel, where_clause, Glib::ustring() /* no sort clause */);
  }

  GlomUtils::transform_and_open(*pDocument, "print_report_to_html.xsl");
}
