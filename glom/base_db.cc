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
#include <glom/libglom/appstate.h>
#include <glom/libglom/standard_table_prefs_fields.h>
#include <glom/libglom/document/document_glom.h>
#include <glom/libglom/data_structure/glomconversions.h>
#include "mode_data/dialog_choose_field.h"
#include "layout_item_dialogs/dialog_field_layout.h"
#include "layout_item_dialogs/dialog_notebook.h"
#include "layout_item_dialogs/dialog_textobject.h"
#include "layout_item_dialogs/dialog_imageobject.h"
//#include "reports/dialog_layout_report.h"
#include <glom/libglom/utils.h>
#include "xsl_utils.h"
#include <glom/libglom/data_structure/glomconversions.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_summary.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_verticalgroup.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_header.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_footer.h>
#include "python_embed/glom_python.h"
#include <glibmm/i18n.h>
//#include <libgnomeui/gnome-app-helper.h>

#include <sstream> //For stringstream

FoundSet::FoundSet()
{
}

FoundSet::FoundSet(const FoundSet& src)
:  m_table_name(src.m_table_name),
   m_where_clause(src.m_where_clause),
   m_sort_clause(src.m_sort_clause)
{
}

FoundSet& FoundSet::operator=(const FoundSet& src)
{
  m_table_name = src.m_table_name;
  m_where_clause = src.m_where_clause;
  m_sort_clause = src.m_sort_clause;

  return *this;
}

bool FoundSet::operator==(const FoundSet& src) const
{
  return (m_table_name == src.m_table_name)
      && (m_where_clause == src.m_where_clause)
      && (m_sort_clause == src.m_sort_clause);
}


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
sharedptr<SharedConnection> Base_DB::connect_to_server(Gtk::Window* parent_window)
{
  Bakery::BusyCursor busy_cursor(parent_window);

  return ConnectionPool::get_and_connect();
}

void Base_DB::handle_error(const std::exception& ex) const
{
  std::cerr << "Internal Error (Base_DB::handle_error()): exception type=" << typeid(ex).name() << ", ex.what()=" << ex.what() << std::endl;

  Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Internal error")), true, Gtk::MESSAGE_WARNING );
  dialog.set_secondary_text(ex.what());
  //TODO: dialog.set_transient_for(*get_application());
  dialog.run();
}

bool Base_DB::handle_error() const
{
  return ConnectionPool::handle_error();
}


Glib::RefPtr<Gnome::Gda::DataModel> Base_DB::query_execute(const Glib::ustring& strQuery, Gtk::Window* parent_window) const
{
  Glib::RefPtr<Gnome::Gda::DataModel> result;

  //TODO: Bakery::BusyCursor busy_cursor(get_app_window());

  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

    /*
    try
    {
      std::cout << "Debug: query_execute():  " << strQuery << std::endl;
    }
    catch(const Glib::Exception& ex)
    {
      std::cout << "Debug: query string could not be converted to std::cout: " << ex.what() << std::endl;
    }
    */

    result = gda_connection->execute_single_command(strQuery);
    if(!result)
    {
      std::cerr << "Glom  Base_DB::query_execute(): Error while executing SQL" << std::endl <<
                   "  " <<  strQuery << std::endl;
      handle_error();
    }
  }
  else
  {
    g_warning("Base_DB::query_execute(): No connection yet.");
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

  type_vecStrings tables = get_table_names_from_database();
  type_vecStrings::const_iterator iterFind = std::find(tables.begin(), tables.end(), table_name);
  bool result = (iterFind != tables.end());

  return result;
}

Base_DB::type_vecStrings Base_DB::get_table_names_from_database(bool ignore_system_tables) const
{
  type_vecStrings result;

  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();
    Glib::RefPtr<Gnome::Gda::DataModel> data_model_tables = gda_connection->get_schema(Gnome::Gda::CONNECTION_SCHEMA_TABLES);
    if(data_model_tables && (data_model_tables->get_n_columns() == 0))
    {
      std::cerr << "Base_DB_Table::get_table_names_from_database(): libgda reported 0 tables for the database." << std::endl;
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

          bool add_it = true;

          if(ignore_system_tables)
          {
            //Check whether it's a system table:
            const Glib::ustring prefix = "glom_system_";
            const Glib::ustring table_prefix = table_name.substr(0, prefix.size());
            if(table_prefix == prefix)
              add_it = false;
          }

          //Ignore the pga_* tables that pgadmin adds when you use it:
          if(table_name.substr(0, 4) == "pga_")
            add_it = false;

          if(add_it)
            result.push_back(table_name);
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
    vecNames.push_back(fields[i]->get_name());
  }

  return vecNames;
}

bool Base_DB::get_field_exists_in_database(const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  type_vecFields vecFields = get_fields_for_table_from_database(table_name);
  type_vecFields::const_iterator iterFind = std::find_if(vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(field_name));
  return iterFind != vecFields.end();
}

Base_DB::type_vecFields Base_DB::get_fields_for_table_from_database(const Glib::ustring& table_name, bool including_system_fields)
{
  type_vecFields result;

  if(table_name.empty())
    return result;

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

  //TODO: Bakery::BusyCursor busy_cursor(get_application());

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
        {
          if(value_name.get_string().empty())
            g_warning("Base_DB::get_fields_for_table_from_database(): value_name is empty.");

          field_info.set_name( value_name.get_string() ); //TODO: get_string() is a dodgy choice. murrayc.
        }

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

        sharedptr<Field> field(new Field()); //TODO: Get glom-specific information from the document?
        field->set_field_info(field_info);
        result.push_back(field);

        ++row;
      }
    }
  }

  if(result.empty())
  {
    //g_warning("Base_DB::get_fields_for_table_from_database(): returning empty result.");
  }

  //Hide system fields.
  type_vecFields::iterator iterFind = std::find_if(result.begin(), result.end(), predicate_FieldHasName<Field>(GLOM_STANDARD_FIELD_LOCK));
  if(iterFind != result.end())
    result.erase(iterFind);

  return result;
}

Base_DB::type_vecFields Base_DB::get_fields_for_table(const Glib::ustring& table_name, bool including_system_fields) const
{
  //Get field definitions from the database:
  type_vecFields fieldsDatabase = get_fields_for_table_from_database(table_name, including_system_fields);

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
      sharedptr<Field> field = *iter;
      const Glib::ustring field_name = field->get_name();

      //Get the field info from the database:
      //This is in the document as well, but it _might_ have changed.
      type_vecFields::const_iterator iterFindDatabase = std::find_if(fieldsDatabase.begin(), fieldsDatabase.end(), predicate_FieldHasName<Field>(field_name));

      if(iterFindDatabase != fieldsDatabase.end() ) //Ignore fields that don't exist in the database anymore.
      {
        Gnome::Gda::FieldAttributes field_info_document = field->get_field_info();

        //Update the Field information that _might_ have changed in the database.
        Gnome::Gda::FieldAttributes field_info = (*iterFindDatabase)->get_field_info();

        //libgda does not tell us whether the field is auto_incremented, so we need to get that from the document.
        field_info.set_auto_increment( field_info_document.get_auto_increment() );

        //libgda does not tell us whether the field is auto_incremented, so we need to get that from the document.
        field_info.set_primary_key( field_info_document.get_primary_key() );

        //libgda does yet tell us correct default_value information so we need to get that from the document.
        field_info.set_default_value( field_info_document.get_default_value() );

        field->set_field_info(field_info);

        result.push_back(*iter);
      }
    }

    //Add any fields that are in the database, but not in the document:
    for(type_vecFields::iterator iter = fieldsDatabase.begin(); iter != fieldsDatabase.end(); ++iter)
    {
      Glib::ustring field_name = (*iter)->get_name();

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

  Glib::ustring strQuery = "SELECT \"pg_group\".\"groname\" FROM \"pg_group\"";
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(strQuery);
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
    Glib::ustring strQuery = "SELECT \"pg_shadow\".\"usename\" FROM \"pg_shadow\"";
    Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(strQuery);
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
    Glib::ustring strQuery = "SELECT \"pg_group\".\"groname\", \"pg_group\".\"grolist\" FROM \"pg_group\" WHERE \"pg_group\".\"groname\" = '" + group_name + "'";
    Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(strQuery);
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
          Glib::ustring strQuery = "SELECT \"pg_user\".\"usename\" FROM \"pg_user\" WHERE \"pg_user\".\"usesysid\" = '" + *iter + "'";
          Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(strQuery);
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

  strQuery += " " + strPrivilege + " ON \"" + table_name + "\" ";

  //This must match the Grant or Revoke:
  strQuery += "TO";

  strQuery += " GROUP \"" + group_name + "\"";

  const bool test = query_execute(strQuery);

  if(test)
  {
    if( (table_name != GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME) && privs.m_create )
    {
      //To create a record, you will usually need write access to the autoincrements table,
      //so grant this too:
      Privileges priv_autoincrements;
      priv_autoincrements.m_view = true;
      priv_autoincrements.m_edit = true;
      set_table_privileges(group_name, GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME, priv_autoincrements);
    }
  }
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
  Glib::ustring strQuery = "SELECT \"pg_class\".\"relacl\" FROM \"pg_class\" WHERE \"pg_class\".\"relname\" = '" + table_name + "'";
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(strQuery);
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
          if(this_group_name == group_name) //Only look at permissions for the requested group->
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

  //g_warning("get_table_privileges(group_name=%s, table_name=%s) returning: %d", group_name.c_str(), table_name.c_str(), result.m_create);
  return result;
}

bool Base_DB::add_standard_tables() const
{
  try
  {
    Document_Glom::type_vecFields pref_fields;
    sharedptr<TableInfo> prefs_table_info = Document_Glom::create_table_system_preferences(pref_fields);

    //Name, address, etc:
    if(!get_table_exists_in_database(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME))
    {
      const bool test = create_table(prefs_table_info, pref_fields);

      if(test)
      {
        //Add the single record:
        query_execute("INSERT INTO \"" GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "\" (\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ID "\") VALUES (1)");

        //Use the database title from the document, if there is one:
        const Glib::ustring system_name = get_document()->get_database_title();
        if(!system_name.empty())
        {
          query_execute("UPDATE \"" GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "\" SET  " "\"" GLOM_STANDARD_TABLE_PREFS_FIELD_NAME "\" = '" + system_name + "' WHERE \"" GLOM_STANDARD_TABLE_PREFS_FIELD_ID "\" = 1");
        }
      }
      else
      {
        g_warning("Base_DB::add_standard_tables(): create_table(prefs) failed.");
        return false;
      }
    }
    else
    {
      //Make sure that it has all the fields it should have,
      //because we sometimes add some in new Glom versions:
      create_table_add_missing_fields(prefs_table_info, pref_fields);
    }

    //Auto-increment next values:
    if(!get_table_exists_in_database(GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME))
    {
      sharedptr<TableInfo> table_info(new TableInfo());
      table_info->set_name(GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME);
      table_info->set_title("System: Auto Increments"); //TODO: Provide standard translations.
      table_info->m_hidden = true;

      Document_Glom::type_vecFields fields;

      sharedptr<Field> primary_key(new Field()); //It's not used, because there's only one record, but we must have one.
      primary_key->set_name(GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_ID);
      primary_key->set_glom_type(Field::TYPE_NUMERIC);
      fields.push_back(primary_key);

      sharedptr<Field> field_table_name(new Field());
      field_table_name->set_name(GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_TABLE_NAME);
      field_table_name->set_glom_type(Field::TYPE_TEXT);
      fields.push_back(field_table_name);

      sharedptr<Field> field_field_name(new Field());
      field_field_name->set_name(GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_FIELD_NAME);
      field_field_name->set_glom_type(Field::TYPE_TEXT);
      fields.push_back(field_field_name);

      sharedptr<Field> field_next_value(new Field());
      field_next_value->set_name(GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_NEXT_VALUE);
      field_next_value->set_glom_type(Field::TYPE_TEXT);
      fields.push_back(field_next_value);

      const bool test = create_table(table_info, fields);
      if(!test)
      {
        g_warning("Base_DB::add_standard_tables(): create_table(autoincrements) failed.");
        return false;
      }

      return true;
    }
    else
    {
      return false;
    }
  }
  catch(const std::exception& ex)
  {
    std::cerr << "Base_DB::add_standard_tables(): caught exception: " << ex.what() << std::endl;
    return false;
  }
}

bool Base_DB::add_standard_groups()
{
  //Add the glom_developer group if it does not exist:
  const Glib::ustring devgroup = GLOM_STANDARD_GROUP_NAME_DEVELOPER;

  const type_vecStrings vecGroups = get_database_groups();
  type_vecStrings::const_iterator iterFind = std::find(vecGroups.begin(), vecGroups.end(), devgroup);
  if(iterFind == vecGroups.end())
  {
    bool test = query_execute("CREATE GROUP \"" GLOM_STANDARD_GROUP_NAME_DEVELOPER "\"");
    if(!test)
    {
      std::cerr << "Glom Base_DB::add_standard_groups(): CREATE GROUP failed when adding the developer group." << std::endl;
      return false;
    }

    //Make sure the current user is in the developer group.
    //(If he is capable of creating these groups then he is obviously a developer, and has developer rights on the postgres server.)
    const Glib::ustring current_user = ConnectionPool::get_instance()->get_user();
    Glib::ustring strQuery = "ALTER GROUP \"" GLOM_STANDARD_GROUP_NAME_DEVELOPER "\" ADD USER \"" + current_user + "\"";
    test = query_execute(strQuery);
    if(!test)
    {
      std::cerr << "Glom Base_DB::add_standard_groups(): ALTER GROUP failed when adding the user to the developer group." << std::endl;
      return false;
    }

    std::cout << "DEBUG: Added user " << current_user << " to glom developer group on postgres server." << std::endl;

    Privileges priv_devs;
    priv_devs.m_view = true;
    priv_devs.m_edit = true;
    priv_devs.m_create = true;
    priv_devs.m_delete = true;

    Document_Glom::type_listTableInfo table_list = get_document()->get_tables(true /* including system prefs */);

    for(Document_Glom::type_listTableInfo::const_iterator iter = table_list.begin(); iter != table_list.end(); ++iter)
    {
      sharedptr<const TableInfo> table_info = *iter;
      if(table_info)
      {
        const Glib::ustring table_name = table_info->get_name();
        if(get_table_exists_in_database(table_name)) //Maybe the table has not been created yet.
          set_table_privileges(devgroup, table_name, priv_devs, true /* developer privileges */);
      }
    }

    //Make sure that it is in the database too:
    GroupInfo group_info;
    group_info.set_name(GLOM_STANDARD_GROUP_NAME_DEVELOPER);
    group_info.m_developer = true;
    get_document()->set_group(group_info);
  }

  return true;
}

Gnome::Gda::Value Base_DB::auto_increment_insert_first_if_necessary(const Glib::ustring& table_name, const Glib::ustring& field_name) const
{
  Gnome::Gda::Value value;

  //Check that the user is allowd to view and edit this table:
  Privileges table_privs = get_current_privs(GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME);
  if(!table_privs.m_view || !table_privs.m_edit)
  {
    //This should not happen:
    std::cerr << "Glom: Base_DB::auto_increment_insert_first_if_necessary(): The current user may not edit the autoincrements table. Any user who has create rights for a table should have edit rights to the autoincrements table." << std::endl;
  }


  const Glib::ustring sql_query = "SELECT \"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME "\".\"next_value\" FROM \"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME "\""
   " WHERE \"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_TABLE_NAME "\" = '" + table_name + "' AND "
          "\"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_FIELD_NAME "\" = '" + field_name + "'";

  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute(sql_query);
  if(!datamodel || (datamodel->get_n_rows() == 0))
  {
    //Start with zero:

    //Insert the row if it's not there.
    const Glib::ustring sql_query = "INSERT INTO \"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME "\" ("
      GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_TABLE_NAME ", " GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_FIELD_NAME ", " GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_NEXT_VALUE
      ") VALUES ('" + table_name + "', '" + field_name + "', 0)";

    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute(sql_query);
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

void Base_DB::recalculate_next_auto_increment_value(const Glib::ustring& table_name, const Glib::ustring& field_name) const
{
  //Make sure that the row exists in the glom system table:
  auto_increment_insert_first_if_necessary(table_name, field_name);

  //Get the max key value in the database:
  const Glib::ustring sql_query = "SELECT MAX(\"" + table_name + "\".\"" + field_name + "\") FROM \"" + table_name + "\"";
  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute(sql_query);
  if(datamodel && datamodel->get_n_rows() && datamodel->get_n_columns())
  {
    //Increment it:
    const Gnome::Gda::Value value_max = datamodel->get_value_at(0, 0);
    long num_max = util_decimal_from_string(value_max.to_string()); //TODO: Is this sensible? Probably not.
    ++num_max;

    //Set it in the glom system table:
    const Gnome::Gda::Value next_value = GlomConversions::parse_value(num_max);
    const Glib::ustring sql_query = "UPDATE \"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME "\" SET "
      "\"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_NEXT_VALUE "\" = " + next_value.to_string() + //TODO: Don't use to_string().
      " WHERE \"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_TABLE_NAME "\" = '" + table_name + "' AND "
             "\"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_FIELD_NAME "\" = '" + field_name + "'";

    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute(sql_query);
    if(!datamodel)
    {
      g_warning("Base_DB::recalculate_next_auto_increment_value(): UPDATE failed.");
    }
  }
  else
  {
    g_warning("Base_DB::recalculate_next_auto_increment_value(): SELECT MAX() failed.");
  }
}

Gnome::Gda::Value Base_DB::get_next_auto_increment_value(const Glib::ustring& table_name, const Glib::ustring& field_name) const
{
  const Gnome::Gda::Value result = auto_increment_insert_first_if_necessary(table_name, field_name);
  long num_result = 0;
  num_result = util_decimal_from_string(result.to_string()); //TODO: Is this sensible? Probably not.


  //Increment the next_value:
  ++num_result;
  const Gnome::Gda::Value next_value = GlomConversions::parse_value(num_result);

  const Glib::ustring sql_query = "UPDATE \"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME "\" SET "
      "\"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_NEXT_VALUE "\" = " + next_value.to_string() +
      " WHERE \"" GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_TABLE_NAME "\" = '" + table_name + "' AND "
            "\""  GLOM_STANDARD_TABLE_AUTOINCREMENTS_FIELD_FIELD_NAME "\" = '" + field_name + "'";

  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute(sql_query);
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

  //Check that the user is allowd to even view this table:
  Privileges table_privs = get_current_privs(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME);
  if(!table_privs.m_view)
    return result;

  const bool optional_org_logo = get_field_exists_in_database(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME, GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_LOGO);

  const Glib::ustring sql_query = "SELECT "
      "\"" GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "\".\"" GLOM_STANDARD_TABLE_PREFS_FIELD_NAME "\", "
      "\"" GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "\".\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_NAME "\", "
      "\"" GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "\".\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET "\", "
      "\"" GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "\".\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET2 "\", "
      "\"" GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "\".\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_TOWN "\", "
      "\"" GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "\".\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTY "\", "
      "\"" GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "\".\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTRY "\", "
      "\"" GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "\".\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_POSTCODE "\""
      + Glib::ustring(optional_org_logo ? ", \"" GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "\".\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_LOGO "\"" : "") +
      " FROM \"" GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "\"";

  try
  {
    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute(sql_query);
    if(datamodel && (datamodel->get_n_rows() != 0))
    {
      result.m_name = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(0, 0));
      result.m_org_name = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(1, 0));
      result.m_org_address_street = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(2, 0));
      result.m_org_address_street2 = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(3, 0));
      result.m_org_address_town = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(4, 0));
      result.m_org_address_county = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(5, 0));
      result.m_org_address_country = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(6, 0));
      result.m_org_address_postcode = GlomConversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(7, 0));

      //We need to be more clever about these column indexes if we add more new fields:
      if(optional_org_logo)
        result.m_org_logo = datamodel->get_value_at(8, 0);

    }
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

  //The logo field was introduced in a later version of Glom.
  //If the user is not in developer mode then the new field has not yet been added:
  Glib::ustring optional_part_logo;
  if(get_field_exists_in_database(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME, GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_LOGO))
  {
    Field field_temp_logo;
    field_temp_logo.set_glom_type(Field::TYPE_IMAGE);
    const Glib::ustring logo_escaped = field_temp_logo.sql(prefs.m_org_logo);
    optional_part_logo =  "\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_LOGO "\" = " + logo_escaped + ", ";
  }

  const Glib::ustring sql_query = "UPDATE \"" GLOM_STANDARD_TABLE_PREFS_TABLE_NAME "\" SET "
      "\"" GLOM_STANDARD_TABLE_PREFS_FIELD_NAME "\" = '" + prefs.m_name + "', "
      + optional_part_logo +
      "\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET "\" = '" + prefs.m_org_address_street + "', "
      "\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET2 "\" = '" + prefs.m_org_address_street2 + "', "
      "\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_TOWN "\" = '" + prefs.m_org_address_town + "', "
      "\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTY "\" = '" + prefs.m_org_address_county + "', "
      "\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTRY "\" = '" + prefs.m_org_address_country + "', "
      "\"" GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_POSTCODE "\" = '" + prefs.m_org_address_postcode + "'"
      " WHERE \"" GLOM_STANDARD_TABLE_PREFS_FIELD_ID "\" = 1";

    try
    {
      Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute(sql_query);
    }
    catch(const std::exception& ex)
    {
      std::cerr << "Base_DB::set_database_preferences(): exception: " << ex.what() << std::endl;
    }

    //Set some information in the document too, so we can use it to recreate the database:
    get_document()->set_database_title(prefs.m_name);
}

bool Base_DB::create_table(const sharedptr<const TableInfo>& table_info, const Document_Glom::type_vecFields& fields_in) const
{
  //std::cout << "Base_DB::create_table(): " << table_info->get_name() << ", title=" << table_info->get_title() << std::endl;

  bool table_creation_succeeded = false;


  Document_Glom::type_vecFields fields = fields_in;

  //Create the standard field too:
  if(std::find_if(fields.begin(), fields.end(), predicate_FieldHasName<Field>(GLOM_STANDARD_FIELD_LOCK)) == fields.end())
  {
    sharedptr<Field> field = sharedptr<Field>::create();
    field->set_name(GLOM_STANDARD_FIELD_LOCK);
    field->set_glom_type(Field::TYPE_TEXT);
    fields.push_back(field);
  }

  //Create SQL to describe all fields in this table:
  Glib::ustring sql_fields;
  for(Document_Glom::type_vecFields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
  {
    //Create SQL to describe this field:
    sharedptr<Field> field = *iter;

    //The field has no gda type, so we set that:
    //This usually comes from the database, but that's a bit strange.
    Gnome::Gda::FieldAttributes info = field->get_field_info();
    info.set_gdatype( Field::get_gda_type_for_glom_type(field->get_glom_type()) );
    field->set_field_info(info); //TODO_Performance

    Glib::ustring sql_field_description = field->get_name() + " " + field->get_sql_type();

    if(field->get_primary_key())
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
    Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute( "CREATE TABLE \"" + table_info->get_name() + "\" (" + sql_fields + ")" );
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

bool Base_DB::create_table_add_missing_fields(const sharedptr<const TableInfo>& table_info, const Document_Glom::type_vecFields& fields) const
{
  const Glib::ustring table_name = table_info->get_name();

  for(Document_Glom::type_vecFields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
  {
    sharedptr<const Field> field = *iter;
    if(!get_field_exists_in_database(table_name, field->get_name()))
    {
      const bool test = postgres_add_column(table_name, field);
      if(!test)
       return test;
    }
  }

  return true;
}

bool Base_DB::postgres_add_column(const Glib::ustring& table_name, const sharedptr<const Field>& field, bool not_extras) const
{
  sharedptr<Field> field_to_add = glom_sharedptr_clone(field);

  Gnome::Gda::FieldAttributes field_info = field_to_add->get_field_info();
  if((field_info.get_gdatype() == Gnome::Gda::VALUE_TYPE_UNKNOWN) || (field_info.get_gdatype() == Gnome::Gda::VALUE_TYPE_NULL))
  {
    field_info.set_gdatype( Field::get_gda_type_for_glom_type(field_to_add->get_glom_type()) );
    field_to_add->set_field_info(field_info);
  }

  const bool bTest = query_execute(  "ALTER TABLE \"" + table_name + "\" ADD \"" + field_to_add->get_name() + "\" " +  field_to_add->get_sql_type() );
  if(bTest)
  {
    if(not_extras)
    {
      //We must do this separately:
      postgres_change_column_extras(table_name, field_to_add, field_to_add, true /* set them even though the fields are the same */);
    }
  }

  return bTest;
}

void Base_DB::postgres_change_column_extras(const Glib::ustring& table_name, const sharedptr<const Field>& field_old, const sharedptr<const Field>& field, bool set_anyway) const
{
  //Gnome::Gda::FieldAttributes field_info = field->get_field_info();
  //Gnome::Gda::FieldAttributes field_info_old = field_old->get_field_info();

  if(field->get_name() != field_old->get_name())
  {
     Glib::RefPtr<Gnome::Gda::DataModel>  datamodel = query_execute( "ALTER TABLE \"" + table_name + "\" RENAME COLUMN \"" + field_old->get_name() + "\" TO \"" + field->get_name() + "\"" );
     if(!datamodel)
     {
       handle_error();
       return;
     }
  }

  if(set_anyway || (field->get_primary_key() != field_old->get_primary_key()))
  {
    //TODO: Check that there is only one primary key.
    Glib::ustring add_or_drop = "ADD";
    if(field_old->get_primary_key() == false)
      add_or_drop = "DROP";

    Glib::RefPtr<Gnome::Gda::DataModel>  datamodel = query_execute( "ALTER TABLE \"" + table_name + "\" " + add_or_drop + " PRIMARY KEY (\"" + field->get_name() + "\")");
    if(!datamodel)
    {
      handle_error();
      return;
    }
  }

  if( !field->get_primary_key() ) //Postgres automatically makes primary keys unique, so we do not need to do that separately if we have already made it a primary key
  {
    if(set_anyway || (field->get_unique_key() != field_old->get_unique_key()))
    {
       Glib::RefPtr<Gnome::Gda::DataModel> datamodel;
       
       //Postgres needs us to add/drop constraints explicitly when changing existing fields, though it can create them implicitly when creating the field:
       if(field->get_unique_key())
       {
         //Add uniqueness:
         datamodel = query_execute( "ALTER TABLE \"" + table_name + "\" ADD CONSTRAINT \"" + field->get_name() + "_key\" UNIQUE (\"" + field->get_name() + "\")" );

       }
       else
       {
         //Remove uniqueness:
         datamodel = query_execute( "ALTER TABLE \"" + table_name + "\" DROP CONSTRAINT \"" + field->get_name() + "_key\"" );
       }

       if(!datamodel)
       {
         handle_error();
         return;
       }
    }

    Gnome::Gda::Value default_value = field->get_default_value();
    Gnome::Gda::Value default_value_old = field_old->get_default_value();

    if(!field->get_auto_increment()) //Postgres auto-increment fields have special code as their default values.
    {
      if(set_anyway || (default_value != default_value_old))
      {
        Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute( "ALTER TABLE \"" + table_name + "\" ALTER COLUMN \""+ field->get_name() + "\" SET DEFAULT " + field->sql(field->get_default_value()) );
        if(!datamodel)
        {
          handle_error();
          return;
        }
      }
    }
  }

  /* This should have been dealt with by postgres_change_column_type(), because postgres uses a different ("serial") field type for auto-incrementing fields.
  if(field_info.get_auto_increment() != field_info_old.get_auto_increment())
  {

  }
  */
 
   /*
    //If the not-nullness has changed:
    if( set_anyway ||  (field->get_field_info().get_allow_null() != field_old->get_field_info().get_allow_null()) )
    {
      Glib::ustring nullness = (field->get_field_info().get_allow_null() ? "NULL" : "NOT NULL");
      query_execute(  "ALTER TABLE " + m_table_name + " ALTER COLUMN " + field->get_name() + "  SET " + nullness);
    }
  */ 
}


bool Base_DB::insert_example_data(const Glib::ustring& table_name) const
{
  const Glib::ustring example_rows = get_document()->get_table_example_data(table_name);
  if(example_rows.empty())
    return true;

  bool insert_succeeded = false;


  //Get field names:
  Document_Glom::type_vecFields vec_fields = get_document()->get_table_fields(table_name);
  Glib::ustring strNames;
  for(Document_Glom::type_vecFields::const_iterator iter = vec_fields.begin(); iter != vec_fields.end(); ++iter)
  {
    //Append it:
    if(!strNames.empty())
      strNames += ", ";

    strNames += "\"" + (*iter)->get_name() + "\"";
  }

  if(strNames.empty())
  {
    g_warning("Base_DB::insert_example_data(): strNames is empty.");
  }

  //Actually insert the data:
  type_vecStrings vec_rows = string_separate(example_rows, "\n");

  for(type_vecStrings::const_iterator iter = vec_rows.begin(); iter != vec_rows.end(); ++iter)
  {
    try
    {
      const Glib::ustring strQuery = "INSERT INTO \"" + table_name + "\" (" + strNames + ") VALUES (" + *iter + ")";
      query_execute(strQuery); //TODO: Test result.
      insert_succeeded = true;
    }
    catch(const ExceptionConnection& ex)
    {
      insert_succeeded = false;
      break;
    }
  }

  for(Document_Glom::type_vecFields::const_iterator iter = vec_fields.begin(); iter != vec_fields.end(); ++iter)
  {
    if((*iter)->get_auto_increment())
      recalculate_next_auto_increment_value(table_name, (*iter)->get_name());
  }

  return insert_succeeded;
}

sharedptr<LayoutItem_Field> Base_DB::offer_field_list(const Glib::ustring& table_name, Gtk::Window* transient_for)
{
  return offer_field_list(sharedptr<LayoutItem_Field>(), table_name, transient_for);
}

sharedptr<LayoutItem_Field> Base_DB::offer_field_list(const sharedptr<const LayoutItem_Field>& start_field, const Glib::ustring& table_name, Gtk::Window* transient_for)
{
  sharedptr<LayoutItem_Field> result;

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_choose_field");

    Dialog_ChooseField* dialog = 0;
    refXml->get_widget_derived("dialog_choose_field", dialog);

    if(dialog)
    {
      if(transient_for)
        dialog->set_transient_for(*transient_for);

      dialog->set_document(get_document(), table_name, start_field);
      //TODO: dialog->set_transient_for(*get_app_window());
      const int response = dialog->run();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen field:
        result = dialog->get_field_chosen();
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

sharedptr<LayoutItem_Field> Base_DB::offer_field_formatting(const sharedptr<const LayoutItem_Field>& start_field, const Glib::ustring& table_name, Gtk::Window* transient_for)
{
  sharedptr<LayoutItem_Field> result;

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_layout_field_properties");

    Dialog_FieldLayout* dialog = 0;
    refXml->get_widget_derived("dialog_layout_field_properties", dialog);

    if(dialog)
    {
      if(transient_for)
        dialog->set_transient_for(*transient_for);

      add_view(dialog);

      dialog->set_field(start_field, table_name);


      const int response = dialog->run();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen field:
        result = dialog->get_field_chosen();
      }

      remove_view(dialog);
      delete dialog;
    }
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  return result;
}

sharedptr<LayoutItem_Text> Base_DB::offer_textobject(const sharedptr<LayoutItem_Text>& start_textobject, Gtk::Window* transient_for)
{
  sharedptr<LayoutItem_Text> result;

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_textobject");

    Dialog_TextObject* dialog = 0;
    refXml->get_widget_derived("window_textobject", dialog);
    if(dialog)
    {
      if(transient_for)
        dialog->set_transient_for(*transient_for);

      dialog->set_textobject(start_textobject, Glib::ustring());
      const int response = dialog->run();
      dialog->hide();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen relationship:
        result = dialog->get_textobject();
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

sharedptr<LayoutItem_Image> Base_DB::offer_imageobject(const sharedptr<LayoutItem_Image>& start_imageobject, Gtk::Window* transient_for)
{
  sharedptr<LayoutItem_Image> result;

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_imageobject");

    Dialog_ImageObject* dialog = 0;
    refXml->get_widget_derived("window_imageobject", dialog);
    if(dialog)
    {
      if(transient_for)
        dialog->set_transient_for(*transient_for);

      dialog->set_imageobject(start_imageobject, Glib::ustring());
      const int response = dialog->run();
      dialog->hide();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen relationship:
        result = dialog->get_imageobject();
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

sharedptr<LayoutItem_Notebook> Base_DB::offer_notebook(const sharedptr<LayoutItem_Notebook>& start_notebook, Gtk::Window* transient_for)
{
  sharedptr<LayoutItem_Notebook> result;

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_notebook");

    Dialog_Notebook* dialog = 0;
    refXml->get_widget_derived("dialog_notebook", dialog);
    if(dialog)
    {
      if(transient_for)
        dialog->set_transient_for(*transient_for);

      dialog->set_notebook(start_notebook);
      //dialog->set_transient_for(*this);
      const int response = dialog->run();
      dialog->hide();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen relationship:
        result = dialog->get_notebook();
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

void Base_DB::fill_full_field_details(const Glib::ustring& parent_table_name, sharedptr<LayoutItem_Field>& layout_item)
{
  const Glib::ustring table_name = layout_item->get_table_used(parent_table_name);

  layout_item->set_full_field_details( get_document()->get_field(table_name, layout_item->get_name()) );
}

void Base_DB::report_build_headerfooter(const FoundSet& found_set, xmlpp::Element& parent_node, const sharedptr<LayoutGroup>& group)
{
  //Add XML node:
  xmlpp::Element* node = parent_node.add_child(group->get_report_part_id());

  //Add child parts:
  type_vecLayoutItems itemsToGet;
  for(LayoutGroup::type_map_items::iterator iterChildren = group->m_map_items.begin(); iterChildren != group->m_map_items.end(); ++iterChildren)
  {
    sharedptr<LayoutItem> item = iterChildren->second;

    sharedptr<LayoutItem_Text> item_text = sharedptr<LayoutItem_Text>::cast_dynamic(item);
    if(item_text)
    {
      report_build_records_text(found_set, *node, item_text);
    }
    else
    {
      sharedptr<LayoutItem_Image> item_image = sharedptr<LayoutItem_Image>::cast_dynamic(item);
      if(item_image)
      {
        report_build_records_image(found_set, *node, item_image);
      }
      else
      {
        sharedptr<LayoutItem_Field> pField = sharedptr<LayoutItem_Field>::cast_dynamic(item);
        if(pField)
        {
          guint col_index = 0; //ignored.
          report_build_records_field(found_set, *node, pField, Glib::RefPtr<Gnome::Gda::DataModel>(), 0, col_index);
        }
        else
        {
          sharedptr<LayoutItem_VerticalGroup> vertical_group = sharedptr<LayoutItem_VerticalGroup>::cast_dynamic(item);
          if(vertical_group)
          {
            //Reuse (a bit hacky) this function for the header and footer:
            guint col_index = 0; //Ignored, because the model is null.
            report_build_records_vertical_group(found_set, *node, vertical_group, Glib::RefPtr<Gnome::Gda::DataModel>(), 0, col_index);
          }
        }
      }
    }
  }

}

void Base_DB::report_build_summary(const FoundSet& found_set, xmlpp::Element& parent_node, const sharedptr<LayoutItem_Summary>& summary)
{
  //Add XML node:
  xmlpp::Element* node = parent_node.add_child(summary->get_report_part_id());

  //Get fields
  type_vecLayoutItems itemsToGet;
  for(LayoutGroup::type_map_items::iterator iterChildren = summary->m_map_items.begin(); iterChildren != summary->m_map_items.end(); ++iterChildren)
  {
    sharedptr<LayoutItem> item = iterChildren->second;

    sharedptr<LayoutItem_GroupBy> pGroupBy = sharedptr<LayoutItem_GroupBy>::cast_dynamic(item);
    if(pGroupBy)
    {
      //Recurse, adding a sub-groupby block:
      report_build_groupby(found_set, *node, pGroupBy);
    }
    else
    {
      sharedptr<LayoutItem_Summary> pSummary = sharedptr<LayoutItem_Summary>::cast_dynamic(item);
      if(pSummary)
      {
        //Recurse, adding a summary block:
        report_build_summary(found_set, *node, pSummary);
      }
      else
      {
        itemsToGet.push_back( glom_sharedptr_clone(item) );
      }
    }
  }

  if(!itemsToGet.empty())
  {
    //Rows, with data:
    report_build_records(found_set, *node, itemsToGet);
  }
}



void Base_DB::report_build_groupby_children(const FoundSet& found_set, xmlpp::Element& node, const sharedptr<LayoutItem_GroupBy>& group_by)
{
  //Get data and add child rows:
  type_vecLayoutItems itemsToGet;
  for(LayoutGroup::type_map_items::iterator iterChildren = group_by->m_map_items.begin(); iterChildren != group_by->m_map_items.end(); ++iterChildren)
  {
    sharedptr<LayoutItem> item = iterChildren->second;

    sharedptr<LayoutItem_GroupBy> pGroupBy = sharedptr<LayoutItem_GroupBy>::cast_dynamic(item);
    if(pGroupBy)
    {
      //Recurse, adding a sub-groupby block:
      report_build_groupby(found_set, node, pGroupBy);
    }
    else
    {
      sharedptr<LayoutItem_Summary> pSummary = sharedptr<LayoutItem_Summary>::cast_dynamic(item);
      if(pSummary)
      {
        //Recurse, adding a summary block:
        report_build_summary(found_set, node, pSummary);
      }
      else
      {
        itemsToGet.push_back( glom_sharedptr_clone(item) );
      }
    }
  }

  if(!itemsToGet.empty())
  {
    //Rows, with data:
    FoundSet found_set_records = found_set;
    found_set_records.m_sort_clause = group_by->get_fields_sort_by();
    report_build_records(found_set_records, node, itemsToGet);
  }
}

void Base_DB::report_build_groupby(const FoundSet& found_set_parent, xmlpp::Element& parent_node, const sharedptr<LayoutItem_GroupBy>& group_by)
{
  //Get the possible heading values.
  if(group_by->get_has_field_group_by())
  {
    sharedptr<LayoutItem_Field> field_group_by = group_by->get_field_group_by();
    fill_full_field_details(found_set_parent.m_table_name, field_group_by);

    //Get the possible group values, ignoring repeats by using GROUP BY.
    const Glib::ustring group_field_table_name = field_group_by->get_table_used(found_set_parent.m_table_name);
    Glib::ustring sql_query = "SELECT \"" + group_field_table_name + "\".\"" + field_group_by->get_name() + "\""
      " FROM \"" + group_field_table_name + "\"";

    if(!found_set_parent.m_where_clause.empty())
      sql_query += " WHERE " + found_set_parent.m_where_clause;

    sql_query += " GROUP BY " + field_group_by->get_name(); //rTODO: And restrict to the current found set.

    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute(sql_query);
    if(datamodel)
    {
      guint rows_count = datamodel->get_n_rows();
      for(guint row = 0; row < rows_count; ++row)
      {
        const Gnome::Gda::Value group_value = datamodel->get_value_at(0 /* col*/, row);

        //Add XML node:
        xmlpp::Element* nodeGroupBy = parent_node.add_child(group_by->get_report_part_id());
        Document_Glom::set_node_attribute_value_as_decimal_double(nodeGroupBy, "border_width", group_by->get_border_width());

        nodeGroupBy->set_attribute("group_field", field_group_by->get_title_or_name());
        nodeGroupBy->set_attribute("group_value",
          GlomConversions::get_text_for_gda_value(field_group_by->get_glom_type(), group_value, field_group_by->get_formatting_used().m_numeric_format) );

        Glib::ustring where_clause = "\"" + group_field_table_name + "\".\"" + field_group_by->get_name() + "\" = " + field_group_by->get_full_field_details()->sql(group_value);
        if(!found_set_parent.m_where_clause.empty())
          where_clause += " AND (" + found_set_parent.m_where_clause + ")";

        FoundSet found_set_records = found_set_parent;
        found_set_records.m_where_clause = where_clause;

        //Secondary fields. For instance, the Contact Name, in addition to the Contact ID that we group by.
        if(!(group_by->m_group_secondary_fields->m_map_items.empty()))
        {
          xmlpp::Element* nodeSecondaryFields = nodeGroupBy->add_child("secondary_fields");

          type_vecLayoutItems itemsToGet;
          for(LayoutGroup::type_map_items::iterator iterChildren = group_by->m_group_secondary_fields->m_map_items.begin(); iterChildren != group_by->m_group_secondary_fields->m_map_items.end(); ++iterChildren)
          {
            sharedptr<LayoutItem> item = iterChildren->second;
            itemsToGet.push_back( glom_sharedptr_clone(item) );
          }

          if(!itemsToGet.empty())
          {
            report_build_records(found_set_records, *nodeSecondaryFields, itemsToGet, true /* one record only */);
          }
        }

        //Get data and add child rows:
        report_build_groupby_children(found_set_records, *nodeGroupBy, group_by);
      }
    }
  }
  else
  {
    //There is no group-by field, so ouput all the found records.
    //For instance, the user could use the GroupBy part just to specify a sort, though that would be a bit of a hack:
    xmlpp::Element* nodeGroupBy = parent_node.add_child(group_by->get_report_part_id()); //We need this to create the HTML table.
    Document_Glom::set_node_attribute_value_as_decimal_double(nodeGroupBy, "border_width", group_by->get_border_width());
    report_build_groupby_children(found_set_parent, *nodeGroupBy, group_by);
  }
}

void Base_DB::report_build_records_get_fields(const FoundSet& found_set, const sharedptr<LayoutGroup>& group, type_vecLayoutFields& items)
{
  for(LayoutGroup::type_map_items::iterator iterChildren = group->m_map_items.begin(); iterChildren != group->m_map_items.end(); ++iterChildren)
  {
    sharedptr<LayoutItem> item = iterChildren->second;

    sharedptr<LayoutItem_VerticalGroup> pVerticalGroup = sharedptr<LayoutItem_VerticalGroup>::cast_dynamic(item);
    if(pVerticalGroup)
    {
      report_build_records_get_fields(found_set, pVerticalGroup, items);
    }
    else
    {
      sharedptr<LayoutItem_Field> pField = sharedptr<LayoutItem_Field>::cast_dynamic(item);
      if(pField)
        items.push_back(pField);
    }
  }
}

void Base_DB::report_build_records(const FoundSet& found_set, xmlpp::Element& parent_node, const type_vecLayoutItems& items, bool one_record_only)
{
  if(!items.empty())
  {
    //Add Field headings:
    for(type_vecLayoutItems::const_iterator iter = items.begin(); iter != items.end(); ++iter)
    {
      sharedptr<LayoutItem> layout_item = *iter;
      sharedptr<LayoutItem_Field> layoutitem_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);

      //This adds a field heading (and therefore, column) for fields, or for a vertical group. 
      xmlpp::Element* nodeFieldHeading = parent_node.add_child("field_heading");
      if(layoutitem_field && layoutitem_field->get_glom_type() == Field::TYPE_NUMERIC)
        nodeFieldHeading->set_attribute("field_type", "numeric"); //TODO: More sophisticated formatting.

      nodeFieldHeading->set_attribute("name", layout_item->get_name()); //Not really necessary, but maybe useful.
      nodeFieldHeading->set_attribute("title", layout_item->get_title_or_name());
    }

    //Get list of fields to get from the database.
    GlomUtils::type_vecLayoutFields fieldsToGet;
    for(type_vecLayoutItems::const_iterator iter = items.begin(); iter != items.end(); ++iter)
    {
      sharedptr<LayoutItem> layout_item = *iter;
      sharedptr<LayoutItem_Field> layoutitem_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
      if(layoutitem_field)
        fieldsToGet.push_back(layoutitem_field);
      else
      {
        sharedptr<LayoutItem_VerticalGroup> vertical_group = sharedptr<LayoutItem_VerticalGroup>::cast_dynamic(layout_item);
        if(vertical_group)
        {
          //Get all the fields in this group:
          report_build_records_get_fields(found_set, vertical_group, fieldsToGet);
        }
      }
    }

    Glib::ustring sql_query = GlomUtils::build_sql_select_with_where_clause(found_set.m_table_name,
      fieldsToGet,
      found_set.m_where_clause, found_set.m_sort_clause);

    //For instance, when we just want to get a name corresponding to a contact ID, and want to ignore duplicates.
    if(one_record_only)
      sql_query += "LIMIT 1";

    bool records_found = false;
    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute(sql_query);
    if(datamodel)
    {
      guint rows_count = datamodel->get_n_rows();
      if(rows_count > 0)
        records_found = true;

      for(guint row = 0; row < rows_count; ++row)
      {
        xmlpp::Element* nodeRow = parent_node.add_child("row");

        guint colField = 0;
        for(type_vecLayoutItems::const_iterator iter = items.begin(); iter != items.end(); ++iter)
        {
          xmlpp::Element* nodeField = 0;

          sharedptr<LayoutItem> item = *iter;
          sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(item);
          if(field)
          {
            report_build_records_field(found_set, *nodeRow, field, datamodel, row, colField);
          }
          else
          {
            sharedptr<LayoutItem_Text> item_text = sharedptr<LayoutItem_Text>::cast_dynamic(item);
            if(item_text)
            {
              report_build_records_text(found_set, *nodeRow, item_text);
            }
            else
            {
              sharedptr<LayoutItem_VerticalGroup> item_verticalgroup = sharedptr<LayoutItem_VerticalGroup>::cast_dynamic(item);
              if(item_verticalgroup)
              {
                report_build_records_vertical_group(found_set, *nodeRow, item_verticalgroup, datamodel, row, colField);
                //TODO
              }
            }
          }

          if(nodeField)
            nodeField->set_attribute("name", item->get_name()); //Not really necessary, but maybe useful.
        }
      }

    }

    //If there are no records, show zero
    //if(!rows_found && show_null_row)
  }
}

void Base_DB::report_build_records_field(const FoundSet& found_set, xmlpp::Element& nodeParent, const sharedptr<const LayoutItem_Field>& field, const Glib::RefPtr<Gnome::Gda::DataModel>& datamodel, guint row, guint& colField, bool vertical)
{
  const Field::glom_field_type field_type = field->get_glom_type();

  xmlpp::Element* nodeField = nodeParent.add_child(field->get_report_part_id());
  if(field_type == Field::TYPE_NUMERIC)
    nodeField->set_attribute("field_type", "numeric"); //TODO: More sophisticated formatting.

  if(vertical)
    nodeField->set_attribute("vertical", "true");

  Gnome::Gda::Value value;
  Glib::ustring text_value;

  if(!datamodel) //We call this for headers and footers too.
  {
    //In this case it can only be a system preferences field.
    //So let's get that data here:
    const Glib::ustring table_used = field->get_table_used(found_set.m_table_name);
    const Glib::ustring query = "SELECT \"" + table_used + "\".\"" + field->get_name() + "\" FROM \""+ table_used + "\" LIMIT 1";
    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute(query);

    if(!datamodel)
      return;

    value = datamodel->get_value_at(colField, row);

    colField = 0;
    row = 0;
  }
  else
  {
    value = datamodel->get_value_at(colField, row);
  }

  nodeField->set_attribute("title", field->get_title_or_name()); //Not always used, but useful.

  //Handle the value:
  if(field_type == Field::TYPE_IMAGE)
     nodeField->set_attribute("image_uri", GlomUtils::create_local_image_uri(value));
  else
  {
    Glib::ustring text_value = GlomConversions::get_text_for_gda_value(field_type, value, field->get_formatting_used().m_numeric_format);

    //The Postgres summary functions return NULL when summarising NULL records, but 0 is more sensible:
    if(text_value.empty() && sharedptr<const LayoutItem_FieldSummary>::cast_dynamic(field) && (field_type == Field::TYPE_NUMERIC))
    {
      //Use get_text_for_gda_value() instead of "0" so we get the correct numerical formatting:
      Gnome::Gda::Value value = GlomConversions::parse_value(0);
      text_value = GlomConversions::get_text_for_gda_value(field_type, value, field->get_formatting_used().m_numeric_format);
    }

    nodeField->set_attribute("value", text_value);
  }

  ++colField;
}

void Base_DB::report_build_records_text(const FoundSet& found_set, xmlpp::Element& nodeParent, const sharedptr<const LayoutItem_Text>& textobject, bool vertical)
{
  //Text object:
  xmlpp::Element* nodeField = nodeParent.add_child(textobject->get_report_part_id()); //We reuse this node type for text objects.
  nodeField->set_attribute("value", textobject->get_text());

  if(vertical)
    nodeField->set_attribute("vertical", "true");
}

void Base_DB::report_build_records_image(const FoundSet& found_set, xmlpp::Element& nodeParent, const sharedptr<const LayoutItem_Image>& imageobject, bool vertical)
{
  //Text object:
  xmlpp::Element* nodeImage = nodeParent.add_child(imageobject->get_report_part_id()); //We reuse this node type for text objects.
  nodeImage->set_attribute("image_uri", imageobject->create_local_image_uri());

  if(vertical)
    nodeImage->set_attribute("vertical", "true");
}

void Base_DB::report_build_records_vertical_group(const FoundSet& found_set, xmlpp::Element& parentNode, const sharedptr<LayoutItem_VerticalGroup>& group, const Glib::RefPtr<Gnome::Gda::DataModel>& datamodel, guint row, guint& field_index)
{
  xmlpp::Element* nodeGroupVertical = parentNode.add_child(group->get_report_part_id());

  for(LayoutGroup::type_map_items::iterator iterChildren = group->m_map_items.begin(); iterChildren != group->m_map_items.end(); ++iterChildren)
  {
    sharedptr<LayoutItem> item = iterChildren->second;

    sharedptr<LayoutItem_VerticalGroup> pVerticalGroup = sharedptr<LayoutItem_VerticalGroup>::cast_dynamic(item);
    if(pVerticalGroup)
    {
      report_build_records_vertical_group(found_set, *nodeGroupVertical, pVerticalGroup, datamodel, row, field_index);
    }
    else
    {
      sharedptr<LayoutItem_Field> pField = sharedptr<LayoutItem_Field>::cast_dynamic(item);
      if(pField)
      {
        report_build_records_field(found_set, *nodeGroupVertical, pField, datamodel, row, field_index, true /* vertical, so we get a row for each field too. */);
      }
      else
      {
        sharedptr<LayoutItem_Text> pText = sharedptr<LayoutItem_Text>::cast_dynamic(item);
        if(pText)
        {
          report_build_records_text(found_set, *nodeGroupVertical, pText, true);
        }
      }
    }
  }
}


void Base_DB::report_build(const FoundSet& found_set, const sharedptr<const Report>& report, Gtk::Window* parent_window)
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

  Glib::ustring table_title = get_document()->get_table_title(found_set.m_table_name);
  if(table_title.empty())
    table_title = found_set.m_table_name;

  nodeRoot->set_attribute("table", table_title);
  if(report->get_show_table_title())
    nodeRoot->set_attribute("show_table_title", "true");


  //The groups:
  xmlpp::Element* nodeParent = nodeRoot;


  nodeRoot->set_attribute("title", report->get_title_or_name());

  type_vecLayoutItems itemsToGet_TopLevel;

  for(LayoutGroup::type_map_items::const_iterator iter = report->m_layout_group->m_map_items.begin(); iter != report->m_layout_group->m_map_items.end(); ++iter)
  {
    sharedptr<LayoutItem> pPart = iter->second;

    //The Group, and the details for each record in the group:
    sharedptr<LayoutItem_GroupBy> pGroupBy = sharedptr<LayoutItem_GroupBy>::cast_dynamic(pPart);
    if(pGroupBy)
      report_build_groupby(found_set, *nodeParent, pGroupBy);
    else
    {
      sharedptr<LayoutItem_Summary> pSummary = sharedptr<LayoutItem_Summary>::cast_dynamic(pPart);
      if(pSummary)
      {
        //Recurse, adding a summary block:
        report_build_summary(found_set, *nodeParent, pSummary);
      }
      else
      {
        sharedptr<LayoutGroup> pGroup = sharedptr<LayoutGroup>::cast_dynamic(pPart);
        if(pGroup)
        {
          sharedptr<LayoutItem_Header> pHeader = sharedptr<LayoutItem_Header>::cast_dynamic(pPart);
          sharedptr<LayoutItem_Footer> pFooter = sharedptr<LayoutItem_Footer>::cast_dynamic(pPart);
          if(pHeader || pFooter)
          {
            //Recurse, adding a summary block:
            report_build_headerfooter(found_set, *nodeParent, pGroup);
          }
        }
        else
          itemsToGet_TopLevel.push_back( glom_sharedptr_clone(pPart) );
      }
    }
  }

  //Add top-level records, outside of any groupby or summary, if fields have been specified:
  if(!itemsToGet_TopLevel.empty())
  {
    xmlpp::Element* nodeGroupBy = nodeParent->add_child("ungrouped_records");
    report_build_records(found_set, *nodeGroupBy, itemsToGet_TopLevel);
  }

  GlomXslUtils::transform_and_open(*pDocument, "print_report_to_html.xsl", parent_window);
}

//static
bool Base_DB::show_warning_no_records_found(Gtk::Window& transient_for)
{
  Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("No Records Found")), true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE);
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(_("New Find"), Gtk::RESPONSE_OK);
  dialog.set_secondary_text(_("Your find criteria did not match any records in the table."));
  dialog.set_transient_for(transient_for);

  const bool find_again = (dialog.run() == Gtk::RESPONSE_OK);
  return find_again;
}

Glib::ustring Base_DB::get_find_where_clause_quick(const Glib::ustring& table_name, const Gnome::Gda::Value& quick_search) const
{
  Glib::ustring strClause;

  const Document_Glom* document = get_document();
  if(document)
  {
    //TODO: Cache the list of all fields, as well as caching (m_Fields) the list of all visible fields:
    const Document_Glom::type_vecFields fields = document->get_table_fields(table_name);

    type_vecLayoutFields fieldsToGet;
    for(Document_Glom::type_vecFields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      Glib::ustring strClausePart;

      sharedptr<const Field> field = *iter;

      bool use_this_field = true;
      if(field->get_glom_type() != Field::TYPE_TEXT)
      {
          use_this_field = false;
      }

      if(use_this_field)
      {
        strClausePart = table_name + "." + field->get_name() + " " + field->sql_find_operator() + " " +  field->sql_find(quick_search);
      }

      if(!strClausePart.empty())
      {
        if(!strClause.empty())
          strClause += " OR ";

        strClause += strClausePart;
      }
    }
  }

  return strClause;
}

sharedptr<Field> Base_DB::get_fields_for_table_one_field(const Glib::ustring& table_name, const Glib::ustring& field_name) const
{
  //Initialize output parameter:
  sharedptr<Field> result;

  if(field_name.empty() || table_name.empty())
    return result;

  type_vecFields fields = get_fields_for_table(table_name);
  type_vecFields::iterator iter = std::find_if(fields.begin(), fields.end(), predicate_FieldHasName<Field>(field_name));
  if(iter != fields.end()) //TODO: Handle error?
  {
    return *iter;
  }

  return sharedptr<Field>();
}

//static:
bool Base_DB::get_field_primary_key_index_for_fields(const type_vecFields& fields, guint& field_column)
{
  //Initialize input parameter:
  field_column = 0;

  //TODO_performance: Cache the primary key?
  guint col = 0;
  guint cols_count = fields.size();
  while(col < cols_count)
  {
    if(fields[col]->get_primary_key())
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

//static:
bool Base_DB::get_field_primary_key_index_for_fields(const type_vecLayoutFields& fields, guint& field_column)
{
  //Initialize input parameter:
  field_column = 0;

  //TODO_performance: Cache the primary key?
  guint col = 0;
  guint cols_count = fields.size();
  while(col < cols_count)
  {
    if(fields[col]->get_full_field_details()->get_primary_key())
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

sharedptr<Field> Base_DB::get_field_primary_key_for_table(const Glib::ustring& table_name) const
{
  const Document_Glom* document = get_document();
  if(document)
  {
    //TODO_Performance:
    Document_Glom::type_vecFields fields = document->get_table_fields(table_name);
    for(Document_Glom::type_vecFields::iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      if((*iter)->get_primary_key())
      {
        return *iter;
      }
    }
  }

  return sharedptr<Field>();
}

void Base_DB::get_table_fields_to_show_for_sequence_add_group(const Glib::ustring& table_name, const Privileges& table_privs, const type_vecFields& all_db_fields, const sharedptr<const LayoutGroup>& group, Base_DB::type_vecLayoutFields& vecFields) const
{
  //g_warning("Box_Data::get_table_fields_to_show_for_sequence_add_group(): table_name=%s, all_db_fields.size()=%d, group->name=%s", table_name.c_str(), all_db_fields.size(), group->get_name().c_str());

  LayoutGroup::type_map_const_items items = group->get_items();
  for(LayoutGroup::type_map_const_items::const_iterator iterItems = items.begin(); iterItems != items.end(); ++iterItems)
  {
    sharedptr<const LayoutItem> item = iterItems->second;

    sharedptr<const LayoutItem_Field> item_field = sharedptr<const LayoutItem_Field>::cast_dynamic(item);
    if(item_field)
    {
      //Get the field info:
      const Glib::ustring field_name = item->get_name();

      if(item_field->get_has_relationship_name()) //If it's a field in a related table.
      {
        //TODO_Performance: get_fields_for_table_one_field() is probably very inefficient
        sharedptr<Field> field = get_fields_for_table_one_field(item_field->get_table_used(table_name), item->get_name());
        if(field)
        {
          sharedptr<LayoutItem_Field> layout_item = glom_sharedptr_clone(item_field); //TODO_Performance: Reduce the copying.
          layout_item->set_full_field_details(field); //Fill in the full field information for later.


          //TODO_Performance: We do this once for each related field, even if there are 2 from the same table:
          const Privileges privs_related = get_current_privs(item_field->get_table_used(table_name));
          layout_item->m_priv_view = privs_related.m_view;
          layout_item->m_priv_edit = privs_related.m_edit;

          vecFields.push_back(layout_item);
        }
        else
        {
          std::cerr << "Base_DB::get_table_fields_to_show_for_sequence_add_group(): related field not found: field=" << item->get_layout_display_name() << std::endl;
        }
      }
      else //It's a regular field in the table:
      {
        type_vecFields::const_iterator iterFind = std::find_if(all_db_fields.begin(), all_db_fields.end(), predicate_FieldHasName<Field>(field_name));

        //If the field does not exist anymore then we won't try to show it:
        if(iterFind != all_db_fields.end() )
        {
          sharedptr<LayoutItem_Field> layout_item = glom_sharedptr_clone(item_field); //TODO_Performance: Reduce the copying here.
          layout_item->set_full_field_details(*iterFind); //Fill the LayoutItem with the full field information.

          //std::cout << "get_table_fields_to_show_for_sequence_add_group(): name=" << layout_item->get_name() << std::endl;

          //Prevent editing of the field if the user may not edit this table:
          layout_item->m_priv_view = table_privs.m_view;
          layout_item->m_priv_edit = table_privs.m_edit;

          vecFields.push_back(layout_item);
        }
      }
    }
    else
    {
      sharedptr<const LayoutGroup> item_group = sharedptr<const LayoutGroup>::cast_dynamic(item);
      if(item_group)
      {
        sharedptr<const LayoutItem_Portal> item_portal = sharedptr<const LayoutItem_Portal>::cast_dynamic(item);
        if(!item_portal) //Do not recurse into portals. They are filled by means of a separate SQL query.
        {
          //Recurse:
          get_table_fields_to_show_for_sequence_add_group(table_name, table_privs, all_db_fields, item_group, vecFields);
        }
      }
    }
  }

  if(vecFields.empty())
  {
    //g_warning("Box_Data::get_table_fields_to_show_for_sequence_add_group(): Returning empty list.");
  }
}

Base_DB::type_vecLayoutFields Base_DB::get_table_fields_to_show_for_sequence(const Glib::ustring& table_name, const Document_Glom::type_mapLayoutGroupSequence& mapGroupSequence) const
{
  //Get field definitions from the database, with corrections from the document:
  type_vecFields all_fields = get_fields_for_table(table_name);

  const Privileges table_privs = get_current_privs(table_name);

  //Get fields that the document says we should show:
  type_vecLayoutFields result;
  const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
  if(pDoc)
  {
    if(mapGroupSequence.empty())
    {
      //No field sequence has been saved in the document, so we use all fields by default, so we start with something visible:

      //Start with the Primary Key as the first field:
      guint iPrimaryKey = 0;
      bool bPrimaryKeyFound = get_field_primary_key_index_for_fields(all_fields, iPrimaryKey);
      Glib::ustring primary_key_field_name;
      if(bPrimaryKeyFound)
      {
        sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
        layout_item->set_full_field_details(all_fields[iPrimaryKey]);

        //Don't use thousands separators with ID numbers:
        layout_item->m_formatting.m_numeric_format.m_use_thousands_separator = false;

        layout_item->set_editable(true); //A sensible default.

        //Prevent editing of the field if the user may not edit this table:
        layout_item->m_priv_view = table_privs.m_view;
        layout_item->m_priv_edit = table_privs.m_edit;

        result.push_back(layout_item);
      }

      //Add the rest:
      for(type_vecFields::const_iterator iter = all_fields.begin(); iter != all_fields.end(); ++iter)
      {
        sharedptr<Field> field_info = *iter;

        if((*iter)->get_name() != primary_key_field_name) //We already added the primary key.
        {
          sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
          layout_item->set_full_field_details(field_info);

          layout_item->set_editable(true); //A sensible default.

          //Prevent editing of the field if the user may not edit this table:
          layout_item->m_priv_view = table_privs.m_view;
          layout_item->m_priv_edit = table_privs.m_edit;

          result.push_back(layout_item);
        }
      }
    }
    else
    {
      type_vecFields vecFieldsInDocument = pDoc->get_table_fields(table_name);

      //We will show the fields that the document says we should:
      for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = mapGroupSequence.begin(); iter != mapGroupSequence.end(); ++iter)
      {
        sharedptr<const LayoutGroup> group = iter->second;

        if(true) //!group->m_hidden)
        {
          //Get the fields:
          get_table_fields_to_show_for_sequence_add_group(table_name, table_privs, all_fields, group, result);
        }
      }
    }
  }

  if(result.empty())
  {
    //g_warning("Box_Data::get_table_fields_to_show_for_sequence_add_group(): Returning empty list.");
  }

  return result;
}

void Base_DB::calculate_field_in_all_records(const Glib::ustring& table_name, const sharedptr<const Field>& field)
{
  sharedptr<const Field> primary_key = get_field_primary_key_for_table(table_name);
  calculate_field_in_all_records(table_name, field, primary_key);
}

void Base_DB::calculate_field_in_all_records(const Glib::ustring& table_name, const sharedptr<const Field>& field, const sharedptr<const Field>& primary_key)
{

  //Get primary key values for every record:
  const Glib::ustring query = "SELECT \"" + table_name + "\".\"" + primary_key->get_name() + "\" FROM \"" + table_name + "\"";
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(query);
  if(!data_model || !data_model->get_n_rows() || !data_model->get_n_columns())
  {
    //HandleError();
    return;
  }

  FieldInRecord field_in_record;
  field_in_record.m_table_name = table_name;
  field_in_record.m_field = field;
  field_in_record.m_key = primary_key;

  //Calculate the value for the field in every record:
  const int rows_count = data_model->get_n_rows();
  for(int row = 0; row < rows_count; ++row)
  {
    const Gnome::Gda::Value primary_key_value = data_model->get_value_at(0, row);
    if(!GlomConversions::value_is_empty(primary_key_value))
    {
      field_in_record.m_key_value = primary_key_value;

      m_FieldsCalculationInProgress.clear();
      calculate_field(field_in_record);
    }
  }
}

void Base_DB::calculate_field(const FieldInRecord& field_in_record)
{
  const Glib::ustring field_name = field_in_record.m_field->get_name();
  //g_warning("Box_Data::calculate_field(): field_name=%s", field_name.c_str());

  //Do we already have this in our list?
  type_field_calcs::iterator iterFind = m_FieldsCalculationInProgress.find(field_name);
  if(iterFind == m_FieldsCalculationInProgress.end()) //If it was not found.
  {
    //Add it:
    CalcInProgress item;
    item.m_field = field_in_record.m_field;
    m_FieldsCalculationInProgress[field_name] = item;

    iterFind = m_FieldsCalculationInProgress.find(field_name); //Always succeeds.
  }

  CalcInProgress& refCalcProgress = iterFind->second;

  //Use the previously-calculated value if possible:
  if(refCalcProgress.m_calc_in_progress)
  {
    //g_warning("  Box_Data::calculate_field(): Circular calculation detected. field_name=%s", field_name.c_str());
    //refCalcProgress.m_value = GlomConversions::get_empty_value(field->get_glom_type()); //Give up.
  }
  else if(refCalcProgress.m_calc_finished)
  {
    //g_warning("  Box_Data::calculate_field(): Already calculated.");

    //Don't bother calculating it again. The correct value is already in the database and layout.
  }
  else
  {
    //g_warning("  Box_Data::calculate_field(): setting calc_in_progress: field_name=%s", field_name.c_str());

    refCalcProgress.m_calc_in_progress = true; //Let the recursive calls to calculate_field() check this.

    sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
    layout_item->set_full_field_details(refCalcProgress.m_field);

    //Calculate dependencies first:
    //TODO: Prevent unncessary recalculations?
    const type_list_field_items fields_needed = get_calculation_fields(field_in_record.m_table_name, field_in_record.m_field);
    for(type_list_field_items::const_iterator iterNeeded = fields_needed.begin(); iterNeeded != fields_needed.end(); ++iterNeeded)
    {
      sharedptr<const LayoutItem_Field> field_item_needed = *iterNeeded;

      if(field_item_needed->get_has_relationship_name())
      {
        //TOOD: Handle related fields? We already handle whole relationships.
      }
      else
      {
        sharedptr<const Field> field_needed = field_item_needed->get_full_field_details();
        if(field_needed)
        {
          if(field_needed->get_has_calculation())
          {
            //g_warning("  calling calculate_field() for %s", iterNeeded->c_str());
            //TODO: What if the field is in a different table?

            FieldInRecord needed_field_in_record(field_in_record.m_table_name, field_needed, field_in_record.m_key, field_in_record.m_key_value);
            calculate_field(needed_field_in_record);
          }
          else
          {
            //g_warning("  not a calculated field->");
          }
        }
      }
    }


    //m_FieldsCalculationInProgress has changed, probably invalidating our iter, so get it again:
    iterFind = m_FieldsCalculationInProgress.find(field_name); //Always succeeds.
    CalcInProgress& refCalcProgress = iterFind->second;

    //Check again, because the value miight have been calculated during the dependencies.
    if(refCalcProgress.m_calc_finished)
    {
      //We recently calculated this value, and set it in the database and layout, so don't waste time doing it again:
    }
    else
    {
      //recalculate:
      //TODO_Performance: We don't know what fields the python calculation will use, so we give it all of them:
      const type_map_fields field_values = get_record_field_values(field_in_record.m_table_name, field_in_record.m_key, field_in_record.m_key_value);
      if(!field_values.empty())
      {
        sharedptr<const Field> field = refCalcProgress.m_field;
        if(field)
        {
          refCalcProgress.m_value = glom_evaluate_python_function_implementation(field->get_glom_type(), field->get_calculation(), field_values, get_document(), field_in_record.m_table_name);

          refCalcProgress.m_calc_finished = true;
          refCalcProgress.m_calc_in_progress = false;

          sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
          layout_item->set_full_field_details(field);

          //show it:
          set_entered_field_data(layout_item, refCalcProgress.m_value ); //TODO: If this record is shown.

          //Add it to the database (even if it is not shown in the view)
          //Using true for the last parameter means we use existing calculations where possible,
          //instead of recalculating a field that is being calculated already, and for which this dependent field is being calculated anyway.
          Document_Glom* document = get_document();
          if(document)
          {
            FieldInRecord field_in_record_layout(layout_item, field_in_record.m_table_name /* parent */, field_in_record.m_key, field_in_record.m_key_value, *document);

            set_field_value_in_database(field_in_record_layout, refCalcProgress.m_value, true); //This triggers other recalculations/lookups.
          }
        }
      }
    }
  }

}

Base_DB::type_map_fields Base_DB::get_record_field_values(const Glib::ustring& table_name, const sharedptr<const Field> primary_key, const Gnome::Gda::Value& primary_key_value)
{
  type_map_fields field_values;

  Document_Glom* document = get_document();
  if(document)
  {
    //TODO: Cache the list of all fields, as well as caching (m_Fields) the list of all visible fields:
    const Document_Glom::type_vecFields fields = document->get_table_fields(table_name);

    //TODO: This seems silly. We should just have a build_sql_select() that can take this container:
    type_vecLayoutFields fieldsToGet;
    for(Document_Glom::type_vecFields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
      layout_item->set_full_field_details(*iter);

      fieldsToGet.push_back(layout_item);
    }

    if(!GlomConversions::value_is_empty(primary_key_value))
    {
      //sharedptr<const Field> fieldPrimaryKey = get_field_primary_key();

      const Glib::ustring query = GlomUtils::build_sql_select_with_key(table_name, fieldsToGet, primary_key, primary_key_value);
      Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(query);

      if(data_model && data_model->get_n_rows())
      {
        int col_index = 0;
        for(Document_Glom::type_vecFields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
        {
          //There should be only 1 row. Well, there could be more but we will ignore them.
          sharedptr<const Field> field = *iter;

          Gnome::Gda::Value value = data_model->get_value_at(col_index, 0);

          //Never give a NULL-type value to the python calculation,
          //to prevent errors:
          if(value.get_value_type() == Gnome::Gda::VALUE_TYPE_NULL)
            value = GlomConversions::get_empty_value_suitable_for_python(field->get_glom_type());

          field_values[field->get_name()] = value;
          ++col_index;
        }
      }
      else
      {
        handle_error();
      }
    }

    if(field_values.empty()) //Maybe there was no primary key, or maybe the record is not yet in the database.
    {
      //Create appropriate empty values:
      for(Document_Glom::type_vecFields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
      {
        sharedptr<const Field> field = *iter;
        field_values[field->get_name()] = GlomConversions::get_empty_value_suitable_for_python(field->get_glom_type());
      }
    }
  }

  return field_values;
}

void Base_DB::set_entered_field_data(const sharedptr<const LayoutItem_Field>& /* field */, const Gnome::Gda::Value& /* value */)
{
  //Override this.
}


void Base_DB::set_entered_field_data(const Gtk::TreeModel::iterator& /* row */, const sharedptr<const LayoutItem_Field>& /* field */, const Gnome::Gda::Value& /* value */)
{
  //Override this.
}

bool Base_DB::set_field_value_in_database(const FieldInRecord& field_in_record, const Gnome::Gda::Value& field_value, bool use_current_calculations, Gtk::Window* parent_window)
{
  return set_field_value_in_database(field_in_record, Gtk::TreeModel::iterator(), field_value, use_current_calculations, parent_window);
}

bool Base_DB::set_field_value_in_database(const FieldInRecord& field_in_record, const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& field_value, bool use_current_calculations, Gtk::Window* parent_window)
{
  //row is invalid, and ignored, for Box_Data_Details.
  if(!(field_in_record.m_field))
  {
    std::cerr << "Base_DB::set_field_value_in_database(): field_in_record.m_field is empty." << std::endl;
    return false;
  }

  if(!(field_in_record.m_key))
  {
    std::cerr << "Base_DB::set_field_value_in_database(): field_in_record.m_key is empty." << std::endl;
    return false;
  }

  const Glib::ustring field_name = field_in_record.m_field->get_name();
  if(!field_name.empty()) //This should not happen.
  {

    Glib::ustring strQuery = "UPDATE \"" + field_in_record.m_table_name + "\"";
    strQuery += " SET \"" + field_in_record.m_field->get_name() + "\" = " + field_in_record.m_field->sql(field_value);
    strQuery += " WHERE \"" + field_in_record.m_key->get_name() + "\" = " + field_in_record.m_key->sql(field_in_record.m_key_value);

    //std::cout << "debug: set_field_value_in_database(): " << std::endl << "  " << strQuery << std::endl;

    try
    {
      Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute(strQuery, parent_window);  //TODO: Handle errors
      if(!datamodel)
      {
        g_warning("Box_Data::set_field_value_in_database(): UPDATE failed.");
        return false; //failed.
      }
    }
    catch(const std::exception& ex)
    {
      handle_error(ex);
      return false;
    }

    //Get-and-set values for lookup fields, if this field triggers those relationships:
    do_lookups(field_in_record, row, field_value);

    //Update related fields, if this field is used in the relationship:
    refresh_related_fields(field_in_record, row, field_value);

    //Calculate any dependent fields.
    //TODO: Make lookups part of this?
    //Prevent circular calculations during the recursive do_calculations:
    {
      //Recalculate any calculated fields that depend on this calculated field.
      //g_warning("Box_Data::set_field_value_in_database(): calling do_calculations");

      do_calculations(field_in_record, !use_current_calculations);
    }
  }

  return true;
}

Gnome::Gda::Value Base_DB::get_field_value_in_database(const FieldInRecord& field_in_record, Gtk::Window* parent_window)
{
  Gnome::Gda::Value result;  //TODO: Return suitable empty value for the field when failing?

  //row is invalid, and ignored, for Box_Data_Details.
  if(!(field_in_record.m_field))
  {
    std::cerr << "Base_DB:gset_field_value_in_database(): field_in_record.m_field is empty." << std::endl;
    return result;
  }

  if(!(field_in_record.m_key))
  {
    std::cerr << "Base_DB::get_field_value_in_database(): field_in_record.m_key is empty." << std::endl;
    return result;
  }

  type_vecLayoutFields list_fields;
  sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
  layout_item->set_full_field_details(field_in_record.m_field);
  list_fields.push_back(layout_item);
  const Glib::ustring sql_query = GlomUtils::build_sql_select_with_key(field_in_record.m_table_name,
      list_fields, field_in_record.m_key, field_in_record.m_key_value);

  Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(sql_query);
  if(data_model)
  {
    if(data_model->get_n_rows())
    {
      result = data_model->get_value_at(0, 0);
    }
  }
  else
  {
    handle_error();
  }

  return result;
}

void Base_DB::do_calculations(const FieldInRecord& field_changed, bool first_calc_field)
{
  //g_warning("Box_Data::do_calculations(): triggered by =%s", field_changed.m_field->get_name().c_str());

  if(first_calc_field)
  {
    //g_warning("  clearing m_FieldsCalculationInProgress");
    m_FieldsCalculationInProgress.clear();
  }

  //Recalculate fields that are triggered by a change of this field's value, not including calculations that these calculations use.
  type_field_calcs calculated_fields = get_calculated_fields(field_changed.m_table_name, field_changed.m_field->get_name()); //TODO: Just return the Fields, not the CalcInProgress.
  for(type_field_calcs::iterator iter = calculated_fields.begin(); iter != calculated_fields.end(); ++iter)
  {
    sharedptr<const Field> field = iter->second.m_field;
    if(field)
    {
      //TODO: What if the field is in another table?
      FieldInRecord triggered_field(field_changed.m_table_name, field, field_changed.m_key, field_changed.m_key_value);
      calculate_field(triggered_field); //And any dependencies.

      //Calculate anything that depends on this.
      do_calculations(triggered_field, false /* recurse, reusing m_FieldsCalculationInProgress */);
    }
  }

  if(first_calc_field)
    m_FieldsCalculationInProgress.clear();
}

Base_DB::type_field_calcs Base_DB::get_calculated_fields(const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  type_field_calcs result;

  const Document_Glom* document = dynamic_cast<const Document_Glom*>(get_document());
  if(document)
  {
    const type_vecFields fields = document->get_table_fields(table_name); //TODO_Performance: Cache this?
    //Examine all fields, not just the the shown fields.
    for(type_vecFields::const_iterator iter = fields.begin(); iter != fields.end();  ++iter)
    {
      sharedptr<const Field> field = *iter;

      //Does this field's calculation use the field?
      const type_list_field_items fields_triggered = get_calculation_fields(table_name, field);
      type_list_field_items::const_iterator iterFind = std::find_if(fields_triggered.begin(), fields_triggered.end(), predicate_FieldHasName<LayoutItem_Field>(field_name));
      if(iterFind != fields_triggered.end())
      {
        if(!(*iterFind)->get_has_relationship_name()) //TODO: skip past related fields instead of just giving up.
        {
          CalcInProgress item;
          item.m_field = field;

          result[field->get_name()] = item;
        }
      }
    }
  }

  return result;
}

Base_DB::type_list_field_items Base_DB::get_calculation_fields(const Glib::ustring& table_name, const sharedptr<const Field>& field)
{
  //TODO: Use regex, for instance with pcre here?
  //TODO: Better?: Run the calculation on some example data, and record the touched fields? But this could not exercise every code path.
  //TODO_Performance: Just cache the result whenever m_calculation changes.

  type_list_field_items result;

  Glib::ustring::size_type index = 0;
  const Glib::ustring calculation = field->get_calculation();
  if(calculation.empty())
    return result;

  Document_Glom* document = get_document();
  if(!document)
    return result;

  const Glib::ustring::size_type count = calculation.size();
  const Glib::ustring prefix = "record[\"";
  const Glib::ustring::size_type prefix_size = prefix.size();

  while(index < count)
  {
    Glib::ustring::size_type pos_find = calculation.find(prefix, index);
    if(pos_find != Glib::ustring::npos)
    {
      Glib::ustring::size_type pos_find_end = calculation.find("\"]", pos_find);
      if(pos_find_end  != Glib::ustring::npos)
      {
        Glib::ustring::size_type pos_start = pos_find + prefix_size;
        const Glib::ustring field_name = calculation.substr(pos_start, pos_find_end - pos_start);

        sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
        layout_item->set_full_field_details( document->get_field(table_name, field_name) );

        result.push_back(layout_item);
        index = pos_find_end + 1;
      }
    }

    ++index;
  }

  return result;
}


void Base_DB::do_lookups(const FieldInRecord& field_in_record, const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& field_value)
{
   //Get values for lookup fields, if this field triggers those relationships:
   //TODO_performance: There is a LOT of iterating and copying here.
   const Glib::ustring strFieldName = field_in_record.m_field->get_name();
   const type_list_lookups lookups = get_lookup_fields(field_in_record.m_table_name, strFieldName);
   //std::cout << "Base_DB::do_lookups(): lookups size=" << lookups.size() << std::endl;
   for(type_list_lookups::const_iterator iter = lookups.begin(); iter != lookups.end(); ++iter)
   {
     sharedptr<const LayoutItem_Field> layout_item = iter->first;

     //std::cout << "Base_DB::do_lookups(): item=" << layout_item->get_name() << std::endl;

     sharedptr<const Relationship> relationship = iter->second;
     const sharedptr<const Field> field_lookup = layout_item->get_full_field_details();
     if(field_lookup)
     {
      sharedptr<const Field> field_source = get_fields_for_table_one_field(relationship->get_to_table(), field_lookup->get_lookup_field());
      if(field_source)
      {
        const Gnome::Gda::Value value = get_lookup_value(field_in_record.m_table_name, iter->second /* relationship */,  field_source /* the field to look in to get the value */, field_value /* Value of to and from fields */);

        const Gnome::Gda::Value value_converted = GlomConversions::convert_value(value, layout_item->get_glom_type());

        FieldInRecord field_in_record_to_set(layout_item, field_in_record.m_table_name /* parent table */, field_in_record.m_key, field_in_record.m_key_value, *(get_document()));

        //Add it to the view:
        set_entered_field_data(row, layout_item, value_converted);
        //m_AddDel.set_value(row, layout_item, value_converted);

        //Add it to the database (even if it is not shown in the view)
        set_field_value_in_database(field_in_record_to_set, row, value_converted); //Also does dependent lookups/recalcs.
        //Glib::ustring strQuery = "UPDATE \"" + m_table_name + "\"";
        //strQuery += " SET " + field_lookup.get_name() + " = " + field_lookup.sql(value);
        //strQuery += " WHERE " + primary_key.get_name() + " = " + primary_key.sql(primary_key_value);
        //query_execute(strQuery);  //TODO: Handle errors

        //TODO: Handle lookups triggered by these fields (recursively)? TODO: Check for infinitely looping lookups.
      }
    }
  }
}


/** Get the fields whose values should be looked up when @a field_name changes, with
 * the relationship used to lookup the value.
 */
Base_DB::type_list_lookups Base_DB::get_lookup_fields(const Glib::ustring& table_name, const Glib::ustring& field_name) const
{
  type_list_lookups result;

  const Document_Glom* document = dynamic_cast<const Document_Glom*>(get_document());
  if(document)
  {
    //Examine all fields, not just the the shown fields (m_Fields):
    const type_vecFields fields = document->get_table_fields(table_name); //TODO_Performance: Cache this?
    //Examine all fields, not just the the shown fields.
    for(type_vecFields::const_iterator iter = fields.begin(); iter != fields.end();  ++iter)
    {
      sharedptr<Field> field = *iter;

      //Examine each field that looks up its data from a relationship:
      if(field && field->get_is_lookup())
      {
        //Get the relationship information:
        sharedptr<Relationship> relationship = field->get_lookup_relationship();
        if(relationship)
        {
          //If the relationship is triggererd by the specified field:
          if(relationship->get_from_field() == field_name)
          {
            //Add it:
            sharedptr<LayoutItem_Field> item = sharedptr<LayoutItem_Field>::create();
            item->set_full_field_details(field);
            result.push_back( type_pairFieldTrigger(item, relationship) );
          }
        }
      }
    }
  }

  return result;
}

void Base_DB::refresh_related_fields(const FieldInRecord& field_in_record_changed, const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& /* field_value */)
{
  //overridden in Box_Data.
}

Gnome::Gda::Value Base_DB::get_lookup_value(const Glib::ustring& table_name, const sharedptr<const Relationship>& relationship, const sharedptr<const Field>& source_field, const Gnome::Gda::Value& key_value)
{
  Gnome::Gda::Value result;

  sharedptr<Field> to_key_field = get_fields_for_table_one_field(relationship->get_to_table(), relationship->get_to_field());
  if(to_key_field)
  {
    //Convert the value, in case the from and to fields have different types:
    const Gnome::Gda::Value value_to_key_field = GlomConversions::convert_value(key_value, to_key_field->get_glom_type());

    Glib::ustring strQuery = "SELECT \"" + relationship->get_to_table() + "\".\"" + source_field->get_name() + "\" FROM \"" +  relationship->get_to_table() + "\"";
    strQuery += " WHERE \"" + to_key_field->get_name() + "\" = " + to_key_field->sql(value_to_key_field);

    Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(strQuery);
    if(data_model && data_model->get_n_rows())
    {
      //There should be only 1 row. Well, there could be more but we will ignore them.
      result = data_model->get_value_at(0, 0);
    }
    else
    {
      handle_error();
    }
  }

  return result;
}

bool Base_DB::get_field_value_is_unique(const Glib::ustring& table_name, const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  bool result = true;  //Arbitrarily default to saying it's unique if we can't get any result.

  const Glib::ustring table_name_used = field->get_table_used(table_name); 
  Glib::ustring strQuery = "SELECT \"" + table_name_used + "\".\"" + field->get_name() + "\" FROM \"" + table_name_used + "\"";
  strQuery += " WHERE \"" + field->get_name() + "\" = " + field->get_full_field_details()->sql(value);

  Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(strQuery);
  if(data_model)
  {
    std::cout << "debug: Base_DB::get_field_value_is_unique(): table_name=" << table_name << ", field name=" << field->get_name() << ", value=" << value.to_string() << ", rows count=" << data_model->get_n_rows() << std::endl;
    //The value is unique for this field, if the query returned no existing rows:

    result = (data_model->get_n_rows() == 0);
  }
  else
  {
    handle_error();
  }

  return result;
}

bool Base_DB::check_entered_value_for_uniqueness(const Glib::ustring& table_name, const sharedptr<const LayoutItem_Field>& layout_field, const Gnome::Gda::Value& field_value, Gtk::Window* parent_window)
{
  return check_entered_value_for_uniqueness(table_name, Gtk::TreeModel::iterator(), layout_field, field_value, parent_window);
}

bool Base_DB::check_entered_value_for_uniqueness(const Glib::ustring& table_name, const Gtk::TreeModel::iterator& /* row */,  const sharedptr<const LayoutItem_Field>& layout_field, const Gnome::Gda::Value& field_value, Gtk::Window* parent_window)
{
  //Check whether the value meets uniqueness constraints, if any:
  const sharedptr<const Field>& field = layout_field->get_full_field_details();
  if(field && (field->get_primary_key() || field->get_unique_key()))
  {
    if(!get_field_value_is_unique(table_name, layout_field, field_value))
    {
      //std::cout << "debug Base_DB::check_entered_value_for_uniqueness(): field=" << layout_field->get_name() << ", value is not unique: " << field_value.to_string() << std::endl;

      //Warn the user and revert the value:
      if(parent_window)
        Frame_Glom::show_ok_dialog(_("Value Is Not Unique"), _("The field's value must be unique, but a record with this value already exists."), *parent_window);
         
      return false; //Failed.
    }
    else
      return true; //Succeed, because the value is unique.
  }
  else
    return true; //Succeed, because the value does not need to be unique.
}
