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

#include "base_db_table_data.h"
#include <glom/libglom/data_structure/glomconversions.h>
#include <glom/application.h>
#include "python_embed/glom_python.h"
#include <sstream>
#include <glibmm/i18n.h>

namespace Glom
{

Base_DB_Table_Data::Base_DB_Table_Data()
{
}

Base_DB_Table_Data::~Base_DB_Table_Data()
{
}

bool Base_DB_Table_Data::refresh_data_from_database()
{
  if(!ConnectionPool::get_instance()->get_ready_to_connect())
    return false;

  return fill_from_database();
}

Gnome::Gda::Value Base_DB_Table_Data::get_entered_field_data_field_only(const sharedptr<const Field>& field) const
{
  sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
  layout_item->set_full_field_details(field); 

  return get_entered_field_data(layout_item);
}

Gnome::Gda::Value Base_DB_Table_Data::get_entered_field_data(const sharedptr<const LayoutItem_Field>& /* field */) const
{
  //Override this to use Field::set_data() too.

  return Gnome::Gda::Value(); //null
}


bool Base_DB_Table_Data::record_new(bool use_entered_data, const Gnome::Gda::Value& primary_key_value)
{
  sharedptr<const Field> fieldPrimaryKey = get_field_primary_key();

  const Glib::ustring primary_key_name = fieldPrimaryKey->get_name();

  type_vecLayoutFields fieldsToAdd = m_FieldsShown;
  if(m_TableFields.empty())
    m_TableFields = get_fields_for_table(m_table_name);

  //Add values for all fields, not just the shown ones:
  //For instance, we must always add the primary key, and fields with default/calculated/lookup values:
  for(type_vecFields::const_iterator iter = m_TableFields.begin(); iter != m_TableFields.end(); ++iter)
  {
    //TODO: Search for the non-related field with the name, not just the field with the name:
    type_vecLayoutFields::const_iterator iterFind = std::find_if(fieldsToAdd.begin(), fieldsToAdd.end(), predicate_FieldHasName<LayoutItem_Field>((*iter)->get_name()));
    if(iterFind == fieldsToAdd.end())
    {
      sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
      layout_item->set_full_field_details(*iter);

      fieldsToAdd.push_back(layout_item);
    }
  }

  Document_Glom* document = get_document();

  //Calculate any necessary field values and enter them:
  for(type_vecLayoutFields::const_iterator iter = fieldsToAdd.begin(); iter != fieldsToAdd.end(); ++iter)
  {
    sharedptr<LayoutItem_Field> layout_item = *iter;

    //If the user did not enter something in this field:
    Gnome::Gda::Value value = get_entered_field_data(layout_item);

    if(Conversions::value_is_empty(value)) //This deals with empty strings too.
    {
      const sharedptr<const Field>& field = layout_item->get_full_field_details();
      if(field)
      {
        //If the default value should be calculated, then calculate it:
        if(field->get_has_calculation())
        {
          const Glib::ustring calculation = field->get_calculation();
          const type_map_fields field_values = get_record_field_values_for_calculation(m_table_name, fieldPrimaryKey, primary_key_value);

          //We need the connection when we run the script, so that the script may use it.
#ifdef GLIBMM_EXCEPTIONS_ENABLED
          // TODO: Is this function supposed to throw an exception?
          sharedptr<SharedConnection> sharedconnection = connect_to_server(App_Glom::get_application());
#else
          std::auto_ptr<ExceptionConnection> error;
          sharedptr<SharedConnection> sharedconnection = connect_to_server(App_Glom::get_application(), error);
          if(error.get() == NULL)
          {
            // Don't evaluate function on error
#endif // GLIBMM_EXCEPTIONS_ENABLED

            const Gnome::Gda::Value value = glom_evaluate_python_function_implementation(field->get_glom_type(), calculation, field_values, document, m_table_name, sharedconnection->get_gda_connection());
            set_entered_field_data(layout_item, value);
#ifndef GLIBMM_EXCEPTIONS_ENABLED
          }
#endif // !GLIBMM_EXCEPTIONS_ENABLED
        }

        //Use default values (These are also specified in postgres as part of the field definition,
        //so we could theoretically just not specify it here.)
        //TODO_Performance: Add has_default_value()?
        if(Conversions::value_is_empty(value))
        {
          const Gnome::Gda::Value default_value = field->get_default_value();
          if(!Conversions::value_is_empty(default_value))
          {
            set_entered_field_data(layout_item, default_value);
          }
        }
      }
    }
  }

  //Get all entered field name/value pairs:
  Glib::ustring strNames;
  Glib::ustring strValues;

  //Avoid specifying the same field twice:
  typedef std::map<Glib::ustring, bool> type_map_added;
  type_map_added map_added;
  Glib::RefPtr<Gnome::Gda::Set> params = Gnome::Gda::Set::create();
  
  for(type_vecLayoutFields::const_iterator iter = fieldsToAdd.begin(); iter != fieldsToAdd.end(); ++iter)
  {
    sharedptr<LayoutItem_Field> layout_item = *iter;
    const Glib::ustring field_name = layout_item->get_name();
    if(!layout_item->get_has_relationship_name()) //TODO: Allow people to add a related record also by entering new data in a related field of the related record.
    {
      type_map_added::const_iterator iterFind = map_added.find(field_name);
      if(iterFind == map_added.end()) //If it was not added already
      {
        Gnome::Gda::Value value;

        const sharedptr<const Field>& field = layout_item->get_full_field_details();
        if(field)
        {
          Field gda_field = *field;
          //Use the specified (generated) primary key value, if there is one:
          if(primary_key_name == field_name && !Conversions::value_is_empty(primary_key_value))
          {
            value = primary_key_value;
          }
          else
          {
            if(use_entered_data)
              value = get_entered_field_data(layout_item);
          }
          gda_field.set_data(value);

          /* //TODO: This would be too many small queries when adding one record.
          //Check whether the value meets uniqueness constraints:
          if(field->get_primary_key() || field->get_unique_key())
          {
            if(!get_field_value_is_unique(m_table_name, layout_item, value))
            {
              //Ignore this field value. TODO: Warn the user about it.
            } 
          }
          */
          if(!gda_field.get_data().is_null())
          {
            if(!strNames.empty())
            {
              strNames += ", ";
              strValues += ", ";
            }
  
            strNames += "\"" + field_name + "\"";
            strValues += gda_field.get_gda_holder_string();
            Glib::RefPtr<Gnome::Gda::Holder> holder = gda_field.get_holder();
            holder->set_not_null(false);
            params->add_holder(holder);
  
            map_added[field_name] = true;
          }
        }
      }
    }
  }

  //Put it all together to create the record with these field values:
  if(!strNames.empty() && !strValues.empty())
  {
    const Glib::ustring strQuery = "INSERT INTO \"" + m_table_name + "\" (" + strNames + ") VALUES (" + strValues + ")";
    const bool test = query_execute(strQuery, params);
    if(!test)
      std::cerr << "Box_Data::record_new(): INSERT failed." << std::endl;
    else
    {
      Gtk::TreeModel::iterator row; // TODO: remove this parameter.
      set_primary_key_value(row, primary_key_value); //Needed by Box_Data_List::on_adddel_user_changed().

      //Update any lookups, related fields, or calculations:
      for(type_vecLayoutFields::const_iterator iter = fieldsToAdd.begin(); iter != fieldsToAdd.end(); ++iter)
      {
        sharedptr<const LayoutItem_Field> layout_item = *iter;
         
        //TODO_Performance: We just set this with set_entered_field_data() above. Maybe we could just remember it.
        const Gnome::Gda::Value field_value = get_entered_field_data(layout_item);

        LayoutFieldInRecord field_in_record(layout_item, m_table_name, fieldPrimaryKey, primary_key_value);

        //Get-and-set values for lookup fields, if this field triggers those relationships:
        do_lookups(field_in_record, row, field_value);

        //Update related fields, if this field is used in the relationship:
        refresh_related_fields(field_in_record, row, field_value);

        //TODO: Put the inserted row into result, somehow? murrayc

        return true; //success
      }
    }
  }
  else
    std::cerr << "Base_DB_Table_Data::record_new(): Empty field names or values." << std::endl;

  return false; //Failed.
}


bool Base_DB_Table_Data::get_related_record_exists(const sharedptr<const Relationship>& relationship, const sharedptr<const Field>& key_field, const Gnome::Gda::Value& key_value)
{
  Bakery::BusyCursor cursor(App_Glom::get_application());

  bool result = false;

  //Don't try doing a NULL=NULL or ""="" relationship:
  if(Glom::Conversions::value_is_empty(key_value))
    return false;

  //TODO_Performance: It's very possible that this is slow.
  //We don't care how many records there are, only whether there are more than zero.
  const Glib::ustring to_field = relationship->get_to_field();
  const Glib::ustring related_table = relationship->get_to_table();

  //TODO_Performance: Is this the best way to just find out whether there is one record that meets this criteria?
  const Glib::ustring query = "SELECT \"" + to_field + "\" FROM \"" + relationship->get_to_table() + "\" WHERE \"" + related_table + "\".\"" + to_field + "\" = " + key_field->sql(key_value);
  Glib::RefPtr<Gnome::Gda::DataModel> records = query_execute_select(query);
  if(!records)
    handle_error();
  else
  {
    //Field contents:
    if(records->get_n_rows())
      result = true;
  }

  return result;
}

bool Base_DB_Table_Data::add_related_record_for_field(const sharedptr<const LayoutItem_Field>& layout_item_parent, 
  const sharedptr<const Relationship>& relationship, 
  const sharedptr<const Field>& primary_key_field, 
  const Gnome::Gda::Value& primary_key_value_provided,
  Gnome::Gda::Value& primary_key_value_used)
{
  Gnome::Gda::Value primary_key_value = primary_key_value_provided;

  const bool related_record_exists = get_related_record_exists(relationship, primary_key_field, primary_key_value);
  if(related_record_exists)
  {
    //No problem, the SQL command below will update this value in the related table.
    primary_key_value_used = primary_key_value; //Let the caller know what related record was created.
    return true;
  }
    

  //To store the entered data in the related field, we would first have to create a related record.
  if(!relationship->get_auto_create())
  {
    //Warn the user:
    //TODO: Make the field insensitive until it can receive data, so people never see this dialog.
    const Glib::ustring message = _("Data may not be entered into this related field, because the related record does not yet exist, and the relationship does not allow automatic creation of new related records.");
#ifdef GLOM_ENABLE_MAEMO
    Hildon::Note dialog(Hildon::NOTE_TYPE_INFORMATION, *App_Glom::get_application(), message);
#else
    Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Related Record Does Not Exist")), true);
    dialog.set_secondary_text(message);
    dialog.set_transient_for(*App_Glom::get_application());
#endif
    dialog.run();

    //Clear the field again, discarding the entered data.
    set_entered_field_data(layout_item_parent, Gnome::Gda::Value());

    return false;
  }
  else
  {
    const bool key_is_auto_increment = primary_key_field->get_auto_increment();

    //If we would have to set an otherwise auto-increment key to add the record.
    if( key_is_auto_increment && !Conversions::value_is_empty(primary_key_value) )
    {
      //Warn the user:
      //TODO: Make the field insensitive until it can receive data, so people never see this dialog.
      const Glib::ustring message = _("Data may not be entered into this related field, because the related record does not yet exist, and the key in the related record is auto-generated and therefore can not be created with the key value in this record.");

#ifdef GLOM_ENABLE_MAEMO
      Hildon::Note dialog(Hildon::NOTE_TYPE_INFORMATION, *App_Glom::get_application(), message);
#else
      Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Related Record Cannot Be Created")), true);
      //TODO: This is a very complex error message:
      dialog.set_secondary_text(message);
      dialog.set_transient_for(*App_Glom::get_application());
#endif
      dialog.run();

      //Clear the field again, discarding the entered data.
      set_entered_field_data(layout_item_parent, Gnome::Gda::Value());

      return false;
    }
    else
    {
      //TODO: Calculate values, and do lookups?

      //Create the related record:
      if(key_is_auto_increment)
      {
        primary_key_value = get_next_auto_increment_value(relationship->get_to_table(), primary_key_field->get_name());

        //Generate the new key value;
      }

      const Glib::ustring strQuery = "INSERT INTO \"" + relationship->get_to_table() + "\" (\"" + primary_key_field->get_name() + "\") VALUES (" + primary_key_field->sql(primary_key_value) + ")";
      const bool test = query_execute(strQuery);
      if(!test)
      {
        std::cerr << "Base_DB_Table_Data::add_related_record_for_field(): INSERT failed." << std::endl;
        return false;
      }

      if(key_is_auto_increment)
      {
        //Set the key in the parent table
        sharedptr<LayoutItem_Field> item_from_key = sharedptr<LayoutItem_Field>::create();
        item_from_key->set_name(relationship->get_from_field());

        //Show the new from key in the parent table's layout:
        set_entered_field_data(item_from_key, primary_key_value);

        //Set it in the database too:
        sharedptr<Field> field_from_key = get_fields_for_table_one_field(relationship->get_from_table(), relationship->get_from_field()); //TODO_Performance.
        if(!field_from_key)
        {
          std::cerr << "Base_DB_Table_Data::add_related_record_for_field(): get_fields_for_table_one_field() failed." << std::endl;
          return false;
        }

        sharedptr<Field> parent_primary_key_field = get_field_primary_key();
        if(!parent_primary_key_field)
        {
          g_warning("Base_DB_Table_Data::add_related_record_for_field(): get_field_primary_key() failed. table = %s", get_table_name().c_str());
          return false;
        }
        else
        {
          const Gnome::Gda::Value parent_primary_key_value = get_primary_key_value_selected();
          if(parent_primary_key_value.is_null())
          {
            g_warning("Base_DB_Table_Data::add_related_record_for_field(): get_primary_key_value_selected() failed. table = %s", get_table_name().c_str());
            return false;
          }
          else
          {
            const Glib::ustring strQuery = "UPDATE \"" + relationship->get_from_table() + "\" SET \"" + relationship->get_from_field() + "\" = " + primary_key_field->sql(primary_key_value) +
              " WHERE \"" + relationship->get_from_table() + "\".\"" + parent_primary_key_field->get_name() + "\" = " + parent_primary_key_field->sql(parent_primary_key_value);
            const bool test = query_execute(strQuery);
            if(!test)
            {
              std::cerr << "Base_DB_Table_Data::add_related_record_for_field(): UPDATE failed." << std::endl;
              return false;
            }
          }
        }
      }
        
      primary_key_value_used = primary_key_value; //Let the caller know what related record was created.
      return true;
    }
  }
}

void Base_DB_Table_Data::on_record_added(const Gnome::Gda::Value& /* primary_key_value */, const Gtk::TreeModel::iterator& /* row */)
{
   //Overridden by some derived classes.
  
   signal_record_changed().emit();
}

void Base_DB_Table_Data::on_record_deleted(const Gnome::Gda::Value& /* primary_key_value */)
{
  //Overridden by some derived classes.
  
  signal_record_changed().emit();
}

bool Base_DB_Table_Data::confirm_delete_record()
{
  //Ask the user for confirmation:
  const Glib::ustring message = _("Are you sure that you would like to delete this record? The data in this record will then be permanently lost.");
#ifdef GLOM_ENABLE_MAEMO
  Hildon::Note dialog(Hildon::NOTE_TYPE_CONFIRMATION_BUTTON, *get_app_window(), message);
#else
  Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Delete record")), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
  dialog.set_secondary_text(message);
  dialog.set_transient_for(*App_Glom::get_application());
#endif
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::DELETE, Gtk::RESPONSE_OK);

  const int response = dialog.run();
  return (response == Gtk::RESPONSE_OK);
}

bool Base_DB_Table_Data::record_delete(const Gnome::Gda::Value& primary_key_value)
{
  sharedptr<Field> field_primary_key = get_field_primary_key();
  if(field_primary_key && !Conversions::value_is_empty(primary_key_value))
  {
    return query_execute( "DELETE FROM \"" + m_table_name + "\" WHERE \"" + m_table_name + "\".\"" + field_primary_key->get_name() + "\" = " + field_primary_key->sql(primary_key_value));
  }
  else
  {
    return false; //no primary key
  }
}

Base_DB_Table_Data::type_signal_record_changed Base_DB_Table_Data::signal_record_changed()
{
  return m_signal_record_changed;
}

} //namespace Glom


