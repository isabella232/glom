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

Box_Data_List_Related::Box_Data_List_Related()
{
}

Box_Data_List_Related::~Box_Data_List_Related()
{
}

void Box_Data_List_Related::initialize(const Glib::ustring& strDatabaseName,  const Relationship& relationship, const Glib::ustring& strForeignKeyValue, const Glib::ustring& from_table_primary_key_value)
{
  m_strKeyField = relationship.get_to_field();
  m_strKeyValue = strForeignKeyValue;

  //TODO:
  //At the moment strForeignKeyValue must be SQLized already.

  Glib::ustring strWhereClause;
  if(!m_strKeyField.empty() && !m_strKeyValue.empty())
    strWhereClause = m_strKeyField + " = " + m_strKeyValue; //TODO: escape them.

  Box_Data_List::initialize(strDatabaseName, relationship.get_to_table(), strWhereClause);
}

void Box_Data_List_Related::fill_from_database()
{
  bool allow_add = true;
 
  if(m_strWhereClause.size())
  {
    Box_Data_List::fill_from_database();

     
    //Is there already one record here?
    if(m_has_one_or_more_records) //This was set by Box_Data_List::fill_from_database().
    {
      //Is the to_field unique? If so, there can not be more than one.
      Field field_to = get_fields_for_table_one_field(m_strTableName, m_strKeyField);
      if(field_to.get_field_info().get_unique_key()) //automatically true if it is a primary key
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

void Box_Data_List_Related::on_record_added(const Glib::ustring& strPrimaryKeyValue)
{
  //Get row of new record:
  guint iRow = 0;
  bool bTest = m_AddDel.get_row_number(strPrimaryKeyValue, iRow);
  if(bTest)
  {
    guint iKey = 0;
    bool bTest = get_field_index(m_strKeyField, iKey);
    if(!bTest)
       std::cout << "Box_Data_List_Related::on_record_added() field not found: " << m_strKeyField << std::endl;

    Glib::ustring strKeyValue = m_AddDel.get_value(iRow, iKey);
    Box_Data_List::on_record_added(strPrimaryKeyValue); //adds blank row.


    //Make sure that the new related record is related,
    //by setting the foreign key:
    //If it's not auto-generated.
    if(strKeyValue.size())
    {
      //It was auto-generated. Tell the parent about it, so it can make a link.
      signal_record_added.emit(strKeyValue);
    }
    else
    {
      //Create the link by setting the foreign key:
      m_AddDel.set_value(iRow, iKey, m_strKeyValue);

      on_AddDel_user_changed(iRow, iKey); //Update the database.
    }
  }
}

Glib::ustring Box_Data_List_Related::get_KeyField() const
{
  return m_strKeyField;
}

void Box_Data_List_Related::on_AddDel_user_added(guint row)
{
  //Like Box_Data_List::on_AddDel_user_added(),
  //but it doesn't allow adding if the new record can not be a related record.
  //This would happen if there is already one related record and the relationship uses the primary key in the related record.

  bool bAllowAdd = true;

  Field field;
  bool test = get_field(m_strKeyField, field);

  Gnome::Gda::FieldAttributes fieldInfo = field.get_field_info();
  if(fieldInfo.get_unique_key() || fieldInfo.get_primary_key())
  {
    if(m_AddDel.get_count() > 0) //If there is already 1 record
      bAllowAdd = false;
  }

  if(bAllowAdd)
  {
    Box_Data_List::on_AddDel_user_added(row);
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

