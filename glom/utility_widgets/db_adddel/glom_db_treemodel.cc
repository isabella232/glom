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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <iostream>
#include "glom_db_treemodel.h"

#include "../../connectionpool.h"
#include "../../data_structure/glomconversions.h" //For util_build_sql
#include "../../utils.h"


DbTreeModelRow::DbTreeModelRow()
: m_values_retrieved(false),
  m_removed(false),
  m_extra(false)
  //m_data_model_row_number(false)
{

}

void DbTreeModelRow::fill_values_if_necessary(DbTreeModel& model, int row)
{
  if(!m_values_retrieved)
  {
    if(row < (int)model.m_data_model_rows_count)
    {
      //It is a row from the database;
      const int cols_count = model.m_data_model_columns_count;
      for(int i = 0; i < cols_count; ++i)
      {
        m_db_values[i] = model.m_gda_datamodel->get_value_at(i, row);
      }

      m_key = model.m_gda_datamodel->get_value_at(model.m_column_index_key, row);

      m_extra = false;
      m_removed = false;
    }
    else
    {
      //g_warning("DbTreeModelRow::fill_values_if_necessary(): Non-db row.");
      if(m_extra)
      {
        //It is an extra row, added with append().
        //Use the default values.
      }
      else if(!m_removed)
      {
        //It must be the last blank placeholder row.
        //m_placeholder = true;
      }
    }

  }

  m_values_retrieved = true; //Don't read them again.
}

void DbTreeModelRow::set_value(DbTreeModel& model, int column, int row, const DbValue& value)
{
  fill_values_if_necessary(model, row);
  m_db_values[column] = value;
}


DbTreeModelRow::DbValue DbTreeModelRow::get_value(DbTreeModel& model, int column, int row)
{
  fill_values_if_necessary(model, row);

  type_vec_values::const_iterator iterFind = m_db_values.find(column);
  if(iterFind != m_db_values.end())
    return iterFind->second;
  else
    return DbValue();
}

DbTreeModel::GlueItem::GlueItem(const DbTreeModel::type_datamodel_iter& row_iter)
: m_datamodel_row_iter( row_iter )
{
}



/*
DbTreeModel::type_datamodel_iter DbTreeModel::GlueItem::get_row_iter() const
{
  return m_row_iter;
}
*/

DbTreeModel::type_datamodel_iter DbTreeModel::GlueItem::get_datamodel_row_iter() const
{
  return m_datamodel_row_iter;
}

DbTreeModel::GlueList::GlueList()
{
}

DbTreeModel::GlueList::~GlueList()
{
  //Delete each GlueItem in the list:
  for(type_listOfGlue::iterator iter = m_list.begin(); iter != m_list.end(); ++iter)
  {
    DbTreeModel::GlueItem* pItem = *iter;
    if(pItem)
      delete pItem;
  }
}

DbTreeModel::GlueItem* DbTreeModel::GlueList::get_existing_item(const type_datamodel_iter& row_iter)
{
  for(type_listOfGlue::iterator iter = m_list.begin(); iter != m_list.end(); ++iter)
  {
    DbTreeModel::GlueItem* pItem = *iter;
    if(pItem->get_datamodel_row_iter() == row_iter) //TODO_Performance: Access m_row_iter directly?
      return pItem;
  }

  return 0;
}

//Intialize static variable:
bool DbTreeModel::m_iface_initialized = false;

DbTreeModel::DbTreeModel(const Gtk::TreeModelColumnRecord& columns, const FoundSet& found_set, const type_vec_fields& column_fields, int column_index_key, bool get_records)
: Glib::ObjectBase( typeid(DbTreeModel) ), //register a custom GType.
  Glib::Object(), //The custom GType is actually registered here.
  m_columns_count(0),
  m_found_set(found_set),
  m_column_fields(column_fields),
  m_column_index_key(column_index_key),
  m_data_model_rows_count(0),
  m_data_model_columns_count(0),
  m_count_extra_rows(0),
  m_count_removed_rows(0),
  m_get_records(get_records),
  m_stamp(1), //When the model's stamp != the iterator's stamp then that iterator is invalid and should be ignored. Also, 0=invalid
  m_pGlueList(0)
{
  if(!m_iface_initialized)
  {
    //GType gtype = G_OBJECT_TYPE(gobj());  //The custom GType created in the Object constructor, from the typeid.
    //Gtk::TreeModel::add_interface( gtype );

    m_iface_initialized = true; //Prevent us from calling add_interface() on the same gtype again.
  }


  //The Column information that can be used with TreeView::append(), TreeModel::iterator[], etc.
  m_columns_count = columns.size(); //1 extra for the key.

  g_assert(m_columns_count == column_fields.size());

  //refresh_from_database();
}

DbTreeModel::~DbTreeModel()
{
  clear();
}

//static:
Glib::RefPtr<DbTreeModel> DbTreeModel::create(const Gtk::TreeModelColumnRecord& columns, const FoundSet& found_set, const type_vec_fields& column_fields, int column_index_key, bool get_records)
{
  return Glib::RefPtr<DbTreeModel>( new DbTreeModel(columns, found_set, column_fields, column_index_key, get_records) );
}

bool DbTreeModel::refresh_from_database(const FoundSet& found_set)
{
  m_found_set = found_set;

  if(!m_get_records)
    return false;


  clear(); //Clear existing shown records.

  //Connect to database:
  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
  {
     m_connection = connection_pool->connect();
  }

  if(m_connection && !m_found_set.m_table_name.empty() && m_get_records)
  {
    const Glib::ustring sql_query = GlomUtils::build_sql_select_with_where_clause(m_found_set.m_table_name, m_column_fields, m_found_set.m_where_clause, m_found_set.m_sort_clause);

    //std::cout << "DbTreeModel: Executing SQL: " << sql_query << std::endl << std::endl;
    m_gda_datamodel = m_connection->get_gda_connection()->execute_single_command(sql_query);
    if(!m_gda_datamodel)
    {
      m_data_model_rows_count = 0;
      m_data_model_columns_count = m_columns_count;

      std::cerr << "DbTreeModel::refresh_from_database(): Error while executing SQL" << std::endl <<
                   "  " <<  sql_query << std::endl;

      ConnectionPool::handle_error();
      return false; //No records were found.
    }
    else
    {
      m_data_model_rows_count = m_gda_datamodel->get_n_rows(); //TODO_Performance: This probably gets all the data.
      m_data_model_columns_count = m_gda_datamodel->get_n_columns();

      /*
      guint rows_to_get = 100;
      if(rows_to_get > m_data_model_rows_count)
        rows_to_get = m_data_model_rows_count;

      for(guint i = 0; i < rows_to_get; ++i)
      {
        iterator iter;
        const bool iter_is_valid = create_iterator(i, iter);
        if(iter_is_valid)
          row_inserted(get_path(iter), iter); //Allow the TreeView to respond to the addition.
      }
      */

      return true;
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
  if(check_treeiter_validity(iter))
  {
    if(column < (int)m_columns_count)
    {
      //Get the correct ValueType from the Gtk::TreeModel::Column's type, so we don't have to repeat it here:
      typeModelColumn::ValueType value_specific;
      value_specific.init( typeModelColumn::ValueType::value_type() );  //TODO: Is there any way to avoid this step?

      //Or, instead of asking the compiler for the TreeModelColumn's ValueType:
      //Glib::Value< DbValue > value_specific;
      //value_specific.init( Glib::Value< DbValue >::value_type() ); //TODO: Is there any way to avoid this step?

      type_datamodel_const_iter dataRowIter = get_datamodel_row_iter_from_tree_row_iter(iter);
      //g_warning("DbTreeModel::get_value_vfunc(): dataRowIter=%d, get_internal_rows_count=%d", dataRowIter, get_internal_rows_count());
      const unsigned int internal_rows_count = get_internal_rows_count();
      if( dataRowIter < internal_rows_count) //!= m_rows.end())
      {
        //const typeRow& dataRow = *dataRowIter;

        //g_warning("DbTreeModel::get_value_vfunc 1: column=%d, row=%d", column, dataRowIter);

        DbValue result;

        DbTreeModelRow& row_details = m_map_rows[dataRowIter]; //Adds it if necessary.
        const int column_sql = column;
        if(column_sql < (int)m_columns_count) //TODO_Performance: Remove the checks.
        {
          if( !(dataRowIter < (internal_rows_count - 1)))
          {
             //std::cout << "DbTreeModel::get_value_vfunc: row " << dataRowIter << " is placeholder" << std::endl;

             //If it's after the database rows then it must be a placeholder row.
             //We have only one of these because iter_n_root_children_vfunc() only adds 1 to the row count.
             //row_details.m_placeholder = true;
          }
          else
          {
            result = row_details.get_value(const_cast<DbTreeModel&>(*this), column_sql, dataRowIter); //m_gda_datamodel->get_value_at(column_sql, dataRowIter); //dataRow.m_db_values[column];
          }
        }
        else
          g_warning("DbTreeModel::get_value_vfunc: column out of bounds: sql_col=%d, max=%d.", column_sql, m_columns_count);


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
    type_datamodel_iter row_iter = get_datamodel_row_iter_from_tree_row_iter(iter);

    //Make the iter_next GtkTreeIter represent the next row:
    ++row_iter;

    //Jump over removed rows:
    while(row_was_removed(row_iter))
      ++row_iter;

   //g_warning("DbTreeModel::iter_next_vfunc(): attempting to return row=%d", row_iter);
    return create_iterator(row_iter, iter_next);
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
  //std::cout << "iter_n_root_children_vfunc(): returning: " << get_internal_rows_count() - m_count_removed_rows + 1 << std::endl;
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

    //TODO_Performance
    /*
    type_datamodel_iter row_iter = m_rows.begin();
    for(int i = 0; i < n; ++i)
    {
      if(row_iter == m_rows.end())
        break;

      ++row_iter;
    }
    */

    //TODO_Performance:
    //Get the nth unremoved row:
    type_datamodel_iter row_iter = 0;
    for(int child_n = 0; child_n < n; ++child_n)
    {
      //Jump over hidden rows:
      while(m_map_rows[row_iter].m_removed)
        ++row_iter;

      ++row_iter;
    }

    while(m_map_rows[row_iter].m_removed)
      ++row_iter;

    return create_iterator(row_iter, iter); //create_iterator(row_iter, iter);
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
  type_datamodel_iter row_iter = get_datamodel_row_iter_from_tree_row_iter(iter);

  //TODO_Performance:
  /*
  int index = 0;
  for(type_datamodel_iter iter_count = m_rows.begin(); iter_count != row_iter; ++iter_count)
  {
    ++index;
  }
  */

  //TODO_Performance:
  //Get the number of non-removed items before this iter, because the path index doesn't care about removed internal stuff.
  int path_index = -1 ;
  if(row_iter > 0) //A row inedx of 0 must mean a path index ir there are _any_ non-removed rows.
  for(type_datamodel_iter i = 0; i <= row_iter; ++i)
  {
    if(!(m_map_rows[i].m_removed))
      ++path_index;
  }

  //g_warning("DbTreeModel::get_path_vfunc(): returning path index %d for internal row %d", path_index, row_iter);

  if(path_index == -1)
    path_index = 0; //This should never happen, but let's not risk having a strange -1 value if it does.

  Gtk::TreeModel::Path path;
  path.push_back(path_index); //path.push_back(index);
  return path;
}

bool DbTreeModel::create_iterator(const type_datamodel_iter& row_iter, DbTreeModel::iterator& iter) const
{
  Glib::RefPtr<DbTreeModel> refModel(const_cast<DbTreeModel*>(this));
  refModel->reference();

  iter.set_model_refptr(refModel);

  const guint count_all_rows = get_internal_rows_count();
//g_warning("DbTreeModel::create_iterator(): row_iter=%d, count=%d", row_iter, count_all_rows);
  if(row_iter >= (count_all_rows)) //row_iter == m_rows.end()) //1 for the placeholder.
  {
    //TreeView seems to use this to identify the last row.
    //g_warning("DbTreeModel::create_iterator(): out of bounds: returning invalid iterator: row_iter=%d", row_iter);
    invalidate_iter(iter);
    return false;
  }
  else
  {
    iter.set_stamp(m_stamp); 
    //Store the std::list iterator in the GtkTreeIter:
    //See also iter_next_vfunc()

    //g_warning("DbTreeModel::create_iterator(): Creating iter to row_index=%d", row_iter);
    if(!m_pGlueList)
    {
     m_pGlueList = new GlueList();
    }

    GlueItem* pItem = m_pGlueList->get_existing_item(row_iter);
    if(!pItem)
    {
      pItem = new GlueItem(row_iter);
      remember_glue_item(pItem);
    }

    //Store the GlueItem in the GtkTreeIter.
    //This will be deleted in the GlueList destructor,
    //which will be called when the old GtkTreeIters are marked as invalid,
    //when the stamp value changes. 
    iter.gobj()->user_data = (void*)pItem;

    return true;
  }
}

bool DbTreeModel::get_iter_vfunc(const Path& path, iterator& iter) const
{
  //g_warning("DbTreeModel::get_iter_vfunc(): path=%s", path.to_string().c_str());

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

DbTreeModel::type_datamodel_iter DbTreeModel::get_datamodel_row_iter_from_tree_row_iter(const iterator& iter) const
{
  const GlueItem* pItem = (const GlueItem*)iter.gobj()->user_data;
  if(pItem)
    return pItem->get_datamodel_row_iter();
  else
  {
    g_warning("DbTreeModel::get_datamodel_row_iter_from_tree_row_iter(): iter has no GlueItem.");
    return type_datamodel_iter();
  }
}

/*
DbTreeModel::type_datamodel_iter DbTreeModel::get_data_row_iter_from_tree_row_iter(const iterator& iter) const
{
  //Don't call this on an invalid iter.
  const GlueItem* pItem = (const GlueItem*)iter.gobj()->user_data;
  if(pItem)
    return pItem->get_datamodel_row_iter();
  else
  {
    g_warning("DbTreeModel::get_datamodel_row_iter_from_tree_row_iter(): iter has no GlueItem.");
    return type_datamodel_iter();
  }
}
*/

/*
DbTreeModel::type_datamodel_iter DbTreeModel::get_datamodel_row_iter_from_tree_row_iter(const iterator& iter) const
{
  //Don't call this on an invalid iter.
  const GlueItem* pItem = (const GlueItem*)iter.gobj()->user_data; 
  return  pItem->get_datamodel_row_iter();
}
*/

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
  if(!check_treeiter_validity(iter))
    return false;

  return Gtk::TreeModel::iter_is_valid(iter);
}

void DbTreeModel::remember_glue_item(GlueItem* item) const
{
  //Add the GlueItem to the model's GlueList, so that
  //it can be deleted when the old GtkTreeIters are marked as invalid:
  if(!m_pGlueList)
  {
    m_pGlueList = new GlueList();
  }

  m_pGlueList->m_list.push_back(item);
}

int DbTreeModel::get_internal_rows_count() const
{
  return m_data_model_rows_count + m_count_extra_rows + 1; //1 for placeholder.
}

DbTreeModel::iterator DbTreeModel::append()
{
  //const size_type existing_size = m_data_model_rows_count;
  //g_warning("DbTreeModel::append(): existing_size = %d", existing_size);
  //m_rows.resize(existing_size + 1);

  //Get aniterator to the last element:
  type_datamodel_iter row_iter = get_internal_rows_count();
  //g_warning("DbTreeModel::append(): new row number=%d", row_iter);
  ++m_count_extra_rows; //So that create_iterator() can succeed.

  //Create the row:
  //row_iter->m_db_values.resize(m_columns_count); 

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
  const bool iter_is_valid = create_iterator(row_iter, iter);
  if(iter_is_valid)
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
    type_datamodel_iter row_iter = get_datamodel_row_iter_from_tree_row_iter(row);

    //TODO: Check column against get_n_columns() too, though it could hurt performance.


    //Cast it to the specific value type.
    //It can't work with anything else anyway.
    typedef Glib::Value< DbValue > ValueDbValue;

    const ValueDbValue* pDbValue = static_cast<const ValueDbValue*>(&value);
    if(!pDbValue)
      g_warning("DbTreeModel::set_value_impl(): value is not a Value< DbValue >.");
    else
    {
      DbTreeModelRow& row_details = m_map_rows[row_iter]; //Adds it if necessary.
      row_details.set_value(*this, column, row_iter, pDbValue->get());

      //TODO: Performance: get_path() is really slow.
      row_changed( get_path(row), row);

      //g_warning("set_value_impl: value=%s", pDbValue->get().to_string().c_str());

      //TODO: DbValue& refValue = row_iter->m_db_values[column];

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
    type_datamodel_iter row_iter = get_datamodel_row_iter_from_tree_row_iter(iter);

    //Remove the row.
    Gtk::TreePath path_deleted = get_path(iter);

    if(!(m_map_rows[row_iter].m_removed))
    {
      m_map_rows[row_iter].m_removed = true;
      ++m_count_removed_rows;

      row_deleted(path_deleted);

      //Get next non-removed row:
      while(row_was_removed(row_iter))
        ++row_iter;

      //Return an iterator to the next row:
      create_iterator(row_iter, iter_result);
    }
  }

  return iter_result;
}


void DbTreeModel::set_key_value(const TreeModel::iterator& iter, const DbValue& value)
{
  //g_warning("DbTreeModel::set_is_placeholder(): val=%d", val);
  if(check_treeiter_validity(iter))
  {
    type_datamodel_iter dataRowIter = get_datamodel_row_iter_from_tree_row_iter(iter);
    m_map_rows[dataRowIter].m_key = value;
  }
}

DbTreeModel::DbValue DbTreeModel::get_key_value(const TreeModel::iterator& iter) const
{   
  if(check_treeiter_validity(iter))
  {
    type_datamodel_iter dataRowIter = get_datamodel_row_iter_from_tree_row_iter(iter);
    type_map_rows::iterator iterFind = m_map_rows.find(dataRowIter);
    if(iterFind != m_map_rows.end())
      return iterFind->second.m_key;
    else
    {
      DbTreeModelRow& row_details = m_map_rows[dataRowIter]; //Adds it if necessary
      row_details.fill_values_if_necessary(const_cast<DbTreeModel&>(*this), dataRowIter);
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
    type_datamodel_iter dataRowIter = get_datamodel_row_iter_from_tree_row_iter(iter);
    return (dataRowIter == ((type_datamodel_iter)get_internal_rows_count() -1));
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

  if(m_pGlueList)
  {
    delete m_pGlueList;
    m_pGlueList = 0;
  }
}

bool DbTreeModel::row_was_removed(const type_datamodel_iter& row_iter) const
{
  type_map_rows::const_iterator iterFind = m_map_rows.find(row_iter);
  if(iterFind != m_map_rows.end())
    return iterFind->second.m_removed;
  else
    return false; //If it was never accessed before then it has never been removed.
}

Gtk::TreeModel::iterator DbTreeModel::get_last_row()
{
  iterator result;

  //Find the last non-removed row:
  int rows_count = get_internal_rows_count();
  if(rows_count)
  {
    type_datamodel_iter row = rows_count - 1;
    --rows_count; //Ignore the placeholder.

    //Step backwards until we find one that is not removed.
    while((m_map_rows.find(row) != m_map_rows.end()) && m_map_rows[row].m_removed)
    {
      if(row)
        --row;
      else
        return result; //failed, because there are no non-removed row.
    }

    //g_warning("DbTreeModel::get_last_row(): returning row=%d", row);
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
    type_datamodel_iter row = rows_count - 1;

    //Step backwards until we find one that is not removed:
    while((m_map_rows.find(row) != m_map_rows.end()) && m_map_rows[row].m_removed)
    {
      if(row)
        --row;
      else
      {
        g_warning("bTreeModel::get_placeholder_row(): Placeholder row not found.");
        return result; //failed, because there are no non-removed rows.
      }
    }

    type_map_rows::const_iterator iter_map = m_map_rows.find(row);
    if(iter_map != m_map_rows.end())
    {
      //g_warning("DbTreeModel::get_last_row(): returning row=%d", row);
      create_iterator(row, result);
    }
  }

  return result;
}

void DbTreeModel::get_record_counts(gulong& total, gulong& found) const
{
  if(m_gda_datamodel)
  {
    found = (gulong)m_gda_datamodel->get_n_rows();

    if(m_found_set.m_where_clause.empty())
      total = found;
    else
    {
      //Ask the database how many records there are in the whole table:
      //TODO: Apparently, this is very slow:
      const Glib::ustring sql_query = "SELECT count(*) FROM \"" + m_found_set.m_table_name + "\"";
      Glib::RefPtr<Gnome::Gda::DataModel> datamodel = m_connection->get_gda_connection()->execute_single_command(sql_query);

      if(datamodel)
      {
        if(datamodel->get_n_rows())
        {
          Gnome::Gda::Value value = datamodel->get_value_at(0, 0);
          total = (gulong)value.get_bigint(); //I discovered that it's a bigint by trying it.
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
