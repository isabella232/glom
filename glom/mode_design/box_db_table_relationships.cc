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
#include <glibmm/i18n.h>

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
  m_colName = m_AddDel.add_column(_("Name"));
  m_colTitle = m_AddDel.add_column(_("Title"));

  m_colFromField = m_AddDel.add_column(_("From Field"), AddDelColumnInfo::STYLE_Choices);
  m_colToTable = m_AddDel.add_column(_("Table"), AddDelColumnInfo::STYLE_Choices);
  m_colToField = m_AddDel.add_column(_("To Field"), AddDelColumnInfo::STYLE_Choices);
  m_colAllowEdit = m_AddDel.add_column(_("Allow Editing"),  AddDelColumnInfo::STYLE_Boolean);
  m_colAutoCreate = m_AddDel.add_column(_("Automatic Creation"),  AddDelColumnInfo::STYLE_Boolean);

  //Connect signals:
  m_AddDel.signal_user_activated().connect(sigc::mem_fun(*this, &Box_DB_Table_Relationships::on_adddel_user_activated));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_DB_Table_Relationships::on_adddel_user_changed));
  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_DB_Table_Relationships::on_adddel_user_added));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_DB_Table_Relationships::on_adddel_user_requested_delete));

  show_all_children();
}

Box_DB_Table_Relationships::~Box_DB_Table_Relationships()
{
}

bool Box_DB_Table_Relationships::fill_from_database()
{
  Bakery::BusyCursor(*get_app_window());

  bool result = Box_DB_Table::fill_from_database();

  //Get relationships from the document:
  Document_Glom::type_vecRelationships vecRelationships = get_document()->get_relationships(m_strTableName);

  m_AddDel.remove_all();

  sharedptr<SharedConnection> sharedconnection = connect_to_server();
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

    //Set combo choices:
    m_AddDel.set_column_choices(m_colFromField, util_vecStrings_from_Fields(get_fields_for_table(m_strTableName)));

    type_vecStrings vecTableNames = get_table_names(true /* ignore_system_tables */);
    type_vecStrings vecTableNames_ustring(vecTableNames.begin(), vecTableNames.end());
    m_AddDel.set_column_choices(m_colToTable, vecTableNames_ustring);

    //To Field choices are different for each row: set in on_adddel_signal_user_activated.

    //Add the relationships:
    for(Document_Glom::type_vecRelationships::iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); iter++)
    {
       const Relationship& relationship = *iter;

       //Name:
       Gtk::TreeModel::iterator iterTree = m_AddDel.add_item(relationship.get_name());
       m_AddDel.set_value(iterTree, m_colName, relationship.get_name());

       //Title:
       m_AddDel.set_value(iterTree, m_colTitle, relationship.get_title());

       //From Field:
       m_AddDel.set_value(iterTree, m_colFromField, relationship.get_from_field());

       //To Table:
       const Glib::ustring& strToTable = relationship.get_to_table();
       m_AddDel.set_value(iterTree, m_colToTable, strToTable);

       //To Field:
       m_AddDel.set_value(iterTree, m_colToField, relationship.get_to_field());

       m_AddDel.set_value(iterTree, m_colAllowEdit, relationship.get_allow_edit());
       m_AddDel.set_value(iterTree, m_colAutoCreate, relationship.get_auto_create());
    }
  }

  fill_end();

  return result;
}

void Box_DB_Table_Relationships::save_to_document()
{ 
  //Build relationships from AddDel:
  Document_Glom::type_vecRelationships vecRelationships;

  for(Gtk::TreeModel::iterator iter = m_AddDel.get_model()->children().begin(); iter != m_AddDel.get_model()->children().end(); ++iter)
  {
    const Glib::ustring name = m_AddDel.get_value(iter, m_colName);
    if(!name.empty())
    {
      Relationship relationship;
      relationship.set_name(name);
      relationship.set_title(m_AddDel.get_value(iter, m_colTitle));
      relationship.set_from_table(m_strTableName);
      relationship.set_from_field(m_AddDel.get_value(iter, m_colFromField));
      relationship.set_to_table(m_AddDel.get_value(iter, m_colToTable));
      relationship.set_to_field(m_AddDel.get_value(iter, m_colToField));
      relationship.set_allow_edit(m_AddDel.get_value_as_bool(iter, m_colAllowEdit));
      relationship.set_auto_create(m_AddDel.get_value_as_bool(iter, m_colAutoCreate));

      vecRelationships.push_back(relationship);
    }
  }

  //Update the Document with these relationships.
  get_document()->set_relationships(m_strTableName, vecRelationships);

  //Call base:
  Box_DB_Table::save_to_document();
}

void Box_DB_Table_Relationships::on_adddel_user_added(const Gtk::TreeModel::iterator& row)
{
  const guint col_with_first_value = m_colName;
  
  if(col_with_first_value == m_colName)
  {
    //The name is the key:
    Glib::ustring new_name = m_AddDel.get_value(row, m_colName);
    if(!new_name.empty())
      m_AddDel.set_value_key(row, new_name);
      
    //Set a suitable starting title, if there is none already:
    Glib::ustring title = m_AddDel.get_value(row, m_colTitle);
    if(title.empty())
    {
      title = Base_DB::util_title_from_string(new_name);
      m_AddDel.set_value(row, m_colTitle, title);
    }
  }
}
  
void Box_DB_Table_Relationships::on_adddel_user_changed(const Gtk::TreeModel::iterator& row, guint col)
{
  if(col == m_colName)
  {
    //The name is the key, so If the name has been changed then the key must change too.
    Glib::ustring new_name = m_AddDel.get_value(row, m_colName);
    if(!new_name.empty())
      m_AddDel.set_value_key(row, new_name);

    //Update the title, if there is none already:
    Glib::ustring title = m_AddDel.get_value(row, m_colTitle);
    if(title.empty())
    {
      title = Base_DB::util_title_from_string(new_name);
      m_AddDel.set_value(row, m_colTitle, title);
    }
  }
  else if(col == m_colToTable)
  {
    //User chose a new table so we need to wipe the field
    //because it might not be a valid field for that table:
    m_AddDel.set_value(row, m_colToField, Glib::ustring("")); //Plain "" goes to bool override.
  }

  set_modified();
}

void Box_DB_Table_Relationships::on_adddel_user_activated(const Gtk::TreeModel::iterator& row, guint col)
{
  if(col == m_colToField)
  {
    Bakery::BusyCursor(*get_app_window());

    const Glib::ustring old_to_field = m_AddDel.get_value(row, m_colToField);

    const Glib::ustring table_name = m_AddDel.get_value(row, m_colToTable);

    if(!table_name.empty())
    {
      //Set list of 'To' fields depending on table:
      m_AddDel.set_value(row, m_colToField, Glib::ustring(""));

      sharedptr<SharedConnection> sharedconnection = connect_to_server();
      if(sharedconnection)
      {
        Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

        type_vecStrings vecFields = util_vecStrings_from_Fields(get_fields_for_table(table_name));

        //This would cause a lot of tedious re-filling:
        //m_AddDel.set_column_choices(m_colToField, vecFields);
        //fill_from_database();

        m_AddDel.set_column_choices(m_colToField, vecFields);

        //Attempt to set the previous value:
        m_AddDel.set_value(row, m_colToField, old_to_field);
      }
    }

  }
}

void Box_DB_Table_Relationships::on_adddel_user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& /* rowEnd TODO */)
{
  //Remove the row:
  m_AddDel.remove_item(rowStart);

  set_modified();
}
