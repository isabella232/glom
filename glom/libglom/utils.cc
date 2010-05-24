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

#include <libglom/libglom_config.h>

#include <libglom/utils.h>
#include <libglom/connectionpool.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/glomconversions.h>

#include <glibmm/i18n.h>

#include <giomm.h>

#include <string.h> // for strchr
#include <sstream> //For stringstream

#include <iostream>
#include <fstream>

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl
#include <iomanip>

#include <stack>

namespace Glom
{

template<class T_Element>
class predicate_UsesRelationshipHasRelationships
{
public:
  predicate_UsesRelationshipHasRelationships(const Glib::ustring& relationship_name, const Glib::ustring& related_relationship_name = Glib::ustring())
  : m_relationship_name(relationship_name),
    m_related_relationship_name(related_relationship_name)
  {
  }

  predicate_UsesRelationshipHasRelationships(const sharedptr<const UsesRelationship> uses_relationship_name, bool first_level_only = false)
  : m_relationship_name(uses_relationship_name->get_relationship_name()),
    m_related_relationship_name(uses_relationship_name->get_related_relationship_name())
  {
    //If first_level_only, search for relationships that have the same top-level relationship, but have no related relationship.
    if(first_level_only)
      m_related_relationship_name = Glib::ustring();
  }

  virtual ~predicate_UsesRelationshipHasRelationships()
  {
  }

  bool operator() (const T_Element& element)
  {
    return (element.get_relationship_name() == m_relationship_name) && (element.get_related_relationship_name() == m_related_relationship_name);
  }

  bool operator() (const sharedptr<T_Element>& element)
  {
    return (element->get_relationship_name() == m_relationship_name) && (element->get_related_relationship_name() == m_related_relationship_name);
  }

  bool operator() (const sharedptr<const T_Element>& element)
  {
    return (element->get_relationship_name() == m_relationship_name) && (element->get_related_relationship_name() == m_related_relationship_name);
  }

private:
  Glib::ustring m_relationship_name, m_related_relationship_name;
};


Glib::ustring Utils::trim_whitespace(const Glib::ustring& text)
{
  //TODO_Performance:

  Glib::ustring result = text;

  //Find non-whitespace from front:
  Glib::ustring::size_type posFront = Glib::ustring::npos;
  Glib::ustring::size_type pos = 0;
  for(Glib::ustring::iterator iter = result.begin(); iter != result.end(); ++iter)
  {
    if(!Glib::Unicode::isspace(*iter))
    {
      posFront = pos;
      break;
    }

    ++pos;
  }

  //Remove the white space from the front:
  result = result.substr(posFront);


 //Find non-whitespace from back:
  Glib::ustring::size_type posBack = Glib::ustring::npos;
  pos = 0;
  for(Glib::ustring::reverse_iterator iter = result.rbegin(); iter != result.rend(); ++iter)
  {
    if(!Glib::Unicode::isspace(*iter))
    {
      posBack = pos;
      break;
    }

    ++pos;
  }

  //Remove the white space from the front:
  result = result.substr(0, result.size() - posBack);

  return result;
}

Glib::ustring Utils::string_replace(const Glib::ustring& src, const Glib::ustring search_for, const Glib::ustring& replace_with)
{
  if(search_for.empty())
  {
    std::cerr << "Utils::string_replace(): search_for was empty." << std::endl;
    return src;
  }

  //std::cout << "debug: Utils::string_replace(): src=" << src << ", search_for=" << search_for << ", replace_with=" << replace_with << std::endl;

  std::string result = src;

  std::string::size_type pos = 0;
  const std::string::size_type len_search = search_for.size();
  const std::string::size_type len_replace = replace_with.size();

  std::string::size_type pos_after_prev = 0;
  while((pos = result.find(search_for, pos_after_prev)) != std::string::npos)
  {
    //std::cout << "  debug: pos=" << pos << ", found=" << search_for << ", in string: " << result.substr(pos_after_prev, 20) << std::endl;
    //std::cout << "  debug: before: result =" << result << ", pos_after_prev=pos_after_prev" << std::endl;
    result.replace(pos, len_search, replace_with);
    //std::cout << "  after: before: result = result" << std::endl;
    pos_after_prev = pos + len_replace;
  }

  return result;

/*
  //TODO_Performance:

  Glib::ustring result;
  const size_t src_length = src.size();
  const size_t search_for_length = search_for.size();
  //const size_t replace_with_length = replace_with.size();

  size_t src_index = 0;
  size_t src_index_section_start = 0;
  while(src_index < src_length)
  {
    const bool found = (src.find(search_for.c_str(), src_index) == src_index);
    if(found)
    {
      result += src.substr(src_index_section_start, src_index - src_index_section_start);
      result += replace_with;

      src_index_section_start = src_index + search_for_length;
      src_index = src_index_section_start;
    }
    else
      ++src_index;
  }

  if(src_index_section_start < src_length)
  {
    result += src.substr(src_index_section_start);
  }

  return result;
*/
}


Glib::RefPtr<Gnome::Gda::SqlBuilder> Utils::build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const Gnome::Gda::SqlExpr& where_clause, const Glib::ustring& extra_join, const type_sort_clause& sort_clause, const Glib::ustring& extra_group_by, guint limit)
{
  //TODO_Performance:
  type_vecConstLayoutFields constFieldsToGet;
  for(type_vecLayoutFields::const_iterator iter = fieldsToGet.begin(); iter != fieldsToGet.end(); ++iter)
  {
    constFieldsToGet.push_back(*iter);
  }

  return build_sql_select_with_where_clause(table_name, constFieldsToGet, where_clause, extra_join, sort_clause, extra_group_by, limit);
}


typedef std::list< sharedptr<const UsesRelationship> > type_list_relationships;

static void add_to_relationships_list(type_list_relationships& list_relationships, const sharedptr<const LayoutItem_Field>& layout_item)
{
  g_return_if_fail(layout_item);

  if(!(layout_item->get_has_relationship_name()))
    return;

  //If this is a related relationship, add the first-level relationship too, so that the related relationship can be defined in terms of it:
  type_list_relationships::const_iterator iterFind = std::find_if(list_relationships.begin(), list_relationships.end(), predicate_UsesRelationshipHasRelationships<UsesRelationship>(layout_item, true /* top_level_only */) );
  if(iterFind == list_relationships.end()) //If the table is not yet in the list:
  {
    sharedptr<UsesRelationship> uses_rel = sharedptr<UsesRelationship>::create();
    uses_rel->set_relationship(layout_item->get_relationship());
    list_relationships.push_front(uses_rel); //These need to be at the front, so that related relationships can use them later in the SQL statement.
  }

  //Add the relationship to the list:
  iterFind = std::find_if(list_relationships.begin(), list_relationships.end(), predicate_UsesRelationshipHasRelationships<UsesRelationship>(layout_item) );
  if(iterFind == list_relationships.end()) //If the table is not yet in the list:
  {
    sharedptr<UsesRelationship> uses_rel = sharedptr<UsesRelationship>::create();
    uses_rel->set_relationship(layout_item->get_relationship());
    uses_rel->set_related_relationship(layout_item->get_related_relationship());
    list_relationships.push_back(uses_rel);
  }


}

void Utils::build_sql_select_add_fields_to_get(const Glib::RefPtr<Gnome::Gda::SqlBuilder>& builder, const Glib::ustring& table_name, const type_vecConstLayoutFields& fieldsToGet, const type_sort_clause& sort_clause)
{
  //Get all relationships used in the query:
  typedef std::list< sharedptr<const UsesRelationship> > type_list_relationships;
  type_list_relationships list_relationships;

  for(type_vecConstLayoutFields::const_iterator iter = fieldsToGet.begin(); iter != fieldsToGet.end(); ++iter)
  {
    sharedptr<const LayoutItem_Field> layout_item = *iter;
    add_to_relationships_list(list_relationships, layout_item);
  }

  for(type_sort_clause::const_iterator iter = sort_clause.begin(); iter != sort_clause.end(); ++iter)
  {
    sharedptr<const LayoutItem_Field> layout_item = iter->first;
    add_to_relationships_list(list_relationships, layout_item);
  }
  
  
  //LEFT OUTER JOIN will get the field values from the other tables,
  //and give us our fields for this table even if there is no corresponding value in the other table.
  for(type_list_relationships::const_iterator iter = list_relationships.begin(); iter != list_relationships.end(); ++iter)
  {
    sharedptr<const UsesRelationship> uses_relationship = *iter;
    sharedptr<const Relationship> relationship = uses_relationship->get_relationship();
    if(relationship->get_has_fields()) //TODO: Handle related_record has_fields.
    {
      // Define the alias name as returned by get_sql_join_alias_name():

      // Specify an alias, to avoid ambiguity when using 2 relationships to the same table.
		  const Glib::ustring alias_name = uses_relationship->get_sql_join_alias_name();

		  // Add the JOIN:
		  if(!uses_relationship->get_has_related_relationship_name())
		  {
		    const guint to_target_id = builder->select_add_target(relationship->get_to_table(), alias_name);

		    builder->select_join_targets(
		      builder->select_add_target(relationship->get_from_table()),
		      to_target_id,
		      Gnome::Gda::SQL_SELECT_JOIN_LEFT,
		      builder->add_cond(
		        Gnome::Gda::SQL_OPERATOR_TYPE_EQ,
		        builder->add_id("\"" + relationship->get_from_table() + "\".\"" + relationship->get_from_field() + "\""),
		        builder->add_id("\"" + alias_name + "\".\"" + relationship->get_to_field() + "\"") ) );
		  }
		  else
		  {
		     UsesRelationship parent_relationship;
		     parent_relationship.set_relationship(relationship);
		     sharedptr<const Relationship> related_relationship = uses_relationship->get_related_relationship();

		     const guint to_target_id = builder->select_add_target(related_relationship->get_to_table(), alias_name);

		     builder->select_join_targets(
		       builder->select_add_target(relationship->get_from_table()), //TODO: Must we use the ID from select_add_target_id()?
		       to_target_id,
		       Gnome::Gda::SQL_SELECT_JOIN_LEFT,
		       builder->add_cond(
		         Gnome::Gda::SQL_OPERATOR_TYPE_EQ,
		         builder->add_id("\"" + parent_relationship.get_sql_join_alias_name() + "\".\"" + related_relationship->get_from_field() + "\""),
		         builder->add_id("\"" + alias_name + "\".\"" + related_relationship->get_to_field() + "\"") ) );
		  }
    }
    else if(relationship->get_has_to_table())
    {
      //It is a relationship that only specifies the table, without specifying linking fields:
      builder->select_add_target(relationship->get_to_table());
    }
  }
  
  bool one_added = false;
  for(type_vecConstLayoutFields::const_iterator iter = fieldsToGet.begin(); iter != fieldsToGet.end(); ++iter)
  {
    Glib::ustring one_sql_part;

    sharedptr<const LayoutItem_Field> layout_item = *iter;
    if(!layout_item)
    {
      g_warn_if_reached();
      continue;
    }

    //Get the parent, such as the table name, or the alias name for the join:
    const Glib::ustring parent = layout_item->get_sql_table_or_join_alias_name(table_name);
      
    const LayoutItem_FieldSummary* fieldsummary = dynamic_cast<const LayoutItem_FieldSummary*>(layout_item.obj());
    if(fieldsummary)
    {
      const guint id_function = builder->add_function(
        fieldsummary->get_summary_type_sql(),
        builder->add_id(layout_item->get_sql_name(table_name)) ); //TODO: It would be nice to specify the table here too.
      builder->add_field_id(id_function);
    }
    else
      builder->select_add_field(layout_item->get_name(), parent);
  
    
    one_added = true;
  }

  if(!one_added)
  {
    std::cerr << "Utils::build_sql_select_fields_to_get(): No fields added: fieldsToGet.size()=" << fieldsToGet.size() << std::endl;
    return;
  }
}


Glib::RefPtr<Gnome::Gda::SqlBuilder> Utils::build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecConstLayoutFields& fieldsToGet, const Gnome::Gda::SqlExpr& where_clause, const Glib::ustring& extra_join, const type_sort_clause& sort_clause, const Glib::ustring& extra_group_by, guint limit)
{
  //Build the whole SQL statement:
  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder = Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
  builder->select_add_target(table_name);

  //Add the fields to SELECT, plus the tables that they are selected FROM.
  Utils::build_sql_select_add_fields_to_get(builder, table_name, fieldsToGet, sort_clause);
  //builder->select_add_field(primary_key->get_name(), table_name);
  
  //TODO:
  //if(!extra_join.empty())
 //   sql_part_leftouterjoin += (' ' + extra_join + ' ');

  //Add the WHERE clause:
  if(!where_clause.empty())
  {
    const int id = builder->import_expression(where_clause);
    builder->set_where(id);
  }

  //Extra GROUP_BY clause for doubly-related records. This must be before the ORDER BY sort clause:
  //TODO:
  //if(!extra_group_by.empty())
  //{
  //  result += (' ' + extra_group_by + ' ');
  //}

  //Sort clause:
  if(!sort_clause.empty())
  {
   for(type_sort_clause::const_iterator iter = sort_clause.begin(); iter != sort_clause.end(); ++iter)
    {
      sharedptr<const LayoutItem_Field> layout_item = iter->first;
      if(layout_item)
      {
        const bool ascending = iter->second;

        //TODO: Avoid the need for the "."
        builder->select_order_by(
          builder->add_id(layout_item->get_sql_table_or_join_alias_name("\"" + table_name) + "\".\"" + layout_item->get_name() + "\""),
          ascending);
      }
    }
  }

  //LIMIT clause:
  if(limit > 0)
  {
    builder->select_set_limit(limit);
  }

  return builder;
}


Glib::RefPtr<Gnome::Gda::SqlBuilder> Utils::build_sql_select_with_key(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const sharedptr<const Field>& key_field, const Gnome::Gda::Value& key_value, guint limit)
{
  //TODO_Performance:
  type_vecConstLayoutFields constFieldsToGet;
  for(type_vecLayoutFields::const_iterator iter = fieldsToGet.begin(); iter != fieldsToGet.end(); ++iter)
  {
    constFieldsToGet.push_back(*iter);
  }

  return build_sql_select_with_key(table_name, constFieldsToGet, key_field, key_value, limit);
}

Gnome::Gda::SqlExpr Utils::build_simple_where_expression(const Glib::ustring& table_name, const sharedptr<const Field>& key_field, const Gnome::Gda::Value& key_value)
{
  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder = Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
  builder->select_add_target(table_name);  //This might not be necessary.
  const guint id = builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_EQ,
    builder->add_id(key_field->get_name()),
    builder->add_expr(key_value));
  builder->set_where(id); //This might not be necessary.

  return builder->export_expression(id);
}

Gnome::Gda::SqlExpr Utils::build_combined_where_expression(const Gnome::Gda::SqlExpr& a, const Gnome::Gda::SqlExpr& b, Gnome::Gda::SqlOperatorType op)
{
  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder =
    Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);

  const guint id = builder->add_cond(op,
    builder->import_expression(a),
    builder->import_expression(b));
   builder->set_where(id);
  return builder->export_expression(id);
}

Glib::RefPtr<Gnome::Gda::SqlBuilder> Utils::build_sql_select_with_key(const Glib::ustring& table_name, const type_vecConstLayoutFields& fieldsToGet, const sharedptr<const Field>& key_field, const Gnome::Gda::Value& key_value, guint limit)
{
  if(!Conversions::value_is_empty(key_value)) //If there is a record to show:
  {
    const Gnome::Gda::SqlExpr where_clause = build_simple_where_expression(table_name, key_field, key_value);
    return Utils::build_sql_select_with_where_clause(table_name, fieldsToGet, where_clause,
      Glib::ustring(), type_sort_clause(),  Glib::ustring(), limit);
  }

  return Glib::RefPtr<Gnome::Gda::SqlBuilder>();
}

Utils::type_list_values_with_second Utils::get_choice_values(const sharedptr<const LayoutItem_Field>& field)
{
  type_list_values_with_second list_values;

  sharedptr<const Relationship> choice_relationship;
  Glib::ustring choice_field, choice_second;
  field->get_formatting_used().get_choices(choice_relationship, choice_field, choice_second);
  if(!choice_relationship)
  {
    //std::cout <<" debug: field has no choices: " << field->get_name() << std::endl;
    return list_values;
  }

  const Glib::ustring to_table = choice_relationship->get_to_table();
  if(to_table.empty())
  {
    g_warning("get_choice_values(): table_name is null. relationship name = %s", glom_get_sharedptr_name(choice_relationship).c_str());
    return list_values;
  }

  const bool with_second = !choice_second.empty();

  //Get possible values from database, sorted by the first column.
  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder =
      Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
  const guint choice_field_id = builder->select_add_field(choice_field, to_table);
  builder->select_add_target(to_table);

  if(with_second)
    builder->select_add_field(choice_second, to_table);

  builder->select_order_by(choice_field_id);

  //std::cout << "debug: get_choice_values(): query: " << sql_query << std::endl;
  //Connect to database:
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  sharedptr<SharedConnection> connection = ConnectionPool::get_instance()->connect();
#else
  std::auto_ptr<ExceptionConnection> conn_error;
  sharedptr<SharedConnection> connection = ConnectionPool::get_instance()->connect(conn_error);
  if(conn_error.get())
    return list_values;
#endif

  if(!connection)
    return list_values;

  const std::string sql_query =
    sqlbuilder_get_full_query(builder);
  //std::cout << "get_choice_values: Executing SQL: " << sql_query << std::endl;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = connection->get_gda_connection()->statement_execute_select(sql_query);
#else
  std::auto_ptr<Glib::Error> error;
  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = connection->get_gda_connection()->statement_execute_select(sql_query, Gnome::Gda::STATEMENT_MODEL_RANDOM_ACCESS, error);
#endif

  if(datamodel)
  {
    const guint count = datamodel->get_n_rows();
    //std::cout << "  result: count=" << count << std::endl;
    for(guint row = 0; row < count; ++row)
    {

      std::pair<Gnome::Gda::Value, Gnome::Gda::Value> itempair;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
      itempair.first = datamodel->get_value_at(0, row);

      if(with_second)
        itempair.second = datamodel->get_value_at(1, row);
#else
      itempair.first = datamodel->get_value_at(0, row, error);

      if(with_second)
        itempair.second = datamodel->get_value_at(1, row, error);
#endif

      list_values.push_back(itempair);
    }
  }
  else
  {
      std::cerr << "Glom  get_choice_values(): Error while executing SQL" << std::endl <<
                   "  " <<  sql_query << std::endl;
  }

  return list_values;
}


Glib::ustring Utils::create_name_from_title(const Glib::ustring& title)
{
  Glib::ustring result = string_replace(title, " ", "");
  return result.lowercase(); //TODO: Maybe they need to be ASCII (not UTF8)?
}

Glib::ustring Utils::string_escape_underscores(const Glib::ustring& text)
{
  Glib::ustring result;
  for(Glib::ustring::const_iterator iter = text.begin(); iter != text.end(); ++iter)
  {
    if(*iter == '_')
      result += "__";
    else
      result += *iter;
  }

  return result;
}

/** Get just the first part of a locale, such as de_DE,
 * ignoring, for instance, .UTF-8 or @euro at the end.
 */
Glib::ustring Utils::locale_simplify(const Glib::ustring& locale_id)
{
  Glib::ustring result = locale_id;

  //Get everything before the .:
  Glib::ustring::size_type posDot = locale_id.find('.');
  if(posDot != Glib::ustring::npos)
  {
    result = result.substr(0, posDot);
  }

  //Get everything before the @:
  const Glib::ustring::size_type posAt = locale_id.find('@');
  if(posAt != Glib::ustring::npos)
  {
    result = result.substr(0, posAt);
  }

  return result;
}

Glib::ustring Utils::locale_language_id(const Glib::ustring& locale_id)
{
  Glib::ustring result;

  const Glib::ustring::size_type posUnderscore = locale_id.find('_');
  if(posUnderscore != Glib::ustring::npos)
  {
    result = locale_id.substr(0, posUnderscore);
  }

  return result;
}

Glib::ustring Utils::create_local_image_uri(const Gnome::Gda::Value& value)
{
  static guint m_temp_image_uri_number = 0;

  Glib::ustring result;

  if(value.get_value_type() == GDA_TYPE_BINARY)
  {
    glong size = 0;
    gconstpointer pData = value.get_binary(size);
    if(size && pData)
    {
      // Note that this is regular binary data, not escaped text representing the data:

      //Save the image to a temporary file and provide the file URI.
      char pchExtraNum[10];
      sprintf(pchExtraNum, "%d", m_temp_image_uri_number);
      result = ("/tmp/glom_report_image_" + Glib::ustring(pchExtraNum) + ".png");
      ++m_temp_image_uri_number;

      std::fstream the_stream(result.c_str(), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
      if(the_stream)
      {
        the_stream.write((char*)pData, size);
      }
    }
    else
       std::cerr << "Utils::create_local_image_uri(): binary GdaValue contains no data." << std::endl;
  }
  //else
  //  std::cerr << "Utils::create_local_image_uri(): type != BINARY" << std::endl;

  if(result.empty())
    result = "/tmp/glom_report_image_invalid.png";

  return ("file://" + result);
}

Glib::ustring Utils::string_from_decimal(guint decimal)
{
  //TODO_Performance:

  std::stringstream stream;
  stream.imbue(std::locale("")); //Use the user's current locale.
  stream << decimal;

  Glib::ustring result;
  stream >> result;

  return result;
}

Glib::ustring Utils::title_from_string(const Glib::ustring& text)
{
  Glib::ustring result;

  bool capitalise_next_char = true;
  for(Glib::ustring::const_iterator iter = text.begin(); iter != text.end(); ++iter)
  {
    const gunichar& ch = *iter;
    if(ch == '_') //Replace _ with space.
    {
      capitalise_next_char = true; //Capitalise all words.
      result += " ";
    }
    else
    {
      if(capitalise_next_char)
        result += Glib::Unicode::toupper(*iter);
      else
        result += *iter;

      capitalise_next_char = false;
    }
  }

  return result;
}

Utils::type_vec_strings Utils::string_separate(const Glib::ustring& str, const Glib::ustring& separator, bool ignore_quoted_separator)
{
  //std::cout << "Utils::string_separate(): separator=" << separator << std::endl;

  type_vec_strings result;

  const Glib::ustring::size_type size = str.size();
  const Glib::ustring::size_type size_separator = separator.size();

  //A stack of quotes, so that we can handle nested quotes, whether they are " or ':
  typedef std::stack<Glib::ustring> type_queue_quotes;
  type_queue_quotes m_current_quotes;

  Glib::ustring::size_type unprocessed_start = 0;
  Glib::ustring::size_type item_start = 0;
  while(unprocessed_start < size)
  {
    //std::cout << "while unprocessed: un_processed_start=" << unprocessed_start << std::endl;
    Glib::ustring::size_type posComma = str.find(separator, unprocessed_start);

    Glib::ustring item;
    if(posComma != Glib::ustring::npos)
    {
      //Check that the separator was not in quotes:
      bool in_quotes = false;

      if(ignore_quoted_separator)
      {
        //std::cout << "  debug: attempting to ignore quoted separators: " << separator << std::endl;

        Glib::ustring::size_type posLastQuote = unprocessed_start;

        //std::cout << "    debug: posLastQuote=" << posLastQuote << std::endl;
        //std::cout << "    debug: posComma=" << posComma << std::endl;


        bool bContinue = true;
        while(bContinue && (posLastQuote < posComma))
        {
          //std::cout << "  continue" << std::endl;
          Glib::ustring closing_quote;
          if(!m_current_quotes.empty())
            closing_quote = m_current_quotes.top();

          //std::cout << "   posLastQuote=" << posLastQuote << std::endl;
          const Glib::ustring::size_type posSingleQuote = str.find("'", posLastQuote);
          const Glib::ustring::size_type posDoubleQuote = str.find("\"", posLastQuote);

         // std::cout << "   posSingleQuote=" << posSingleQuote << "posDoubleQuote=" << posDoubleQuote << std::endl;

          //Which quote, if any, is first:
          Glib::ustring::size_type posFirstQuote = posSingleQuote;
          if( (posDoubleQuote != Glib::ustring::npos) && (posDoubleQuote < posFirstQuote) )
            posFirstQuote = posDoubleQuote;

          //Ignore quotes that are _after_ the separator:
          if( posFirstQuote >= posComma)
            posFirstQuote = Glib::ustring::npos;

          //std::cout << "   posFirstQuote=" << posFirstQuote << std::endl;

          //If any quote character was found:
          if(posFirstQuote != Glib::ustring::npos)
          {
            //std::cout << "quote found: posFirstQuote=" << posFirstQuote << std::endl;

            //Which quote was it?
            const Glib::ustring first_quote =  (posFirstQuote == posSingleQuote ? "'" : "\"");
            //std::cout << "   first_quote=" << first_quote << std::endl;

            //Was it an expected closing quote, if we expected any:
            if(first_quote == closing_quote)
            {
              //std::cout << "   popping quote" << std::endl;
              //Yes, so remove that quote from our stack, because we found the closing quote:
              m_current_quotes.pop();
            }
            else
            {
              //std::cout << "   pushing quote" << std::endl;
              //This must be an opening quote, so remember it:
              m_current_quotes.push(first_quote);
            }

            posLastQuote = posFirstQuote + 1; //Do the next find after the quote.
          }
          else
          {
            //There were no quotes, or no closing quotes:
            bContinue = false;
          }
        } //while(bContinue)

        //If there were any unclosed quotes then this separator must have been in quotes:
        if(!m_current_quotes.empty())
          in_quotes = true;
      } //If ignore_quoted_separator

      if(!in_quotes) //or if we don't care about quotes.
      {
        //std::cout << "!in_quotes" << std::endl;

        //Store this item, and start the next item after it:
        item = str.substr(item_start, posComma - item_start);
        //std::cout << "  ITEM. pos_comma=" << posComma << ", ITEM= " << item << std::endl;
        item_start = posComma + size_separator;
      }
      else
      {
        //std::cout << "in quotes." << std::endl;
        // Continue behind separator
        unprocessed_start = posComma + size_separator;
        // Do not add this item to the result, because it was quoted.
        continue;
      }

      unprocessed_start = posComma + size_separator; //The while loops stops when this is empty.
    }
    else //if no separator found:
    {
        item = str.substr(item_start);
        unprocessed_start = size; //Stop.
    }

    item = string_trim(item, " ");
    result.push_back(item);
  } //while

  return result;
}

Glib::ustring Utils::string_trim(const Glib::ustring& str, const Glib::ustring& to_remove)
{
   Glib::ustring result = str;

   //Remove from the start:
   Glib::ustring::size_type posOpenBracket = result.find(to_remove);
   if(posOpenBracket == 0)
   {
      result = result.substr(to_remove.size());
   }

   //Remove from the end:
   Glib::ustring::size_type posCloseBracket = result.rfind(to_remove);
   if(posCloseBracket == (result.size() - to_remove.size()))
   {
    result = result.substr(0, posCloseBracket);
   }

  return result;
}

Glib::ustring Utils::string_remove_suffix(const Glib::ustring& str, const Glib::ustring& suffix, bool case_sensitive)
{
  //There is also g_string_has_suffix(), but I assume that is case sensitive. murrayc.

  const Glib::ustring::size_type size = str.size();
  const Glib::ustring::size_type suffix_size = suffix.size();
  if(size < suffix_size)
    return str;

  const Glib::ustring possible_suffix = str.substr(size - suffix_size);

  if(case_sensitive)
  {
    if(possible_suffix == suffix)
      return str.substr(0, size - suffix_size);
  }
  else
  {
    if(g_ascii_strcasecmp(possible_suffix.c_str(), suffix.c_str()) == 0) //TODO: I don't understand the warnings about using this function in the glib documentation. murrayc.
      return str.substr(0, size - suffix_size);
  }

  return str;
}



bool Utils::file_exists(const Glib::ustring& uri)
{
  if(uri.empty())
     return false;

  //Check whether file exists already:
  {
    // Try to examine the input file.
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);

#ifdef GLIBMM_EXCEPTIONS_ENABLED
    try
    {
      return file->query_exists();
    }
    catch(const Gio::Error& /* ex */)
    {
      return false; //Something went wrong. It does not exist.
    }
#else
      return file->query_exists();
#endif
  }
}

std::string Utils::sqlbuilder_get_full_query(
    const Glib::RefPtr<Gnome::Gda::Connection>& connection,
    const Glib::ustring& query,
    const Glib::RefPtr<const Gnome::Gda::Set>& params)
{
  Glib::ustring result = "glom_query_not_parsed";

  try
  {
    Glib::RefPtr<Gnome::Gda::SqlParser> parser = connection->create_parser();
    if(parser)
    {
      Glib::RefPtr<Gnome::Gda::Statement> stmt = parser->parse_string(query);
      if(stmt)
        result = stmt->to_sql(params);
    }
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << "sqlbuilder_get_full_query(): exception while parsing query: " << ex.what() << std::endl;
  }
  catch(const std::exception& ex)
  {
    std::cerr << "sqlbuilder_get_full_query(): exception while parsing query: " << ex.what() << std::endl;
  }

  //Convert to something that std::cout should be able to handle.
  const Glib::ScopedPtr<char> buf(g_convert_with_fallback(
    result.raw().data(), result.raw().size(),
    "ISO-8859-1", "UTF-8",
    (char*)"?",
    0, 0, 0));
  return std::string(buf.get());
}

std::string Utils::sqlbuilder_get_full_query(
  const Glib::RefPtr<const Gnome::Gda::SqlBuilder>& builder)
{
  Glib::ustring result = "glom_query_not_parsed";

  try
  {
    Glib::RefPtr<Gnome::Gda::Statement> stmt = builder->get_statement();
    if(stmt)
      result = stmt->to_sql();
  }
  catch(const Gnome::Gda::SqlError& ex)
  {
    std::cerr << "sqlbuilder_get_full_query(): SqlError exception while getting query: " << ex.what() << std::endl;
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << "sqlbuilder_get_full_query(): exception (" << typeid(ex).name() << ") while getting query: " << ex.what() << std::endl;
  }
  catch(const std::exception& ex)
  {
    std::cerr << "sqlbuilder_get_full_query(): exception (" << typeid(ex).name() << ") while getting query: " << ex.what() << std::endl;
  }

  //Convert to something that std::cout should be able to handle.
  const Glib::ScopedPtr<char> buf(g_convert_with_fallback(
    result.raw().data(), result.raw().size(),
    "ISO-8859-1", "UTF-8",
    (char*)"?",
    0, 0, 0));
  return std::string(buf.get());
}

Gnome::Gda::SqlExpr Utils::get_find_where_clause_quick(Document* document, const Glib::ustring& table_name, const Gnome::Gda::Value& quick_search)
{
  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder =
    Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
  builder->select_add_target(table_name);

  if(!document)
  {
    std::cerr << "Utils::get_find_where_clause_quick(): document was null." << std::endl;
    return Gnome::Gda::SqlExpr();
  }

  //TODO: Cache the list of all fields, as well as caching (m_Fields) the list of all visible fields:
  const Document::type_vec_fields fields = document->get_table_fields(table_name);

  guint previous_and_id = 0;
  typedef std::vector< sharedptr<LayoutItem_Field> > type_vecLayoutFields;
  type_vecLayoutFields fieldsToGet;
  for(Document::type_vec_fields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
  {
    Glib::ustring strClausePart;

    sharedptr<const Field> field = *iter;

    bool use_this_field = true;
    if(field->get_glom_type() != Field::TYPE_TEXT)
    {
      use_this_field = false;
    }

    if(use_this_field)
    {
      const guint eq_id = builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_EQ, //TODO: Ue field->sql_find_operator().
        builder->add_id(field->get_name()),
        builder->add_expr(quick_search)); //Use  field->sql_find(quick_search);

      guint and_id = 0;
      if(previous_and_id)
      {
        and_id = builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_AND,
        previous_and_id, eq_id);
      }

      previous_and_id = and_id;
    }
  }

  if(previous_and_id)
  {
    builder->set_where(previous_and_id); //This might be unnecessary.
    return builder->export_expression(previous_and_id);
  }
  else
  {
    return Gnome::Gda::SqlExpr();
  }
}

} //namespace Glom
