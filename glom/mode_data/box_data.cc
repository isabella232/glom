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

#include "box_data.h"
#include "../data_structure/glomconversions.h"
#include "config.h"
#include <libintl.h>

Box_Data::Box_Data()
: m_Button_Find(gettext("Find")),
  m_pDialogLayout(0)
{
  m_bUnstoredData = false;

  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_data_layout"); //TODO: Use a generic layout dialog?
  if(refXml)
  {
    refXml->get_widget_derived("window_data_layout", m_pDialogLayout);
    if(m_pDialogLayout)
      m_pDialogLayout->signal_hide().connect( sigc::mem_fun(*this, &Box_Data::on_dialog_layout_hide) );
  }
    
  //Connect signals:
  m_Button_Find.signal_clicked().connect(sigc::mem_fun(*this, &Box_Data::on_Button_Find));
}

Box_Data::~Box_Data()
{
}



Glib::ustring Box_Data::get_WhereClause() const
{
  Glib::ustring strClause;

  //Look at each field entry and build e.g. 'Name = "Bob"'
  for(type_vecFields::const_iterator iter = m_Fields.begin(); iter != m_Fields.end(); ++iter)
  {
    const Field& field = *iter;
    Glib::ustring strClausePart;

     g_warning("Box_Data::get_WhereClause(): field: name=%s", field.get_name().c_str());
     
    const Gnome::Gda::Value data = get_entered_field_data(field);
    
    if(!GlomConversions::value_is_empty(data))
    {
      Gnome::Gda::FieldAttributes fieldInfo = field.get_field_info();
      //TODO: "table.name"
      strClausePart = fieldInfo.get_name() + " LIKE " +  field.sql(data); //% is mysql wildcard for 0 or more characters.
    }

    if(!strClausePart.empty())
      strClause += strClausePart + " ";
  }

  g_warning("Box_Data::get_WhereClause(): %s", strClause.c_str());
  return strClause;
}

void Box_Data::on_Button_Find()
{
  //Make sure that the cell is updated:
  //m_AddDel.finish_editing();

  signal_find.emit(get_WhereClause());
}

bool Box_Data::record_new_from_entered()
{
  //This should not be used it the table has an auto-increment primary key:

  //Get all entered field name/value pairs:
  Glib::ustring strNames;
  Glib::ustring strValues;

  for(type_vecFields::const_iterator iter = m_Fields.begin(); iter != m_Fields.end(); ++iter)
  {
    const Field& field = *iter;
    Gnome::Gda::Value value = get_entered_field_data(field);
      
    Gnome::Gda::FieldAttributes fieldInfo = field.get_field_info();
    Glib::ustring strFieldValue = field.sql(value);

    if(!strFieldValue.empty())
    {
      if(!strNames.empty())
      {
        strNames += ", ";
        strValues += ", ";
      }

      strNames += field.get_name();;
      strValues += strFieldValue;
    }
  }

  //Put it all together to create the record with these field values:
  if(!strNames.empty() && !strValues.empty())
  {
    Glib::ustring strQuery = "INSERT INTO " + m_strTableName + " (" + strNames + ") VALUES (" + strValues + ")";
    return Query_execute(strQuery);
  }
  else
  {
    return false; //failed.
  }
}

void Box_Data::set_unstored_data(bool bVal)
{
  m_bUnstoredData = bVal;
}

bool Box_Data::get_unstored_data() const
{
  return m_bUnstoredData;
}

void Box_Data::fill_from_database()
{
  set_unstored_data(false);

  Box_DB_Table::fill_from_database();
}

bool Box_Data::confirm_discard_unstored_data() const
{
  if(get_unstored_data())
  {
    //Ask user to confirm loss of data:
    Gtk::MessageDialog dialog(gettext("This data can not be stored in the database because you have not provided a primary key.\nDo you really want to discard this data?"),
      false, Gtk::MESSAGE_QUESTION, (Gtk::ButtonsType)(Gtk::BUTTONS_OK | Gtk::BUTTONS_CANCEL) );
    int iButton = dialog.run();

    return (iButton == 0); //0 for YES, 1 for NO.
  }
  else
  {
    return true; //no data to lose.
  }
}

void Box_Data::show_layout_dialog()
{
  if(m_pDialogLayout)
  {
    m_pDialogLayout->set_document(m_layout_name, get_document(), m_strTableName, m_Fields);
    m_pDialogLayout->show();
  }
}

void Box_Data::on_dialog_layout_hide()
{
  //Re-fill view, in case the layout has changed;
  fill_from_database();
}

Box_Data::type_vecFields Box_Data::get_fields_to_show() const
{
  return get_table_fields_to_show(m_strTableName);
}

Box_Data::type_vecFields Box_Data::get_table_fields_to_show(const Glib::ustring& table_name) const
{
  //Get field definitions from the database, with corrections from the document:
  type_vecFields all_fields = get_fields_for_table(table_name);

  //Get fields that the document says we should show:
  type_vecFields result;
  const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
  if(pDoc)
  {
    Document_Glom::type_mapLayoutGroupSequence mapGroupSequence =  pDoc->get_data_layout_groups_plus_new_fields(m_layout_name, table_name);
    if(mapGroupSequence.empty())
    {
      //No field sequence has been saved in the document, so we use all fields by default, so we start with something visible:
   
      //Start with the Primary Key as the first field:
      guint iPrimaryKey = 0;
      bool bPrimaryKeyFound = get_field_primary_key(all_fields, iPrimaryKey);
      Glib::ustring primary_key_field_name;
      if(bPrimaryKeyFound)
      {
        primary_key_field_name = all_fields[iPrimaryKey].get_name();
        result.push_back( all_fields[iPrimaryKey] );
      }

      //Add the rest:
      for(type_vecFields::const_iterator iter = all_fields.begin(); iter != all_fields.end(); ++iter)
      {
        Field field_info = *iter;

        if(iter->get_name() != primary_key_field_name) //We already added the primary key.
          result.push_back(field_info);
      }
    }
    else
    {
      type_vecFields vecFieldsInDocument = pDoc->get_table_fields(table_name);
      
      //We will show the fields that the document says we should:
      for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = mapGroupSequence.begin(); iter != mapGroupSequence.end(); ++iter)
      {
        const LayoutGroup& group = iter->second;

        if(true) //!group.m_hidden)
        {
          //Get the fields:
          for(LayoutGroup::type_map_items::const_iterator iterItems = group.m_map_items.begin(); iterItems != group.m_map_items.end(); ++iterItems)
          {
            const LayoutItem& item = iterItems->second;
            
            //Get the field info:
            Glib::ustring field_name = item.m_field_name;
            type_vecFields::const_iterator iterFind = std::find_if(all_fields.begin(), all_fields.end(), predicate_FieldHasName<Field>(field_name));

            //If the field does not exist anymore then we won't try to show it:
            if(iterFind != all_fields.end() )
            {
               result.push_back(*iterFind);
            }
          }
        }
        
      }
    }
  }

  return result;
}

guint Box_Data::generate_next_auto_increment(const Glib::ustring& table_name, const Glib::ustring field_name)
{
  //This is a workaround for postgres problems. Ideally, we need to use the postgres serial type and find out how to get the generated value after we add a row.

  guint result = 0;
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = Query_execute("SELECT max(" + field_name + ") FROM " + table_name);
  if(!data_model && data_model->get_n_rows())
    handle_error();
  else
  {
    //The result should be 1 row with 1 column
    Gnome::Gda::Value value = data_model->get_value_at(0, 0);
    
    //It probably has a specific numeric type, but I am being lazy. murrayc
    result = util_decimal_from_string(value.to_string());
    ++result;
  }

  return result;
}

/** Get the fields whose values should be looked up when @a field_name changes, with
 * the relationship used to lookup the value.
 */
Box_Data::type_list_lookups Box_Data::get_lookup_fields(const Glib::ustring& field_name) const
{
  type_list_lookups result;

  const Document_Glom* document = dynamic_cast<const Document_Glom*>(get_document());
  if(document)
  {
    for(type_vecFields::const_iterator iter = m_Fields.begin(); iter != m_Fields.end();  ++iter)
    {
      //Examine each field that looks up its data from a relationship:
      if(iter->get_is_lookup())
      {
        //Get the relationship information:
        Relationship relationship;
        bool test = document->get_relationship(m_strTableName, iter->get_lookup_relationship(), relationship);
        if(test)
        {
          //If the relationship is triggererd by the specified field:
          if(relationship.get_from_field() == field_name)
          {
            //Add it:
            result.push_back( type_pairFieldTrigger(*iter, relationship) );
          }
        }
      }
    }
  }

  return result;
}

Gnome::Gda::Value Box_Data::get_lookup_value(const Relationship& relationship, const Field& source_field, const Gnome::Gda::Value& key_value)
{
  Gnome::Gda::Value result;
  
  Field to_key_field;
  bool test = get_fields_for_table_one_field(relationship.get_to_table(), relationship.get_to_field(), to_key_field);
  if(test)
  {
    Glib::ustring strQuery = "SELECT " + relationship.get_to_table() + "." + source_field.get_name() + " FROM " +  relationship.get_to_table();
    strQuery += " WHERE " + relationship.get_to_field() + " = " + to_key_field.sql(key_value);

    Glib::RefPtr<Gnome::Gda::DataModel> data_model = Query_execute(strQuery);
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
  



  
  
