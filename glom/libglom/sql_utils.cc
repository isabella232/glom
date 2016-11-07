/* Glom
 *
 * Copyright (C) 2001-2016 Murray Cumming
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

#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/connectionpool.h>
#include <libglom/sql_utils.h>
#include <libglom/algorithms_utils.h>
#include <iostream>

namespace Glom
{

namespace SqlUtils
{

template<typename T_Container>
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

Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const Gnome::Gda::SqlExpr& where_clause, const std::shared_ptr<const Relationship>& extra_join, const type_sort_clause& sort_clause, guint limit)
{
  //TODO_Performance:
  const auto constFieldsToGet = Utils::const_list(fieldsToGet);

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
Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_count_rows(const Glib::RefPtr<const Gnome::Gda::SqlBuilder>& sql_query)
{
  Glib::RefPtr<Gnome::Gda::SqlBuilder> result;

  if(!sql_query)
  {
    std::cerr << G_STRFUNC << ": sql_query was null.\n";
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
    auto uses_rel = std::make_shared<UsesRelationship>();
    uses_rel->set_relationship(layout_item->get_relationship());
    list_relationships.emplace_front(uses_rel); //These need to be at the front, so that related relationships can use them later in the SQL statement.
  }

  //Add the relationship to the list:
  iterFind = find_if_uses_relationship_has_relationship(list_relationships, layout_item);
  if(iterFind == list_relationships.end()) //If the table is not yet in the list:
  {
    auto uses_rel = std::make_shared<UsesRelationship>();
    uses_rel->set_relationship(layout_item->get_relationship());
    uses_rel->set_related_relationship(layout_item->get_related_relationship());
    list_relationships.emplace_back(uses_rel);
  }

}

static void builder_add_join(const Glib::RefPtr<Gnome::Gda::SqlBuilder>& builder, const std::shared_ptr<const UsesRelationship>& uses_relationship)
{
  auto relationship = uses_relationship->get_relationship();
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
    auto related_relationship = uses_relationship->get_related_relationship();

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

void build_sql_select_add_fields_to_get(const Glib::RefPtr<Gnome::Gda::SqlBuilder>& builder, const Glib::ustring& table_name, const type_vecConstLayoutFields& fieldsToGet, const type_sort_clause& sort_clause, bool extra_join)
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


Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecConstLayoutFields& fieldsToGet, const Gnome::Gda::SqlExpr& where_clause, const std::shared_ptr<const Relationship>& extra_join, const type_sort_clause& sort_clause, guint limit)
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
    build_sql_select_add_fields_to_get(builder, table_name, fieldsToGet, sort_clause, (bool)extra_join);

    if(extra_join)
    {
      auto uses_relationship = std::make_shared<UsesRelationship>();
      uses_relationship->set_relationship(extra_join);
      builder_add_join(builder, uses_relationship);
    }

    //Add the WHERE clause:
    if(!where_clause.empty())
    {
      const auto id = builder->import_expression(where_clause);
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


Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_with_key(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const std::shared_ptr<const Field>& key_field, const Gnome::Gda::Value& key_value, const type_sort_clause& sort_clause, guint limit)
{
  //TODO_Performance:
  const auto constFieldsToGet = Utils::const_list(fieldsToGet);

  return build_sql_select_with_key(table_name, constFieldsToGet, key_field, key_value, sort_clause, limit);
}

Gnome::Gda::SqlExpr build_simple_where_expression(const Glib::ustring& table_name, const std::shared_ptr<const Field>& key_field, const Gnome::Gda::Value& key_value)
{
  if(!key_field)
  {
    std::cerr << G_STRFUNC << ": key_field was empty\n";
    return Gnome::Gda::SqlExpr();
  }

  auto builder = Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
  builder->select_add_target(table_name);  //This might not be necessary.
  const Gnome::Gda::SqlBuilder::Id id = builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_EQ,
                                                          builder->add_field_id(key_field->get_name(), table_name),
                                                          builder->add_expr(key_value));
  builder->set_where(id); //This might not be necessary.

  return builder->export_expression(id);
}

Gnome::Gda::SqlExpr build_combined_where_expression(const Gnome::Gda::SqlExpr& a, const Gnome::Gda::SqlExpr& b, Gnome::Gda::SqlOperatorType op)
{
  auto builder =
          Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);

  const Gnome::Gda::SqlBuilder::Id id = builder->add_cond(op,
                                                          builder->import_expression(a),
                                                          builder->import_expression(b));
  builder->set_where(id);
  return builder->export_expression(id);
}

Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_select_with_key(const Glib::ustring& table_name, const type_vecConstLayoutFields& fieldsToGet, const std::shared_ptr<const Field>& key_field, const Gnome::Gda::Value& key_value, const type_sort_clause& sort_clause, guint limit)
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

  return build_sql_select_with_where_clause(table_name, fieldsToGet, where_clause,
                                                   std::shared_ptr<const Relationship>(), sort_clause, limit);
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
    std::cerr << G_STRFUNC << ": No connection yet.\n";
    return Glib::RefPtr<Gnome::Gda::Connection>();
  }

  auto gda_connection = sharedconnection->get_gda_connection();

  return gda_connection;
}


std::string sqlbuilder_get_full_query(
        const Glib::RefPtr<const Gnome::Gda::SqlBuilder>& builder)
{
  auto connection = get_connection();
  if(!connection)
  {
    //TODO: Just use the correct provider, without an actual connection?
    std::cerr << G_STRFUNC << ": There is no connection, so the SQL statement might not be created correctly.\n";
  }

  Glib::ustring result = "glom_query_not_parsed";

  try
  {
    auto stmt = builder->get_statement();
    if(!stmt)
    {
      std::cerr << G_STRFUNC << ": builder->get_statement() failed.\n";
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
  const auto buf = Glib::make_unique_ptr_gfree(g_convert_with_fallback(
          result.raw().data(), result.raw().size(),
          "ISO-8859-1", "UTF-8",
          (char*)"?",
          nullptr, nullptr, nullptr));

  const Glib::ustring str = std::string(buf.get());
  if(str.empty())
  {
    std::cerr << G_STRFUNC << ": Returning an empty string.\n";
  }

  return str;
}

Gnome::Gda::SqlExpr get_find_where_clause_quick(const std::shared_ptr<const Document>& document, const Glib::ustring& table_name, const Gnome::Gda::Value& quick_search)
{
  if(table_name.empty())
  {
    std::cerr << G_STRFUNC << ": table_name is empty.\n";
    return Gnome::Gda::SqlExpr();
  }

  if(Conversions::value_is_empty(quick_search))
    return Gnome::Gda::SqlExpr();

  auto builder =
          Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
  builder->select_add_target(table_name);

  //We need to add some fields to select,
  //because otherwise the SqlBuilder would not contain a valid query.
  builder->select_add_field("*", table_name);

  if(!document)
  {
    std::cerr << G_STRFUNC << ": document was null.\n";
    return Gnome::Gda::SqlExpr();
  }

  //We need the connection to generate the correct SQL syntax:
  auto connection = get_connection();
  if(!connection)
  {
    std::cerr << G_STRFUNC << ": connection was null.\n";
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
    std::cerr << G_STRFUNC << ": Returning null SqlExpr\n";
    return Gnome::Gda::SqlExpr();
  }
}

Glib::RefPtr<Gnome::Gda::SqlBuilder> build_sql_update_with_where_clause(
        const Glib::ustring& table_name,
        const std::shared_ptr<const Field>& field, const Gnome::Gda::Value& value,
        const Gnome::Gda::SqlExpr& where_clause)
{
  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder;

  if(!field || field->get_name().empty())
  {
    std::cerr << G_STRFUNC << ": field was null or its name was empty.\n";
    return builder;
  }

  if(table_name.empty())
  {
    std::cerr << G_STRFUNC << ": table_name was empty.\n";
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
      const auto id = builder->import_expression(where_clause);
      builder->set_where(id);
    }
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception: " << ex.what() << std::endl;
  }

  return builder;
}

} //namespace SqlUtils

} //namespace Glom

