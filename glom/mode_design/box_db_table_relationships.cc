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

#include <libglademm.h>
#include "box_db_table_relationships.h"

Box_DB_Table_Relationships::Box_DB_Table_Relationships()
{
  init();
}

Box_DB_Table_Relationships::Box_DB_Table_Relationships(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Box_DB_Table(cobject, refGlade)
{
  init();
}

void Box_DB_Table_Relationships::init()
{
  pack_start(m_AddDel);
  m_colName = m_AddDel.add_column(gettext("Name"));

  m_colFromField = m_AddDel.add_column(gettext("Field"), AddDelColumnInfo::STYLE_Choices);
  m_colToTable = m_AddDel.add_column(gettext("Table"), AddDelColumnInfo::STYLE_Choices);
  m_colToField = m_AddDel.add_column(gettext("Field"), AddDelColumnInfo::STYLE_Choices);

  //Connect signals:
  m_AddDel.signal_user_activated().connect(sigc::mem_fun(*this, &Box_DB_Table_Relationships::on_AddDel_user_activated));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_DB_Table_Relationships::on_AddDel_user_changed));

  show_all_children();
}

Box_DB_Table_Relationships::~Box_DB_Table_Relationships()
{
}

void Box_DB_Table_Relationships::fill_from_database()
{
  Bakery::BusyCursor(*get_app_window());
    
  Box_DB_Table::fill_from_database();

  //Get relationships from the document:
  Document_Glom::type_vecRelationships vecRelationships = m_pDocument->get_relationships(m_strTableName);

  m_AddDel.remove_all();
  
  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

    //Set combo choices:
    m_AddDel.set_column_choices(m_colFromField, util_vecStrings_from_Fields(get_fields_for_table(m_strTableName)));

    type_vecStrings vecTableNames = get_table_names();
    type_vecStrings vecTableNames_ustring(vecTableNames.begin(), vecTableNames.end());
    m_AddDel.set_column_choices(m_colToTable, vecTableNames_ustring);

    //To Field choices are different for each row: set in on_AddDel_signal_user_activated.

    //Add the relationships:
    for(Document_Glom::type_vecRelationships::iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); iter++)
    {
       const Relationship& relationship = *iter;

       //Name:
       guint uiRow = m_AddDel.add_item(relationship.get_name());

       //From Field:
       m_AddDel.set_value(uiRow, m_colFromField, relationship.get_from_field());

       //To Table:
       const Glib::ustring& strToTable = relationship.get_to_table();
       m_AddDel.set_value(uiRow, m_colToTable, strToTable);

       //To Field:
       m_AddDel.set_value(uiRow, m_colToField, relationship.get_to_field());

    }
  }
  
  fill_end();
}

void Box_DB_Table_Relationships::save_to_document()
{
  //Build relationships from AddDel:
  Document_Glom::type_vecRelationships vecRelationships;

  for(guint i = 0; i < m_AddDel.get_count(); i++)
  {
    Relationship relationship;
    relationship.set_name(m_AddDel.get_value(i, m_colName));
    relationship.set_from_table(m_strTableName);
    relationship.set_from_field(m_AddDel.get_value(i, m_colFromField));
    relationship.set_to_table(m_AddDel.get_value(i, m_colToTable));
    relationship.set_to_field(m_AddDel.get_value(i, m_colToField));

    vecRelationships.push_back(relationship);
  }

  //Update the Document with these relationships.
  m_pDocument->set_relationships(m_strTableName, vecRelationships);

  //Call base:
  Box_DB_Table::save_to_document();
}

Box_DB_Table_Relationships::type_vecStrings Box_DB_Table_Relationships::util_vecStrings_from_Fields(const type_vecFields& fields)
{
  //Get vector of field names, suitable for a combo box:

  type_vecStrings vecNames;
  for(type_vecFields::size_type i = 0; i < fields.size(); i++)
  {
    vecNames.push_back(fields[i].get_name());
  }

  return vecNames;
}

void Box_DB_Table_Relationships::on_AddDel_user_changed(guint row, guint col)
{
  if(col == m_colToTable)
  {
    //User chose a new table so we need to wipe the field
    //because it might not be a valid field for that table:
    m_AddDel.set_value(row, m_colToField, Glib::ustring("")); //Plain "" goes to bool override.
  }

  set_modified();
}

void Box_DB_Table_Relationships::on_AddDel_user_activated(guint row, guint col)
{
  if(col == m_colToField)
  {
    Bakery::BusyCursor(*get_app_window());
        
    const Glib::ustring& strTableName = m_AddDel.get_value(row, m_colToTable);

    if(strTableName.size())
    {
      //Set list of 'To' fields depending on table:
      m_AddDel.set_value(row, m_colToField, Glib::ustring(""));

      sharedptr<SharedConnection> sharedconnection = connect_to_server();
      if(sharedconnection)
      {
        Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();
     
        type_vecStrings vecFields = util_vecStrings_from_Fields(get_fields_for_table(strTableName));

        //This would cause a lot of tedious re-filling:
        //m_AddDel.set_column_choices(m_colToField, vecFields);
        //fill_from_database();

        m_AddDel.set_column_choices(m_colToField, vecFields);
      }
    }

  }
}


