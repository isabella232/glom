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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include <libglom/libglom_config.h>

#include <libglom/utils.h>
#include <libglom/connectionpool.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/glomconversions.h>

#include <giomm/file.h>
#include <giomm/resource.h>
#include <glibmm/convert.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <glibmm/i18n.h>


#include <string.h> // for strchr
#include <sstream> //For stringstream

#include <iostream>
#include <fstream>

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl
#include <iomanip>

#include <stack>

#include <fcntl.h> //For close(). This include is needed in mingw on MS Windows.

namespace Glom
{

template
<typename T_Container>
auto find_if_uses_relationship_has_relationship(T_Container& container, const std::shared_ptr<const UsesRelationship> uses_relationship_name, bool first_level_only = false) -> decltype(container.begin())
{
  const Glib::ustring relationship_name(uses_relationship_name->get_relationship_name());
  Glib::ustring related_relationship_name(uses_relationship_name->get_related_relationship_name());

  //If first_level_only, search for relationships that have the same top-level relationship, but have no related relationship.
  if(first_level_only)
    related_relationship_name = Glib::ustring();

  return Utils::find_if(container,
    [&relationship_name, &related_relationship_name](const typename T_Container::value_type& element)
    {
      //Assume that element is a shared_ptr<>.

      return (element->get_relationship_name() == relationship_name) && (element->get_related_relationship_name() == related_relationship_name);
    }
  );
}

Glib::ustring Utils::trim_whitespace(const Glib::ustring& text)
{
  //TODO_Performance:

  Glib::ustring result = text;

  //Find non-whitespace from front:
  Glib::ustring::size_type posFront = Glib::ustring::npos;
  Glib::ustring::size_type pos = 0;
  for(const auto& item : result)
  {
    if(!Glib::Unicode::isspace(item))
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
  for(auto iter = result.rbegin(); iter != result.rend(); ++iter)
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

Glib::ustring Utils::string_replace(const Glib::ustring& src, const Glib::ustring& search_for, const Glib::ustring& replace_with)
{
  if(search_for.empty())
  {
    std::cerr << G_STRFUNC << ": search_for was empty." << std::endl;
    return src;
  }

  //std::cout << "debug: " << G_STRFUNC << ": src=" << src << ", search_for=" << search_for << ", replace_with=" << replace_with << std::endl;

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
    const bool found = (src.find(search_for, src_index) == src_index);
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

Glib::ustring Utils::string_clean_for_xml(const Glib::ustring& src)
{
  // The form feed character may not be in XML, even if escaped.
  // So lets just lose it.
  // Other unusual characters, such as &, are escaped by libxml later.
  // TODO_Performance: Find a quicker way to do this.
  return string_replace(src, "\f", Glib::ustring());
}


Glib::RefPtr<Gnome::Gda::SqlBuilder> Utils::build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const Gnome::Gda::SqlExpr& where_clause, const std::shared_ptr<const Relationship>& extra_join, const type_sort_clause& sort_clause, guint limit)
{
  //TODO_Performance:
  type_vecConstLayoutFields constFieldsToGet;
  for(const auto& field : fieldsToGet)
  {
    constFieldsToGet.push_back(field);
  }

  return build_sql_select_with_where_clause(table_name, constFieldsToGet, where_clause, extra_join, sort_clause, limit);
}

/** Build a SQL query to discover how many rows a SQL query would return if it was run.
 *
 * This uses a COUNT * on a the @a sql_query as a sub-statement.
 * Be careful not to include ORDER BY clauses in the supplied SQL query, because that would make it unnecessarily slow.
 *
 * @sql_query A SQL query.
 * @result The number of rows.
 */
Glib::RefPtr<Gnome::Gda::SqlBuilder> Utils::build_sql_select_count_rows(const Glib::RefPtr<const Gnome::Gda::SqlBuilder>& sql_query)
{
  Glib::RefPtr<Gnome::Gda::SqlBuilder> result;

  if(!sql_query)
  {
    std::cerr << G_STRFUNC << ": sql_query was null." << std::endl;
    return result;
  }

  result = Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);

  //Note that the alias is just because the SQL syntax requires it - we get an error if we don't use it.
  //Be careful not to include ORDER BY clauses in this, because that would make it unnecessarily slow:
  const guint target_id = result->add_sub_select( sql_query->get_sql_statement() );
  result->select_add_target_id(target_id, "glomarbitraryalias");

  const Gnome::Gda::SqlBuilder::Id id_function = result->add_function("COUNT", result->add_id("*"));
  result->add_field_value_id(id_function);

  return result;
}

typedef std::list< std::shared_ptr<const UsesRelationship> > type_list_relationships;

static void add_to_relationships_list(type_list_relationships& list_relationships, const std::shared_ptr<const LayoutItem_Field>& layout_item)
{
  g_return_if_fail(layout_item);

  if(!(layout_item->get_has_relationship_name()))
    return;

  //If this is a related relationship, add the first-level relationship too, so that the related relationship can be defined in terms of it:
  auto iterFind = find_if_uses_relationship_has_relationship(list_relationships, layout_item, true /* top_level_only */);
  if(iterFind == list_relationships.end()) //If the table is not yet in the list:
  {
    std::shared_ptr<UsesRelationship> uses_rel = std::make_shared<UsesRelationship>();
    uses_rel->set_relationship(layout_item->get_relationship());
    list_relationships.push_front(uses_rel); //These need to be at the front, so that related relationships can use them later in the SQL statement.
  }

  //Add the relationship to the list:
  iterFind = find_if_uses_relationship_has_relationship(list_relationships, layout_item);
  if(iterFind == list_relationships.end()) //If the table is not yet in the list:
  {
    std::shared_ptr<UsesRelationship> uses_rel = std::make_shared<UsesRelationship>();
    uses_rel->set_relationship(layout_item->get_relationship());
    uses_rel->set_related_relationship(layout_item->get_related_relationship());
    list_relationships.push_back(uses_rel);
  }

}

static void builder_add_join(const Glib::RefPtr<Gnome::Gda::SqlBuilder>& builder, const std::shared_ptr<const UsesRelationship>& uses_relationship)
{
  std::shared_ptr<const Relationship> relationship = uses_relationship->get_relationship();
  if(!relationship->get_has_fields()) //TODO: Handle related_record has_fields.
  {
    if(relationship->get_has_to_table())
    {
      //It is a relationship that only specifies the table, without specifying linking fields:
      builder->select_add_target(relationship->get_to_table());
    }

    return;
  }

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
        builder->add_field_id(relationship->get_from_field(), relationship->get_from_table()),
        builder->add_field_id(relationship->get_to_field(), alias_name)));
  }
  else
  {
     UsesRelationship parent_relationship;
     parent_relationship.set_relationship(relationship);
     std::shared_ptr<const Relationship> related_relationship = uses_relationship->get_related_relationship();

     const guint to_target_id = builder->select_add_target(related_relationship->get_to_table(), alias_name);

     builder->select_join_targets(
       builder->select_add_target(relationship->get_from_table()), //TODO: Must we use the ID from select_add_target_id()?
       to_target_id,
       Gnome::Gda::SQL_SELECT_JOIN_LEFT,
       builder->add_cond(
         Gnome::Gda::SQL_OPERATOR_TYPE_EQ,
         builder->add_field_id(related_relationship->get_from_field(), parent_relationship.get_sql_join_alias_name()),
         builder->add_field_id(related_relationship->get_to_field(), alias_name) ) );
  }
}

void Utils::build_sql_select_add_fields_to_get(const Glib::RefPtr<Gnome::Gda::SqlBuilder>& builder, const Glib::ustring& table_name, const type_vecConstLayoutFields& fieldsToGet, const type_sort_clause& sort_clause, bool extra_join)
{
  //Get all relationships used in the query:
  type_list_relationships list_relationships;

  for(const auto& layout_item : fieldsToGet)
  {
    add_to_relationships_list(list_relationships, layout_item);
  }

  for(const auto& the_pair : sort_clause)
  {
    add_to_relationships_list(list_relationships, the_pair.first);
  }


  //LEFT OUTER JOIN will get the field values from the other tables,
  //and give us our fields for this table even if there is no corresponding value in the other table.
  for(const auto& uses_relationship : list_relationships)
  {
    builder_add_join(builder, uses_relationship);
  }

  bool one_added = false;
  for(const auto& layout_item : fieldsToGet)
  {
    Glib::ustring one_sql_part;

    if(!layout_item)
    {
      g_warn_if_reached();
      continue;
    }

    //Get the parent, such as the table name, or the alias name for the join:
    const Glib::ustring parent = layout_item->get_sql_table_or_join_alias_name(table_name);

    //TODO: Use std::dynamic_pointer_cast?
    const auto fieldsummary = dynamic_cast<const LayoutItem_FieldSummary*>(layout_item.get());
    if(fieldsummary)
    {
      const Gnome::Gda::SqlBuilder::Id id_function = builder->add_function(
        fieldsummary->get_summary_type_sql(),
        builder->add_field_id(layout_item->get_name(), table_name));
      builder->add_field_value_id(id_function);
    }
    else
    {
      const Glib::ustring field_name = layout_item->get_name();
      if(!field_name.empty())
      {
        const Gnome::Gda::SqlBuilder::Id id = builder->select_add_field(field_name, parent);

        //Avoid duplicate records with doubly-related fields:
        if(extra_join)
          builder->select_group_by(id);
      }
    }


    one_added = true;
  }

  if(!one_added)
  {
    std::cerr << G_STRFUNC << ": No fields added: fieldsToGet.size()=" << fieldsToGet.size() << std::endl;
    return;
  }
}


Glib::RefPtr<Gnome::Gda::SqlBuilder> Utils::build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecConstLayoutFields& fieldsToGet, const Gnome::Gda::SqlExpr& where_clause, const std::shared_ptr<const Relationship>& extra_join, const type_sort_clause& sort_clause, guint limit)
{
  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder;

  //Build the whole SQL statement:
  try
  {
    builder = Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
    builder->select_add_target(table_name);

    //Add the fields to SELECT, plus the tables that they are selected FROM.
    //We tell it whether extra_join is empty, so it can do an extra GROUP BY if necessary.
    //TODO: Try to use DISTINCT instead, with a proper test case.
    Utils::build_sql_select_add_fields_to_get(builder, table_name, fieldsToGet, sort_clause, (bool)extra_join);

    if(extra_join)
    {
      std::shared_ptr<UsesRelationship> uses_relationship = std::make_shared<UsesRelationship>();
      uses_relationship->set_relationship(extra_join);
      builder_add_join(builder, uses_relationship);
    }

    //Add the WHERE clause:
    if(!where_clause.empty())
    {
      const int id = builder->import_expression(where_clause);
      builder->set_where(id);
    }

    //Sort clause:
    if(!sort_clause.empty())
    {
      for(const auto& the_pair : sort_clause)
      {
        auto layout_item = the_pair.first;
        if(layout_item)
        {
          const auto ascending = the_pair.second;

          //TODO: Avoid the need for the "."
          builder->select_order_by(
            builder->add_field_id(layout_item->get_name(), layout_item->get_sql_table_or_join_alias_name(table_name)),
            ascending);
        }
      }
    }

    //LIMIT clause:
    if(limit > 0)
    {
      builder->select_set_limit(limit);
    }
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception: " << ex.what() << std::endl;
  }

  return builder;
}


Glib::RefPtr<Gnome::Gda::SqlBuilder> Utils::build_sql_select_with_key(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const std::shared_ptr<const Field>& key_field, const Gnome::Gda::Value& key_value, const type_sort_clause& sort_clause, guint limit)
{
  //TODO_Performance:
  type_vecConstLayoutFields constFieldsToGet;
  for(const auto& field : fieldsToGet)
  {
    constFieldsToGet.push_back(field);
  }

  return build_sql_select_with_key(table_name, constFieldsToGet, key_field, key_value, sort_clause, limit);
}

Gnome::Gda::SqlExpr Utils::build_simple_where_expression(const Glib::ustring& table_name, const std::shared_ptr<const Field>& key_field, const Gnome::Gda::Value& key_value)
{
  if(!key_field)
  {
    std::cerr << G_STRFUNC << ": key_field was empty" << std::endl;
    return Gnome::Gda::SqlExpr();
  }
  
  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder = Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
  builder->select_add_target(table_name);  //This might not be necessary.
  const Gnome::Gda::SqlBuilder::Id id = builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_EQ,
    builder->add_field_id(key_field->get_name(), table_name),
    builder->add_expr(key_value));
  builder->set_where(id); //This might not be necessary.

  return builder->export_expression(id);
}

Gnome::Gda::SqlExpr Utils::build_combined_where_expression(const Gnome::Gda::SqlExpr& a, const Gnome::Gda::SqlExpr& b, Gnome::Gda::SqlOperatorType op)
{
  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder =
    Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);

  const Gnome::Gda::SqlBuilder::Id id = builder->add_cond(op,
    builder->import_expression(a),
    builder->import_expression(b));
   builder->set_where(id);
  return builder->export_expression(id);
}

Glib::RefPtr<Gnome::Gda::SqlBuilder> Utils::build_sql_select_with_key(const Glib::ustring& table_name, const type_vecConstLayoutFields& fieldsToGet, const std::shared_ptr<const Field>& key_field, const Gnome::Gda::Value& key_value, const type_sort_clause& sort_clause, guint limit)
{
  //We choose instead to have no where clause in this case,
  //because that is useful to some callers:
  //if(Conversions::value_is_empty(key_value)) //If there is a record to show:
  //  return Glib::RefPtr<Gnome::Gda::SqlBuilder>();

  Gnome::Gda::SqlExpr where_clause;
  if(!Conversions::value_is_empty(key_value) && key_field)
  {
    where_clause = build_simple_where_expression(table_name, key_field, key_value);
  }

  return Utils::build_sql_select_with_where_clause(table_name, fieldsToGet, where_clause,
    std::shared_ptr<const Relationship>(), sort_clause, limit);
}

Utils::type_list_values_with_second Utils::get_choice_values_all(const Document* document, const std::shared_ptr<const LayoutItem_Field>& field)
{
  return get_choice_values(document, field,
    Gnome::Gda::Value() /* means get all with no WHERE clause */);
}

Utils::type_list_values_with_second Utils::get_choice_values(const Document* document, const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& foreign_key_value)
{
  //TODO: Reduce duplication between this and get_choice_values(field).

  type_list_values_with_second result;

  //We allows this, so this method can be used to get all records in a related table:
  /*
  if(Conversions::value_is_empty(foreign_key_value))
  {
    std::cout << G_STRFUNC << "debug: foreign_key_value is empty." << std::endl;
    return result;
  }
  */

  const Formatting& format = field->get_formatting_used();
  std::shared_ptr<const Relationship> choice_relationship;
  std::shared_ptr<const LayoutItem_Field> layout_choice_first;
  std::shared_ptr<const LayoutGroup> layout_choice_extra;
  Formatting::type_list_sort_fields choice_sort_fields;
  bool choice_show_all = false;
  format.get_choices_related(choice_relationship, layout_choice_first, layout_choice_extra, choice_sort_fields, choice_show_all);

  if(!choice_relationship)
  {
    std::cerr << G_STRFUNC << ": !choice_relationship." << std::endl;
    return result;
  }

  Utils::type_vecConstLayoutFields fields;
  fields.push_back(layout_choice_first);

  if(layout_choice_extra)
  {
    for(const auto& item : layout_choice_extra->get_items_recursive())
    {
      const auto& item_field = std::dynamic_pointer_cast<const LayoutItem_Field>(item);
      if(item_field)
         fields.push_back(item_field); //TODO: Don't ignore other usable items such as static text.
    }
  }

  const Glib::ustring to_table = choice_relationship->get_to_table();
  const std::shared_ptr<const Field> to_field = document->get_field(to_table, choice_relationship->get_to_field());

  if(!to_field)
  {
    std::cerr << G_STRFUNC << ": to_field is null." << std::endl;
  }

  //Default to some sort order rather than none:
  if(choice_sort_fields.empty())
  {
    choice_sort_fields.push_back( type_pair_sort_field(layout_choice_first, true /* ascending */));
  }

  //TODO: Support related relationships (in the UI too):
  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder = Utils::build_sql_select_with_key(
    to_table,
    fields,
    to_field,
    foreign_key_value,
    choice_sort_fields);

  if(!builder)
  {
    std::cerr << G_STRFUNC << ": builder is null." << std::endl;
    return result;
  }

  //TODO: builder->select_order_by(choice_field_id);

  //Connect to database and get the related values:
  std::shared_ptr<SharedConnection> connection = ConnectionPool::get_instance()->connect();

  if(!connection)
  {
    std::cerr << G_STRFUNC << ": connection is null." << std::endl;
    return result;
  }

  const std::string sql_query =
    Utils::sqlbuilder_get_full_query(builder);
  //std::cout << "debug: sql_query=" << sql_query << std::endl;
  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = connection->get_gda_connection()->statement_execute_select(sql_query);

  if(datamodel)
  {
    const guint count = datamodel->get_n_rows();
    const guint cols_count = datamodel->get_n_columns();
    for(guint row = 0; row < count; ++row)
    {

      std::pair<Gnome::Gda::Value, type_list_values> itempair;
      itempair.first = datamodel->get_value_at(0, row);

      if(layout_choice_extra && (cols_count > 1))
      {
        type_list_values list_values;
        for(guint i = 1; i < cols_count; ++i)
        {
          list_values.push_back(datamodel->get_value_at(i, row));
        }

        itempair.second = list_values;
      }

      result.push_back(itempair);
    }
  }
  else
  {
      std::cerr << G_STRFUNC << ": Error while executing SQL" << std::endl <<
                   "  " <<  sql_query << std::endl;
      return result;
  }

  return result;
}

Glib::ustring Utils::create_name_from_title(const Glib::ustring& title)
{
  Glib::ustring result = string_replace(title, " ", "");
  return result.lowercase(); //TODO: Maybe they need to be ASCII (not UTF8)?
}

Glib::ustring Utils::string_escape_underscores(const Glib::ustring& text)
{
  Glib::ustring result;
  for(const auto& item : text)
  {
    if(item == '_')
      result += "__";
    else
      result += item;
  }

  return result;
}

/** Get just the first part of a locale, such as de_DE,
 * ignoring, for instance, .UTF-8 or \@euro at the end.
 */
Glib::ustring Utils::locale_simplify(const Glib::ustring& locale_id)
{
  Glib::ustring result = locale_id;

  //At least Ubuntu Natty provides a long string such as this: LC_CTYPE=en_US.UTF-8;LC_NUMERIC=en_US.UTF-8;LC_TIME=en_US.UTF-8;LC_COLLATE=en_US.UTF-8;LC_MONETARY=en_US.UTF-8;LC_MESSAGES=en_AG.utf8;LC_PAPER=en_US.UTF-8;LC_NAME=en_US.UTF-8;LC_ADDRESS=en_US.UTF-8;LC_TELEPHONE=en_US.UTF-8;LC_MEASUREMENT=en_US.UTF-8;LC_IDENTIFICATION=en_US.UTF-8
  //In Ubuntu Maverick, and earlier, it was apparently a simple string such as en_US.UTF-8.

  //Look for LC_ALL or LC_COLLATE
  //(We use the locale name only to identify translations
  //Otherwise just start with the whole string.
  Glib::ustring::size_type posCategory = result.find("LC_ALL=");
  if(posCategory != Glib::ustring::npos)
  {
    result = result.substr(posCategory);
  }
  else
  {
    posCategory = result.find("LC_COLLATE=");
    if(posCategory != Glib::ustring::npos)
    {
      result = result.substr(posCategory);
    }
  }

  //Get everything before the .:
  const Glib::ustring::size_type posDot = result.find('.');
  if(posDot != Glib::ustring::npos)
  {
    result = result.substr(0, posDot);
  }

  //Get everything before the @:
  const Glib::ustring::size_type posAt = result.find('@');
  if(posAt != Glib::ustring::npos)
  {
    result = result.substr(0, posAt);
  }

  //Get everything after the =, if any:
  const Glib::ustring::size_type posEquals = result.find('=');
  if(posEquals != Glib::ustring::npos)
  {
    result = result.substr(posEquals + 1);
  }

  return result;
}

Glib::ustring Utils::locale_language_id(const Glib::ustring& locale_id)
{
  const Glib::ustring::size_type posUnderscore = locale_id.find('_');
  if(posUnderscore != Glib::ustring::npos)
  {
    return locale_id.substr(0, posUnderscore);
  }
  else
  {
    //We assume that this locale ID specifies a language but no specific country.
    return locale_id;
  }
}

Glib::ustring Utils::create_local_image_uri(const Gnome::Gda::Value& value)
{
  static guint m_temp_image_uri_number = 0;

  Glib::ustring result;

  if(value.get_value_type() == GDA_TYPE_BINARY)
  {
    long size = 0;
    gconstpointer pData = value.get_binary(size);
    if(size && pData)
    {
      // Note that this is regular binary data, not escaped text representing the data:

      //Save the image to a temporary file and provide the file URI.
      char pchExtraNum[10];
      sprintf(pchExtraNum, "%d", m_temp_image_uri_number);
      result = ("/tmp/glom_report_image_" + Glib::ustring(pchExtraNum) + ".png");
      ++m_temp_image_uri_number;

      std::fstream the_stream(result, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
      if(the_stream)
      {
        the_stream.write((char*)pData, size);
      }
    }
    else
       std::cerr << G_STRFUNC << ": binary GdaValue contains no data." << std::endl;
  }
  //else
  //  std::cerr << G_STRFUNC << ": type != BINARY" << std::endl;

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
  for(const auto& ch : text)
  {
    if(ch == '_') //Replace _ with space.
    {
      capitalise_next_char = true; //Capitalise all words.
      result += " ";
    }
    else
    {
      if(capitalise_next_char)
        result += Glib::Unicode::toupper(ch);
      else
        result += ch;

      capitalise_next_char = false;
    }
  }

  return result;
}

Utils::type_vec_strings Utils::string_separate(const Glib::ustring& str, const Glib::ustring& separator, bool ignore_quoted_separator)
{
  //std::cout << "debug: " << G_STRFUNC << ": separator=" << separator << std::endl;

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
  // Try to examine the input file.
  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
  return file_exists(file);
}

bool Utils::file_exists(const Glib::RefPtr<Gio::File>& file)
{
  try
  {
    return file->query_exists();
  }
  catch(const Gio::Error& /* ex */)
  {
    return false; //Something went wrong. It does not exist.
  }
}

//TODO: This is a duplicate of the one in db_utils.cc:
//Merge all db utilities into db_utils in glom 1.24:
static Glib::RefPtr<Gnome::Gda::Connection> get_connection()
{
  std::shared_ptr<SharedConnection> sharedconnection;
  try
  {
     sharedconnection = ConnectionPool::get_and_connect();
  }
  catch(const Glib::Error& error)
  {
    std::cerr << G_STRFUNC << ": " << error.what() << std::endl;
  }

  if(!sharedconnection)
  {
    std::cerr << G_STRFUNC << ": No connection yet." << std::endl;
    return Glib::RefPtr<Gnome::Gda::Connection>();
  }

  Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

  return gda_connection;
}

std::string Utils::sqlbuilder_get_full_query(
  const Glib::RefPtr<const Gnome::Gda::SqlBuilder>& builder)
{
  Glib::RefPtr<Gnome::Gda::Connection> connection = get_connection();
  if(!connection)
  {
    //TODO: Just use the correct provider, without an actual connection?
    std::cerr << G_STRFUNC << ": There is no connection, so the SQL statement might not be created correctly." << std::endl;
  }

  Glib::ustring result = "glom_query_not_parsed";

  try
  {
    Glib::RefPtr<Gnome::Gda::Statement> stmt = builder->get_statement();
    if(!stmt)
    {
      std::cerr << G_STRFUNC << ": builder->get_statement() failed." << std::endl;
      return result;
    }

    if(connection)
    {
      result = connection->statement_to_sql(stmt,
          Gnome::Gda::STATEMENT_SQL_PARAMS_AS_VALUES | Gnome::Gda::STATEMENT_SQL_PRETTY);
    }
    else
      result = stmt->to_sql();
  }
  catch(const Gnome::Gda::SqlError& ex)
  {
    std::cerr << G_STRFUNC << ": SqlError exception while getting query: " << ex.what() << std::endl;
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << G_STRFUNC << ": exception (" << typeid(ex).name() << ") while getting query: " << ex.what() << std::endl;
  }
  catch(const std::exception& ex)
  {
    std::cerr << G_STRFUNC << ": exception (" << typeid(ex).name() << ") while getting query: " << ex.what() << std::endl;
  }

  //Convert to something that std::cout should be able to handle.
  const Glib::ScopedPtr<char> buf(g_convert_with_fallback(
    result.raw().data(), result.raw().size(),
    "ISO-8859-1", "UTF-8",
    (char*)"?",
    0, 0, 0));

  const Glib::ustring str = std::string(buf.get());
  if(str.empty())
  {
    std::cerr << G_STRFUNC << ": Returning an empty string." << std::endl;
  }

  return str;
}

Gnome::Gda::SqlExpr Utils::get_find_where_clause_quick(const Document* document, const Glib::ustring& table_name, const Gnome::Gda::Value& quick_search)
{
  if(table_name.empty())
  {
    std::cerr << G_STRFUNC << ": table_name is empty." << std::endl;
    return Gnome::Gda::SqlExpr();
  }

  if(Conversions::value_is_empty(quick_search))
    return Gnome::Gda::SqlExpr();
  
  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder =
    Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
  builder->select_add_target(table_name);

  //We need to add some fields to select,
  //because otherwise the SqlBuilder would not contain a valid query.
  builder->select_add_field("*", table_name);

  if(!document)
  {
    std::cerr << G_STRFUNC << ": document was null." << std::endl;
    return Gnome::Gda::SqlExpr();
  }

  //We need the connection to generate the correct SQL syntax:
  Glib::RefPtr<Gnome::Gda::Connection> connection = get_connection();
  if(!connection)
  {
    std::cerr << G_STRFUNC << ": connection was null." << std::endl;
    return Gnome::Gda::SqlExpr();
  }

  //TODO: Cache the list of all fields, as well as caching (m_Fields) the list of all visible fields:
  const Document::type_vec_fields fields = document->get_table_fields(table_name);

  guint previous_id = 0;
  for(const auto& field : fields)
  {
    Glib::ustring strClausePart;

    bool use_this_field = true;
    if(field->get_glom_type() != Field::glom_field_type::TEXT)
    {
      use_this_field = false;
    }

    if(use_this_field)
    {
      //std::cout << "Using field: " << field->get_name() << std::endl;
      const guint eq_id = builder->add_cond(field->sql_find_operator(),
        builder->add_field_id(field->get_name(), table_name),
        builder->add_expr( field->sql_find(quick_search, connection) )); //sql_find() modifies the value for the operator.

      if(previous_id)
      {
        const guint or_id = builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_OR,
          previous_id, eq_id);
        previous_id = or_id;
      }
      else
        previous_id = eq_id;
    }
  }

  if(previous_id)
  {
    builder->set_where(previous_id); //This might be unnecessary.
    //std::cout << G_STRFUNC << ": builder: " << sqlbuilder_get_full_query(builder) << std::endl; 
    return builder->export_expression(previous_id);
  }
  else
  {
    std::cerr << G_STRFUNC << ": Returning null SqlExpr" << std::endl;
    return Gnome::Gda::SqlExpr();
  }
}

Glib::RefPtr<Gnome::Gda::SqlBuilder> Utils::build_sql_update_with_where_clause(
  const Glib::ustring& table_name,
  const std::shared_ptr<const Field>& field, const Gnome::Gda::Value& value,
  const Gnome::Gda::SqlExpr& where_clause)
{
  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder;

  if(!field || field->get_name().empty())
  {
    std::cerr << G_STRFUNC << ": field was null or its name was empty." << std::endl;
    return builder;
  }

  if(table_name.empty())
  {
    std::cerr << G_STRFUNC << ": table_name was empty." << std::endl;
    return builder;
  }

  //Build the whole SQL statement:
  try
  {
    builder = Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_UPDATE);
    builder->set_table(table_name);

    builder->add_field_value_as_value(field->get_name(), value);

    //Add the WHERE clause:
    if(!where_clause.empty())
    {
      const int id = builder->import_expression(where_clause);
      builder->set_where(id);
    }
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception: " << ex.what() << std::endl;
  }

  return builder;
}

bool Utils::delete_directory(const Glib::RefPtr<Gio::File>& directory)
{
  try
  {
    if(!(directory->query_exists()))
      return true;

    //(Recursively) Delete any child files and directories,
    //so we can delete this directory.
    Glib::RefPtr<Gio::FileEnumerator> enumerator = directory->enumerate_children();

    Glib::RefPtr<Gio::FileInfo> info = enumerator->next_file();
    while(info)
    {
      Glib::RefPtr<Gio::File> child = directory->get_child(info->get_name());
      bool removed_child = false;
      if(child->query_file_type() == Gio::FILE_TYPE_DIRECTORY)
        removed_child = delete_directory(child);
      else
        removed_child = child->remove();

      if(!removed_child)
         return false;

      info = enumerator->next_file();
    }

    //Delete the actual directory:
    if(!directory->remove())
      return false;
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception from Gio::File: " << ex.what() << std::endl;
    return false;
  }

  return true;
}

bool Utils::delete_directory(const std::string& uri)
{
  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
  return delete_directory(file);
}

bool Utils::delete_file(const std::string& uri)
{
  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
  if(file->query_file_type() == Gio::FILE_TYPE_DIRECTORY)
  {
    std::cerr << G_STRFUNC << ": The file is a directory." << std::endl;
    return false;
  }

  try
  {
    if(!file->remove())
      return false;
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception from Gio::File: " << ex.what() << std::endl;
    return false;
  }

  return true;
}

/** For instance, to find the first file in the directory with a .glom extension.
 */
Glib::ustring Utils::get_directory_child_with_suffix(const Glib::ustring& uri_directory, const std::string& suffix, bool recursive)
{
  Glib::RefPtr<Gio::File> directory = Gio::File::create_for_uri(uri_directory);
  Glib::RefPtr<Gio::FileEnumerator> enumerator = directory->enumerate_children();

  Glib::RefPtr<Gio::FileInfo> info = enumerator->next_file();
  while(info)
  {
    Glib::RefPtr<const Gio::File> child = directory->get_child(info->get_name());

    const Gio::FileType file_type = child->query_file_type();
    if(file_type == Gio::FILE_TYPE_REGULAR)
    {
      //Check the filename:
      const std::string basename = child->get_basename();
      if(string_remove_suffix(basename, suffix) != basename)
        return child->get_uri();
    }
    else if(recursive && file_type == Gio::FILE_TYPE_DIRECTORY)
    {
      //Look in sub-directories too:
      const Glib::ustring result = get_directory_child_with_suffix(child->get_uri(), suffix, recursive);
      if(!result.empty())
        return result;
    }

    info = enumerator->next_file();
  }

  return Glib::ustring();
}

Glib::ustring Utils::get_file_uri_without_extension(const Glib::ustring& uri)
{
  if(uri.empty())
    return uri;

  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
  if(!file)
    return uri; //Actually an error.

  const Glib::ustring filename_part = file->get_basename();

  const Glib::ustring::size_type pos_dot = filename_part.rfind(".");
  if(pos_dot == Glib::ustring::npos)
    return uri; //There was no extension, so just return the existing URI.
  else
  {
    const Glib::ustring filename_part_without_ext = filename_part.substr(0, pos_dot);

    //Use the Gio::File API to manipulate the URI:
    Glib::RefPtr<Gio::File> parent = file->get_parent();
    Glib::RefPtr<Gio::File> file_without_extension = parent->get_child(filename_part_without_ext);

    return file_without_extension->get_uri();
  }
}

std::string Utils::get_file_path_without_extension(const std::string& filepath)
{
  if(filepath.empty())
    return filepath;

  Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(filepath);
  if(!file)
    return filepath; //Actually an error.

  const Glib::ustring filename_part = file->get_basename();

  const Glib::ustring::size_type pos_dot = filename_part.rfind(".");
  if(pos_dot == Glib::ustring::npos)
    return filepath; //There was no extension, so just return the existing URI.
  else
  {
    const Glib::ustring filename_part_without_ext = filename_part.substr(0, pos_dot);

    //Use the Gio::File API to manipulate the URI:
    Glib::RefPtr<Gio::File> parent = file->get_parent();
    Glib::RefPtr<Gio::File> file_without_extension = parent->get_child(filename_part_without_ext);

    return file_without_extension->get_path();
  }
}

Glib::ustring Utils::get_list_of_layout_items_for_display(const LayoutGroup::type_list_items& list_layout_fields)
{
  Glib::ustring result;
  for(const auto& item : list_layout_fields)
  {
    if(item)
    {
      if(!result.empty())
       result += ", ";

      result += item->get_layout_display_name();
    }
  }

  return result;
}

Glib::ustring Utils::get_list_of_layout_items_for_display(const std::shared_ptr<const LayoutGroup>& layout_group)
{
  if(layout_group)
    return get_list_of_layout_items_for_display(layout_group->m_list_items);
  else
    return Glib::ustring();
}

Glib::ustring Utils::get_list_of_sort_fields_for_display(const Formatting::type_list_sort_fields& sort_fields)
{
  Glib::ustring text;
  for(const auto& the_pair : sort_fields)
  {
    const auto item = the_pair.first;
    if(!item)
      continue;
    
    if(!text.empty())
      text += ", ";

    text += item->get_layout_display_name();
    //TODO: Show Ascending/Descending?
  }

  return text;
}

std::string Utils::get_temp_file_path(const std::string& prefix, const std::string& extension)
{
  //Get a temporary file path:
  std::string filepath;
  try
  {
    const std::string prefix_pattern = prefix + "XXXXXX" + extension;
    const int filehandle = Glib::file_open_tmp(filepath, prefix_pattern);
    ::close(filehandle);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Glib::file_open_tmp() failed" << std::endl;
    return filepath;
  }
  
  if(filepath.empty())
  {
    std::cerr << G_STRFUNC << ": Glib::file_open_tmp() returned an empty filepath" << std::endl;
  }

  return filepath;
}

Glib::ustring Utils::get_temp_file_uri(const std::string& prefix, const std::string& extension)
{
  try
  {
    const std::string filepath = get_temp_file_path(prefix, extension);
    return Glib::filename_to_uri(filepath);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception from filename_to_uri(): " << ex.what() << std::endl;
    return std::string();
  }
}

std::string Utils::get_temp_directory_path(const std::string& prefix)
{
  std::string result;

  const std::string pattern = Glib::build_filename(
    Glib::get_tmp_dir(), prefix + "XXXXXX");

  //We must copy the pattern, because mkdtemp() modifies it:
  char* c_pattern = g_strdup(pattern.c_str());
  
  const char* filepath = g_mkdtemp(c_pattern);
  if(filepath)
    result = filepath;

  return result;
}

Glib::ustring Utils::get_temp_directory_uri(const std::string& prefix)
{
  try
  {
    const std::string filepath = get_temp_directory_path(prefix);
    return Glib::filename_to_uri(filepath);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception from filename_to_uri(): " << ex.what() << std::endl;
    return Glib::ustring();
  }
}

LayoutGroup::type_list_const_items Utils::get_layout_items_plus_primary_key(const LayoutGroup::type_list_const_items& items, const Document* document, const Glib::ustring& table_name)
{
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document was null." << std::endl;
    return items;
  }

  const auto field_primary_key = document->get_field_primary_key(table_name);
  if(!field_primary_key)
  {
    std::cerr << G_STRFUNC << ": Could not find the primary key." << std::endl;
    return items;
  }

  std::shared_ptr<LayoutItem_Field> pk_layout_item = std::make_shared<LayoutItem_Field>();
  pk_layout_item->set_hidden();
  pk_layout_item->set_full_field_details(field_primary_key);
  
  if(find_if_layout_item_field_is_same_field_exists(items, pk_layout_item))
    return items; //It is already in the list:

  LayoutGroup::type_list_const_items items_plus_pk = items;
  items_plus_pk.push_back(pk_layout_item);
  return items_plus_pk;
}

//TODO: Avoid the horrible code duplication with the const version.
LayoutGroup::type_list_items Utils::get_layout_items_plus_primary_key(const LayoutGroup::type_list_items& items, const Document* document, const Glib::ustring& table_name)
{
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document was null." << std::endl;
    return items;
  }

  const auto field_primary_key = document->get_field_primary_key(table_name);
  if(!field_primary_key)
  {
    std::cerr << G_STRFUNC << ": Could not find the primary key." << std::endl;
    return items;
  }

  std::shared_ptr<LayoutItem_Field> pk_layout_item = std::make_shared<LayoutItem_Field>();
  pk_layout_item->set_hidden();
  pk_layout_item->set_full_field_details(field_primary_key);
  
  if(find_if_layout_item_field_is_same_field_exists(items, pk_layout_item))
    return items; //It is already in the list:

  LayoutGroup::type_list_items items_plus_pk = items;
  items_plus_pk.push_back(pk_layout_item);
  return items_plus_pk;
}

bool Utils::script_check_for_pygtk2(const Glib::ustring& script)
{
  //There is probably other code that this will not catch,
  //but this is better than nothing.
  //TODO: Instead override python's import mechanism somehow?
  if(script.find("import pygtk") != std::string::npos)
    return false;

  return true;
}

bool Utils::get_resource_exists(const std::string& resource_path)
{
  return Gio::Resource::get_file_exists_global_nothrow(resource_path);
}

} //namespace Glom
