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

#include "box_db_table_definition.h"
#include "../../../config.h"
#include <libintl.h>

Box_DB_Table_Definition::Box_DB_Table_Definition()
{
  init();
}

Box_DB_Table_Definition::Box_DB_Table_Definition(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Box_DB_Table(cobject, refGlade)
{
  init();
}

void Box_DB_Table_Definition::init()
{
  m_strHint = gettext("Click [Edit] to edit the field definition in more detail.\nUse the Mode menu to see Data or perform a Find.");

  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_field_definition_edit");
  if(refXml)
    refXml->get_widget_derived("window_field_definition_edit", m_pDialog);

  pack_start(m_AddDel);
  m_colName = m_AddDel.add_column(gettext("Name"));

  m_colTitle = m_AddDel.add_column(gettext("Title"));

  m_colType = m_AddDel.add_column(gettext("Type"), AddDelColumnInfo::STYLE_Choices);
  m_AddDel.set_column_width(m_colType, 100); //TODO: Auto-size columns.

  //Set Type choices:

  Field::type_map_type_names mapFieldTypes = Field::get_usable_type_names();
  AddDel::type_vecStrings vecTypes;
  for(Field::type_map_type_names ::iterator iter = mapFieldTypes.begin(); iter != mapFieldTypes.end();++iter)
  {
    const Glib::ustring& strName = (*iter).second;
    vecTypes.push_back(strName);
  }

  m_AddDel.set_column_choices(m_colType, vecTypes);

  m_colUnique = m_AddDel.add_column("Unique", AddDelColumnInfo::STYLE_Boolean);
  m_colPrimaryKey = m_AddDel.add_column("Primary Key", AddDelColumnInfo::STYLE_Boolean);

  //Connect signals:
  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_AddDel_add));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_AddDel_delete));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_AddDel_changed));
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_AddDel_edit));

  //React to changes in the field properties:
  m_pDialog->signal_apply().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_Properties_apply));
}

Box_DB_Table_Definition::~Box_DB_Table_Definition()
{
}

void Box_DB_Table_Definition::fill_field_row(const Gtk::TreeModel::iterator& iter, const Field& field)
{
  m_AddDel.set_value_key(iter, field.get_name());
  
  m_AddDel.set_value(iter, m_colName, field.get_name());

  Glib::ustring title = field.get_title();
  m_AddDel.set_value(iter, m_colTitle, title);

  Gnome::Gda::FieldAttributes fieldinfo = field.get_field_info();

  //Type:
  Field::glom_field_type fieldType = Field::get_glom_type_for_gda_type(fieldinfo.get_gdatype()); //Could be TYPE_INVALID if the gda type is not one of ours.

  Glib::ustring strType = Field::get_type_name( fieldType );
  m_AddDel.set_value(iter, m_colType, strType);

  //Unique:
  bool bUnique = fieldinfo.get_unique_key();

  m_AddDel.set_value(iter, m_colUnique, bUnique);

  //Primary Key:
  bool bPrimaryKey = fieldinfo.get_primary_key();
  m_AddDel.set_value(iter, m_colPrimaryKey, bPrimaryKey);
}

void Box_DB_Table_Definition::fill_from_database()
{
  Box_DB_Table::fill_from_database();
  
  try
  {
    //Fields:
    m_AddDel.remove_all();

    Field::type_map_type_names mapFieldTypes = Field::get_type_names();

    for(type_vecFields::iterator iter = m_Fields.begin(); iter != m_Fields.end(); iter++)
    {       
      const Field& field = *iter;
             
      //Name:
      Gtk::TreeModel::iterator iter= m_AddDel.add_item(field.get_name());
      fill_field_row(iter, field);
    }
  }
  catch(std::exception& ex)
  {
    handle_error(ex);
  }

  fill_end();
}

void Box_DB_Table_Definition::on_AddDel_add(const Gtk::TreeModel::iterator& row)
{
  Glib::ustring strName = m_AddDel.get_value(row, m_colName);
  if(!strName.empty())
  {
    bool bTest = Query_execute( "ALTER TABLE " + m_strTableName + " ADD " + strName + " NUMERIC" ); //TODO: Get schema type for Field::TYPE_NUMERIC
    if(bTest)
    {
      //Show the new field (fill in the other cells):
      
      fill_fields();

      //fill_from_database(); //We can not change the structure in a cell renderer signal handler.

      //This must match the SQL statement above:
      Field field;
      field.set_name(strName);
      field.set_glom_type(Field::TYPE_NUMERIC);

      Gnome::Gda::FieldAttributes field_info = field.get_field_info();
      field_info.set_gdatype( Field::get_gda_type_for_glom_type(Field::TYPE_NUMERIC) );
      field.set_field_info(field_info);
      
      fill_field_row(row, field);
      
      //m_AddDel.select_item(row, m_colTitle, true); //Start editing the title
    }
  }
}

void Box_DB_Table_Definition::on_AddDel_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator&  rowEnd)
{
  for(Gtk::TreeModel::iterator iter = rowStart; iter != rowEnd; ++iter)
  {
    Glib::ustring strName = m_AddDel.get_value_key(iter);
    if(!strName.empty())
    {
      Query_execute( "ALTER TABLE " + m_strTableName + " DROP COLUMN " + strName );
    }
  }

  fill_from_database();
}

void Box_DB_Table_Definition::on_AddDel_changed(const Gtk::TreeModel::iterator& row, guint /* col */)
{
  //Get old field definition:
  Document_Glom* pDoc = static_cast<Document_Glom*>(get_document());
  if(pDoc)
  {
    //Glom-specific stuff: //TODO_portiter
    const Glib::ustring strFieldNameBeingEdited = m_AddDel.get_value_key(row);
    
    pDoc->get_field(m_strTableName, strFieldNameBeingEdited, m_Field_BeingEdited);

    //Get DB field info: (TODO: This might be unnecessary).
    type_vecFields::const_iterator iterFind = std::find_if( m_Fields.begin(), m_Fields.end(), predicate_FieldHasName<Field>(strFieldNameBeingEdited) );
    if(iterFind != m_Fields.end()) //If it was found:
    {
      m_Field_BeingEdited = *iterFind;

      //Get new field definition:
      Field fieldNew = get_field_definition(row);
       
      //Change it:
      if(m_Field_BeingEdited != fieldNew) //If it has really changed.
      {
        //If we are changing a non-glom type:
       //Refuse to edit field definitions that were not created by glom:
       if(Field::get_glom_type_for_gda_type( m_Field_BeingEdited.get_field_info().get_gdatype() )  == Field::TYPE_INVALID)
       {
         Gtk::MessageDialog dialog(gettext("This database field was created or edited outside of Glom. It has a data type that is not supported by Glom. Your system administrator may be able to correct this."));
         dialog.set_transient_for(*get_app_window());
         dialog.run();
       }
       else
         change_definition(m_Field_BeingEdited, fieldNew);
      }
    }
  }
}

void Box_DB_Table_Definition::on_AddDel_edit(const Gtk::TreeModel::iterator& row)
{
  m_Field_BeingEdited = get_field_definition(row);
  
  m_pDialog->set_field(m_Field_BeingEdited); //By-value.
 
  //m_pDialog->set_modified(false); //Disable [Apply] at start.
  
  m_pDialog->show_all();
}

Field Box_DB_Table_Definition::get_field_definition(const Gtk::TreeModel::iterator& row)
{
  Field fieldResult;

  //Get old field definition (to preserve anything that the user doesn't have access to):

  //Glom field definition:
  Document_Glom* pDoc = static_cast<Document_Glom*>(get_document());
  if(pDoc)
  {
    const Glib::ustring& strFieldNameBeforeEdit = m_AddDel.get_value(row, 0);

    Document_Glom::type_vecFields vecFields= pDoc->get_table_fields(m_strTableName);
    Document_Glom::type_vecFields::iterator iterFind = std::find_if( vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(strFieldNameBeforeEdit) );
    if(iterFind != vecFields.end()) //If it was found:
    {
      fieldResult = *iterFind;
    }
    else
    {
      //Start with a default:
      fieldResult = Field();
    }
  }

  //DB field definition:

  //Start with original definitions, so that we preserve things like UNSIGNED.
  //TODO maybe use document's fieldinfo instead of m_Fields.
  Field field_temp = m_Fields[row];
  Gnome::Gda::FieldAttributes fieldInfo = field_temp.get_field_info();

  //Name:
  Glib::ustring strName = m_AddDel.get_value(row, m_colName);
  fieldInfo.set_name(strName);

  //Title:
  Glib::ustring title = m_AddDel.get_value(row, m_colTitle);
  fieldResult.set_title(title);

  //Type:
  const Glib::ustring& strType = m_AddDel.get_value(row, m_colType);

  Field::glom_field_type glom_type =  Field::get_type_for_name(strType);
  Gnome::Gda::ValueType fieldType = Field::get_gda_type_for_glom_type(glom_type);
  
  //Unique:
  bool bUnique = m_AddDel.get_value_as_bool(row, m_colUnique);
  fieldInfo.set_unique_key(bUnique);

  //Primary Key:
  bool bPrimaryKey = m_AddDel.get_value_as_bool(row, m_colPrimaryKey);
  fieldInfo.set_primary_key(bPrimaryKey);

  fieldInfo.set_gdatype(fieldType);

  //Put it together:
  fieldResult.set_field_info(fieldInfo);
      
  return fieldResult;
}

void Box_DB_Table_Definition::on_Properties_apply()
{
  Field field_New = m_pDialog->get_field();

  if(m_Field_BeingEdited != field_New)
  {
    //If we are changing a non-glom type:
    //Refuse to edit field definitions that were not created by glom:
    if( Field::get_glom_type_for_gda_type(m_Field_BeingEdited.get_field_info().get_gdatype()) == Field::TYPE_INVALID )
    {
      Gtk::MessageDialog dialog(gettext("This database field was created or edited outside of Glom. It has a data type that is not supported by Glom. Your system administrator may be able to correct this."));
      dialog.set_transient_for(*get_app_window());
      dialog.run();
    }
    else
    {
      change_definition(m_Field_BeingEdited, field_New);
      m_Field_BeingEdited = field_New;
    }

    //Update the list:
    fill_from_database();
  }

  m_pDialog->hide();
}

void Box_DB_Table_Definition::change_definition(const Field& fieldOld, Field field)
{
  Bakery::BusyCursor(*get_app_window());
    
  //DB field defintion:

  postgres_change_column(fieldOld, field);

   /*
    //Get a data model:
    Gnome::Gda::Command command("SELECT * FROM " + m_strTableName + " LIMIT 1"); //LIMIT 1 means we get only the first row.
    Glib::RefPtr<Gnome::Gda::DataModel> data_model = gda_connection->execute_single_command(command);

    if(data_model)
    {
      Gnome::Gda::FieldAttributes field_info = field.get_field_info();
      bool test = data_model->update_column(field_info);
      if(!test)
      {
        g_warning("Box_DB_Table_Definition::change_definition(): libgda update_column() failed.");
        handle_error();
      }
      else
        g_warning("Box_DB_Table_Definition::change_definition(): libgda says that update_column() succeeded.");
    }
  }
  */

  
  /* Old MySQL stuff:
  if(fieldInfoOld.get_primary_key())
  {
    //If we are modifying a primary key then we first need to stop it from being the primary key.
    //Otherwise mysql complains that there is a conflict. Silly mysql.

    Gnome::Gda::FieldAttributes fieldInfoNonPrimary = fieldInfoOld;
    fieldInfoNonPrimary.set_primary_key(false);
    fieldInfoNonPrimary.set_auto_increment(false);
    Query_execute( "ALTER TABLE " + m_strTableName + " CHANGE " + strFieldNameOld + " " + fieldInfoNonPrimary.get_name() + " " + get_field_definition_for_sql(fieldInfoNonPrimary) );

    Query_execute( "ALTER TABLE " + m_strTableName + " DROP PRIMARY KEY");
  }
  else if(fieldInfoOld.get_unique_key() && fieldInfo.get_unique_key())
  {
    //It doesn't seem possible to turn off UNIQUE.
    //And specifying it a second time gives an error.

    fieldInfo.set_unique_key(false);
  }
  */

   //MySQL does this all with ALTER_TABLE, with "CHANGE" followed by the same details used with "CREATE TABLE",
   //MySQL also makes it easier to change the type.
   // but Postgres uses various subcommands, such as  "ALTER COLUMN", and "RENAME".
  
    
  //Extra Glom field definitions:
  Document_Glom* pDoc = static_cast<Document_Glom*>(get_document());
  if(pDoc)
  {
    //Get Table's fields:
    Document_Glom::type_vecFields vecFields = pDoc->get_table_fields(m_strTableName);
    
    //Find old field:
    const Glib::ustring field_name_old = fieldOld.get_name();
    Document_Glom::type_vecFields::iterator iterFind = std::find_if( vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(field_name_old) );
    if(iterFind != vecFields.end()) //If it was found:
    {
      //Change it to the new Fields's value:
      Field& refField = *iterFind;
      refField = field;
    }
    else
    {
      //Add it, because it's not there already:
      vecFields.push_back(field);
    }

    pDoc->set_table_fields(m_strTableName, vecFields);

    //Update field names where they are used in relationships.
    if(field_name_old != field.get_name())
      pDoc->change_field_name(m_strTableName, field_name_old, field.get_name());
  }


  //Update UI:

  fill_fields();
  //fill_from_database(); //We should not change the database definition in a cell renderer signal handler.

  //Select the same field again:
  m_AddDel.select_item(field.get_name(), m_colName, false);
}

void Box_DB_Table_Definition::fill_fields()
{
  m_Fields = get_fields_for_table(m_strTableName);
}

void  Box_DB_Table_Definition::postgres_add_column(const Field& field, bool not_extras)
{
  bool bTest = Query_execute(  "ALTER TABLE " + m_strTableName + " ADD " + field.get_name() + " " +  field.get_sql_type() );
  if(bTest)
  {
    if(not_extras)
    {
      //We must do this separately:
      postgres_change_column_extras(field, field, true /* set them even though the fields are the same */);
    }
  }
}

void  Box_DB_Table_Definition::postgres_change_column(const Field& field_old, const Field& field)
{
  Gnome::Gda::FieldAttributes field_info = field.get_field_info();
  Gnome::Gda::FieldAttributes field_info_old = field_old.get_field_info();
  
  //If the underlying data type has changed:
  if( field_info.get_gdatype() != field_info_old.get_gdatype() )
  {
    postgres_change_column_type(field_old, field); //This will also change everything else at the same time.
  }
  else
  {      
    //Change other stuff, without changing the type:
    postgres_change_column_extras(field_old, field);
  }
}
  
void  Box_DB_Table_Definition::postgres_change_column_type(const Field& field_old, const Field& field)
{
  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

    bool new_column_created = false;
    
    //If the datatype has changed:
    if(field.get_field_info().get_gdatype() != field_old.get_field_info().get_gdatype())
    {
      //We have to create a new table, and move the data across:
      //See http://www.postgresql.org/docs/faqs/FAQ.html#4.4
      //     BEGIN;
      //
      //    UPDATE tab SET new_col = CAST(old_col AS new_data_type);
      //    ALTER TABLE tab DROP COLUMN old_col;
      //    COMMIT;

      Glib::RefPtr<Gnome::Gda::Transaction> transaction = Gnome::Gda::Transaction::create("glom_transaction_change_field_type");
      bool test = gda_connection->begin_transaction(transaction);
      if(test)
      {
        //TODO: Warn about a delay, and possible loss of precision, before actually doing this.
        //TODO: Try to use a unique name for the temp column:
     
        Field fieldTemp = field;
        fieldTemp.set_name("glom_temp_column");
        postgres_add_column(fieldTemp); //This might also involves several commands.


        //TODO: postgres seems to give an error if the data can not be converted (for instance if the text is not a numeric digit when converting to numeric) instead of using 0.
        /*
        Maybe, for instance:
        http://groups.google.de/groups?hl=en&lr=&ie=UTF-8&frame=right&th=a7a62337ad5a8f13&seekm=23739.1073660245%40sss.pgh.pa.us#link5
        UPDATE _table
        SET _bbb = to_number(substring(_aaa from 1 for 5), '99999')
        WHERE _aaa <> '     ';  
        */
        Glib::ustring conversion_command;
        switch(field.get_glom_type())
        {
          case Field::TYPE_NUMERIC: //CAST does not work if the destination type is numeric.
          {
            conversion_command = "to_number( " + field_old.get_name() + ", '999999999.99' )";
            break;
          }
          case Field::TYPE_DATE: //CAST does not work if the destination type is numeric.
          {
            conversion_command = "to_date( " + field_old.get_name() + ", 'YYYYMMDD' )"; //TODO: standardise date storage format.
            break;
          }
          case Field::TYPE_TIME: //CAST does not work if the destination type is numeric.
          {
            conversion_command = "to_timestamp( " + field_old.get_name() + ", 'HHMMSS' )";  //TODO: standardise time storage format.
            break;
          }
          default:
          {
            conversion_command = "CAST(" +  field_old.get_name() + " AS " + field.get_sql_type() + ")";
            break;
          }
        }
          
          
        Glib::RefPtr<Gnome::Gda::DataModel> datamodel = Query_execute( "UPDATE " + m_strTableName + " SET  " + fieldTemp.get_name() + " = " + conversion_command );  //TODO: Not full type details.
        if(datamodel)
        {
          datamodel = Query_execute( "ALTER TABLE " + m_strTableName + " DROP COLUMN " +  field_old.get_name() );
          if(datamodel)
          {
            datamodel = Query_execute( "ALTER TABLE " + m_strTableName + " RENAME COLUMN " + fieldTemp.get_name() + " TO " + field.get_name() );
            if(datamodel)
            {
               bool test = gda_connection->commit_transaction(transaction);
               if(!test)
               {
                 handle_error();
               }
               else
                 new_column_created = true;
             }
           }
         }   //TODO: Abandon the transaction if something failed.
       }
     }  /// If the datatype has changed:

    if(!new_column_created) //We don't need to change anything else if everything was alrady correctly set as a new column:
      postgres_change_column_extras(field_old, field);
  }
}


void Box_DB_Table_Definition::postgres_change_column_extras(const Field& field_old, const Field& field, bool set_anyway)
{
  Gnome::Gda::FieldAttributes field_info = field.get_field_info();
  Gnome::Gda::FieldAttributes field_info_old = field_old.get_field_info();

  if(field_info.get_name() != field_info_old.get_name())
  {
     Glib::RefPtr<Gnome::Gda::DataModel>  datamodel = Query_execute( "ALTER TABLE " + m_strTableName + " RENAME COLUMN " + field_info_old.get_name() + " TO " + field_info.get_name() );
     if(!datamodel)
     {
       handle_error();
       return;
     }
  }

  if(set_anyway || (field_info.get_primary_key() != field_info_old.get_primary_key()))
  {
    //TODO: Check that there is only one primary key.
    Glib::ustring add_or_drop = "ADD";
    if(field_info_old.get_primary_key() == false)
      add_or_drop = "DROP";

    Glib::RefPtr<Gnome::Gda::DataModel>  datamodel = Query_execute( "ALTER TABLE " + m_strTableName + " " + add_or_drop + " PRIMARY KEY (" + field_info.get_name() + ")");
    if(!datamodel)
    {
      handle_error();
      return;
    }
  }

  if( !field_info.get_primary_key() ) //Postgres automatically makes primary keys unique, so we do not need to do that separately if we have already made it a primary key
  {
    if(set_anyway || (field_info.get_unique_key() != field_info_old.get_unique_key()))
    {
       /* TODO: Is there an easier way than adding an index manually?
       Glib::RefPtr<Gnome::Gda::DataModel>  datamodel = Query_execute( "ALTER TABLE " + m_strTableName + " RENAME COLUMN " + field_info_old.get_name() + " TO " + field_info.get_name() );
       if(!datamodel)
       {
         handle_error();
         return;
       }
       */
    }

    Gnome::Gda::Value default_value = field_info.get_default_value();
    Gnome::Gda::Value default_value_old = field_info_old.get_default_value();

    if(!field_info.get_auto_increment()) //Postgres auto-increment fields have special code as their default values.
    {
      if(set_anyway || (default_value != default_value_old))
      {
        Glib::RefPtr<Gnome::Gda::DataModel> datamodel = Query_execute( "ALTER TABLE " + m_strTableName + " ALTER COLUMN "+ field_info.get_name() + " SET DEFAULT " + field.sql(field_info.get_default_value()) );
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
    if( set_anyway ||  (field.get_field_info().get_allow_null() != field_old.get_field_info().get_allow_null()) )
    {
      Glib::ustring nullness = (field.get_field_info().get_allow_null() ? "NULL" : "NOT NULL");
      Query_execute(  "ALTER TABLE " + m_strTableName + " ALTER COLUMN " + field.get_name() + "  SET " + nullness);
    }
  */
   
  
}



