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
#include <glom/libglom/data_structure/glomconversions.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_summary.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_verticalgroup.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_header.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_footer.h>
#include "python_embed/glom_python.h"
#include <glom/glom_postgres.h>
#include <glom/glom_privs.h>
#include <glibmm/i18n.h>
//#include <libgnomeui/gnome-app-helper.h>

#include <sstream> //For stringstream

namespace Glom
{

FoundSet::FoundSet()
{
}

FoundSet::FoundSet(const FoundSet& src)
:  m_table_name(src.m_table_name),
   m_extra_join(src.m_extra_join),
   m_where_clause(src.m_where_clause),
   m_extra_group_by(src.m_extra_group_by),
   m_sort_clause(src.m_sort_clause)
{
}

FoundSet& FoundSet::operator=(const FoundSet& src)
{
  m_table_name = src.m_table_name;
  m_extra_join = src.m_extra_join;
  m_where_clause = src.m_where_clause;
  m_extra_group_by = src.m_extra_group_by;
  m_sort_clause = src.m_sort_clause;

  return *this;
}

bool FoundSet::operator==(const FoundSet& src) const
{
  return (m_table_name == src.m_table_name)
      && (m_extra_join == src.m_extra_join)
      && (m_where_clause == src.m_where_clause)
      && (m_extra_group_by == src.m_extra_group_by)
      && (m_sort_clause == src.m_sort_clause);
}



template<class T_Element>
class predicate_LayoutItemIsEqual
{
public:
  predicate_LayoutItemIsEqual(const sharedptr<const T_Element>& layout_item)
  : m_layout_item(layout_item)
  {
  }

  virtual ~predicate_LayoutItemIsEqual()
  {
  }

  bool operator() (const sharedptr<const T_Element>& layout_item) const
  {
    if(!m_layout_item && !layout_item)
      return true;

    if(layout_item && m_layout_item)
    {
      return m_layout_item->is_same_field(layout_item);
      //std::cout << "          debug: name1=" << m_layout_item->get_name() << ", name2=" << layout_item->get_name() << ", result=" << result << std::endl; 
      //return result;
    }
    else
      return false;
  }

protected:
  sharedptr<const T_Element> m_layout_item;
};


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

void Base_DB::handle_error(const std::exception& ex)
{
  std::cerr << "Internal Error (Base_DB::handle_error()): exception type=" << typeid(ex).name() << ", ex.what()=" << ex.what() << std::endl;

  Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Internal error")), true, Gtk::MESSAGE_WARNING );
  dialog.set_secondary_text(ex.what());
  //TODO: dialog.set_transient_for(*get_application());
  dialog.run();
}

bool Base_DB::handle_error()
{
  return ConnectionPool::handle_error();
}

//static:
Glib::RefPtr<Gnome::Gda::DataModel> Base_DB::query_execute(const Glib::ustring& strQuery, Gtk::Window* /* parent_window */)
{
  Glib::RefPtr<Gnome::Gda::DataModel> result;

  //TODO: Bakery::BusyCursor busy_cursor(get_app_window());

  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

   const App_Glom* app = App_Glom::get_application();
   if(app && app->get_show_sql_debug())
   {
      try
      {
        std::cout << "Debug: query_execute():  " << strQuery << std::endl;
      }
      catch(const Glib::Exception& ex)
      {
        std::cout << "Debug: query string could not be converted to std::cout: " << ex.what() << std::endl;
      }
    }
    
    result = gda_connection->execute_select_command(strQuery);
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
        if(G_VALUE_TYPE(value.gobj()) ==  G_TYPE_STRING)
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

Base_DB::type_vecFields Base_DB::get_fields_for_table_from_database(const Glib::ustring& table_name, bool /* including_system_fields */)
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

    Glib::RefPtr<Gnome::Gda::Parameter> param_table_name = Gnome::Gda::Parameter::create("name", table_name);
    //Gnome::Gda::Parameter param_table_name("name", table_name);
    Glib::RefPtr<Gnome::Gda::ParameterList> param_list = Gnome::Gda::ParameterList::create();
    param_list->add_parameter(param_table_name);

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
        Glib::RefPtr<Gnome::Gda::Column> field_info;

        //Get the field name:
        Gnome::Gda::Value value_name = data_model_fields->get_value_at(DATAMODEL_FIELDS_COL_NAME, row);
        if(value_name.get_value_type() ==  G_TYPE_STRING)
        {
          if(value_name.get_string().empty())
            g_warning("Base_DB::get_fields_for_table_from_database(): value_name is empty.");

          field_info->set_name( value_name.get_string() ); //TODO: get_string() is a dodgy choice. murrayc.
        }

        //Get the field type:
        //This is a string representation of the type, so we need to discover the GType for it:
        Gnome::Gda::Value value_fieldtype = data_model_fields->get_value_at(DATAMODEL_FIELDS_COL_TYPE, row);
        if(value_fieldtype.get_value_type() ==  G_TYPE_STRING)
        {
           Glib::ustring schema_type_string = value_fieldtype.get_string();
           if(!schema_type_string.empty())
           {
             ConnectionPool* connection_pool = ConnectionPool::get_instance();
             const FieldTypes* pFieldTypes = connection_pool->get_field_types();
             if(pFieldTypes)
             {
               GType gdatype = pFieldTypes->get_gdavalue_for_schema_type_string(schema_type_string);
               field_info->set_g_type(gdatype);
             }
           }
        }

        //Get the default value:
        /* libgda does not return this correctly yet. TODO: check bug http://bugzilla.gnome.org/show_bug.cgi?id=143576
        Gnome::Gda::Value value_defaultvalue = data_model_fields->get_value_at(DATAMODEL_FIELDS_COL_DEFAULTVALUE, row);
        if(value_defaultG_VALUE_TYPE(value.gobj()) ==  G_TYPE_STRING)
          field_info->set_default_value(value_defaultvalue);
        */

        //Get whether it can be null:
        Gnome::Gda::Value value_notnull = data_model_fields->get_value_at(DATAMODEL_FIELDS_COL_NOTNULL, row);
        if(value_notnull.get_value_type() ==  G_TYPE_BOOLEAN)
          field_info->set_allow_null(value_notnull.get_boolean());


        //Get whether it is a primary key:
        Gnome::Gda::Value value_primarykey = data_model_fields->get_value_at(DATAMODEL_FIELDS_COL_PRIMARYKEY, row);
        if(value_primarykey.get_value_type() ==  G_TYPE_BOOLEAN)
          field_info->set_primary_key( value_primarykey.get_boolean() );

        //Get whether it is unique
        Gnome::Gda::Value value_unique = data_model_fields->get_value_at(DATAMODEL_FIELDS_COL_UNIQUEINDEX, row);
        if(value_unique.get_value_type() ==  G_TYPE_BOOLEAN)
          field_info->set_unique_key( value_unique.get_boolean() );
        else if(field_info->get_primary_key()) //All primaries keys are unique, so force this.
          field_info->set_unique_key();

        //Get whether it is autoincrements
        /* libgda does not provide this yet.
        Gnome::Gda::Value value_autoincrement = data_model_fields->get_value_at(DATAMODEL_FIELDS_COL_AUTOINCREMENT, row);
        if(value_autoincrement.get_value_type() ==  G_TYPE_BOOLEAN)
          field_info->set_auto_increment( value_autoincrement.get_bool() );
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
        Glib::RefPtr<Gnome::Gda::Column> field_info_document = field->get_field_info();

        //Update the Field information that _might_ have changed in the database.
        Glib::RefPtr<Gnome::Gda::Column> field_info = (*iterFindDatabase)->get_field_info();

        //libgda does not tell us whether the field is auto_incremented, so we need to get that from the document.
        field_info->set_auto_increment( field_info_document->get_auto_increment() );

        //libgda does not tell us whether the field is auto_incremented, so we need to get that from the document.
        field_info->set_primary_key( field_info_document->get_primary_key() );

        //libgda does yet tell us correct default_value information so we need to get that from the document.
        field_info->set_default_value( field_info_document->get_default_value() );

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

  const type_vecStrings vecGroups = Privs::get_database_groups();
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
          Privs::set_table_privileges(devgroup, table_name, priv_devs, true /* developer privileges */);
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
  Privileges table_privs = Privs::get_current_privs(GLOM_STANDARD_TABLE_AUTOINCREMENTS_TABLE_NAME);
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
    value = Conversions::parse_value(Field::TYPE_NUMERIC, "0", success, true /* iso_format */);
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
    long num_max = Utils::decimal_from_string(value_max.to_string()); //TODO: Is this sensible? Probably not.
    ++num_max;

    //Set it in the glom system table:
    const Gnome::Gda::Value next_value = Conversions::parse_value(num_max);
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
  num_result = Utils::decimal_from_string(result.to_string()); //TODO: Is this sensible? Probably not.


  //Increment the next_value:
  ++num_result;
  const Gnome::Gda::Value next_value = Conversions::parse_value(num_result);

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

SystemPrefs Base_DB::get_database_preferences() const
{
  //if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  //  add_standard_tables();

  SystemPrefs result;

  //Check that the user is allowd to even view this table:
  Privileges table_privs = Privs::get_current_privs(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME);
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
      result.m_name = Conversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(0, 0));
      result.m_org_name = Conversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(1, 0));
      result.m_org_address_street = Conversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(2, 0));
      result.m_org_address_street2 = Conversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(3, 0));
      result.m_org_address_town = Conversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(4, 0));
      result.m_org_address_county = Conversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(5, 0));
      result.m_org_address_country = Conversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(6, 0));
      result.m_org_address_postcode = Conversions::get_text_for_gda_value(Field::TYPE_TEXT, datamodel->get_value_at(7, 0));

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

bool Base_DB::create_table_with_default_fields(const Glib::ustring& table_name)
{
  if(table_name.empty())
    return false;
  
  bool created = false;

  //Primary key:
  sharedptr<Field> field_primary_key(new Field());
  field_primary_key->set_name(table_name + "_id");
  field_primary_key->set_title(table_name + " ID");
  field_primary_key->set_primary_key();
  field_primary_key->set_auto_increment();

  Glib::RefPtr<Gnome::Gda::Column> field_info = field_primary_key->get_field_info();
  field_info->set_allow_null(false);
  field_primary_key->set_field_info(field_info);

  field_primary_key->set_glom_type(Field::TYPE_NUMERIC);
  //std::cout << "field_primary_key->get_auto_increment():" << field_primary_key->get_auto_increment() << std::endl;

  type_vecFields fields;
  fields.push_back(field_primary_key);

  //Description:
  sharedptr<Field> field_description(new Field());
  field_description->set_name("description");
  field_description->set_title(_("Description")); //Use a translation, because the original locale will be marked as non-English if the current locale is non-English.
  field_description->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_description);

  //Comments:
  sharedptr<Field> field_comments(new Field());
  field_comments->set_name("comments");
  field_comments->set_title(_("Comments"));
  field_comments->set_glom_type(Field::TYPE_TEXT);
  field_comments->m_default_formatting.set_text_format_multiline();
  fields.push_back(field_comments);

  sharedptr<TableInfo> table_info(new TableInfo());
  table_info->set_name(table_name);
  table_info->set_title( Utils::title_from_string( table_name ) ); //Start with a title that might be appropriate.

  created = create_table(table_info, fields);

    //Create a table with 1 "ID" field:
   //MSYQL:
    //query_execute( "CREATE TABLE \"" + table_name + "\" (" + primary_key_name + " INT NOT NULL AUTO_INCREMENT PRIMARY KEY)" );
    //query_execute( "INSERT INTO \"" + table_name + "\" VALUES (0)" );

    //PostgresSQL:
    //query_execute( "CREATE TABLE " + table_name + " (\"" + primary_key_name + "\" serial NOT NULL  PRIMARY KEY)" );

    //query_execute( "CREATE TABLE \"" + table_name + "\" (" +
    //  field_primary_key->get_name() + " numeric NOT NULL  PRIMARY KEY," + 
    //  extra_field_description + "varchar, " +
    //  extra_field_comments + "varchar" +
    //  ")" );

  if(created)
  {
    //Save the changes in the document:
    Document_Glom* document = get_document();
    if(document)
    {
      document->add_table(table_info);
      document->set_table_fields(table_info->get_name(), fields);
    }
  }

  return created;
}

bool Base_DB::create_table(const sharedptr<const TableInfo>& table_info, const Document_Glom::type_vecFields& fields_in) const
{
  //std::cout << "Base_DB::create_table(): " << table_info->get_name() << ", title=" << table_info->get_title() << std::endl;

  bool table_creation_succeeded = false;


  Document_Glom::type_vecFields fields = fields_in;

  //Create the standard field too:
  //(We don't actually use this yet)
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
    Glib::RefPtr<Gnome::Gda::Column> info = field->get_field_info();
    info->set_g_type( Field::get_gda_type_for_glom_type(field->get_glom_type()) );
    field->set_field_info(info); //TODO_Performance

    Glib::ustring sql_field_description = "\"" + field->get_name() + "\" " + field->get_sql_type();

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
      const bool test = GlomPostgres::postgres_add_column(table_name, field);
      if(!test)
       return test;
    }
  }

  return true;
}

bool Base_DB::insert_example_data(const Glib::ustring& table_name) const
{
  const Glib::ustring example_rows = get_document()->get_table_example_data(table_name);
  if(example_rows.empty())
  {
    //std::cout << "debug: Base_DB::insert_example_data(): No example data available." << std::endl;
    return true;
  }

  //std::cout << "debug: inserting example_rows for table: " << table_name << std::endl;

  bool insert_succeeded = true;


  //Get field names:
  Document_Glom::type_vecFields vec_fields = get_document()->get_table_fields(table_name);

  Glib::ustring strNames;
  unsigned int fields_count = 0;
  for(Document_Glom::type_vecFields::const_iterator iter = vec_fields.begin(); iter != vec_fields.end(); ++iter)
  {
    //Append it:
    if(!strNames.empty())
    {
      strNames += ", ";
      ++fields_count;
    }

    strNames += "\"" + (*iter)->get_name() + "\"";
  }

  if(strNames.empty())
  {
    g_warning("Base_DB::insert_example_data(): strNames is empty.");
  }

  //Actually insert the data:
  const type_vecStrings vec_rows = Utils::string_separate(example_rows, "\n", false /* ignore \n inside quotes. */);
  //std::cout << "  debug: Base_DB::insert_example_data(): number of rows of data: " << vec_rows.size() << std::endl;

  for(type_vecStrings::const_iterator iter = vec_rows.begin(); iter != vec_rows.end(); ++iter)
  {
    try
    {
      //Check that the row contains the correct number of columns.
      //This check will slow this down, but it seems useful:
      //TODO: This can only work if we can distinguish , inside "" and , outside "":
      const Glib::ustring& row_data = *iter;
      if(!row_data.empty())
      {
        const type_vecStrings vec_values = Utils::string_separate(row_data, ",", true /* ignore , inside quotes */);
        if(true) //vec_values.size() == fields_count)
	{
          //Do not allow any unescaped newlines in the row data.
          //Note that this is checking for newlines ("\n"), not escaped newlines ("\\n").
          if(row_data.find("\n") == Glib::ustring::npos) 
          {
            const Glib::ustring strQuery = "INSERT INTO \"" + table_name + "\" (" + strNames + ") VALUES (" + row_data + ")";
            //std::cout << "debug: before query: " << strQuery << std::endl;
            query_execute(strQuery); //TODO: Test result.
            //std::cout << "debug: after query: " << strQuery << std::endl;
            insert_succeeded = true;
          }
          else
          {
            std::cerr << "Glom: Base_DB::insert_example_data(): nested newline found in row data: " << row_data << std::endl;
          }
        }
        else
        {
          std::cerr << "Glom: Base_DB::insert_example_data(): vec_values.size()=" << vec_values.size() << " != fields_count="<< fields_count << std::endl;
        }
      }
    }
    catch(const ExceptionConnection& ex)
    {
      std::cerr << "debug: Base_DB::insert_example_data(): query_execute() failed: " << ex.what() << std::endl;
   
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
      int response = Glom::Utils::dialog_run_with_help(dialog, "window_textobject");
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
      const int response = Glom::Utils::dialog_run_with_help(dialog, "window_imageobject");
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
      const int response = Glom::Utils::dialog_run_with_help(dialog, "dialog_notebook");
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
        strClausePart = "\"" + table_name + "\".\"" + field->get_name() + "\" " + field->sql_find_operator() + " " +  field->sql_find(quick_search);
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

void Base_DB::get_table_fields_to_show_for_sequence_add_group(const Glib::ustring& table_name, const Privileges& table_privs, const type_vecFields& all_db_fields, const sharedptr<LayoutGroup>& group, Base_DB::type_vecLayoutFields& vecFields) const
{
  //g_warning("Box_Data::get_table_fields_to_show_for_sequence_add_group(): table_name=%s, all_db_fields.size()=%d, group->name=%s", table_name.c_str(), all_db_fields.size(), group->get_name().c_str());

  LayoutGroup::type_map_items items = group->get_items();
  for(LayoutGroup::type_map_items::iterator iterItems = items.begin(); iterItems != items.end(); ++iterItems)
  {
    sharedptr<LayoutItem> item = iterItems->second;

    sharedptr<LayoutItem_Field> item_field = sharedptr<LayoutItem_Field>::cast_dynamic(item);
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
          sharedptr<LayoutItem_Field> layout_item = item_field;
          layout_item->set_full_field_details(field); //Fill in the full field information for later.


          //TODO_Performance: We do this once for each related field, even if there are 2 from the same table:
          const Privileges privs_related = Privs::get_current_privs(item_field->get_table_used(table_name));
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
          sharedptr<LayoutItem_Field> layout_item = item_field;
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
      sharedptr<LayoutGroup> item_group = sharedptr<LayoutGroup>::cast_dynamic(item);
      if(item_group)
      {
        sharedptr<LayoutItem_Portal> item_portal = sharedptr<LayoutItem_Portal>::cast_dynamic(item);
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

  const Privileges table_privs = Privs::get_current_privs(table_name);

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
        sharedptr<LayoutGroup> group = iter->second;

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

  LayoutFieldInRecord field_in_record;
  field_in_record.m_table_name = table_name;

  sharedptr<LayoutItem_Field> layoutitem_field = sharedptr<LayoutItem_Field>::create();
  layoutitem_field->set_full_field_details(field);
  field_in_record.m_field = layoutitem_field;
  field_in_record.m_key = primary_key;

  //Calculate the value for the field in every record:
  const int rows_count = data_model->get_n_rows();
  for(int row = 0; row < rows_count; ++row)
  {
    const Gnome::Gda::Value primary_key_value = data_model->get_value_at(0, row);
    if(!Conversions::value_is_empty(primary_key_value))
    {
      field_in_record.m_key_value = primary_key_value;

      m_FieldsCalculationInProgress.clear();
      calculate_field(field_in_record);
    }
  }
}

void Base_DB::calculate_field(const LayoutFieldInRecord& field_in_record)
{
  const Glib::ustring field_name = field_in_record.m_field->get_name();
  //g_warning("Box_Data::calculate_field(): field_name=%s", field_name.c_str());

  //Do we already have this in our list?
  type_field_calcs::iterator iterFind = m_FieldsCalculationInProgress.find(field_name);
  if(iterFind == m_FieldsCalculationInProgress.end()) //If it was not found.
  {
    //Add it:
    CalcInProgress item;
    item.m_field = field_in_record.m_field->get_full_field_details();
    m_FieldsCalculationInProgress[field_name] = item;

    iterFind = m_FieldsCalculationInProgress.find(field_name); //Always succeeds.
  }

  CalcInProgress& refCalcProgress = iterFind->second;

  //Use the previously-calculated value if possible:
  if(refCalcProgress.m_calc_in_progress)
  {
    //g_warning("  Box_Data::calculate_field(): Circular calculation detected. field_name=%s", field_name.c_str());
    //refCalcProgress.m_value = Conversions::get_empty_value(field->get_glom_type()); //Give up.
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
    const type_list_const_field_items fields_needed = get_calculation_fields(field_in_record.m_table_name, field_in_record.m_field);
    for(type_list_const_field_items::const_iterator iterNeeded = fields_needed.begin(); iterNeeded != fields_needed.end(); ++iterNeeded)
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

            LayoutFieldInRecord needed_field_in_record(field_item_needed, field_in_record.m_table_name, field_in_record.m_key, field_in_record.m_key_value);
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
      const type_map_fields field_values = get_record_field_values_for_calculation(field_in_record.m_table_name, field_in_record.m_key, field_in_record.m_key_value);
      if(!field_values.empty())
      {
        sharedptr<const Field> field = refCalcProgress.m_field;
        if(field)
        {
          //We need the connection when we run the script, so that the script may use it.
          sharedptr<SharedConnection> sharedconnection = connect_to_server(0 /* parent window */);

          refCalcProgress.m_value = glom_evaluate_python_function_implementation(field->get_glom_type(), field->get_calculation(), field_values, get_document(), field_in_record.m_table_name, sharedconnection->get_gda_connection());

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
            LayoutFieldInRecord field_in_record_layout(layout_item, field_in_record.m_table_name /* parent */, field_in_record.m_key, field_in_record.m_key_value);

            set_field_value_in_database(field_in_record_layout, refCalcProgress.m_value, true); //This triggers other recalculations/lookups.
          }
        }
      }
    }
  }

}

Base_DB::type_map_fields Base_DB::get_record_field_values_for_calculation(const Glib::ustring& table_name, const sharedptr<const Field> primary_key, const Gnome::Gda::Value& primary_key_value)
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

    if(!Conversions::value_is_empty(primary_key_value))
    {
      //sharedptr<const Field> fieldPrimaryKey = get_field_primary_key();

      const Glib::ustring query = Utils::build_sql_select_with_key(table_name, fieldsToGet, primary_key, primary_key_value);
      Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(query);

      if(data_model && data_model->get_n_rows())
      {
        int col_index = 0;
        for(Document_Glom::type_vecFields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
        {
          //There should be only 1 row. Well, there could be more but we will ignore them.
          sharedptr<const Field> field = *iter;

          Gnome::Gda::Value value = data_model->get_value_at(col_index, 0);

          //Never give a NULL-type value to the python calculation for types that don't use them:
          //to prevent errors:
          if(value.is_null())
            value = Conversions::get_empty_value(field->get_glom_type());

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
        field_values[field->get_name()] = Conversions::get_empty_value(field->get_glom_type());
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

bool Base_DB::set_field_value_in_database(const LayoutFieldInRecord& field_in_record, const Gnome::Gda::Value& field_value, bool use_current_calculations, Gtk::Window* parent_window)
{
  return set_field_value_in_database(field_in_record, Gtk::TreeModel::iterator(), field_value, use_current_calculations, parent_window);
}

bool Base_DB::set_field_value_in_database(const LayoutFieldInRecord& layoutfield_in_record, const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& field_value, bool use_current_calculations, Gtk::Window* parent_window)
{
  Document_Glom* document = get_document();
  g_assert(document);

  const FieldInRecord field_in_record = layoutfield_in_record.get_fieldinrecord(*document);

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
    do_lookups(layoutfield_in_record, row, field_value);

    //Update related fields, if this field is used in the relationship:
    refresh_related_fields(layoutfield_in_record, row, field_value);

    //Calculate any dependent fields.
    //TODO: Make lookups part of this?
    //Prevent circular calculations during the recursive do_calculations:
    {
      //Recalculate any calculated fields that depend on this calculated field.
      //g_warning("Box_Data::set_field_value_in_database(): calling do_calculations");

      do_calculations(layoutfield_in_record, !use_current_calculations);
    }
  }

  return true;
}

Gnome::Gda::Value Base_DB::get_field_value_in_database(const LayoutFieldInRecord& field_in_record, Gtk::Window* /* parent_window */)
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

  type_vecConstLayoutFields list_fields;
  sharedptr<const LayoutItem_Field> layout_item = field_in_record.m_field;
  list_fields.push_back(layout_item);
  const Glib::ustring sql_query = Utils::build_sql_select_with_key(field_in_record.m_table_name,
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

void Base_DB::do_calculations(const LayoutFieldInRecord& field_changed, bool first_calc_field)
{
  //std::cout << "debug: Base_DB::do_calcualtions(): field_changed=" << field_changed.m_field->get_name() << std::endl;
 
  if(first_calc_field)
  {
    //g_warning("  clearing m_FieldsCalculationInProgress");
    m_FieldsCalculationInProgress.clear();
  }

  //Recalculate fields that are triggered by a change of this field's value, not including calculations that these calculations use.
  
  type_list_const_field_items calculated_fields = get_calculated_fields(field_changed.m_table_name, field_changed.m_field);
  //std::cout << "  debug: calculated_field.size()=" << calculated_fields.size() << std::endl;
  for(type_list_const_field_items::const_iterator iter = calculated_fields.begin(); iter != calculated_fields.end(); ++iter)
  {
    sharedptr<const LayoutItem_Field> field = *iter;
    if(field)
    {
      //std::cout << "debug: recalcing field: " << field->get_name() << std::endl;
      //TODO: What if the field is in another table?
      LayoutFieldInRecord triggered_field(field, field_changed.m_table_name, field_changed.m_key, field_changed.m_key_value);
      calculate_field(triggered_field); //And any dependencies.

      //Calculate anything that depends on this.
      do_calculations(triggered_field, false /* recurse, reusing m_FieldsCalculationInProgress */);
    }
  }

  if(first_calc_field)
    m_FieldsCalculationInProgress.clear();
}

Base_DB::type_list_const_field_items Base_DB::get_calculated_fields(const Glib::ustring& table_name, const sharedptr<const LayoutItem_Field>& field)
{
  //std::cout << "debug: Base_DB::get_calculated_fields field=" << field->get_name() << std::endl;

  type_list_const_field_items result;

  const Document_Glom* document = dynamic_cast<const Document_Glom*>(get_document());
  if(document)
  {
    //Look at each field in the table, and get lists of what fields trigger their calculations, 
    //so we can see if our field is in any of those lists:

    const type_vecFields fields = document->get_table_fields(table_name); //TODO_Performance: Cache this?
    //Examine all fields, not just the the shown fields.
    //TODO: How do we trigger relcalculation of related fields if necessary?
    for(type_vecFields::const_iterator iter = fields.begin(); iter != fields.end();  ++iter)
    {
      sharedptr<Field> field_to_examine = *iter;
      sharedptr<LayoutItem_Field> layoutitem_field_to_examine = sharedptr<LayoutItem_Field>::create();
      layoutitem_field_to_examine->set_full_field_details(field_to_examine);

      //std::cout << "  debug: examining field=" << field_to_examine->get_name() << std::endl;

      //Does this field's calculation use the field?
      const type_list_const_field_items fields_triggered = get_calculation_fields(table_name, layoutitem_field_to_examine);
      //std::cout << "    debug: field_triggered.size()=" << fields_triggered.size() << std::endl;
      type_list_const_field_items::const_iterator iterFind = std::find_if(fields_triggered.begin(), fields_triggered.end(), predicate_LayoutItemIsEqual<LayoutItem_Field>(field));
      if(iterFind != fields_triggered.end())
      {
        //std::cout << "      debug: FOUND: name=" << layoutitem_field_to_examine->get_name() << std::endl;
        //Tell the caller that this field is triggered by the specified field:
        //TODO: Test related fields too?
        
        result.push_back(layoutitem_field_to_examine);
      }
    }
  }

  return result;
}

Base_DB::type_list_const_field_items Base_DB::get_calculation_fields(const Glib::ustring& table_name, const sharedptr<const LayoutItem_Field>& layoutitem_field)
{
  //TODO: Use regex, for instance with pcre here?
  //TODO: Better?: Run the calculation on some example data, and record the touched fields? But this could not exercise every code path.
  //TODO_Performance: Just cache the result whenever m_calculation changes.

  type_list_const_field_items result;

  sharedptr<const Field> field = layoutitem_field->get_full_field_details();
  if(!field)
    return result;

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

        sharedptr<Field> field_found = document->get_field(table_name, field_name);
        if(field)
        {
          sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
          layout_item->set_full_field_details(field_found);

          result.push_back(layout_item);
        }

        index = pos_find_end + 1;
      }
    }

    ++index;
  }

  //Check the use of related records too:
  const Field::type_list_strings relationships_used = field->get_calculation_relationships();
  for(Field::type_list_strings::const_iterator iter = relationships_used.begin(); iter != relationships_used.end(); ++iter)
  {
    sharedptr<Relationship> relationship = document->get_relationship(table_name, *iter);
    if(relationship)
    {
      //If the field uses this relationship then it should be triggered by a change in the key that specifies which record the relationship points to:
      const Glib::ustring field_from_name = relationship->get_from_field();
      sharedptr<Field> field_from = document->get_field(table_name, field_from_name);
      if(field_from)
      {
        sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
        layout_item->set_full_field_details(field_from);

        result.push_back(layout_item);
      }
    }
  }

  return result;
}


void Base_DB::do_lookups(const LayoutFieldInRecord& field_in_record, const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& field_value)
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

        const Gnome::Gda::Value value_converted = Conversions::convert_value(value, layout_item->get_glom_type());

        LayoutFieldInRecord field_in_record_to_set(layout_item, field_in_record.m_table_name /* parent table */, field_in_record.m_key, field_in_record.m_key_value);

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

void Base_DB::refresh_related_fields(const LayoutFieldInRecord& /* field_in_record_changed */, const Gtk::TreeModel::iterator& /* row */, const Gnome::Gda::Value& /* field_value */)
{
  //overridden in Box_Data.
}

Gnome::Gda::Value Base_DB::get_lookup_value(const Glib::ustring& /* table_name */, const sharedptr<const Relationship>& relationship, const sharedptr<const Field>& source_field, const Gnome::Gda::Value& key_value)
{
  Gnome::Gda::Value result;

  sharedptr<Field> to_key_field = get_fields_for_table_one_field(relationship->get_to_table(), relationship->get_to_field());
  if(to_key_field)
  {
    //Convert the value, in case the from and to fields have different types:
    const Gnome::Gda::Value value_to_key_field = Conversions::convert_value(key_value, to_key_field->get_glom_type());

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
    //std::cout << "debug: Base_DB::get_field_value_is_unique(): table_name=" << table_name << ", field name=" << field->get_name() << ", value=" << Gnome::Gda::value_to_string(value) << ", rows count=" << data_model->get_n_rows() << std::endl;
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
      //std::cout << "debug Base_DB::check_entered_value_for_uniqueness(): field=" << layout_field->get_name() << ", value is not unique: " << Gnome::Gda::value_to_string(field_value) << std::endl;

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

bool Base_DB::get_relationship_exists(const Glib::ustring& table_name, const Glib::ustring& relationship_name)
{
  Document_Glom* document = get_document();
  if(document)
  { 
    sharedptr<Relationship> relationship = document->get_relationship(table_name, relationship_name);
    if(relationship)
      return true;
  }
 
  return false;
}

sharedptr<const LayoutItem_Field> Base_DB::get_field_is_from_non_hidden_related_record(const sharedptr<const LayoutItem_Portal>& portal) const
{
  //Find the first field that is from a non-hidden related table.
  sharedptr<LayoutItem_Field> result;

  if(!portal)
    return result;

  const Document_Glom* document = get_document();
  if(!document)
    return result;

  const Glib::ustring parent_table_name = portal->get_table_used(Glib::ustring() /* parent table - not relevant */); 

  LayoutItem_Portal::type_map_const_items items = portal->get_items(); 
  for(LayoutItem_Portal::type_map_const_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
  {
    sharedptr<const LayoutItem_Field> field = sharedptr<const LayoutItem_Field>::cast_dynamic(iter->second);
    if(field)
    {
      if(field->get_has_relationship_name())
      {
        const Glib::ustring table_name = field->get_table_used(parent_table_name);
        if(!(document->get_table_is_hidden(table_name)))
          return field;
      }

    }
  }

  return result;
}

sharedptr<const LayoutItem_Field> Base_DB::get_field_identifies_non_hidden_related_record(const sharedptr<const LayoutItem_Portal>& portal, sharedptr<const Relationship>& used_in_relationship) const
{
  //Find the first field that is from a non-hidden related table.
  sharedptr<LayoutItem_Field> result;

  const Document_Glom* document = get_document();
  if(!document)
    return result;

  const Glib::ustring parent_table_name = portal->get_table_used(Glib::ustring() /* parent table - not relevant */);

  LayoutItem_Portal::type_map_const_items items = portal->get_items(); 
  for(LayoutItem_Portal::type_map_const_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
  {
    sharedptr<const LayoutItem_Field> field = sharedptr<const LayoutItem_Field>::cast_dynamic(iter->second);
    if(field && !(field->get_has_relationship_name()))
    {
      sharedptr<Relationship> relationship = document->get_field_used_in_relationship_to_one(parent_table_name, field->get_name());
      if(relationship)
      {
        const Glib::ustring table_name = relationship->get_to_table();
        if(!(table_name.empty()))
        {
          if(!(document->get_table_is_hidden(table_name)))
          {
            used_in_relationship = relationship;
            return field;
          }
        }
      }
    }
  }

  return result;
}

sharedptr<const UsesRelationship> Base_DB::get_portal_navigation_relationship_automatic(const sharedptr<LayoutItem_Portal>& portal, bool& navigation_main) const
{
  //Initialize output parameters:
  navigation_main = false;

  const Document_Glom* document = get_document();
  
  //If the related table is not hidden then we can just navigate to that:
  const Glib::ustring direct_related_table_name = portal->get_table_used(Glib::ustring() /* parent table - not relevant */);
  if(!(document->get_table_is_hidden(direct_related_table_name)))
  {
    //Non-hidden tables can just be shown directly. Navigate to it: 
    navigation_main = true;
    return sharedptr<UsesRelationship>();
  }
  else
  {
    //If the related table is hidden, 
    //then find a suitable related non-hidden table by finding the first layout field that mentions one:
    sharedptr<const LayoutItem_Field> field = get_field_is_from_non_hidden_related_record(portal);
    if(field)
    {
      return field; //Returns the relationship part. (A relationship belonging to the portal's related table.)
      //sharedptr<UsesRelationship> result = sharedptr<UsesRelationship>::create();
      //result->set_relationship( portal->get_relationship() );
      //result->set_related_relationship( field->get_relationship() );

      //return result;
    }
    else
    {
      //Instead, find a key field that's used in a relationship, 
      //and pretend that we are showing the to field as a related field:
      sharedptr<const Relationship> used_in_relationship;
      sharedptr<const LayoutItem_Field> field_identifies = get_field_identifies_non_hidden_related_record(portal, used_in_relationship);
      if(field_identifies)
      {
        sharedptr<UsesRelationship> result = sharedptr<UsesRelationship>::create();
       
        sharedptr<Relationship> rel_nonconst = sharedptr<Relationship>::cast_const(used_in_relationship);
        result->set_relationship(rel_nonconst);

        return result;
      }
    }
  }

  //There was no suitable related table to show:
  return sharedptr<UsesRelationship>(); 
}

bool Base_DB::get_primary_key_is_in_foundset(const FoundSet& found_set, const Gnome::Gda::Value& primary_key_value)
{
  //TODO_Performance: This is probably called too often, when we should know that the key is in the found set.
  sharedptr<const Field> primary_key = get_field_primary_key_for_table(found_set.m_table_name);
  
  type_vecLayoutFields fieldsToGet;
    
  sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
  layout_item->set_full_field_details(primary_key);
  fieldsToGet.push_back(layout_item);


  Glib::ustring where_clause;
  if(!found_set.m_where_clause.empty())
    where_clause = "(" + found_set.m_where_clause + ") AND ";

  where_clause += "(\"" + primary_key->get_name() + "\"=" + primary_key->sql(primary_key_value) + ")";

  const Glib::ustring query = Utils::build_sql_select_with_where_clause(found_set.m_table_name, fieldsToGet, where_clause);
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(query);

  if(data_model && data_model->get_n_rows())
  {
    //std::cout << "debug: Record found: " << query << std::endl;
    return true; //A record was found in the record set with this value.
  }
  else  
    return false;
}

} //namespace Glom
