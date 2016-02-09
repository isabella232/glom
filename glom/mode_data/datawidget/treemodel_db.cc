/* Glom
 *
 * Copyright (C) 2001-2005 Murray Cumming
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

#include <iostream>
#include "treemodel_db.h"

#include <libglom/connectionpool.h>
#include <libglom/data_structure/glomconversions.h> //For util_build_sql
#include <libglom/utils.h>
#include <libglom/db_utils.h>
#include <libgdamm/datamodel.h>
#include <libgdamm/dataaccesswrapper.h>
#include <libgdamm/datamodelarray.h>

#include <glom/appwindow.h>

namespace Glom
{

DbTreeModelRow::DbTreeModelRow()
: m_values_retrieved(false),
  m_removed(false),
  m_extra(false)
  //m_data_model_row_number(false)
{

}

void DbTreeModelRow::fill_values_if_necessary(DbTreeModel& model, int row)
{
  //std::cout << "debug: " << G_STRFUNC << ": row=" << row << std::endl;
  //if(row == 1000)
  //{
  //  std::cout << "1000\n";
  //}

  if(m_values_retrieved)
  {
     //std::cout << "debug: " << G_STRFUNC << ": already retrieved\n";
  }
  else
  {
    //std::cout << "debug: " << G_STRFUNC << ": retrieving for row=" << row << std::endl;

    if((row < (int)model.m_data_model_rows_count) && model.m_gda_datamodel)
    {
      auto iter = model.m_gda_datamodel->create_iter();
      if(iter)
      {
        iter->move_to_row(row);

        //It is a row from the database;
        const int cols_count = model.m_data_model_columns_count;
        for(int i = 0; i < cols_count; ++i)
        {
          try
          {
            m_db_values[i] = iter->get_value_at(i);
          }
          catch(const Glib::Error& ex)
          {
            // This is quite possible, for example for unset dates. jhs
            std::cerr << G_STRFUNC << ": get_value_at() failed for column=" << i << ", with exception: " << ex.what() << std::endl;
          }

          //std::cout << "  debug: col=" << i << ", GType=" << m_db_values[i].get_value_type() << ", string=" << m_db_values[i].to_string() << std::endl;
        }

        //TODO: Use iter->get_value_at(i) with try/catch instead when we can depend on libgdamm >= 4.99.4.2
        try
        {
          m_key = iter->get_value_at(model.m_column_index_key);
        }
        catch(const Glib::Error& ex)
        {
          // This is quite possible, for example for unset dates. jhs
          std::cerr << G_STRFUNC << ": get_value_at() failed for model.m_column_index_key=" << model.m_column_index_key << ", with exception: " << ex.what() << std::endl;
        }

        m_extra = false;
        m_removed = false;
      }
    }
    else
    {
      //std::cerr << G_STRFUNC << ": Non-db row.\n";
      if(m_extra)
      {
        //std::cout << "debug: " << G_STRFUNC << ": using default value\n";

        //It is an extra row, added with append().
      }
      else if(!m_removed)
      {
        //It must be the last blank placeholder row.
        //m_placeholder = true;
      }

      //Create default values, if necessary, of the correct types:
      //Examine the columns in the returned DataModel:
      const Glib::RefPtr<const Gnome::Gda::DataModel> datamodel = model.m_gda_datamodel;
      for(guint col = 0; col < model.m_data_model_columns_count; ++col)
      {
        if(m_db_values.find(col) == m_db_values.end()) //If there is not already a value in the map for this column.
        {
          if(!datamodel) //though this should not happen.
          {
            m_db_values[col] = Gnome::Gda::Value();
          }
          else
          {
            const Glib::RefPtr<const Gnome::Gda::Column> column = datamodel->describe_column(col);

            //We don't just create a Gda::Value of the column's gda type,
            //because we should use a NULL-type Gda::Value as the initial value for some fields:
            const Field::glom_field_type glom_type = Field::get_glom_type_for_gda_type(column->get_g_type());
            m_db_values[col] = Glom::Conversions::get_empty_value(glom_type);
          }
        }
      }
    }

  }

  m_values_retrieved = true; //Don't read them again.
}

void DbTreeModelRow::set_value(DbTreeModel& model, int column, int row, const DbValue& value)
{
  fill_values_if_necessary(model, row);

  //Check that the value has the correct type:
  /*
  Glib::RefPtr<const Gnome::Gda::Column> gdacolumn = model.m_gda_datamodel->describe_column(column);
  const GType debug_type_in = value.get_value_type();
  const auto debug_type_expected = gdacolumn->get_g_type();
  if(debug_type_in != debug_type_expected)
  {
    std::cout << "debug: " << G_STRFUNC << ": expected GType=" << debug_type_expected << ", but received GType=" << debug_type_in << std::endl;
    if(debug_type_expected)
      std::cout << "  expected GType name=\"" << g_type_name(debug_type_expected) << "\"\n";

    if(debug_type_in)
      std::cout << "  received GType name=\"" << g_type_name(debug_type_in) << "\"\n";
  }
  */

  m_db_values[column] = value;
}


DbTreeModelRow::DbValue DbTreeModelRow::get_value(DbTreeModel& model, int column, int row)
{
  fill_values_if_necessary(model, row);

  const auto iterFind = m_db_values.find(column);
  if(iterFind != m_db_values.end())
    return iterFind->second;
  else
  {
    std::cout << "debug: " << G_STRFUNC << ": column not found.\n";
    return DbValue();
  }
}

//Intialize static variable:
bool DbTreeModel::m_iface_initialized = false;

DbTreeModel::DbTreeModel(const FoundSet& found_set, const type_vec_const_layout_items& layout_items, bool get_records, bool find_mode, Base_DB::type_vecConstLayoutFields& fields_shown)
: Glib::ObjectBase( typeid(DbTreeModel) ), //register a custom GType.
  Glib::Object(), //The custom GType is actually registered here.
  m_columns_count(0),
  m_found_set(found_set),
  m_column_index_key(-1), //means it's not set yet.
  m_data_model_rows_count(0),
  m_data_model_columns_count(0),
  m_count_extra_rows(0),
  m_count_removed_rows(0),
  m_get_records(get_records),
  m_find_mode(find_mode),
  m_stamp(1) //When the model's stamp != the iterator's stamp then that iterator is invalid and should be ignored. Also, 0=invalid
{
  if(!m_iface_initialized)
  {
    //GType gtype = G_OBJECT_TYPE(gobj());  //The custom GType created in the Object constructor, from the typeid.
    //Gtk::TreeModel::add_interface( gtype );

    m_iface_initialized = true; //Prevent us from calling add_interface() on the same gtype again.
  }

  //Database columns:;
  {
    for(const auto& item : layout_items)
    {
      auto item_field = std::dynamic_pointer_cast<const LayoutItem_Field>(item);
      if(item_field)
      {
        if(item_field->get_glom_type() == Field::glom_field_type::INVALID)
          std::cerr << G_STRFUNC << ": field has invalid type. field name: " << item_field->get_name() << std::endl;

        m_column_fields.emplace_back(item_field);
      }
    }
  }

  fields_shown = m_column_fields;

  {
    //Find the primary key:
    int column_index_key = 0;
    bool key_found = false;
    for(const auto& layout_item : m_column_fields)
    {
      if(!layout_item)
        continue;

      if( !(layout_item->get_has_relationship_name()) )
      {
        const auto field_full = layout_item->get_full_field_details();
        if(!field_full)
          std::cerr << G_STRFUNC << ": The layout item (" << layout_item->get_name() << ") has no field details.\n";
        else if(field_full->get_primary_key() )
        {
          key_found = true;
          break;
        }
      }

      ++column_index_key;
    }

    if(!key_found)
    {
      std::cerr << G_STRFUNC << ": no primary key field found in the list of items:\n";
      for(const auto& layout_item : m_column_fields)
      {
        if(layout_item)
          std::cerr << G_STRFUNC << ":   field: " << layout_item->get_name() << std::endl;
      }
      
      return;
    }
      
    m_column_index_key = column_index_key;
      
    //The Column information that can be used with TreeView::append(), TreeModel::iterator[], etc.
    m_columns_count = m_column_fields.size();
    refresh_from_database(m_found_set);
  }
}


DbTreeModel::~DbTreeModel()
{
  clear();
}


Glib::RefPtr<DbTreeModel> DbTreeModel::create(const FoundSet& found_set, const type_vec_const_layout_items& layout_items, bool get_records, bool find_mode, Base_DB::type_vecConstLayoutFields& fields_shown)
{
  return Glib::RefPtr<DbTreeModel>( new DbTreeModel(found_set, layout_items, get_records, find_mode, fields_shown) );
}

Glib::RefPtr<DbTreeModel> DbTreeModel::create(const FoundSet& found_set, const type_vec_layout_items& layout_items, bool get_records, bool find_mode, Base_DB::type_vecConstLayoutFields& fields_shown)
{
  //Create a const version of the input, because C++ can't convert it automatically:
  type_vec_const_layout_items const_items;
  const_items.insert(const_items.end(), layout_items.begin(), layout_items.end());

  return create(found_set, const_items, get_records, find_mode, fields_shown);
}

bool DbTreeModel::refresh_from_database(const FoundSet& found_set)
{
  //std::cout << "DbTreeModel::refresh_from_database()\n";
  m_found_set = found_set;

  if(!m_get_records && !m_find_mode)
    return false;

  clear(); //Clear existing shown records.

  if(m_find_mode)
  {
    m_get_records = false; //Otherwise it would not make sense.

    //Use a dummy DataModel that has the same columns and types,
    //but which does not use a real database table,
    //so we can use it to add find criteria.
    auto model_array = Gnome::Gda::DataModelArray::create(m_column_fields.size());
    m_gda_datamodel = model_array;

    int col = 0;
    for(const auto& layout_item : m_column_fields)
    {
      if(layout_item)
      {
        const auto glom_type = layout_item->get_glom_type();
        const GType gda_type = Field::get_gda_type_for_glom_type(glom_type);
        model_array->set_column_g_type(col, gda_type);
        ++col;
      }
    }

    //Add at least an initial row:
  m_gda_datamodel->append_row(); //TODO: Handle adding.
    return true;
  }


  //Connect to database:
  auto connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
  {
     m_connection = connection_pool->connect();
  }

  if(m_found_set.m_table_name.empty())
    std::cerr << G_STRFUNC << ": found_set.m_table_name is empty.\n";

  if(m_connection && !m_found_set.m_table_name.empty() && m_get_records)
  {
    auto sql_query = Utils::build_sql_select_with_where_clause(m_found_set.m_table_name, m_column_fields, m_found_set.m_where_clause, m_found_set.m_extra_join, m_found_set.m_sort_clause);
    //std::cout << "debug: " << G_STRFUNC << ":  " << sql_query << std::endl;

    m_gda_datamodel = DbUtils::query_execute_select(sql_query, true /* use_cursor */);

    if(!m_gda_datamodel)
    {
      m_data_model_rows_count = 0;
      m_data_model_columns_count = m_columns_count;

      std::cerr << G_STRFUNC << ": error executing SQL. SQL query: \n";
      std::cerr << G_STRFUNC << ":   " << Utils::sqlbuilder_get_full_query(sql_query) << std::endl;
      ConnectionPool::handle_error_cerr_only();
      return false; //No records were found.
    }
    else
    {
      // If using the sqlite backed, then use a DataAccessWrapper to allow random access. This is necessary
      // since we use move_to_row() on a created iterator in
      // fill_values_if_necessary(), which does not work if the iterator
      // does not support it (for example the one for Sqlite recordsets does
      // not). The alternative would be to acquire a random-access model
      // directly here for SQLite, but this would
      // a) make this code dependent on the database backend used.
      // b) fetch rows we perhaps don't need, if only the first few rows of
      // a table are accessed.
      auto connection = ConnectionPool::get_instance();
      if(connection && !connection->get_backend_supports_cursor())
        m_gda_datamodel = Gnome::Gda::DataAccessWrapper::create(m_gda_datamodel);

      //This doesn't work with cursor-based models: const int count = m_gda_datamodel->get_n_rows();
      //because rows count is -1 until we have iterated to the last row.
      auto sql_query_without_sort = Utils::build_sql_select_with_where_clause(m_found_set.m_table_name, m_column_fields, m_found_set.m_where_clause, m_found_set.m_extra_join, type_sort_clause());
      const int count = DbUtils::count_rows_returned_by(sql_query_without_sort);
      if(count < 0)
      {
        std::cerr << G_STRFUNC << ": count is < 0\n";
        m_data_model_rows_count = 0;
      }
      else
      {
        m_data_model_rows_count = count;
      }

      //std::cout << "  rows count=" << m_data_model_rows_count << std::endl;

      m_data_model_columns_count = m_gda_datamodel->get_n_columns();

      return (m_data_model_rows_count > 0); //false is not really a failure, but the caller needs to know whether the foundset found any records.
    }
  }
  else
  {
    m_data_model_columns_count = m_columns_count;
    return false; //No records were found.
  }
}

Gtk::TreeModelFlags DbTreeModel::get_flags_vfunc() const
{
   return (Gtk::TreeModelFlags)(Gtk::TREE_MODEL_LIST_ONLY | Gtk::TREE_MODEL_ITERS_PERSIST);
}

int DbTreeModel::get_n_columns_vfunc() const
{
  return m_columns_count; //including the key.
}

GType DbTreeModel::get_column_type_vfunc(int index) const
{
  if(index < (int)m_columns_count)
    return typeModelColumn::ValueType::value_type();
  else
    return 0;
}

void DbTreeModel::get_value_vfunc(const TreeModel::iterator& iter, int column, Glib::ValueBase& value) const
{
  //std::cout << "debug: " << G_STRFUNC << ": column=" << column << std::endl;

  if(check_treeiter_validity(iter))
  {
    //std::cout << "  debug: DbTreeModel::get_value_vfunc() 1\n";

    if(column < (int)m_columns_count)
    {
       //std::cout << "  debug: DbTreeModel::get_value_vfunc() 1.1\n";

      //Get the correct ValueType from the Gtk::TreeModel::Column's type, so we don't have to repeat it here:
      //(This would be a custom boxed type for our Gda::Value (stored inside the TreeModel's Glib::Value just as an int or char* would be stored in it.)
      //std::cout << "debug: " << G_STRFUNC << ": column=" << column << ", value type=" << g_type_name(typeModelColumn::ValueType::value_type()) << std::endl;

      typeModelColumn::ValueType value_specific;
      value_specific.init( typeModelColumn::ValueType::value_type() );  //TODO: Is there any way to avoid this step?

      //Or, instead of asking the compiler for the TreeModelColumn's ValueType:
      //Glib::Value< DbValue > value_specific;
      //value_specific.init( Glib::Value< DbValue >::value_type() ); //TODO: Is there any way to avoid this step?

      type_datamodel_row_index datamodel_row = get_datamodel_row_index_from_tree_row_iter(iter);
      //g_warning("DbTreeModel::get_value_vfunc(): datamodel_row=%d, get_internal_rows_count=%d", datamodel_row, get_internal_rows_count());
      const unsigned int internal_rows_count = get_internal_rows_count();
      if( datamodel_row < internal_rows_count) //!= m_rows.end())
      {
         //std::cout << "  debug: DbTreeModel::get_value_vfunc() 1.2\n";

        //const typeRow& dataRow = *datamodel_row;

        //g_warning("DbTreeModel::get_value_vfunc 1: column=%d, row=%d", column, datamodel_row);

        DbValue result;

        DbTreeModelRow& row_details = m_map_rows[datamodel_row]; //Adds it if necessary.
        const int column_sql = column;
        if(column_sql < (int)m_columns_count) //TODO_Performance: Remove the checks.
        {
           //std::cout << "  debug: DbTreeModel::get_value_vfunc() 1.3\n";

          if( !(datamodel_row < (internal_rows_count - 1)))
          {
              //std::cout << "  debug: DbTreeModel::get_value_vfunc() 1.4\n";

             //std::cout << "DbTreeModel::get_value_vfunc: row " << datamodel_row << " is placeholder\n";

             //If it's after the database rows then it must be a placeholder row.
             //We have only one of these because iter_n_root_children_vfunc() only adds 1 to the row count.
             //row_details.m_placeholder = true;
          }
          else
          {
            /*
            std::cout << "  debug: DbTreeModel::get_value_vfunc() 1.4b: column_sql=" << column_sql << std::endl;
            //Examine the columns in the returned DataModel:
            for(int col = 0; col < m_gda_datamodel->get_n_columns(); ++col)
            {
              auto column = m_gda_datamodel->describe_column(col);
              std::cout << "    debug: column index=" << col << ", name=" << column->get_name() << ", type=" << g_type_name(column->get_g_type()) << std::endl;
            }
            */

            //Double check that the result has the correct type:
            //Glib::RefPtr<const Gnome::Gda::Column> column = m_gda_datamodel->describe_column(column_sql);
            //const GType gtype_expected = column->get_g_type();

            result = row_details.get_value(const_cast<DbTreeModel&>(*this), column_sql, datamodel_row); //m_gda_datamodel->get_value_at(column_sql, datamodel_row); //dataRow.m_db_values[column];

            /*
            if((result.get_value_type() != 0) && (result.get_value_type() != gtype_expected))
            {
              std::cout << "debug: " << G_STRFUNC << ": column_sql=" << column_sql << ", describe_column() returned GType: " << gtype_expected << " but get_value() returned GType: " << result.get_value_type() << std::endl;
            }
            */
          }
        }
        else
          g_warning("DbTreeModel::get_value_vfunc: column out of bounds: sql_col=%d, max=%d.", column_sql, m_columns_count);


        /*
        GType debug_type = result.get_value_type();
        std::cout << "debug: " << G_STRFUNC << ": result value type: GType=" << debug_type << std::endl;
        if(debug_type)
          std::cout << "    GType name=\"" << g_type_name(debug_type) << "\"\n";
        */

        value_specific.set(result); //The compiler would complain if the type was wrong.
        value.init( Glib::Value< DbValue >::value_type() ); //TODO: Is there any way to avoid this step? Can't it copy the type as well as the value?
        value = value_specific;
      }
    }
  }
}

bool DbTreeModel::iter_next_vfunc(const iterator& iter, iterator& iter_next) const
{
  if( check_treeiter_validity(iter) )
  {
    //Get the current row:
    type_datamodel_row_index datamodel_row = get_datamodel_row_index_from_tree_row_iter(iter);
    //std::cout << "debug: " << G_STRFUNC << ":" << datamodel_row << std::endl;

    //Make the iter_next GtkTreeIter represent the next row:
    ++datamodel_row;

    //Jump over removed rows:
    while(row_was_removed(datamodel_row))
      ++datamodel_row;

   //g_warning("DbTreeModel::iter_next_vfunc(): attempting to return row=%d", datamodel_row);
    return create_iterator(datamodel_row, iter_next);
  }
  else
    iter_next = iterator(); //Set is as invalid, as the TreeModel documentation says that it should be.

  return false; //There is no next row.
}

bool DbTreeModel::iter_children_vfunc(const iterator& parent, iterator& iter) const
{
  return iter_nth_child_vfunc(parent, 0, iter);
}

bool DbTreeModel::iter_has_child_vfunc(const iterator& /* iter */) const
{
  return false;
}

int DbTreeModel::iter_n_children_vfunc(const iterator& iter) const
{
  if(!check_treeiter_validity(iter))
    return iter_n_root_children_vfunc();

  return 0; //There are no children
}

int DbTreeModel::iter_n_root_children_vfunc() const
{
  //std::cout << "debug: " << G_STRFUNC << ": returning: " << get_internal_rows_count() - m_count_removed_rows + 1 << std::endl;
  return get_internal_rows_count() - m_count_removed_rows;
}

void DbTreeModel::invalidate_iter(iterator& iter) const
{
  iter.set_stamp(0);
}

bool DbTreeModel::iter_nth_child_vfunc(const iterator& parent, int /* n */, iterator& iter) const
{
  if(!check_treeiter_validity(parent))
  {
    iter = iterator(); //Set is as invalid, as the TreeModel documentation says that it should be.
    return false;
  }

  iter = iterator(); //Set is as invalid, as the TreeModel documentation says that it should be.
  return false; //There are no children.
}

bool DbTreeModel::iter_nth_root_child_vfunc(int n, iterator& iter) const
{
//g_warning("DbTreeModel::iter_nth_root_child_vfunc(): n=%d", n);

  //if(m_data_model_rows_count == 0)
  //  return false;  //There are no children.

  if(true) //n < (int)m_data_model_rows_count)
  {
    //Store the row_index in the GtkTreeIter:
    //See also iter_next_vfunc()

    //TODO_Performance:
    //Get the nth unremoved row:
    type_datamodel_row_index datamodel_row = 0;
    for(int child_n = 0; child_n < n; ++child_n)
    {
      //Jump over hidden rows:
      while(m_map_rows[datamodel_row].m_removed)
        ++datamodel_row;

      ++datamodel_row;
    }

    while(m_map_rows[datamodel_row].m_removed)
      ++datamodel_row;

    return create_iterator(datamodel_row, iter); //create_iterator(datamodel_row, iter);
  }

  return false; //There are no children.
}


bool DbTreeModel::iter_parent_vfunc(const iterator& child, iterator& iter) const
{
  if(!check_treeiter_validity(child))
  {
    invalidate_iter(iter); //Set is as invalid, as the TreeModel documentation says that it should be.
    return false;
  }

  invalidate_iter(iter); //Set is as invalid, as the TreeModel documentation says that it should be.
  return false; //There are no children, so no parents.
}

Gtk::TreeModel::Path DbTreeModel::get_path_vfunc(const iterator& iter) const
{
  type_datamodel_row_index datamodel_row = get_datamodel_row_index_from_tree_row_iter(iter);

  //TODO_Performance:
  //Get the number of non-removed items before this iter, because the path index doesn't care about removed internal stuff.
  int path_index = -1;
  if(datamodel_row > 0) //A row index of 0 must mean a path index if there are _any_ non-removed rows.
  {
    for(type_datamodel_row_index i = 0; i <= datamodel_row; ++i)
    {
      if(!(m_map_rows[i].m_removed))
        ++path_index;
    }
  }

  //g_warning("DbTreeModel::get_path_vfunc(): returning path index %d for internal row %d", path_index, datamodel_row);

  if(path_index == -1)
    path_index = 0; //This should never happen, but let's not risk having a strange -1 value if it does.

  Gtk::TreeModel::Path path;
  path.push_back(path_index); //path.push_back(index);
  return path;
}

bool DbTreeModel::create_iterator(const type_datamodel_row_index& datamodel_row, DbTreeModel::iterator& iter) const
{
  Glib::RefPtr<DbTreeModel> refModel(const_cast<DbTreeModel*>(this));
  refModel->reference();

  iter.set_model_refptr(refModel);

  const guint count_all_rows = get_internal_rows_count();
  //g_warning("DbTreeModel::create_iterator(): datamodel_row=%d, count=%d", datamodel_row, count_all_rows);
  if(datamodel_row >= (count_all_rows)) //datamodel_row == m_rows.end()) //1 for the placeholder.
  {
    //TreeView seems to use this to identify the last row.
    //g_warning("DbTreeModel::create_iterator(): out of bounds: returning invalid iterator: datamodel_row=%d", datamodel_row);
    invalidate_iter(iter);
    return false;
  }
  else
  {
    iter.set_stamp(m_stamp);
    //Store the std::list iterator in the GtkTreeIter:
    //See also iter_next_vfunc()

    //Store the row number in the GtkTreeIter.
    iter.gobj()->user_data = GINT_TO_POINTER(datamodel_row);

    return true;
  }
}

bool DbTreeModel::get_iter_vfunc(const Path& path, iterator& iter) const
{
   //std::cout << ": path=" << path << std::endl;

   unsigned sz = path.size();
   if(!sz)
   {
     invalidate_iter(iter); //Set is as invalid, as the TreeModel documentation says that it should be.
     return false;
   }

   if(sz > 1) //There are no children.
   {
     invalidate_iter(iter); //Set is as invalid, as the TreeModel documentation says that it should be.
     return false;
   }

   return iter_nth_root_child_vfunc(path[0], iter);
}

DbTreeModel::type_datamodel_row_index DbTreeModel::get_datamodel_row_index_from_tree_row_iter(const iterator& iter) const
{
  return GPOINTER_TO_INT(iter.gobj()->user_data);
}


bool DbTreeModel::check_treeiter_validity(const iterator& iter) const
{
  if(!(iter->get_model_gobject()))
    return false;

  // Anything that modifies the model's structure should change the model's stamp,
  // so that old iters are ignored.
  return m_stamp == iter.get_stamp();
}

bool DbTreeModel::iter_is_valid(const iterator& iter) const
{
  return check_treeiter_validity(iter);
}


int DbTreeModel::get_internal_rows_count() const
{
  return m_data_model_rows_count + m_count_extra_rows + 1; //1 for placeholder.
}

DbTreeModel::iterator DbTreeModel::append()
{
  //const size_type existing_size = m_data_model_rows_count;
  //std::cerr << G_STRFUNC << ": existing_size = " << existing_size << std::endl;
  //m_rows.resize(existing_size + 1);

  //Get aniterator to the last element:
  type_datamodel_row_index datamodel_row = get_internal_rows_count();
  //std::cerr << G_STRFUNC << ": new row number=" << datamodel_row << std::endl;
  ++m_count_extra_rows; //So that create_iterator() can succeed.

  //Create the row:
  //datamodel_row->m_db_values.resize(m_columns_count);

  for(unsigned int column_number = 0; column_number < m_columns_count; ++column_number)
  {
      // Set the data in the row cells:
      // It is more likely that you would be reusing existing data from some other data structure,
      // instead of generating the data here.

      //char buffer[20]; //You could use a std::stringstream instead.
      //g_snprintf(buffer, sizeof(buffer), "%d, %d", row_number, column_number);

      //(m_rows[row_number]).m_db_values[column_number] = buffer; //Note that allcolumns here are of the same type.
  }

  //Create the iterator to the new row:
  iterator iter;
  const auto the_iter_is_valid = create_iterator(datamodel_row, iter);
  if(the_iter_is_valid)
  {
    row_inserted(get_path(iter), iter); //Allow the TreeView to respond to the addition.
  }

  return iter;
}


void DbTreeModel::set_value_impl(const iterator& row, int column, const Glib::ValueBase& value)
{
  if(iter_is_valid(row))
  {
    //Get the index from the user_data:
    type_datamodel_row_index datamodel_row = get_datamodel_row_index_from_tree_row_iter(row);

    //TODO: Check column against get_n_columns() too, though it could hurt performance.


    //Cast it to the specific value type.
    //It can't work with anything else anyway.
    typedef Glib::Value< DbValue > ValueDbValue;

    const auto pDbValue = static_cast<const ValueDbValue*>(&value);
    if(!pDbValue)
      std::cerr << G_STRFUNC << ": value is not a Value< DbValue >.\n";
    else
    {
      DbTreeModelRow& row_details = m_map_rows[datamodel_row]; //Adds it if necessary.
      row_details.set_value(*this, column, datamodel_row, pDbValue->get());

      //TODO: Performance: get_path() is really slow.
      row_changed( get_path(row), row);

      //g_warning("set_value_impl: value=%s", pDbValue->get().to_string().c_str());

      //TODO: DbValue& refValue = datamodel_row->m_db_values[column];

      //refValue = pDbValue->get();

      //TODO: Performance: get_path() is really slow.
      //row_changed( get_path(row), row);
    }
  }
}


DbTreeModel::iterator DbTreeModel::erase(const iterator& iter)
{
  iterator iter_result;

  if(iter_is_valid(iter))
  {
    //Get the index from the user_data:
    type_datamodel_row_index datamodel_row = get_datamodel_row_index_from_tree_row_iter(iter);

    //Remove the row.
    Gtk::TreeModel::Path path_deleted = get_path(iter);

    if(!(m_map_rows[datamodel_row].m_removed))
    {
      m_map_rows[datamodel_row].m_removed = true;
      ++m_count_removed_rows;

      row_deleted(path_deleted);

      //Get next non-removed row:
      while(row_was_removed(datamodel_row))
        ++datamodel_row;

      //Return an iterator to the next row:
      create_iterator(datamodel_row, iter_result);
    }
  }

  return iter_result;
}


void DbTreeModel::set_key_value(const TreeModel::iterator& iter, const DbValue& value)
{
  //g_warning("DbTreeModel::set_is_placeholder(): val=%d", val);
  if(check_treeiter_validity(iter))
  {
    type_datamodel_row_index datamodel_row = get_datamodel_row_index_from_tree_row_iter(iter);
    m_map_rows[datamodel_row].m_key = value;
  }
}

DbTreeModel::DbValue DbTreeModel::get_key_value(const TreeModel::iterator& iter) const
{
  if(check_treeiter_validity(iter))
  {
    type_datamodel_row_index datamodel_row = get_datamodel_row_index_from_tree_row_iter(iter);
    auto iterFind = m_map_rows.find(datamodel_row);
    if(iterFind != m_map_rows.end())
      return iterFind->second.m_key;
    else
    {
      DbTreeModelRow& row_details = m_map_rows[datamodel_row]; //Adds it if necessary
      row_details.fill_values_if_necessary(const_cast<DbTreeModel&>(*this), datamodel_row);
      return row_details.m_key;
    }
  }

  return DbValue();
}

bool DbTreeModel::get_is_placeholder(const TreeModel::iterator& iter) const
{
  //g_warning("DbTreeModel::g et_is_placeholder()");
  if(check_treeiter_validity(iter))
  {
    type_datamodel_row_index datamodel_row = get_datamodel_row_index_from_tree_row_iter(iter);
    return (datamodel_row == ((type_datamodel_row_index)get_internal_rows_count() -1));
  }

  return false;
}

void DbTreeModel::set_is_not_placeholder(const TreeModel::iterator& iter)
{
  if(get_is_placeholder(iter))
  {
    //This row is an extra row (after a refresh it will probably be a row in the database).
    //So now we need a new row to be the placeholder.
    append();
  }
}

void DbTreeModel::clear()
{
  //m_rows.clear();
  m_map_rows.clear();
}

bool DbTreeModel::row_was_removed(const type_datamodel_row_index& datamodel_row) const
{
  const auto iterFind = m_map_rows.find(datamodel_row);
  if(iterFind != m_map_rows.end())
    return iterFind->second.m_removed;
  else
    return false; //If it was never accessed before then it has never been removed.
}

Gtk::TreeModel::iterator DbTreeModel::get_last_row()
{
  iterator result;

  //Find the last non-removed row:
  const int rows_count = get_internal_rows_count();
  if(rows_count)
  {
    type_datamodel_row_index row = rows_count - 1;
    
    if(row > 0) //This should always be true, because there is always a placeholder.
      --row; //Ignore the placeholder.
   
    //Step backwards until we find one that is not removed.
    while((m_map_rows.find(row) != m_map_rows.end()) && m_map_rows[row].m_removed)
    {
      if(row)
        --row;
      else
        return result; //failed, because there are no non-removed row.
    }

    //std::cout << G_STRFUNC << ": debug: returning row=" << row << std::endl;
    create_iterator(row, result);
  }

  return result;
}

Gtk::TreeModel::iterator DbTreeModel::get_placeholder_row()
{
  iterator result;

  //Find the last non-removed row:
  const int rows_count = get_internal_rows_count();
  if(rows_count)
  {
    type_datamodel_row_index row = rows_count - 1;

    //Step backwards until we find one that is not removed:
    while((m_map_rows.find(row) != m_map_rows.end()) && m_map_rows[row].m_removed)
    {
      if(row)
        --row;
      else
      {
        std::cerr << G_STRFUNC << ": Placeholder row not found.\n";
        return result; //failed, because there are no non-removed rows.
      }
    }

    const auto iter_map = m_map_rows.find(row);
    if(iter_map != m_map_rows.end())
    {
      //std::cerr << G_STRFUNC << ": returning row=" << row << std::endl;
      create_iterator(row, result);
    }
  }

  return result;
}

void DbTreeModel::get_record_counts(gulong& total, gulong& found) const
{
  if(m_gda_datamodel)
  {
    //This doesn't work with iter-only models (it returns -1): found = (gulong)m_gda_datamodel->get_n_rows();
    found = m_data_model_rows_count;

    if(m_found_set.m_where_clause.empty())
      total = found;
    else
    {
      //Ask the database how many records there are in the whole table:
      //TODO: Apparently, this is very slow:

      auto builder =
        Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);

      const auto id_function = builder->add_function("count", builder->add_id("*")); //TODO: Is * allowed here?
      builder->add_field_value_id(id_function);

      builder->select_add_target(m_found_set.m_table_name);

      auto datamodel = DbUtils::query_execute_select(builder);

      if(datamodel)
      {
        if(datamodel->get_n_rows())
        {
          const auto value = datamodel->get_value_at(0, 0);
	  // This will probably fail on Windows, where a long is only 32 bits wide.
          total = static_cast<gulong>(value.get_int64()); //I discovered that it's a int64 by trying it.
        }
      }
    }
  }
  else
  {
    total = 0;
    found = 0;
  }
}

} //namespace Glom
