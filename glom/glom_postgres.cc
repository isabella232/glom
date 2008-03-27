/* Glom
 *
 * Copyright (C) 2001-2006 Murray Cumming
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

#include "glom_postgres.h"

namespace Glom
{

bool GlomPostgres::postgres_add_column(const Glib::ustring& table_name, const sharedptr<const Field>& field, bool not_extras)
{
  sharedptr<Field> field_to_add = glom_sharedptr_clone(field);

  Glib::RefPtr<Gnome::Gda::Column> field_info = field_to_add->get_field_info();
  if((field_info->get_g_type() == G_TYPE_NONE) || (field_info->get_g_type() == GDA_TYPE_NULL))
  {
    field_info->set_g_type( Field::get_gda_type_for_glom_type(field_to_add->get_glom_type()) );
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

sharedptr<Field> GlomPostgres::postgres_change_column_extras(const Glib::ustring& table_name, const sharedptr<const Field>& field_old, const sharedptr<const Field>& field, bool set_anyway)
{
  //Glib::RefPtr<Gnome::Gda::Column> field_info = field->get_field_info();
  //Glib::RefPtr<Gnome::Gda::Column> field_info_old = field_old->get_field_info();

  sharedptr<Field> result = glom_sharedptr_clone(field);

  if(field->get_name() != field_old->get_name())
  {
     Glib::RefPtr<Gnome::Gda::DataModel>  datamodel = query_execute( "ALTER TABLE \"" + table_name + "\" RENAME COLUMN \"" + field_old->get_name() + "\" TO \"" + field->get_name() + "\"" );
     if(!datamodel)
     {
       handle_error();
       return result;
     }
  }

  bool primary_key_was_set = false;
  bool primary_key_was_unset = false;
  if(set_anyway || (field->get_primary_key() != field_old->get_primary_key()))
  {
     //TODO: Check that there is only one primary key.
     //When unsetting a primary key, ask which one should replace it.

     Glib::RefPtr<Gnome::Gda::DataModel> datamodel;
       
     //TODO: Somehow discover whether these constraint names really exists, so we can be more robust in strange situations. This needs libgda 2, which has GDA_CONNECTION_SCHEMA_CONSTRAINTS.
     //Postgres needs us to add/drop constraints explicitly when changing existing fields, though it can create them implicitly when creating the field:
     if(field->get_primary_key())
     {
       //Set field as primary key :
       datamodel = query_execute( "ALTER TABLE \"" + table_name + "\" ADD PRIMARY KEY (\"" + field->get_name() + "\")");
       primary_key_was_set = true;

       //Tell the caller about a constraint:
       result->set_unique_key(); //All primary keys are unique.
     }
     else
     {
       //Unset field as primary key:
       datamodel = query_execute( "ALTER TABLE \"" + table_name + "\" DROP CONSTRAINT \"" + table_name + "_pkey\"" );
       primary_key_was_unset = true;

       //Make sure that the caller knows that a field stops being unique when it stops being a primary key, 
       //because its uniqueness was just a side-effect of it being a primary key.
       result->set_unique_key(false); //All primary keys are unique.
     }

     if(!datamodel)
     {
       handle_error();
       return result;
     }

     if(primary_key_was_set && field_old->get_unique_key())
     {
       //If the key already had a uniqueness constraint, without also being a primary key, remove that now, because it is superfluous and we will not expect it later:
       Glib::RefPtr<Gnome::Gda::DataModel>  datamodel = query_execute( "ALTER TABLE \"" + table_name + "\" DROP CONSTRAINT \"" + field->get_name() + "_key\"" );
      if(!datamodel)
      {
        handle_error();
        return result;
      }
    }
  }

  if(set_anyway || (field->get_unique_key() != field_old->get_unique_key()))
  {
    Glib::RefPtr<Gnome::Gda::DataModel> datamodel;
       
    //Postgres needs us to add/drop constraints explicitly when changing existing fields, though it can create them implicitly when creating the field:
    if(!primary_key_was_set && field->get_unique_key()) //Postgres automatically makes primary keys unique, so we do not need to do that separately if we have already made it a primary key
    {
      //Add uniqueness:
      Glib::RefPtr<Gnome::Gda::DataModel>  datamodel = query_execute( "ALTER TABLE \"" + table_name + "\" ADD CONSTRAINT \"" + field->get_name() + "_key\" UNIQUE (\"" + field->get_name() + "\")" );
      if(!datamodel)
      {
        handle_error();
        return result;
      }
    }
    else if(!primary_key_was_unset && !field->get_unique_key()) //This would implicitly have removed the uniqueness constraint which was in the primary key constraint 
    {      
      if(field->get_primary_key())
      {
        //Primary keys must be unique:
        result->set_unique_key();
      }
      else
      {
        //Remove uniqueness:
        Glib::RefPtr<Gnome::Gda::DataModel>  datamodel = query_execute( "ALTER TABLE \"" + table_name + "\" DROP CONSTRAINT \"" + field->get_name() + "_key\"" );
        if(!datamodel)
        {
          handle_error();
          return result;
        }
      }
    }


    const Gnome::Gda::Value default_value = field->get_default_value();
    const Gnome::Gda::Value default_value_old = field_old->get_default_value();

    if(!field->get_auto_increment()) //Postgres auto-increment fields have special code as their default values.
    {
      if(set_anyway || (default_value != default_value_old))
      {
        Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute( "ALTER TABLE \"" + table_name + "\" ALTER COLUMN \""+ field->get_name() + "\" SET DEFAULT " + field->sql(field->get_default_value()) );
        if(!datamodel)
        {
          handle_error();
          return result;
        }
      }
    }
  }

  /* This should have been dealt with by postgres_change_column_type(), because postgres uses a different ("serial") field type for auto-incrementing fields.
  if(field_info->get_auto_increment() != field_info_old.get_auto_increment())
  {

  }
  */
 
   /*
    //If the not-nullness has changed:
    if( set_anyway ||  (field->get_field_info().get_allow_null() != field_old->get_field_info().get_allow_null()) )
    {
      Glib::ustring nullness = (field->get_field_info().get_allow_null() ? "NULL" : "NOT NULL");
      query_execute(  "ALTER TABLE \"" + m_table_name + "\" ALTER COLUMN \"" + field->get_name() + "\"  SET " + nullness);
    }
  */ 

  return result;
}

GlomPostgres::type_vecStrings GlomPostgres::pg_list_separate(const Glib::ustring& str)
{
  //Remove the first { and the last }:
  Glib::ustring without_brackets = Utils::string_trim(str, "{");
  without_brackets = Utils::string_trim(without_brackets, "}");

  //Get the comma-separated items:
  return Utils::string_separate(without_brackets, ",");
}

} //namespace Glom
