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

DbTreeModelRow::DbTreeModelRow()
: m_placeholder(false)
{

}

DbTreeModel::GlueItem::GlueItem(int row_number)
: m_row_number(row_number)
{
}

int DbTreeModel::GlueItem::get_row_number() const
{
  return m_row_number;
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
    delete pItem;
  }
}
     

DbTreeModel::DbTreeModel(const Gtk::TreeModelColumnRecord& columns)
: Glib::ObjectBase( typeid(DbTreeModel) ), //register a custom GType.
  Glib::Object(), //The custom GType is actually registered here.
  m_columns_count(0),
  m_stamp(1), //When the model's stamp != the iterator's stamp then that iterator is invalid and should be ignored. Also, 0=invalid
  m_pGlueList(0)
{
  GType gtype = G_OBJECT_TYPE(gobj());  //The custom GType created in the Object constructor, from the typeid.
  Gtk::TreeModel::add_interface( gtype );


  //The Column information that can be used with TreeView::append(), TreeModel::iterator[], etc.
  m_columns_count = columns.size();
  m_listModelColumns.resize(m_columns_count);
  for(unsigned int column_number = 0; column_number < m_columns_count; ++column_number)
  {
    m_column_record.add( m_listModelColumns[column_number] );
  }
}

DbTreeModel::~DbTreeModel()
{
  if(m_pGlueList)
  {
    delete m_pGlueList;
  }
}

//static:
Glib::RefPtr<DbTreeModel> DbTreeModel::create(const Gtk::TreeModelColumnRecord& columns)
{
  return Glib::RefPtr<DbTreeModel>( new DbTreeModel(columns) );
}

Gtk::TreeModelFlags DbTreeModel::get_flags_vfunc() const
{
   return Gtk::TREE_MODEL_LIST_ONLY; // | Gtk::TREE_MODEL_ITERS_PERSIST
}

int DbTreeModel::get_n_columns_vfunc() const
{
  return m_columns_count;
}

GType DbTreeModel::get_column_type_vfunc(int index) const
{
  if(index <= (int)m_listModelColumns.size())
    return m_listModelColumns[index].type();
  else
    return 0;
}

void DbTreeModel::get_value_vfunc(const TreeModel::iterator& iter, int column, Glib::ValueBase& value) const
{
  if(check_treeiter_validity(iter))
  {
    if(column <= (int)m_listModelColumns.size())
    {
      //Get the correct ValueType from the Gtk::TreeModel::Column's type, so we don't have to repeat it here:
      typeModelColumn::ValueType value_specific;
      value_specific.init( typeModelColumn::ValueType::value_type() );  //TODO: Is there any way to avoid this step?

      //Or, instead of asking the compiler for the TreeModelColumn's ValueType:
      //Glib::Value< DbValue > value_specific;
      //value_specific.init( Glib::Value< DbValue >::value_type() ); //TODO: Is there any way to avoid this step?
      
      typeListOfRows::const_iterator dataRowIter = get_data_row_iter_from_tree_row_iter(iter);
      if(dataRowIter != m_rows.end())
      {
        const typeRow& dataRow = *dataRowIter;

        DbValue result = dataRow.m_db_values[column];

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
    //initialize the iterator:
    iter_next = iterator();
    iter_next.set_stamp(m_stamp);
    
    //Get the current row:
    const GlueItem* pItem = (const GlueItem*)iter.gobj()->user_data;
    typeListOfRows::size_type row_index = pItem->get_row_number();
        
    //Make the iter_next GtkTreeIter represent the next row:
    row_index++;
    if( row_index < m_rows.size() )
    { 
      //Put the index of the next row in a GlueItem in iter_next:
      //GlueItem* pItemNew = new GlueItem(row_index);
      //iter_next.gobj()->user_data = (void*)pItemNew;
      //
      //remember_glue_item(pItemNew);
      
      iter_next = create_iterator(row_index);
      
      return true; //success
    }
  }
  else
    iter_next = iterator(); //Set is as invalid, as the TreeModel documentation says that it should be.

  return false; //There is no next row.
}

bool DbTreeModel::iter_children_vfunc(const iterator& parent, iterator& iter) const
{
  return iter_nth_child_vfunc(parent, 0, iter);
}

bool DbTreeModel::iter_has_child_vfunc(const iterator& iter) const
{
  return (iter_n_children_vfunc(iter) > 0);
}

int DbTreeModel::iter_n_children_vfunc(const iterator& iter) const
{
  if(!check_treeiter_validity(iter))
    return 0;
    
  return 0; //There are no children
}

int DbTreeModel::iter_n_root_children_vfunc() const
{
  return m_rows.size();
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
  if(n < (int)m_rows.size())
  {
    iter = iterator(); //clear the input parameter.
    iter.set_stamp(m_stamp);

    //Store the row_index in the GtkTreeIter:
    //See also iter_next_vfunc()

    unsigned row_index = n;

    //This will be deleted in the GlueList destructor, when old iterators are marked as invalid.
    //GlueItem* pItem = new GlueItem(row_index);
    //iter.gobj()->user_data = pItem;
    //
    //remember_glue_item(pItem);
    
    iter = create_iterator(row_index);
   
    return true;
  }
  
  return false; //There are no children.  
}
  

bool DbTreeModel::iter_parent_vfunc(const iterator& child, iterator& iter) const
{
  if(!check_treeiter_validity(child))
  {
    iter = iterator(); //Set is as invalid, as the TreeModel documentation says that it should be.
    return false;
  }

  iter = iterator(); //Set is as invalid, as the TreeModel documentation says that it should be.
  return false; //There are no children, so no parents.
}

Gtk::TreeModel::Path DbTreeModel::get_path_vfunc(const iterator& /* iter */) const
{
   //TODO:
   return Path();
}

DbTreeModel::iterator DbTreeModel::create_iterator(typeListOfRows::size_type row_index) const
{
  iterator iter(const_cast<DbTreeModel*>(this));
  
  //TODO_Performance:
  if(row_index >= m_rows.size())
  {
    iter.set_stamp(0); //Invalid/end.
  }
  else
  {
    iter.set_stamp(m_stamp);
    
    //Store the row_index in the GtkTreeIter:
    //See also iter_next_vfunc()
    //TODO: Store a pointer to some more complex data type such as a typeListOfRows::iterator.
  
    //g_warning("DbTreeModel::create_iterator(): Creating iter to row_index=%d", row_index);
    GlueItem* pItem = new GlueItem(row_index);
  
    //Store the GlueItem in the GtkTreeIter.
    //This will be deleted in the GlueList destructor,
    //which will be called when the old GtkTreeIters are marked as invalid,
    //when the stamp value changes. 
    iter.gobj()->user_data = (void*)pItem;
  
    remember_glue_item(pItem);
  }
   
  return iter;
}

bool DbTreeModel::get_iter_vfunc(const Path& path, iterator& iter) const
{
   unsigned sz = path.size();
   if(!sz)
   {
     iter = iterator(); //Set is as invalid, as the TreeModel documentation says that it should be.
     return false;
   }

   if(sz > 1) //There are no children.
   {
     iter = iterator(); //Set is as invalid, as the TreeModel documentation says that it should be.
     return false; 
   }

   //This is a new GtkTreeIter, so it needs the current stamp value.
   //See the comment in the constructor.
   iter = create_iterator( path[0] );
   
   return true;
}

Gtk::TreeModelColumn< DbTreeModel::DbValue >& DbTreeModel::get_model_column(int column)
{
  return m_listModelColumns[column];
}

DbTreeModel::typeListOfRows::iterator DbTreeModel::get_data_row_iter_from_tree_row_iter(const iterator& iter)
{
  //Don't call this on an invalid iter.
  const GlueItem* pItem = (const GlueItem*)iter.gobj()->user_data;

  typeListOfRows::size_type row_index = pItem->get_row_number();
  if( row_index > m_rows.size() )
    return m_rows.end();
  else
  {
    g_warning("DbTreeModel::get_data_row_iter_from_tree_row_iter(): row_index=%d", row_index);
    
    typeListOfRows::iterator dataIter = m_rows.begin();
    for(typeListOfRows::size_type i = 0; i < row_index; ++i)
    {
      ++dataIter;
    }
    
    return dataIter;
      
    //return m_rows.begin() + row_index; //TODO: Performance.
  }
}

DbTreeModel::typeListOfRows::const_iterator DbTreeModel::get_data_row_iter_from_tree_row_iter(const iterator& iter) const
{
  //Don't call this on an invalid iter.
  const GlueItem* pItem = (const GlueItem*)iter.gobj()->user_data;
  
  typeListOfRows::size_type row_index = pItem->get_row_number();
  if( row_index > m_rows.size() )
    return m_rows.end();
  else
    return m_rows.begin() + row_index; //TODO: Performance.
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

DbTreeModel::iterator DbTreeModel::append()
{
  const size_type existing_size = m_rows.size();
  m_rows.resize(existing_size + 1);
  const size_type row_index = existing_size;
 
  //Create the row:
  m_rows[row_index].m_db_values.resize(m_columns_count); 

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
  return create_iterator(row_index);
}

void DbTreeModel::set_value_impl(const iterator& row, int column, const Glib::ValueBase& value)
{
  if(iter_is_valid(row))
  {
    //Get the index from the user_data:
    const GlueItem* pItem = (const GlueItem*)row.gobj()->user_data;
    typeListOfRows::size_type row_index = pItem->get_row_number();
    if( row_index <= m_rows.size() )
    {
      //TODO: Check column against get_n_columns() too, though it could hurt performance.
      
    
      //Cast it to the specific value type.
      //It can't work with anything else anyway.
      typedef Glib::Value< DbValue > ValueDbValue;
      
      const ValueDbValue* pDbValue = static_cast<const ValueDbValue*>(&value);
      if(!pDbValue)
        g_warning("DbTreeModel::set_value_impl(): value is not a Value< DbValue >.");
      else
      {
        DbValue& refValue = (m_rows[row_index]).m_db_values[column];
        
        refValue = pDbValue->get();
      }
    }
  }
}

DbTreeModel::iterator DbTreeModel::erase(const iterator& iter)
{
  if(iter_is_valid(iter))
  {
    //Get the index from the user_data:
    const GlueItem* pItem = (const GlueItem*)iter.gobj()->user_data;
    typeListOfRows::size_type row_index = pItem->get_row_number();
    if( row_index <= m_rows.size() )
    {
      //Get an iterator to the row:
      //TODO_performance: Store an iterator instead of an index in the GlueItem?
      typeListOfRows::iterator iterRow = m_rows.begin();
      typeListOfRows::size_type i = 0;
      while((i < row_index) && (iterRow != m_rows.end()))
      {
        ++i;
        ++iterRow;
      }
      
      //Remove the row.
      /*  typeListOfRows::iterator iterNextRow = */
      m_rows.erase(iterRow);
      
      //Other iterators are now invalid, so change the stamp value to mark them as invalid.
      ++m_stamp;
      
      //Return an iterator to the next row:
      return create_iterator(row_index);
    }
  }
  
  return iterator();
}

void DbTreeModel::set_is_placeholder(const TreeModel::iterator& iter, bool val)
{
  if(check_treeiter_validity(iter))
  {
    typeListOfRows::iterator dataRowIter = get_data_row_iter_from_tree_row_iter(iter);
    if(dataRowIter != m_rows.end())
    {
       dataRowIter->m_placeholder = val;
    }
  }
}

bool DbTreeModel::get_is_placeholder(const TreeModel::iterator& iter) const
{
  if(check_treeiter_validity(iter))
  {
    typeListOfRows::const_iterator dataRowIter = get_data_row_iter_from_tree_row_iter(iter);
    if(dataRowIter != m_rows.end())
    {
       return dataRowIter->m_placeholder;
    }
  }
  
  return false;
}
