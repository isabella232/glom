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
: m_placeholder(false),
  m_debug(0)
{

}

DbTreeModel::GlueItem::GlueItem(const DbTreeModel::typeListOfRows::iterator& row_iter)
: m_row_iter(row_iter)
{
}

DbTreeModel::typeListOfRows::iterator DbTreeModel::GlueItem::get_row_iter() const
{
  return m_row_iter;
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

DbTreeModel::GlueItem* DbTreeModel::GlueList::get_existing_item(const typeListOfRows::iterator& row_iter)
{
  for(type_listOfGlue::iterator iter = m_list.begin(); iter != m_list.end(); ++iter)
  {
    DbTreeModel::GlueItem* pItem = *iter;
    if(pItem->get_row_iter() == row_iter) //TODO_Performance: Access m_row_iter directly?
      return pItem;
  }
  
  return 0;
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
}

DbTreeModel::~DbTreeModel()
{
  clear();
}

//static:
Glib::RefPtr<DbTreeModel> DbTreeModel::create(const Gtk::TreeModelColumnRecord& columns)
{
  return Glib::RefPtr<DbTreeModel>( new DbTreeModel(columns) );
}

Gtk::TreeModelFlags DbTreeModel::get_flags_vfunc() const
{
   return (Gtk::TreeModelFlags)(Gtk::TREE_MODEL_LIST_ONLY | Gtk::TREE_MODEL_ITERS_PERSIST);
}

int DbTreeModel::get_n_columns_vfunc() const
{
  return m_columns_count;
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
    //Get the current row:
    typeListOfRows::iterator row_iter = get_data_row_iter_from_tree_row_iter(iter);
        
    //Make the iter_next GtkTreeIter represent the next row:
    ++row_iter;
    
    create_iterator(row_iter, iter_next);
    
    return true; //success
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
  return m_rows.size();
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
  if(n < (int)m_rows.size())
  {
    //Store the row_index in the GtkTreeIter:
    //See also iter_next_vfunc()
    
    //TODO_Performance
    typeListOfRows::iterator row_iter = m_rows.begin();
    for(int i = 0; i < n; ++i)
    {
      if(row_iter == m_rows.end())
        break;
        
      ++row_iter;
    }
    
    create_iterator(row_iter, iter);
   
    return true;
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
   typeListOfRows::iterator row_iter = get_data_row_iter_from_tree_row_iter(iter);
   
   //TODO_Performance:
   int index = 0;
   for(typeListOfRows::iterator iter_count = m_rows.begin(); iter_count != row_iter; ++iter_count)
   {
     ++index;
   }
   
   Gtk::TreeModel::Path path;
   path.push_back(index);
   return path;
}

void DbTreeModel::create_iterator(const typeListOfRows::iterator& row_iter, DbTreeModel::iterator& iter) const
{
  Glib::RefPtr<DbTreeModel> refModel(const_cast<DbTreeModel*>(this));
  refModel->reference();
  
  iter.set_model_refptr(refModel);
  
  if(row_iter == m_rows.end())
  {
    iter.set_stamp(0);
  }
  else
  {
    iter.set_stamp(m_stamp);
    
    //Store the std::list iterator in the GtkTreeIter:
    //See also iter_next_vfunc()
    
    //g_warning("DbTreeModel::create_iterator(): Creating iter to row_index=%d", row_index);
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

DbTreeModel::typeListOfRows::iterator DbTreeModel::get_data_row_iter_from_tree_row_iter(const iterator& iter) const
{
  //Don't call this on an invalid iter.
  const GlueItem* pItem = (const GlueItem*)iter.gobj()->user_data;
  if(pItem)
    return pItem->get_row_iter();
  else
  {
    g_warning("DbTreeModel::get_data_row_iter_from_tree_row_iter(): iter has no GlueItem.");
    return typeListOfRows::iterator();
  }
}

/*
DbTreeModel::typeListOfRows::const_iterator DbTreeModel::get_data_row_iter_from_tree_row_iter(const iterator& iter) const
{
  //Don't call this on an invalid iter.
  const GlueItem* pItem = (const GlueItem*)iter.gobj()->user_data; 
  return  pItem->get_row_iter();
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

DbTreeModel::iterator DbTreeModel::append()
{
  const size_type existing_size = m_rows.size();
  //g_warning("DbTreeModel::append(): existing_size = %d", existing_size);
  m_rows.resize(existing_size + 1);
    
  //Get a std::list iterator to the last element:
  typeListOfRows::iterator row_iter = m_rows.end();
  --row_iter;
  row_iter->m_debug = existing_size;
 
  //Create the row:
  row_iter->m_db_values.resize(m_columns_count); 

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
  create_iterator(row_iter, iter);
  
  row_inserted(get_path(iter), iter); //Allow the TreeView to respond to the addition.
    
  return iter;
}

void DbTreeModel::set_value_impl(const iterator& row, int column, const Glib::ValueBase& value)
{
  if(iter_is_valid(row))
  {
    //Get the index from the user_data:
    typeListOfRows::iterator row_iter = get_data_row_iter_from_tree_row_iter(row);

    //TODO: Check column against get_n_columns() too, though it could hurt performance.
    
  
    //Cast it to the specific value type.
    //It can't work with anything else anyway.
    typedef Glib::Value< DbValue > ValueDbValue;
    
    const ValueDbValue* pDbValue = static_cast<const ValueDbValue*>(&value);
    if(!pDbValue)
      g_warning("DbTreeModel::set_value_impl(): value is not a Value< DbValue >.");
    else
    {
      //g_warning("set_value_impl: value=%s", pDbValue->get().to_string().c_str());
      
      DbValue& refValue = row_iter->m_db_values[column];
      
      refValue = pDbValue->get();
      
      //TODO: Performance: get_path() is really slow.
      row_changed( get_path(row), row);
    }
  }
}

DbTreeModel::iterator DbTreeModel::erase(const iterator& iter)
{
  iterator iter_result;
      
  if(iter_is_valid(iter))
  {
    //Get the index from the user_data:
    typeListOfRows::iterator row_iter = get_data_row_iter_from_tree_row_iter(iter);
   
    //Remove the row.
    Gtk::TreePath path_deleted = get_path(iter);
    typeListOfRows::iterator iterNextRow = m_rows.erase(row_iter);
    
    row_deleted(path_deleted);
    
    //Return an iterator to the next row:
    create_iterator(iterNextRow, iter_result);
  }
  
  return iter_result;
}

void DbTreeModel::set_is_placeholder(const TreeModel::iterator& iter, bool val)
{
  //g_warning("DbTreeModel::set_is_placeholder(): val=%d", val);
  if(check_treeiter_validity(iter))
  {
    typeListOfRows::iterator dataRowIter = get_data_row_iter_from_tree_row_iter(iter);
    if(dataRowIter != m_rows.end())
    {
       //g_warning("  DbTreeModel::set_is_placeholder() settting.");
       dataRowIter->m_placeholder = val;
    }
  }
}

bool DbTreeModel::get_is_placeholder(const TreeModel::iterator& iter) const
{
  //g_warning("DbTreeModel::g et_is_placeholder()");
  if(check_treeiter_validity(iter))
  {
    typeListOfRows::const_iterator dataRowIter = get_data_row_iter_from_tree_row_iter(iter);
    if(dataRowIter != m_rows.end())
    {
       //g_warning("  DbTreeModel::set_is_placeholder(): returning %d", dataRowIter->m_placeholder);
       return dataRowIter->m_placeholder;
    }
  }
  
  return false;
}

void DbTreeModel::clear()
{
  m_rows.clear();
  
  if(m_pGlueList)
  {
    delete m_pGlueList;
    m_pGlueList = 0;
  }
}
