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

#include "combochoices.h"
#include <libglom/data_structure/glomconversions.h>
#include <libglom/document/document.h>
#include <libglom/connectionpool.h>
#include <libglom/utils.h>
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl


namespace Glom
{

namespace DataWidgetChildren
{

ComboChoices::ComboChoices()
: m_related_show_all(false)
{
  init();
}

void ComboChoices::init()
{
}

ComboChoices::~ComboChoices()
{
}

bool ComboChoices::refresh_data_from_database_with_foreign_key(const Gnome::Gda::Value& foreign_key_value)
{
  if(!m_related_relationship || !m_related_field)
  {
    std::cerr << G_STRFUNC << ": !m_related_relationship or !m_related_field." << std::endl;
    return false;
  }

  if(Conversions::value_is_empty(foreign_key_value))
  {
    //Clear the choices list:
    type_list_values_with_second list_values;
    set_choices_with_second(list_values);
    return true;
  }

  if(m_related_show_all)
  {
    //The list should be set in set_choices_related() instead.
    std::cerr << G_STRFUNC << ": Called with m_related_show_all=true." << std::endl;
    return false;
  }

  Utils::type_vecLayoutFields fields;
  fields.push_back(m_related_field);
  if(m_related_field_second)
    fields.push_back(m_related_field_second);

  //std::cout << G_STRFUNC << "debug: m_related_field=" << m_related_field->get_name() << ", m_related_field_second" << m_related_field_second->get_name() << std::endl;

  if(!m_related_to_field)
  {
    std::cerr << G_STRFUNC << ": m_related_to_field is null." << std::endl;
  }

  //TODO: Support related relationships (in the UI too):
  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder = Utils::build_sql_select_with_key(
    m_related_relationship->get_to_table(),
    fields,
    m_related_to_field,
    foreign_key_value);

  if(!builder)
  {
    std::cerr << G_STRFUNC << ": builder is null." << std::endl;
    return false;
  }

  //TODO: builder->select_order_by(choice_field_id);

  //Connect to database and get the related values:
  sharedptr<SharedConnection> connection = ConnectionPool::get_instance()->connect();

  if(!connection)
  {
    std::cerr << G_STRFUNC << ": connection is null." << std::endl;
    return false;
  }

  const std::string sql_query =
    Utils::sqlbuilder_get_full_query(builder);
  //std::cout << "get_choice_values: Executing SQL: " << sql_query << std::endl;
  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = connection->get_gda_connection()->statement_execute_select(sql_query);

  if(datamodel)
  {
    type_list_values_with_second list_values;

    const guint count = datamodel->get_n_rows();
    const guint cols_count = datamodel->get_n_columns();
    //std::cout << "  result: count=" << count << std::endl;
    for(guint row = 0; row < count; ++row)
    {

      std::pair<Gnome::Gda::Value, Gnome::Gda::Value> itempair;
      itempair.first = datamodel->get_value_at(0, row);

      if(m_related_field_second && (cols_count > 1))
        itempair.second = datamodel->get_value_at(1, row);

      list_values.push_back(itempair);
    }

    const Gnome::Gda::Value old_value = get_value();
    set_choices_with_second(list_values);
    set_value(old_value); //Try to preserve the value, even in iter-based ComboBoxes.
  }
  else
  {
      std::cerr << G_STRFUNC << ": Error while executing SQL" << std::endl <<
                   "  " <<  sql_query << std::endl;
      return false;
  }

  return true;
}

void ComboChoices::set_choices_related(const Document* document, const sharedptr<const Relationship>& relationship, const Glib::ustring& /* field */, const Glib::ustring& /* field_second */, bool show_all)
{
  //Note that field_second is used in derived classes.

  m_related_relationship = relationship;
  m_related_show_all = show_all;

  m_related_field.clear();
  m_related_field_second.clear();
  if(m_related_relationship)
  {
    const Glib::ustring to_table = m_related_relationship->get_to_table();
    m_related_to_field = document->get_field(to_table, m_related_relationship->get_to_field());
  }

  type_list_values_with_second list_values;

  //Set the values now because if it will be the same regardless of the foreign key value.
  //Otherwise show them when refresh_data_from_database_with_foreign_key() is called.
  if(relationship && show_all)
  {
    sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::cast_dynamic(get_layout_item());
    list_values = Utils::get_choice_values_all(document, layout_item, m_related_field, m_related_field_second);
  }

  const Gnome::Gda::Value old_value = get_value();
  set_choices_with_second(list_values);
  set_value(old_value); //Try to preserve the value, even in iter-based ComboBoxes.
}

} //namespace DataWidgetChildren
} //namespace Glom
