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
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glom/glom_postgres.h>
#include "../../../config.h"
#include <glibmm/i18n.h>

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
  //m_strHint = _("Click [Edit] to edit the field definition in more detail.\nUse the Mode menu to see Data or perform a Find.");

  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_field_definition_edit");
  if(refXml)
    refXml->get_widget_derived("window_field_definition_edit", m_pDialog);

  add_view(m_pDialog); //Give it access to the document.

  pack_start(m_AddDel);
  m_colName = m_AddDel.add_column(_("Name"));
  m_AddDel.prevent_duplicates(m_colName); //Don't allow adding of fields that already exist.
  m_AddDel.set_prevent_duplicates_warning(_("This field already exists. Please choose a different field name"));

  m_colTitle = m_AddDel.add_column(_("Title"));

  m_colType = m_AddDel.add_column(_("Type"), AddDelColumnInfo::STYLE_Choices);
  m_AddDel.set_column_width(m_colType, 100); //TODO: Auto-size columns.

  //Set Type choices:

  Field::type_map_type_names mapFieldTypes = Field::get_usable_type_names();
  AddDel::type_vecStrings vecTypes;
  for(Field::type_map_type_names ::iterator iter = mapFieldTypes.begin(); iter != mapFieldTypes.end();++iter)
  {
    const Glib::ustring& name = (*iter).second;
    vecTypes.push_back(name);
  }

  m_AddDel.set_column_choices(m_colType, vecTypes);

  m_colUnique = m_AddDel.add_column("Unique", AddDelColumnInfo::STYLE_Boolean);
  m_colPrimaryKey = m_AddDel.add_column("Primary Key", AddDelColumnInfo::STYLE_Boolean);

  //Connect signals:
  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_adddel_add));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_adddel_delete));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_adddel_changed));
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_adddel_edit));

  //React to changes in the field properties:
  m_pDialog->signal_apply().connect(sigc::mem_fun(*this, &Box_DB_Table_Definition::on_Properties_apply));
}

Box_DB_Table_Definition::~Box_DB_Table_Definition()
{
  if(m_pDialog)
  {
    remove_view(m_pDialog);
    delete m_pDialog;
  }
}

void Box_DB_Table_Definition::fill_field_row(const Gtk::TreeModel::iterator& iter, const sharedptr<const Field>& field)
{
  m_AddDel.set_value_key(iter, field->get_name());

  m_AddDel.set_value(iter, m_colName, field->get_name());

  const Glib::ustring title = field->get_title();
  m_AddDel.set_value(iter, m_colTitle, title);

  //Type:
  Field::glom_field_type fieldType = Field::get_glom_type_for_gda_type(field->get_field_info().get_gdatype()); //Could be TYPE_INVALID if the gda type is not one of ours.

  const Glib::ustring strType = Field::get_type_name_ui( fieldType );
  m_AddDel.set_value(iter, m_colType, strType);

  //Unique:
  const bool bUnique = field->get_unique_key();

  m_AddDel.set_value(iter, m_colUnique, bUnique);

  //Primary Key:
  const bool bPrimaryKey = field->get_primary_key();
  m_AddDel.set_value(iter, m_colPrimaryKey, bPrimaryKey);
}

bool Box_DB_Table_Definition::fill_from_database()
{
  bool result = Box_DB_Table::fill_from_database();

  if(!(ConnectionPool::get_instance()->get_ready_to_connect()))
    return false;

  fill_fields();

  try
  {
    //Fields:
    m_AddDel.remove_all();

    Field::type_map_type_names mapFieldTypes = Field::get_type_names_ui();

    for(type_vecFields::iterator iter = m_vecFields.begin(); iter != m_vecFields.end(); iter++)
    {
      const sharedptr<const Field>& field = *iter;

      //Name:
      Gtk::TreeModel::iterator iter= m_AddDel.add_item(field->get_name());
      fill_field_row(iter, field);
    }

    result = true;
  }
  catch(const std::exception& ex)
  {
    handle_error(ex);
    result = false;
  }

  fill_end();

  return result;
}

void Box_DB_Table_Definition::on_adddel_add(const Gtk::TreeModel::iterator& row)
{
  Glib::ustring name = m_AddDel.get_value(row, m_colName);
  if(!name.empty())
  {
    bool bTest = query_execute( "ALTER TABLE \"" + m_table_name + "\" ADD \"" + name + "\" NUMERIC", get_app_window()); //TODO: Get schema type for Field::TYPE_NUMERIC
    if(bTest)
    {
      //Show the new field (fill in the other cells):

      fill_fields();

      //fill_from_database(); //We cannot change the structure in a cell renderer signal handler.

      //This must match the SQL statement above:
      sharedptr<Field> field(new Field());
      field->set_name(name);
      field->set_title( util_title_from_string(name) ); //Start with a title that might be useful.
      field->set_glom_type(Field::TYPE_NUMERIC);

      Gnome::Gda::FieldAttributes field_info = field->get_field_info();
      field_info.set_gdatype( Field::get_gda_type_for_glom_type(Field::TYPE_NUMERIC) );
      field->set_field_info(field_info);

      fill_field_row(row, field);

      //Store the generated title in the document:
      on_adddel_changed(row, m_colTitle);

      //m_AddDel.select_item(row, m_colTitle, true); //Start editing the title
    }
  }
}

void Box_DB_Table_Definition::on_adddel_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd)
{
  Gtk::TreeModel::iterator iterAfterEnd = rowEnd;
  if(iterAfterEnd != m_AddDel.get_model()->children().end())
    ++iterAfterEnd;

  for(Gtk::TreeModel::iterator iter = rowStart; iter != iterAfterEnd; ++iter)
  {
    Glib::ustring name = m_AddDel.get_value_key(iter);
    if(!name.empty())
    {
      query_execute( "ALTER TABLE \"" + m_table_name + "\" DROP COLUMN \"" + name + "\"", get_app_window());

      //Remove it from all layouts, reports, etc:
      get_document()->remove_field(m_table_name, name);
    }
  }

  fill_fields();
  fill_from_database();
}

void Box_DB_Table_Definition::on_adddel_changed(const Gtk::TreeModel::iterator& row, guint /* col */)
{
  //Get old field definition:
  Document_Glom* pDoc = static_cast<Document_Glom*>(get_document());
  if(pDoc)
  {
    const Glib::ustring strFieldNameBeingEdited = m_AddDel.get_value_key(row);

    sharedptr<const Field> constfield = pDoc->get_field(m_table_name, strFieldNameBeingEdited);
    m_Field_BeingEdited = constfield;

    //Get DB field info: (TODO: This might be unnecessary).
    type_vecFields::const_iterator iterFind = std::find_if( m_vecFields.begin(), m_vecFields.end(), predicate_FieldHasName<Field>(strFieldNameBeingEdited) );
    if(iterFind != m_vecFields.end()) //If it was found:
    {
      sharedptr<const Field> constfield = *iterFind;
      m_Field_BeingEdited = constfield;

      //Get new field definition:
      sharedptr<Field> fieldNew = get_field_definition(row);

      //Change it:
      if(*m_Field_BeingEdited != *fieldNew) //If it has really changed.
      {
        //If we are changing a non-glom type:
        //Refuse to edit field definitions that were not created by glom:
        if(Field::get_glom_type_for_gda_type( m_Field_BeingEdited->get_field_info().get_gdatype() )  == Field::TYPE_INVALID)
        {
          Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Invalid database structure")), true);
          dialog.set_secondary_text(_("This database field was created or edited outside of Glom. It has a data type that is not supported by Glom. Your system administrator may be able to correct this."));
          //TODO: dialog.set_transient_for(*get_application());
          dialog.run();
        }
        else
        {
          sharedptr<Field> fieldNewWithModifications = change_definition(m_Field_BeingEdited, fieldNew);

          //Update the row to show any extra changes (such as unique being set/unset whenever the primary key is set/unset) 
          fill_field_row(row, fieldNewWithModifications);
        }
      }
    }
  }
}

void Box_DB_Table_Definition::on_adddel_edit(const Gtk::TreeModel::iterator& row)
{
  sharedptr<const Field> constfield = get_field_definition(row);
  m_Field_BeingEdited = constfield;

  m_pDialog->set_field(m_Field_BeingEdited, m_table_name);

  //m_pDialog->set_modified(false); //Disable [Apply] at start.

  m_pDialog->show();
}

sharedptr<Field> Box_DB_Table_Definition::get_field_definition(const Gtk::TreeModel::iterator& row)
{
  sharedptr<Field> fieldResult;

  //Get old field definition (to preserve anything that the user doesn't have access to):

  const Glib::ustring strFieldNameBeforeEdit = m_AddDel.get_value_key(row);

  //Glom field definition:
  Document_Glom* pDoc = static_cast<Document_Glom*>(get_document());
  if(pDoc)
  {
    Document_Glom::type_vecFields vecFields= pDoc->get_table_fields(m_table_name);
    Document_Glom::type_vecFields::iterator iterFind = std::find_if( vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(strFieldNameBeforeEdit) );

    if((iterFind != vecFields.end()) && (*iterFind)) //If it was found:
    {
      fieldResult = glom_sharedptr_clone(*iterFind);
    }
    else
    {
      //Start with a default:
      fieldResult = sharedptr<Field>(new Field());
    }
  }


  //DB field definition:

  //Start with original definitions, so that we preserve things like UNSIGNED.
  //TODO maybe use document's fieldinfo instead of m_vecFields.
  sharedptr<const Field> field_temp = get_fields_for_table_one_field(m_table_name, strFieldNameBeforeEdit);
  if(field_temp)
  {
    Gnome::Gda::FieldAttributes fieldInfo = field_temp->get_field_info();

    //Name:
    const Glib::ustring name = m_AddDel.get_value(row, m_colName);
    fieldInfo.set_name(name);

    //Title:
    const Glib::ustring title = m_AddDel.get_value(row, m_colTitle);
    fieldResult->set_title(title);

    //Type:
    const Glib::ustring& strType = m_AddDel.get_value(row, m_colType);

    const Field::glom_field_type glom_type =  Field::get_type_for_ui_name(strType);
    Gnome::Gda::ValueType fieldType = Field::get_gda_type_for_glom_type(glom_type);

    //Unique:
    const bool bUnique = m_AddDel.get_value_as_bool(row, m_colUnique);
    fieldInfo.set_unique_key(bUnique);

    //Primary Key:
    const bool bPrimaryKey = m_AddDel.get_value_as_bool(row, m_colPrimaryKey);
    fieldInfo.set_primary_key(bPrimaryKey);

    fieldInfo.set_gdatype(fieldType);

    //Put it together:
    fieldResult->set_field_info(fieldInfo);
  }

  return fieldResult;
}

void Box_DB_Table_Definition::on_Properties_apply()
{
  sharedptr<Field> field_New = m_pDialog->get_field();

  if(*m_Field_BeingEdited != *field_New)
  {
    //If we are changing a non-glom type:
    //Refuse to edit field definitions that were not created by glom:
    if( Field::get_glom_type_for_gda_type(m_Field_BeingEdited->get_field_info().get_gdatype()) == Field::TYPE_INVALID )
    {
      Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Invalid database structure")), true);
         dialog.set_secondary_text(_("This database field was created or edited outside of Glom. It has a data type that is not supported by Glom. Your system administrator may be able to correct this."));
      //TODO: dialog.set_transient_for(*get_app_window());
      dialog.run();
    }
    else
    {
      //Warn about recalculation:
      bool change_def = true;
      if(field_New->get_has_calculation())
      {
        if(field_New->get_calculation() != m_Field_BeingEdited->get_calculation())
        {
          //TODO: Only show this when there are > 100 records?
          Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Recalculation Required")), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
          dialog.set_secondary_text(_("You have changed the calculation used by this field so Glom must recalculate the value in all records. If the table contains many records then this could take a long time."));
          //dialog.set_transient_for(*get_app_window());
          dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
          dialog.add_button(_("Recalculate"), Gtk::RESPONSE_OK);
          change_def = (dialog.run() == Gtk::RESPONSE_OK);
        }
      }

      if(change_def)
      {
        change_definition(m_Field_BeingEdited, field_New);
        m_Field_BeingEdited = field_New;
      }
    }

    //Update the list:
    fill_from_database();
  }

  m_pDialog->hide();
}

sharedptr<Field> Box_DB_Table_Definition::change_definition(const sharedptr<const Field>& fieldOld, const sharedptr<const Field>& field)
{
  Bakery::BusyCursor busy_cursor(get_app_window());

  //DB field defintion:

  sharedptr<Field> result;

  try
  {
    result = postgres_change_column(fieldOld, field);
  }
  catch(const std::exception& ex) //In case the database reports an error.
  {
    handle_error(ex);

    //Give up. Don't update the document. Hope that we can read the current field properties from the database.
    fill_fields();
    //fill_from_database(); //We should not change the database definition in a cell renderer signal handler.

    //Select the same field again:
    m_AddDel.select_item(field->get_name(), m_colName, false);

    return glom_sharedptr_clone(field); 
  }

   //MySQL does this all with ALTER_TABLE, with "CHANGE" followed by the same details used with "CREATE TABLE",
   //MySQL also makes it easier to change the type.
   // but Postgres uses various subcommands, such as  "ALTER COLUMN", and "RENAME".

  //Extra Glom field definitions:
  Document_Glom* pDoc = static_cast<Document_Glom*>(get_document());
  if(pDoc)
  {
    //Get Table's fields:
    Document_Glom::type_vecFields vecFields = pDoc->get_table_fields(m_table_name);

    //Find old field:
    const Glib::ustring field_name_old = fieldOld->get_name();
    Document_Glom::type_vecFields::iterator iterFind = std::find_if( vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(field_name_old) );
    if(iterFind != vecFields.end()) //If it was found:
    {
      //Change it to the new Fields's value:
      sharedptr<Field> refField = *iterFind;
      *refField = *result; //Remember, result is field with any necessary changes due to constraints.
    }
    else
    {
      //Add it, because it's not there already:
      vecFields.push_back( glom_sharedptr_clone(result) );
    }

    pDoc->set_table_fields(m_table_name, vecFields);

    //Update field names where they are used in relationships or on layouts:
    if(field_name_old != field->get_name())
    {
      pDoc->change_field_name(m_table_name, field_name_old, field->get_name());
    }

    //Recalculate if necessary:
    if(field->get_has_calculation())
    {
      const Glib::ustring calculation = field->get_calculation();
      if(calculation != fieldOld->get_calculation())
        calculate_field_in_all_records(m_table_name, field);
    }
  }


  //Update UI:

  fill_fields();
  //fill_from_database(); //We should not change the database definition in a cell renderer signal handler.

  //Select the same field again:
  m_AddDel.select_item(field->get_name(), m_colName, false);

  return result;
}

void Box_DB_Table_Definition::fill_fields()
{
  m_vecFields = get_fields_for_table(m_table_name);
}

sharedptr<Field> Box_DB_Table_Definition::postgres_change_column(const sharedptr<const Field>& field_old, const sharedptr<const Field>& field)
{
  const Gnome::Gda::FieldAttributes field_info = field->get_field_info();
  const Gnome::Gda::FieldAttributes field_info_old = field_old->get_field_info();

  //If the underlying data type has changed:
  if( field_info.get_gdatype() != field_info_old.get_gdatype() )
  {
    postgres_change_column_type(field_old, field); //This will also change everything else at the same time.
    return glom_sharedptr_clone(field);
  }
  else
  {
    //Change other stuff, without changing the type:
    return GlomPostgres::postgres_change_column_extras(m_table_name, field_old, field);
  }
}


void  Box_DB_Table_Definition::postgres_change_column_type(const sharedptr<const Field>& field_old, const sharedptr<const Field>& field)
{
  sharedptr<SharedConnection> sharedconnection = connect_to_server(get_app_window());
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

    bool new_column_created = false;

    //If the datatype has changed:
    if(field->get_field_info().get_gdatype() != field_old->get_field_info().get_gdatype())
    {
      //We have to create a new table, and move the data across:
      //See http://www.postgresql.org/docs/faqs/FAQ.html#4.4
      //     BEGIN;
      //
      //    UPDATE tab SET new_col = CAST(old_col AS new_data_type);
      //    ALTER TABLE tab DROP COLUMN old_col;
      //    COMMIT;

      Glib::RefPtr<Gnome::Gda::Transaction> transaction = Gnome::Gda::Transaction::create("glom_transaction_change_field_type");
      const bool test = gda_connection->begin_transaction(transaction);
      if(test)
      {
        //TODO: Warn about a delay, and possible loss of precision, before actually doing this.
        //TODO: Try to use a unique name for the temp column:

        sharedptr<Field> fieldTemp = glom_sharedptr_clone(field);
        fieldTemp->set_name("glom_temp_column");
        GlomPostgres::postgres_add_column(m_table_name, fieldTemp); //This might also involves several commands.


        bool conversion_failed = false;
        if(Field::get_conversion_possible(field_old->get_glom_type(), field->get_glom_type()))
        {
          //TODO: postgres seems to give an error if the data cannot be converted (for instance if the text is not a numeric digit when converting to numeric) instead of using 0.
          /*
          Maybe, for instance:
          http://groups.google.de/groups?hl=en&lr=&ie=UTF-8&frame=right&th=a7a62337ad5a8f13&seekm=23739.1073660245%40sss.pgh.pa.us#link5
          UPDATE _table
          SET _bbb = to_number(substring(_aaa from 1 for 5), '99999')
          WHERE _aaa <> '     ';  
          */
          Glib::ustring conversion_command;
          const Glib::ustring field_name_old_quoted = "\"" + field_old->get_name() + "\"";
          const Field::glom_field_type old_field_type = field_old->get_glom_type();
          switch(field->get_glom_type())
          {
            case Field::TYPE_BOOLEAN: //CAST does not work if the destination type is boolean.
            {
              if(old_field_type == Field::TYPE_NUMERIC)
              {
                conversion_command = "(CASE WHEN " + field_name_old_quoted + " >0 THEN true "
                                           "WHEN " + field_name_old_quoted + " = 0 THEN false "
                                           "WHEN " + field_name_old_quoted + " IS NULL THEN false END)";
              }
              else if(old_field_type == Field::TYPE_TEXT)
                conversion_command = "(" + field_name_old_quoted + " !~~* \'false\')"; // !~~* means ! ILIKE.
              else //Dates and Times:
                conversion_command = "(" + field_name_old_quoted + " IS NOT NULL')";

              break;
            }
            case Field::TYPE_NUMERIC: //CAST does not work if the destination type is numeric.
            {
              if(old_field_type == Field::TYPE_BOOLEAN)
              {
                conversion_command = "(CASE WHEN " + field_name_old_quoted + " = true THEN 1 "
                                           "WHEN " + field_name_old_quoted + " = false THEN 0 "
                                           "WHEN " + field_name_old_quoted + " IS NULL THEN 0 END)";
              }
              else
              {
                //We use to_number, with textcat() so that to_number always has usable data.
                //Otherwise, it says 
                //invalid input syntax for type numeric: " "
                //
                //We must use single quotes with the 0, otherwise it says "column 0 does not exist.".
                conversion_command = "to_number( textcat(\'0\', " + field_name_old_quoted + "), '999999999.99999999' )";
              }

              break;
            }
            case Field::TYPE_DATE: //CAST does not work if the destination type is numeric.
            {
              conversion_command = "to_date( " + field_name_old_quoted + ", 'YYYYMMDD' )"; //TODO: standardise date storage format.
              break;
            }
            case Field::TYPE_TIME: //CAST does not work if the destination type is numeric.
            {
              conversion_command = "to_timestamp( " + field_name_old_quoted + ", 'HHMMSS' )";  //TODO: standardise time storage format.
              break;
            }
            default:
            {
              //To Text:

              //bool to text:
              if(old_field_type == Field::TYPE_BOOLEAN)
              {
                 conversion_command = "(CASE WHEN " + field_name_old_quoted + " = true THEN \'true\' "
                                            "WHEN " + field_name_old_quoted + " = false THEN \'false\' "
                                            "WHEN " + field_name_old_quoted + " IS NULL THEN \'false\' END)";
              }
              else
              {
                //This works for most to-text conversions:
                conversion_command = "CAST(\"" +  field_old->get_name() + "\" AS " + field->get_sql_type() + ")";
              }
              break;
            }
          }

          Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute( "UPDATE \"" + m_table_name + "\" SET \"" + fieldTemp->get_name() + "\" = " + conversion_command, get_app_window());  //TODO: Not full type details.
          if(!datamodel)
            conversion_failed = true;
        }

        if(!conversion_failed)
        {
          Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute( "ALTER TABLE \"" + m_table_name + "\" DROP COLUMN \"" +  field_old->get_name() + "\"", get_app_window());
          if(datamodel)
          {
            datamodel = query_execute( "ALTER TABLE \"" + m_table_name + "\" RENAME COLUMN \"" + fieldTemp->get_name() + "\" TO \"" + field->get_name() + "\"", get_app_window());
            if(datamodel)
            {
              const bool test = gda_connection->commit_transaction(transaction);
              if(!test)
              {
                handle_error();
              }
              else
                new_column_created = true;
            }
          }
        }
      }   //TODO: Abandon the transaction if something failed.
    }  /// If the datatype has changed:

    if(!new_column_created) //We don't need to change anything else if everything was alrady correctly set as a new column:
      GlomPostgres::postgres_change_column_extras(m_table_name, field_old, field);
  }
}




