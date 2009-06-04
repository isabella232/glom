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

#include <glom/base_db_table_data.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/standard_table_prefs_fields.h>
#include <glom/application.h>
#include <glom/python_embed/glom_python.h>
#include <glom/utils_ui.h>
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

Gtk::TreeModel::iterator Base_DB_Table_Data::get_row_selected()
{
  //This in meaningless for Details, 
  //but is overridden for list views.
  return Gtk::TreeModel::iterator();
}

bool Base_DB_Table_Data::record_new_with_entered_data(const Gnome::Gda::Value& primary_key_value, const type_field_values& field_values)
{
  type_field_values field_values_plus_entered = field_values;

  //Add values for all fields that default to something, not just the shown ones:
  //For instance, we must always add the primary key, and fields with default/calculated/lookup values:
  const type_vecLayoutFields fieldsOnLayout = m_FieldsShown;
  for(type_vecLayoutFields::iterator iter = fieldsOnLayout.begin(); iter != fieldsOnLayout.end(); ++iter)
  {
    //Check that we don't have a value for this field already:
    //(This gives priority to specified fields values rather than entered field values.)
    //TODO: Search for the non-related field with the name, not just the field with the name:
    type_field_values::const_iterator iterFind = std::find_if(field_values_plus_entered.begin(), field_values_plus_entered.end(), predicate_FieldHasName<LayoutItem_Field>((*iter)->get_name()));
    if(iterFind == field_values_plus_entered.end())
    {
      sharedptr<const LayoutItem_Field> layoutitem = *iter;
      sharedptr<const Field> field = layoutitem->get_full_field_details();
      type_field_and_value field_and_value(field, Gnome::Gda::Value());
      field_values_plus_entered.push_back(field_and_value);
    }
  }

  return record_new(m_table_name, primary_key_value, field_values_plus_entered);
}

bool Base_DB_Table_Data::get_related_record_exists(const sharedptr<const Relationship>& relationship, const sharedptr<const Field>& key_field, const Gnome::Gda::Value& key_value)
{
  BusyCursor cursor(App_Glom::get_application());

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
    Gtk::MessageDialog dialog(Utils::bold_message(_("Related Record Does Not Exist")), true);
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
      Gtk::MessageDialog dialog(Utils::bold_message(_("Related Record Cannot Be Created")), true);
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
      //Create the related record:
      if(key_is_auto_increment)
      {
        primary_key_value = get_next_auto_increment_value(relationship->get_to_table(), primary_key_field->get_name());

        //Generate the new key value.
      }

      const bool added = record_new(relationship->get_to_table(), primary_key_value, type_field_values());
      if(!added)
      {
        std::cerr << "Base_DB_Table_Data::add_related_record_for_field(): record_new() failed." << std::endl;
      }

      if(key_is_auto_increment)
      {
        //Set the key in the parent table:
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
            Glib::RefPtr<Gnome::Gda::Set> params = Gnome::Gda::Set::create();
            params->add_holder(primary_key_field->get_holder(primary_key_value));
            params->add_holder(parent_primary_key_field->get_holder(parent_primary_key_value));
            const Glib::ustring strQuery = "UPDATE \"" + relationship->get_from_table() + "\" SET \"" + relationship->get_from_field() + "\" = " + primary_key_field->get_gda_holder_string() +
              " WHERE \"" + relationship->get_from_table() + "\".\"" + parent_primary_key_field->get_name() + "\" = " +  parent_primary_key_field->get_gda_holder_string();
            const bool test = query_execute(strQuery, params);
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
  Gtk::MessageDialog dialog(Utils::bold_message(_("Delete record")), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
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
    Glib::RefPtr<Gnome::Gda::Set> params = Gnome::Gda::Set::create();
    params->add_holder(field_primary_key->get_holder(primary_key_value));
    return query_execute( "DELETE FROM \"" + m_table_name + "\" WHERE \"" + m_table_name + "\".\"" + field_primary_key->get_name() + "\" = " + field_primary_key->get_gda_holder_string(), params);
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


void Base_DB_Table_Data::refresh_related_fields(const LayoutFieldInRecord& field_in_record_changed, const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& /* field_value */)
{
  //std::cout << "DEBUG: Base_DB_Table_Data::refresh_related_fields()" << std::endl;

  if(field_in_record_changed.m_table_name != m_table_name)
    return; //TODO: Handle these too?

  //Get values for lookup fields, if this field triggers those relationships:
  //TODO_performance: There is a LOT of iterating and copying here.
  //const Glib::ustring strFieldName = field_in_record_changed.m_field->get_name();
  type_vecLayoutFields fieldsToGet = get_related_fields(field_in_record_changed.m_field);

  if(!fieldsToGet.empty())
  {
    const Glib::ustring query = Utils::build_sql_select_with_key(field_in_record_changed.m_table_name, fieldsToGet, field_in_record_changed.m_key, field_in_record_changed.m_key_value);
    //std::cout << "DEBUG: Base_DB_Table_Data::refresh_related_fields(): query=" << query << std::endl;

    Glib::RefPtr<Gnome::Gda::DataModel> result = query_execute_select(query);
    if(!result)
    {
      std::cerr << "Base_DB_Table_Data::refresh_related_fields(): no result." << std::endl;
      handle_error();
    }
    else
    {
      //Field contents:
      if(result->get_n_rows())
      {
        type_vecLayoutFields::const_iterator iterFields = fieldsToGet.begin();

        const guint cols_count = result->get_n_columns();
        if(cols_count <= 0)
        {
          std::cerr << "Base_DB_Table_Data::refresh_related_fields(): The result had 0 columns" << std::endl;
        }

        for(guint uiCol = 0; uiCol < cols_count; uiCol++)
        {
          const Gnome::Gda::Value value = result->get_value_at(uiCol, 0 /* row */);
          sharedptr<LayoutItem_Field> layout_item = *iterFields;
          if(!layout_item)
            std::cerr << "Base_DB_Table_Data::refresh_related_fields(): The layout_item was null." << std::endl;
          else
          {
            //std::cout << "DEBUG: Box_Data_List::refresh_related_fields(): field_name=" << layout_item->get_name() << std::endl;
            //std::cout << "  DEBUG: Box_Data_List::refresh_related_fields(): value_as_string=" << value.to_string()  << std::endl;

            //m_AddDel.set_value(row, layout_item, value);
            set_entered_field_data(row, layout_item, value);
            //g_warning("addedel size=%d", m_AddDel.get_count());
          }

          ++iterFields;
        }
      }
      else
        std::cerr << "Base_DB_Table_Data::refresh_related_fields(): no records found." << std::endl;
    }
  }
}

/** Get the shown fields that are in related tables, via a relationship using @a field_name changes.
 */
Base_DB_Table_Data::type_vecLayoutFields Base_DB_Table_Data::get_related_fields(const sharedptr<const LayoutItem_Field>& field) const
{
  type_vecLayoutFields result;

  const Document* document = dynamic_cast<const Document*>(get_document());
  if(document)
  {
    const Glib::ustring field_name = field->get_name(); //At the moment, relationships can not be based on related fields on the from side.
    for(type_vecLayoutFields::const_iterator iter = m_FieldsShown.begin(); iter != m_FieldsShown.end();  ++iter)
    {
      sharedptr<LayoutItem_Field> layout_field = *iter;
      //Examine each field that looks up its data from a relationship:
      if(layout_field->get_has_relationship_name())
      {
        //Get the relationship information:
        sharedptr<const Relationship> relationship = document->get_relationship(m_table_name, layout_field->get_relationship_name());
        if(relationship)
        {
          //If the relationship uses the specified field:
          if(relationship->get_from_field() == field_name)
          {
            //Add it:
            result.push_back(layout_field);
          }
        }
      }
    }
  }

  return result;
}

} //namespace Glom


