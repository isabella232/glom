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

#include "box_data_list_related.h"
#include "../data_structure/glomconversions.h"
#include <libintl.h>

Box_Data_List_Related::Box_Data_List_Related()
{
}

Box_Data_List_Related::~Box_Data_List_Related()
{
}

void Box_Data_List_Related::init_db_details(const Glib::ustring& strDatabaseName,  const Relationship& relationship, const Gnome::Gda::Value& foreign_key_value, const Gnome::Gda::Value&  /* from_table_primary_key_value */)
{
  m_key_value = foreign_key_value;

  bool found = get_fields_for_table_one_field(relationship.get_to_table(), relationship.get_to_field(), m_key_field);
  if(found)
  {           
    if(!GlomConversions::value_is_empty(m_key_value))
    {
      Glib::ustring strWhereClause = m_key_field.get_name() + " = " + m_key_field.sql(m_key_value);

      Box_Data_List::init_db_details(strDatabaseName, relationship.get_to_table(), strWhereClause);
    }
    //TODO: Clear the list if there is no key value?
  }
}

void Box_Data_List_Related::fill_from_database()
{
  bool allow_add = true;
 
  if(!m_strWhereClause.empty())
  {
    Box_Data_List::fill_from_database();

     
    //Is there already one record here?
    if(m_has_one_or_more_records) //This was set by Box_Data_List::fill_from_database().
    {
      //Is the to_field unique? If so, there can not be more than one.
      if(m_key_field.get_field_info().get_unique_key()) //automatically true if it is a primary key
        allow_add = false;
    }

    //TODO: Disable add if the from_field already has a value and the to_field is auto-incrementing because
    //- we can not override the auto-increment in the to_field.
    //- we can not change the value in the from_field to the new auto_increment value in the to_field.
  }
  else
  {
    //No Foreign Key value, so just show the field names:

    Box_DB_Table::fill_from_database();

    fill_column_titles();

    fill_end();
  }

  m_AddDel.set_allow_add(allow_add);
}

void Box_Data_List_Related::on_record_added(const Gnome::Gda::Value& primary_key_value)
{
  //Get row of new record:
  Gtk::TreeModel::iterator iter = m_AddDel.get_row(primary_key_value.to_string());
  if(iter)
  {
    guint iKey = 0;
    bool bTest = get_field_column_index(m_key_field.get_name(), iKey);
    if(!bTest)
       std::cout << "Box_Data_List_Related::on_record_added() field not found: " << m_key_field.get_name() << std::endl;

    Gnome::Gda::Value key_value = m_AddDel.get_value_as_value(iter, iKey);
    Box_Data_List::on_record_added(key_value); //adds blank row.

    //Make sure that the new related record is related,
    //by setting the foreign key:
    //If it's not auto-generated.
    if(!GlomConversions::value_is_empty(key_value))
    {
      //It was auto-generated. Tell the parent about it, so it can make a link.
      signal_record_added.emit(key_value);
    }
    else
    {
      //Create the link by setting the foreign key:
      m_AddDel.set_value(iter, iKey, key_value);

      on_adddel_user_changed(iter, iKey); //Update the database.
    }
  }
}

Field Box_Data_List_Related::get_key_field() const
{
  return m_key_field;
}

void Box_Data_List_Related::on_adddel_user_added(const Gtk::TreeModel::iterator& row)
{
  //Like Box_Data_List::on_adddel_user_added(),
  //but it doesn't allow adding if the new record can not be a related record.
  //This would happen if there is already one related record and the relationship uses the primary key in the related record.

  bool bAllowAdd = true;

  const Gnome::Gda::FieldAttributes fieldInfo = m_key_field.get_field_info();
  if(fieldInfo.get_unique_key() || fieldInfo.get_primary_key())
  {
    if(m_AddDel.get_count() > 0) //If there is already 1 record
      bAllowAdd = false;
  }

  if(bAllowAdd)
  {
    Box_Data_List::on_adddel_user_added(row);
  }
  else
  {
    //Tell user that they can't do that:
    Gtk::MessageDialog dialog(gettext("You attempted to add a new related record, \nbut there can only be one related record, \nbecause the relationship uses a unique key."),
      Gtk::MESSAGE_WARNING);
    dialog.run();

    //Replace with correct values:
    fill_from_database();
  }
}

void Box_Data_List_Related::enable_buttons()
{
}

