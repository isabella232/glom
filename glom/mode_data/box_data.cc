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
#include "../data_structure/layout/layoutitem_field.h"
#include "../glom_python.h"
#include "config.h"
#include <libintl.h>

Box_Data::Box_Data()
: m_Button_Find(Gtk::Stock::FIND),
  m_pDialogLayout(0)
{
  m_bUnstoredData = false;
   

    
  //Connect signals:
  m_Button_Find.signal_clicked().connect(sigc::mem_fun(*this, &Box_Data::on_Button_Find));
}

Box_Data::~Box_Data()
{
}

void Box_Data::init_db_details(const Glib::ustring& strTableName, const Glib::ustring& strWhereClause)
{
  m_strWhereClause = strWhereClause;

  Box_DB_Table::init_db_details(strTableName);
}

void Box_Data::refresh_db_details(const Glib::ustring& strWhereClause)
{
  m_strWhereClause = strWhereClause;

  Box_DB_Table::refresh_db_details();
}

Glib::ustring Box_Data::get_WhereClause() const
{
  Glib::ustring strClause;

  //Look at each field entry and build e.g. 'Name = "Bob"'
  for(type_vecLayoutFields::const_iterator iter = m_Fields.begin(); iter != m_Fields.end(); ++iter)
  {
    Glib::ustring strClausePart;

    const Gnome::Gda::Value data = get_entered_field_data(*iter);

    if(!GlomConversions::value_is_empty(data))
    {
      const Field field = iter->m_field;

      bool use_this_field = true;
      if(field.get_glom_type() == Field::TYPE_BOOLEAN) //TODO: We need an intermediate state for boolean fields, so that they can be ignored in searches.
      {
        if(!data.get_bool())
          use_this_field = false;
      }

      if(use_this_field)
      {
        //TODO: "table.name"
        strClausePart = field.get_name() + " LIKE " +  field.sql(data); //% is mysql wildcard for 0 or more characters.
      }
    }

    if(!strClausePart.empty())
      strClause += strClausePart + " ";
  }

  return strClause;
}

void Box_Data::on_Button_Find()
{
  //Make sure that the cell is updated:
  //m_AddDel.finish_editing();

  signal_find.emit(get_WhereClause());
}

Glib::RefPtr<Gnome::Gda::DataModel> Box_Data::record_new(bool use_entered_data, const Gnome::Gda::Value& primary_key_value)
{
  Field fieldPrimaryKey;
  get_field_primary_key(fieldPrimaryKey);

  const Glib::ustring primary_key_name = fieldPrimaryKey.get_name();

  type_vecLayoutFields fieldsToAdd = m_Fields;

  //Add the primary key if it is not normally shown:
  type_vecLayoutFields::const_iterator iterFindPrimary = std::find_if(fieldsToAdd.begin(), fieldsToAdd.end(), predicate_FieldHasName<LayoutItem_Field>(primary_key_name));
  if(iterFindPrimary == fieldsToAdd.end())
  {
    LayoutItem_Field layout_item;
    layout_item.set_name(primary_key_name);
    layout_item.m_field = fieldPrimaryKey;

    fieldsToAdd.push_back(layout_item);
  }

  //Calculate any necessary field values and enter them:
  for(type_vecLayoutFields::const_iterator iter = m_Fields.begin(); iter != m_Fields.begin(); ++iter)
  {
    const LayoutItem_Field& layout_item = *iter;

    //If the user did not enter something in this field:
    Gnome::Gda::Value value = get_entered_field_data(layout_item);
    if(GlomConversions::value_is_empty(value)) //This deals with empty strings too.
    {
      const Field& field = layout_item.m_field;

      //If the default value should be calculated, then calculate it:
      const Glib::ustring calculation = field.get_calculation(); //TODO_Performance: Use a get_has_calculation() method.
      if(!calculation.empty())
      {
        Gnome::Gda::Value value = glom_evaluate_python_function_implementation(field.get_glom_type(), calculation);
        set_entered_field_data(layout_item, value);
      }
    }
  }

  //Get all entered field name/value pairs:
  Glib::ustring strNames;
  Glib::ustring strValues;

  //Avoid specifying the same field twice:
  typedef std::map<Glib::ustring, bool> type_map_added;
  type_map_added map_added;

  for(type_vecLayoutFields::const_iterator iter = fieldsToAdd.begin(); iter != fieldsToAdd.end(); ++iter)
  {
    const LayoutItem_Field& layout_item = *iter;
    const Glib::ustring field_name = layout_item.get_name();
    if(!layout_item.get_has_relationship_name()) //TODO: Allow people to add a related record also by entering new data in a related field of the related record.
    {
      type_map_added::const_iterator iterFind = map_added.find(field_name);
      if(iterFind == map_added.end()) //If it was not added already
      {
        Gnome::Gda::Value value;

        const Field& field = layout_item.m_field;

        //Use the specified (generated) primary key value, if there is one:
        if(primary_key_name == field_name && !GlomConversions::value_is_empty(primary_key_value))
        {
          value = primary_key_value;
        }
        else
        {
          if(use_entered_data || !field.get_calculation().empty()) //TODO_Performance: Use a get_has_calculation() method.
            value = get_entered_field_data(layout_item);
        }

        Glib::ustring strFieldValue = field.sql(value);

        if(!strFieldValue.empty())
        {
          if(!strNames.empty())
          {
            strNames += ", ";
            strValues += ", ";
          }

          strNames += field_name;
          strValues += strFieldValue;

          map_added[field_name] = true;
        }
      }
    }
  }

  //Put it all together to create the record with these field values:
  if(!strNames.empty() && !strValues.empty())
  {
    Glib::ustring strQuery = "INSERT INTO " + m_strTableName + " (" + strNames + ") VALUES (" + strValues + ")";
    return Query_execute(strQuery);
  }
  else
    return Glib::RefPtr<Gnome::Gda::DataModel>();
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
    Gtk::MessageDialog dialog(gettext("<b>No primary key value</b>"), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL );
    dialog.set_secondary_text(gettext("This data can not be stored in the database because you have not provided a primary key.\nDo you really want to discard this data?"));
    //TODO: dialog.set_transient_for(*this);
    int iButton = dialog.run();

    return (iButton == Gtk::RESPONSE_OK);
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
  //Re-fill view, in case the layout has changed:
  fill_from_database();
}

Box_Data::type_vecLayoutFields Box_Data::get_fields_to_show() const
{
  if(m_strTableName.empty())
  {
    return type_vecLayoutFields();
  }
  else
    return get_table_fields_to_show(m_strTableName);
}

void Box_Data::get_table_fields_to_show_add_group(const Glib::ustring& table_name, const Privileges& table_privs, const type_vecFields& all_db_fields, const LayoutGroup& group, Box_Data::type_vecLayoutFields& vecFields) const
{
  //g_warning("Box_Data::get_table_fields_to_show_add_group(): table_name=%s, all_db_fields.size()=%d, group.name=%s", table_name.c_str(), all_db_fields.size(), group.get_name().c_str());

  const Document_Glom* document = get_document();

  LayoutGroup::type_map_const_items items = group.get_items();
  for(LayoutGroup::type_map_const_items::const_iterator iterItems = items.begin(); iterItems != items.end(); ++iterItems)
  {
    const LayoutItem* item = iterItems->second;

    const LayoutItem_Field* item_field = dynamic_cast<const LayoutItem_Field*>(item);
    if(item_field)
    {
      //Get the field info:
      const Glib::ustring field_name = item->get_name();

      if(item_field->get_has_relationship_name()) //If it's a field in a related table.
      {
        //Get the full field information:
        const Glib::ustring relationship_name = item_field->get_relationship_name();
        Relationship relationship;
        bool test = document->get_relationship(table_name, relationship_name, relationship);
        if(test)
        {
          Field field;
          //TODO_Performance: get_fields_for_table_one_field() is probably very inefficient
          bool test = get_fields_for_table_one_field(relationship.get_to_table(), item->get_name(), field);
          if(test)
          {
            LayoutItem_Field layout_item = *item_field; //TODO_Performance: Reduce the copying.
            layout_item.m_field = field; //Fill in the full field information for later.

            //TODO_Performance: We do this once for each related field, even if there are 2 from the same table:
            const Privileges privs_related = get_current_privs(relationship.get_to_table());
            layout_item.m_priv_view = privs_related.m_view;
            layout_item.m_priv_edit = privs_related.m_edit;

            vecFields.push_back(layout_item);
          }
        }
      }
      else //It's a regular field in the table:
      {
        type_vecFields::const_iterator iterFind = std::find_if(all_db_fields.begin(), all_db_fields.end(), predicate_FieldHasName<Field>(field_name));

        //If the field does not exist anymore then we won't try to show it:
        if(iterFind != all_db_fields.end() )
        {
          LayoutItem_Field layout_item = *item_field; //TODO_Performance: Reduce the copying here.
          layout_item.m_field = *iterFind; //Fill the LayoutItem with the full field information.

          //Prevent editing of the field if the user may not edit this table:
          layout_item.m_priv_view = table_privs.m_view;
          layout_item.m_priv_edit = table_privs.m_edit;

          vecFields.push_back(layout_item);
        }
      }
    }
    else
    {
      const LayoutGroup* item_group = dynamic_cast<const LayoutGroup*>(item);
      if(item_group)
      {
        //Recurse:
        get_table_fields_to_show_add_group(table_name, table_privs, all_db_fields, *item_group, vecFields);
      }
    }
  }

  if(vecFields.empty())
  {
    //g_warning("Box_Data::get_table_fields_to_show_add_group(): Returning empty list.");
  } 
}

Box_Data::type_vecLayoutFields Box_Data::get_table_fields_to_show(const Glib::ustring& table_name) const
{
  const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
  if(pDoc)
  {
    Document_Glom::type_mapLayoutGroupSequence mapGroupSequence =  pDoc->get_data_layout_groups_plus_new_fields(m_layout_name, table_name);
    return get_table_fields_to_show(table_name, mapGroupSequence);
  }
  else
    return type_vecLayoutFields();
}

Box_Data::type_vecLayoutFields Box_Data::get_table_fields_to_show(const Glib::ustring& table_name, const Document_Glom::type_mapLayoutGroupSequence& mapGroupSequence) const
{
  //Get field definitions from the database, with corrections from the document:
  type_vecFields all_fields = get_fields_for_table(table_name);

  const Privileges table_privs = get_current_privs(table_name);

  //Get fields that the document says we should show:
  type_vecLayoutFields result;
  const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
  if(pDoc)
  {
    if(mapGroupSequence.empty())
    {
      //No field sequence has been saved in the document, so we use all fields by default, so we start with something visible:

      //Start with the Primary Key as the first field:
      guint iPrimaryKey = 0;
      bool bPrimaryKeyFound = get_field_primary_key_index(all_fields, iPrimaryKey);
      Glib::ustring primary_key_field_name;
      if(bPrimaryKeyFound)
      {
        LayoutItem_Field layout_item;
        layout_item.set_name(primary_key_field_name);
        layout_item.m_field = all_fields[iPrimaryKey];

        //Don't use thousands separators with ID numbers:
        layout_item.m_numeric_format.m_use_thousands_separator = false;

        layout_item.set_editable(true); //A sensible default.

        //Prevent editing of the field if the user may not edit this table:
        layout_item.m_priv_view = table_privs.m_view;
        layout_item.m_priv_edit = table_privs.m_edit;

        result.push_back(layout_item);
      }

      //Add the rest:
      for(type_vecFields::const_iterator iter = all_fields.begin(); iter != all_fields.end(); ++iter)
      {
        const Field& field_info = *iter;

        if(iter->get_name() != primary_key_field_name) //We already added the primary key.
        {
          LayoutItem_Field layout_item;
          layout_item.set_name(iter->get_name());
          layout_item.m_field = field_info;

          layout_item.set_editable(true); //A sensible default.

          //Prevent editing of the field if the user may not edit this table:
          layout_item.m_priv_view = table_privs.m_view;
          layout_item.m_priv_edit = table_privs.m_edit;

          result.push_back(layout_item);
        }
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
          get_table_fields_to_show_add_group(table_name, table_privs, all_fields, group, result);
        }
      }
    }
  }

  if(result.empty())
  {
    //g_warning("Box_Data::get_table_fields_to_show_add_group(): Returning empty list.");
  }

  return result;
}

Gnome::Gda::Value Box_Data::generate_next_auto_increment(const Glib::ustring& table_name, const Glib::ustring field_name)
{
  //This is a workaround for postgres problems. Ideally, we need to use the postgres serial type and find out how to get the generated value after we add a row.

  double result = 0;
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = Query_execute("SELECT max(" + field_name + ") FROM " + table_name);
  if(!data_model && data_model->get_n_rows())
    handle_error();
  else
  {
    //The result should be 1 row with 1 column
    Gnome::Gda::Value value = data_model->get_value_at(0, 0);

    //It's a Gnome::Gda::Value::TYPE_NUMERIC, but the GdaNumeric struct is not easy to handle, so let's hack around it:
    //if(value.is_number())
    //  result = value.get_integer();
    //else
    result = util_decimal_from_string(value.to_string());

    ++result; 
  }

  //Get a string representation of the number, so we can put it back in a NUMERIC Gda::Value:

  return GlomConversions::parse_value(result);
}

/** Get the fields that are in related tables, via a relationship using @a field_name changes.
 */
Box_Data::type_vecLayoutFields Box_Data::get_related_fields(const Glib::ustring& field_name) const
{
  type_vecLayoutFields result;

  const Document_Glom* document = dynamic_cast<const Document_Glom*>(get_document());
  if(document)
  {
    for(type_vecLayoutFields::const_iterator iter = m_Fields.begin(); iter != m_Fields.end();  ++iter)
    {
      const LayoutItem_Field& layout_field = *iter;
      //Examine each field that looks up its data from a relationship:
      if(layout_field.get_has_relationship_name())
      {
        //Get the relationship information:
        Relationship relationship;
        bool test = document->get_relationship(m_strTableName, layout_field.get_relationship_name(), relationship);
        if(test)
        {
          //If the relationship uses the specified field:
          if(relationship.get_from_field() == field_name)
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

/** Get the fields whose values should be looked up when @a field_name changes, with
 * the relationship used to lookup the value.
 */
Box_Data::type_list_lookups Box_Data::get_lookup_fields(const Glib::ustring& field_name) const
{
  type_list_lookups result;

  const Document_Glom* document = dynamic_cast<const Document_Glom*>(get_document());
  if(document)
  {
    for(type_vecLayoutFields::const_iterator iter = m_Fields.begin(); iter != m_Fields.end();  ++iter)
    {
      const Field& field = iter->m_field;
      //Examine each field that looks up its data from a relationship:
      if(field.get_is_lookup())
      {
        //Get the relationship information:
        Relationship relationship;
        bool test = document->get_relationship(m_strTableName, field.get_lookup_relationship(), relationship);
        if(test)
        {
          //If the relationship is triggererd by the specified field:
          if(relationship.get_from_field() == field_name)
          {
            //Add it:
            LayoutItem_Field item;
            item.set_name(field.get_name());
            item.m_field = field;
            result.push_back( type_pairFieldTrigger(item, relationship) );
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
  
Document_Glom::type_mapLayoutGroupSequence Box_Data::get_data_layout_groups(const Glib::ustring& layout)
{
  Document_Glom::type_mapLayoutGroupSequence layout_groups;

  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  if(document)
  {
    if(!m_strTableName.empty())
    {
      //Get the layout information from the document:
      layout_groups = document->get_data_layout_groups_plus_new_fields(layout, m_strTableName);

      const Privileges table_privs = get_current_privs(m_strTableName);

      //Fill in the field information for the fields mentioned in the layout:
      for(Document_Glom::type_mapLayoutGroupSequence::iterator iterGroups = layout_groups.begin(); iterGroups != layout_groups.end(); ++iterGroups)
      {
        fill_layout_group_field_info(iterGroups->second, table_privs);
      }
    }
  }

  return layout_groups;
}

void Box_Data::fill_layout_group_field_info(LayoutGroup& group, const Privileges& table_privs)
{ 
  const Document_Glom* document = get_document();

  LayoutGroup::type_map_items items = group.get_items();
  for(LayoutGroup::type_map_items::iterator iter = items.begin(); iter != items.end(); ++iter)
  {
    LayoutItem* item = iter->second;
    LayoutItem_Field* item_field = dynamic_cast<LayoutItem_Field*>(item);
    if(item_field) //If is a field rather than some other layout item
    {

      if(item_field->get_has_relationship_name()) //If it's a field in a related table.
      {
        //Get the full field information:
        const Glib::ustring relationship_name = item_field->get_relationship_name();
        Relationship relationship;
        bool test = document->get_relationship(m_strTableName, relationship_name, relationship);
        if(test)
        {
          Field field;
          //TODO: get_fields_for_table_one_field() is probably very inefficient
          bool test = get_fields_for_table_one_field(relationship.get_to_table(), item->get_name(), field);
          if(test)
          {
            item_field->m_field = field;

            //TODO_Performance: Don't do this repeatedly for the same table.
            const Privileges privs = get_current_privs(relationship.get_to_table());
            item_field->m_priv_view = privs.m_view;
            item_field->m_priv_edit = privs.m_edit;
          }
        }
      }
      else
      {
        //Get the field info:
        Field field;
        bool found = get_fields_for_table_one_field(m_strTableName, item_field->get_name(), field);
        if(found)
        {
          item_field->m_field = field; //TODO_Performance: Just use this as the output arg?
          item_field->m_priv_view = table_privs.m_view;
          item_field->m_priv_edit = table_privs.m_edit;
        }
      }
    }
    else
    {
      LayoutGroup* item_group = dynamic_cast<LayoutGroup*>(item);
      if(item_group) //If it is a group
      {
        //recurse, to fill the fields info in this group:
        fill_layout_group_field_info(*item_group, table_privs);
      }
    }
  }
}

//static:
bool Box_Data::get_field_primary_key_index(const type_vecFields& fields, guint& field_column)
{
  //Initialize input parameter:
  field_column = 0;

  //TODO_performance: Cache the primary key?
  guint col = 0;
  guint cols_count = fields.size();
  while(col < cols_count)
  {
    if(fields[col].get_field_info().get_primary_key())
    {
      field_column = col;
      return true;
    }
    else
    {
      ++col;
    }
  }

  return false; //Not found.
}

//static:
bool Box_Data::get_field_primary_key_index(const type_vecLayoutFields& fields, guint& field_column)
{
  //Initialize input parameter:
  field_column = 0;

  //TODO_performance: Cache the primary key?
  guint col = 0;
  guint cols_count = fields.size();
  while(col < cols_count)
  {
    if(fields[col].m_field.get_field_info().get_primary_key())
    {
      field_column = col;
      return true;
    }
    else
    {
      ++col;
    }
  }

  return false; //Not found.
}

bool Box_Data::record_delete(const Gnome::Gda::Value& primary_key_value)
{

  Field field_primary_key;
  bool test = get_field_primary_key(field_primary_key);
  if(test && !GlomConversions::value_is_empty(primary_key_value))
  {
    return Query_execute( "DELETE FROM " + m_strTableName + " WHERE " + m_strTableName + "." + field_primary_key.get_name() + " = " + field_primary_key.sql(primary_key_value) );
  }
  else
  {
    return false; //no primary key
  }
}

bool Box_Data::get_field(const Glib::ustring& name, Field& field) const
{
  type_vecLayoutFields::const_iterator iterFind = std::find_if( m_Fields.begin(), m_Fields.end(), predicate_FieldHasName<LayoutItem_Field>(name) );
  if(iterFind != m_Fields.end()) //If it was found:
  {
    field = iterFind->m_field;
    return true;
  }
  else
  {
    return false; //not found.
  }
}

Glib::ustring Box_Data::build_sql_select(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const Field& primary_key_field, const Gnome::Gda::Value& primary_key_value)
{
  if(!GlomConversions::value_is_empty(primary_key_value)) //If there is a record to show:
  {
    const Glib::ustring where_clause = m_strTableName + "." + primary_key_field.get_name() + " = " + primary_key_field.sql(primary_key_value);
    return build_sql_select_with_where_clause(table_name, fieldsToGet, where_clause);
  }

  return Glib::ustring();
}

Glib::ustring Box_Data::build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const Glib::ustring& where_clause)
{
  Glib::ustring result;

  Document_Glom* document = get_document();

  Glib::ustring sql_part_fields;

  typedef std::list<Relationship> type_list_relationships;
  type_list_relationships list_relationships;

  for(type_vecLayoutFields::const_iterator iter =  fieldsToGet.begin(); iter != fieldsToGet.end(); ++iter)
  {
    if(iter != fieldsToGet.begin())
      sql_part_fields += ", ";


    if(!iter->get_has_relationship_name())
    {
      sql_part_fields += ( table_name + "." );
    }
    else
    {
      Glib::ustring relationship_name = iter->get_relationship_name();
      Relationship relationship;
      bool test = document->get_relationship(table_name, relationship_name, relationship);
      if(test)
      {
        const Glib::ustring field_table_name = relationship.get_to_table();
        sql_part_fields += ( field_table_name + "." );

        //Add the relationship to the list:
        type_list_relationships::const_iterator iterFind = std::find_if(list_relationships.begin(), list_relationships.end(), predicate_FieldHasName<Relationship>( relationship_name ) );
        if(iterFind == list_relationships.end()) //If the table is not yet in the list:
          list_relationships.push_back(relationship);
      }
    }

    sql_part_fields += iter->get_name();
  }

  result =  "SELECT " + sql_part_fields +
    " FROM " + table_name;

  //LEFT OUTER JOIN will get the field values from the other tables, and give us our fields for this table even if there is no corresponding value in the other table.
  Glib::ustring sql_part_leftouterjoin; 
  for(type_list_relationships::const_iterator iter = list_relationships.begin(); iter != list_relationships.end(); ++iter)
  {
    const Relationship& relationship = *iter;
    sql_part_leftouterjoin += " LEFT OUTER JOIN " + relationship.get_to_table() +
      " ON (" + relationship.get_from_table() + "." + relationship.get_from_field() + " = " +
      relationship.get_to_table() + "." + relationship.get_to_field() +
      ")";
  }

  result += sql_part_leftouterjoin;

  if(!where_clause.empty())
    result += " WHERE " + where_clause;

  return result;
}

Glib::ustring Box_Data::get_layout_item_table_name(const LayoutItem_Field& layout_item, const Glib::ustring table_name)
{
  if(!layout_item.get_has_relationship_name())
    return table_name;
  else
  {
    const Glib::ustring relationship_name = layout_item.get_relationship_name();
    Relationship relationship;
    Document_Glom* document = get_document();
    bool test = document->get_relationship(table_name, relationship_name, relationship);
    if(test)
     return relationship.get_to_table();
  }

  return Glib::ustring();
}

bool Box_Data::get_related_record_exists(const Relationship& relationship, const Field& key_field, const Gnome::Gda::Value& key_value)
{
  bool result = false;

  //TODO_Performance: It's very possible that this is slow.
  //We don't care how many records there are, only whether there are more than zero.
  const Glib::ustring to_field = relationship.get_to_table() = relationship.get_to_field();

  //TODO_Performance: Is this the best way to just find out whether there is one record that meets this criteria?
  const Glib::ustring query = "SELECT " + to_field + " FROM " + relationship.get_to_table() + " WHERE " + to_field + " = " + key_field.sql(key_value);
  Glib::RefPtr<Gnome::Gda::DataModel> records = Query_execute(query);
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

bool Box_Data::add_related_record_for_field(const LayoutItem_Field& layout_item_parent, const Relationship& relationship, const Field& primary_key_field, const Gnome::Gda::Value& primary_key_value_provided)
{
  Gnome::Gda::Value primary_key_value = primary_key_value_provided;

  const bool related_record_exists = get_related_record_exists(relationship, primary_key_field, primary_key_value);
  if(related_record_exists)
  {
    //No problem, the SQL command below will update this value in the related table.
  }
  else
  {
    //To store the entered data in the related field, we would first have to create a related record.
    if(!relationship.get_auto_create())
    {
      //Warn the user:
      //TODO: Make the field insensitive until it can receive data, so people never see this dialog.
      Gtk::MessageDialog dialog(gettext("<b>Related Record Does Not Exist</b>"), true);
      dialog.set_secondary_text(gettext("Data may not be entered into this related field, because the related record does not yet exist, and the relationship does not allow automatic creation of new related records."));
      dialog.set_transient_for(*get_app_window());
      dialog.run();

      //Clear the field again, discarding the entered data.
      set_entered_field_data(layout_item_parent, Gnome::Gda::Value());

      return false;
    }
    else
    {
      const bool key_is_auto_increment = primary_key_field.get_field_info().get_auto_increment();

      //If we would have to set an otherwise auto-increment key to add the record.
      if( key_is_auto_increment && !GlomConversions::value_is_empty(primary_key_value) )
      {
        //Warn the user:
        //TODO: Make the field insensitive until it can receive data, so people never see this dialog.
        Gtk::MessageDialog dialog(gettext("<b>Related Record Can Not Be Created</b>"), true);
        //TODO: This is a very complex error message:
        dialog.set_secondary_text(gettext("Data may not be entered into this related field, because the related record does not yet exist, and the key in the related record is auto-generated and therefore can not be created with the key value in this record."));
        dialog.set_transient_for(*get_app_window());
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
          primary_key_value = generate_next_auto_increment(relationship.get_to_table(), primary_key_field.get_name());

          //Generate the new key value;
        }

        const Glib::ustring strQuery = "INSERT INTO " + relationship.get_to_table() + " (" + primary_key_field.get_name() + ") VALUES (" + primary_key_field.sql(primary_key_value) + ")";
        bool test = Query_execute(strQuery);
        if(test)
        {
          if(key_is_auto_increment)
          {
            //Set the key in the parent table
            LayoutItem_Field item_from_key;
            item_from_key.set_name(relationship.get_from_field());

            //Show the new from key in the parent table's layout:
            set_entered_field_data(item_from_key, primary_key_value);

            //Set it in the database too:
            Field field_from_key;
            bool test = get_fields_for_table_one_field(relationship.get_from_table(), relationship.get_from_field(), field_from_key); //TODO_Performance.
            if(test)
            {
              Field parent_primary_key_field;
              bool test = get_field_primary_key(parent_primary_key_field);
              if(!test)
                return false;
              else
              {
                const Gnome::Gda::Value parent_primary_key_value = get_primary_key_value_selected();

                const Glib::ustring strQuery = "UPDATE  " + relationship.get_from_table() + " SET " + relationship.get_from_field() + " = " + primary_key_field.sql(primary_key_value) +
                  " WHERE " + relationship.get_from_table() + "." + parent_primary_key_field.get_name() + " = " + parent_primary_key_field.sql(parent_primary_key_value);
                bool test = Query_execute(strQuery);
                return test;
              }
            }
          }
        }
      }
    }

  }

  return true;
}
