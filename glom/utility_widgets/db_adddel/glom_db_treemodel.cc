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

#include <glom/libglom/connectionpool.h>
#include <glom/libglom/data_structure/glomconversions.h> //For util_build_sql
#include <glom/libglom/utils.h>

#include "glom/application.h"

namespace Glom
{

#ifndef GLIBMM_VFUNCS_ENABLED
  // C vfuncs implementation. Merely copied from gtkmm.
  GtkTreeModelFlags DbTreeModel::glom_get_flags_impl(GtkTreeModel* model)
  {
    Glib::ObjectBase* const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)model));
    if(obj_base)
    {
      Glom::DbTreeModel* cpp_model = dynamic_cast<Glom::DbTreeModel*>(obj_base);
      g_assert(cpp_model);
      return GtkTreeModelFlags(cpp_model->get_flags_vfunc());
    }
    else
      return GtkTreeModelFlags(0);
  }

  gint DbTreeModel::glom_get_n_columns_impl(GtkTreeModel* model)
  {
    Glib::ObjectBase* const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)model));
    if(obj_base)
    {
      Glom::DbTreeModel* cpp_model = dynamic_cast<Glom::DbTreeModel*>(obj_base);
      g_assert(cpp_model);
      return cpp_model->get_n_columns_vfunc();
    }
    else
      return 0;
  }
  
  GType DbTreeModel::glom_get_column_type_impl(GtkTreeModel* model, gint index)
  {
    Glib::ObjectBase* const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)model));
    if(obj_base)
    {
      Glom::DbTreeModel* cpp_model = dynamic_cast<Glom::DbTreeModel*>(obj_base);
      g_assert(cpp_model);
      return cpp_model->get_column_type_vfunc(index);
    }
    else
      return G_TYPE_NONE;
  }

  void DbTreeModel::glom_get_value_impl(GtkTreeModel* model, GtkTreeIter* iter, gint column, GValue* value)
  {
    Glib::ObjectBase* const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)model));
    if(obj_base)
    {
      Glom::DbTreeModel* cpp_model = dynamic_cast<Glom::DbTreeModel*>(obj_base);
      g_assert(cpp_model);
      cpp_model->get_value_vfunc(Gtk::TreeModel::iterator(model, iter), column, *reinterpret_cast<Glib::ValueBase*>(value));
    }
  }

  gboolean DbTreeModel::glom_iter_next_impl(GtkTreeModel* model, GtkTreeIter* iter)
  {
    Glib::ObjectBase* const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)model));
    if(obj_base)
    {
      Glom::DbTreeModel* cpp_model = dynamic_cast<Glom::DbTreeModel*>(obj_base);
      g_assert(cpp_model);

      TreeModel::iterator iter_input = TreeModel::iterator(model, iter);
      TreeModel::iterator iter_next(model, iter);
      const gboolean retval = cpp_model->iter_next_vfunc(iter_input, iter_next);

      if(retval)
        *iter = *(iter_next.gobj());
      
      return retval;
    }
    else
      return FALSE;
  }

  gboolean DbTreeModel::glom_iter_children_impl(GtkTreeModel* model, GtkTreeIter* iter, GtkTreeIter* parent)
  {
    Glib::ObjectBase* const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)model));
    if(obj_base)
    {
      Glom::DbTreeModel* cpp_model = dynamic_cast<Glom::DbTreeModel*>(obj_base);
      g_assert(cpp_model);

      TreeModel::iterator iter_out(model, iter);
      const gboolean retval = cpp_model->iter_children_vfunc(TreeModel::iterator(model, parent), iter_out);

      if(retval)
        *iter = *(iter_out.gobj());
      
      return retval;
    }
    else
      return FALSE;
  }

  gboolean DbTreeModel::glom_iter_has_child_impl(GtkTreeModel* model, GtkTreeIter* iter)
  {
    Glib::ObjectBase* const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)model));
    if(obj_base)
    {
      Glom::DbTreeModel* cpp_model = dynamic_cast<Glom::DbTreeModel*>(obj_base);
      g_assert(cpp_model);
      return cpp_model->iter_has_child_vfunc(TreeModel::iterator(model, iter));
    }
    else
      return FALSE;
  }

  gint DbTreeModel::glom_iter_n_children_impl(GtkTreeModel* model, GtkTreeIter* iter)
  {
    Glib::ObjectBase* const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)model));
    if(obj_base)
    {
      Glom::DbTreeModel* cpp_model = dynamic_cast<Glom::DbTreeModel*>(obj_base);
      g_assert(cpp_model);
      return cpp_model->iter_n_children_vfunc(TreeModel::iterator(model, iter));
    }
    else
      return 0;
  }

  gboolean DbTreeModel::glom_iter_nth_child_impl(GtkTreeModel* model, GtkTreeIter* iter, GtkTreeIter* parent, gint n)
  {
    Glib::ObjectBase* const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)model));
    if(obj_base)
    {
      Glom::DbTreeModel* cpp_model = dynamic_cast<Glom::DbTreeModel*>(obj_base);
      g_assert(cpp_model);

      TreeModel::iterator iter_out(model, iter);
      gboolean retval = false;

      // Deal with this special case, documented in the C docs as:
      // "As a special case, if @parent is %NULL, then the nth root node is set.":
      if(!parent)
        retval = cpp_model->iter_nth_root_child_vfunc(n, iter_out);
      else
        retval = cpp_model->iter_nth_child_vfunc(TreeModel::iterator(model, parent), n, iter_out);

      if(retval)
        *iter = *(iter_out.gobj());

      return retval;
    }
    else
      return FALSE;
  }

  gboolean DbTreeModel::glom_iter_parent_impl(GtkTreeModel* model, GtkTreeIter* iter, GtkTreeIter* child)
  {
    Glib::ObjectBase* const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)model));
    if(obj_base)
    {
      Glom::DbTreeModel* cpp_model = dynamic_cast<Glom::DbTreeModel*>(obj_base);
      g_assert(cpp_model);

      TreeModel::iterator iter_out(model, iter);
      const gboolean retval = cpp_model->iter_parent_vfunc(TreeModel::iterator(model, child), iter_out);

      if(retval)
        *iter = *(iter_out.gobj());

      return retval;
    }
    else
      return FALSE;
  }

  GtkTreePath* DbTreeModel::glom_get_path_impl(GtkTreeModel* model, GtkTreeIter* iter)
  {
    Glib::ObjectBase* const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)model));
    if(obj_base)
    {
      Glom::DbTreeModel* cpp_model = dynamic_cast<Glom::DbTreeModel*>(obj_base);
      g_assert(cpp_model);
      return cpp_model->get_path_vfunc(TreeModel::iterator(model, iter)).gobj_copy();
    }
    else
      return NULL;
  }

  gboolean DbTreeModel::glom_get_iter_impl(GtkTreeModel* model, GtkTreeIter* iter, GtkTreePath* path)
  {
    Glib::ObjectBase* const obj_base = static_cast<Glib::ObjectBase*>(Glib::ObjectBase::_get_current_wrapper((GObject*)model));
    if(obj_base)
    {
      Glom::DbTreeModel* cpp_model = dynamic_cast<Glom::DbTreeModel*>(obj_base);
      g_assert(cpp_model);

      TreeModel::iterator iter_out(model, iter);

      const gboolean retval = cpp_model->get_iter_vfunc(Gtk::TreeModel::Path(path, true), iter_out);

      if(retval)
        *iter = *(iter_out.gobj());

      return retval;
    }
    else
      return FALSE;
  }
#endif // !GLIBMM_VFUNCS_ENABLED
} // namespace Glom

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
  //std::cout << "DbTreeModelRow::fill_values_if_necessary(): row=" << row << std::endl;
  //if(row == 1000)
  //{
  //  std::cout << "1000" << std::endl;  
  //}

  if(m_values_retrieved)
  {
     //std::cout << "debug: DbTreeModelRow::fill_values_if_necessary(): already retrieved" << std::endl;
  }
  else
  {
    //std::cout << "debug: DbTreeModelRow::fill_values_if_necessary(): retrieving for row=" << row << std::endl;
  
    if((row < (int)model.m_data_model_rows_count) && model.m_gda_datamodel)
    {
      Glib::RefPtr<Gnome::Gda::DataModelIter> iter = model.m_gda_datamodel->create_iter();
      if(iter)
      {
        iter->move_to_row(row);

        //It is a row from the database;
        const int cols_count = model.m_data_model_columns_count;
        for(int i = 0; i < cols_count; ++i)
        {
          Glib::RefPtr<const Gnome::Gda::Holder> holder = iter->get_holder_for_field(i);
          if(holder)
            m_db_values[i] = holder->get_value(); //TODO_gda: Why not just use get_value_at()?
          else
          {
            // This is quite possible, for example for unset dates. jhs 
            //std::cerr << "DbTreeModelRow::fill_values_if_necessary(): NULL Gnome::Gda::Holder for field=" << i << std::endl;
          }

          //std::cout << "  debug: col=" << i << ", GType=" << m_db_values[i].get_value_type() << ", string=" << m_db_values[i].to_string() << std::endl;
        }

        Glib::RefPtr<const Gnome::Gda::Holder> holder = iter->get_holder_for_field(model.m_column_index_key);
        if(holder)
          m_key = holder->get_value();  //TODO_gda: Why not just use get_value_at()?
        else
          std::cerr << "DbTreeModelRow::fill_values_if_necessary(): NULL Gnome::Gda::Holder for key field" << std::endl;

        m_extra = false;
        m_removed = false;
      }
    }
    else
    {
      //g_warning("DbTreeModelRow::fill_values_if_necessary(): Non-db row.");
      if(m_extra)
      {
        //std::cout << "  debug: DbTreeModelRow::fill_values_if_necessary(): using default value" << std::endl;
  
        //It is an extra row, added with append().
      }
      else if(!m_removed)
      {
        //It must be the last blank placeholder row.
        //m_placeholder = true;
      }

      //Create default values, if necessary, of the correct types:
      //Examine the columns in the returned DataModel:
      for(guint col = 0; col < model.m_data_model_columns_count; ++col)
      {
        if(m_db_values.find(col) == m_db_values.end()) //If there is not already a value in the map for this column.
        {
          Glib::RefPtr<Gnome::Gda::Column> column = model.m_gda_datamodel->describe_column(col);

          //We don't just create a Gda::Value of the column's gda type, 
          //because we should use a NULL-type Gda::Value as the initial value for some fields:
          const Field::glom_field_type glom_type = Field::get_glom_type_for_gda_type(column->get_g_type());
          m_db_values[col] = Glom::Conversions::get_empty_value(glom_type);
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
  const GType debug_type_expected = gdacolumn->get_g_type();
  if(debug_type_in != debug_type_expected)
  {
    std::cout << "debug: DbTreeModelRow::set_value(): expected GType=" << debug_type_expected << ", but received GType=" << debug_type_in << std::endl;
    if(debug_type_expected)
      std::cout << "  expected GType name=\"" << g_type_name(debug_type_expected) << "\"" << std::endl;

    if(debug_type_in)
      std::cout << "  received GType name=\"" << g_type_name(debug_type_in) << "\"" << std::endl;
  }
  */

  m_db_values[column] = value;
}


DbTreeModelRow::DbValue DbTreeModelRow::get_value(DbTreeModel& model, int column, int row)
{
  fill_values_if_necessary(model, row);

  type_vec_values::const_iterator iterFind = m_db_values.find(column);
  if(iterFind != m_db_values.end())
    return iterFind->second;
  else
  {
    std::cout << "debug: DbTreeModelRow::get_value(): column not found." << std::endl;
    return DbValue();
  }
}

//Intialize static variable:
bool DbTreeModel::m_iface_initialized = false;

DbTreeModel::DbTreeModel(const Gtk::TreeModelColumnRecord& columns, const FoundSet& found_set, const type_vec_fields& column_fields, int column_index_key, bool get_records, bool find_mode)
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
  m_find_mode(find_mode),
  m_stamp(1) //When the model's stamp != the iterator's stamp then that iterator is invalid and should be ignored. Also, 0=invalid
{
  if(!m_iface_initialized)
  {
    //GType gtype = G_OBJECT_TYPE(gobj());  //The custom GType created in the Object constructor, from the typeid.
    //Gtk::TreeModel::add_interface( gtype );

#ifndef GLIBMM_VFUNCS_ENABLED
    GtkTreeModelIface* iface = GTK_TREE_MODEL_GET_IFACE(gobj());
    iface->get_flags = glom_get_flags_impl;
    iface->get_n_columns = glom_get_n_columns_impl;
    iface->get_column_type = glom_get_column_type_impl;
    iface->get_value = glom_get_value_impl;
    iface->iter_next = glom_iter_next_impl;
    iface->iter_children = glom_iter_children_impl;
    iface->iter_has_child = glom_iter_has_child_impl;
    iface->iter_n_children = glom_iter_n_children_impl;
    iface->iter_nth_child = glom_iter_nth_child_impl;
    iface->iter_parent = glom_iter_parent_impl;
    iface->get_path = glom_get_path_impl;
    iface->get_iter = glom_get_iter_impl;
#endif // !GLIBMM_VFUNCS_ENABLED

    m_iface_initialized = true; //Prevent us from calling add_interface() on the same gtype again.
  }


  //The Column information that can be used with TreeView::append(), TreeModel::iterator[], etc.
  m_columns_count = columns.size(); //1 extra for the key.

  g_assert(m_columns_count == column_fields.size());

  refresh_from_database(m_found_set);
}

DbTreeModel::~DbTreeModel()
{
  clear();
}

//static:
Glib::RefPtr<DbTreeModel> DbTreeModel::create(const Gtk::TreeModelColumnRecord& columns, const FoundSet& found_set, const type_vec_fields& column_fields, int column_index_key, bool get_records, bool find_mode)
{
  return Glib::RefPtr<DbTreeModel>( new DbTreeModel(columns, found_set, column_fields, column_index_key, get_records, find_mode) );
}

bool DbTreeModel::refresh_from_database(const FoundSet& found_set)
{
  //std::cout << "DbTreeModel::refresh_from_database()" << std::endl;
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
    Glib::RefPtr<Gnome::Gda::DataModelArray> model_array = Gnome::Gda::DataModelArray::create(m_column_fields.size());
    m_gda_datamodel = model_array;

    int col = 0;
    for(type_vec_fields::const_iterator iter = m_column_fields.begin(); iter != m_column_fields.end(); ++iter)
    {
      sharedptr<const LayoutItem_Field> layout_item = *iter;
      if(layout_item)
      {
        const Field::glom_field_type glom_type = layout_item->get_glom_type();
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
  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
     m_connection = connection_pool->connect();
#else
     std::auto_ptr<ExceptionConnection> error;
     m_connection = connection_pool->connect(error);
     // Ignore error. The connection presence is checked below
#endif // GLIBMM_EXCEPTIONS_ENABLED
  }

  if(m_found_set.m_table_name.empty())
    std::cerr << "DEBUG: refresh_from_database(): found_set.m_table_name is empty." << std::endl;
    
  if(m_connection && !m_found_set.m_table_name.empty() && m_get_records)
  {
    const Glib::ustring sql_query = Utils::build_sql_select_with_where_clause(m_found_set.m_table_name, m_column_fields, m_found_set.m_where_clause, m_found_set.m_extra_join, m_found_set.m_sort_clause, m_found_set.m_extra_group_by);
    //std::cout << "  Debug: DbTreeModel::refresh_from_database():  " << sql_query << std::endl;

    const App_Glom* app = App_Glom::get_application();
    if(app && app->get_show_sql_debug())
    {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
      try
      {
#endif //GLIBMM_EXCEPTIONS_ENABLED
        std::cout << "Debug: DbTreeModel::refresh_from_database():  " << sql_query << std::endl;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
      }
      catch(const Glib::Exception& ex)
      {
        std::cout << "Debug: query string could not be converted to std::cout: " << ex.what() << std::endl;
      }
#endif //GLIBMM_EXCEPTIONS_ENABLED
    }

    Glib::RefPtr<Gnome::Gda::SqlParser> parser = m_connection->get_gda_connection()->create_parser();
    Glib::RefPtr<Gnome::Gda::Statement> stmt = parser->parse_string(sql_query);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    try
    {
      //Specify the STATEMENT_MODEL_CURSOR, so that libgda only gets the rows that we actually use.
      m_gda_datamodel = m_connection->get_gda_connection()->statement_execute_select(stmt, Gnome::Gda::STATEMENT_MODEL_CURSOR_FORWARD);

      // Use a DataAccessWrapper to allow random access. This is necessary
      // since we use move_to_row() on a created iterator in
      // fill_values_if_necessary(), which does not work if the iterator
      // does not support it (for example the one for Sqlite recordsets does
      // not). The alternative would be to acquire a random-access model
      // directly here for SQLite, but this would
      // a) make this code dependent on the database backend used.
      // b) fetch rows we perhaps don't need, if only the first few rows of
      // a table are accessed.
      // TODO_Performance: The unnecessary (for PostgreSQL) extra indirection might theoretically make this slower.
      m_gda_datamodel = Gnome::Gda::DataAccessWrapper::create(m_gda_datamodel);

      if(app && app->get_show_sql_debug())
        std::cout << "  Debug: DbTreeModel::refresh_from_database(): The query execution has finished." << std::endl;

      //Examine the columns in the returned DataModel:
      /*
      for(int col = 0; col < m_gda_datamodel->get_n_columns(); ++col)
      {
        Glib::RefPtr<Gnome::Gda::Column> column = m_gda_datamodel->describe_column(col);
        std::cout << "  debug: column index=" << col << ", name=" << column->get_name() << ", type=" << g_type_name(column->get_g_type()) << std::endl;
      }
      */
    }
    catch(const Glib::Exception& ex)
    {
      std::cerr << "DbTreeModel::refresh_from_database(): Glib::Exception caught." << std::endl;
      m_gda_datamodel.clear(); //So that it is 0, so we can handle it below.
    }
    catch(const std::exception& ex)
    {
      std::cerr << "DbTreeModel::refresh_from_database(): std::exception caught." << std::endl;
      m_gda_datamodel.clear(); //So that it is 0, so we can handle it below.
    }
#else
    std::auto_ptr<Glib::Error> error;
    m_gda_datamodel = m_connection->get_gda_connection()->statement_execute_select(stmt, set, error);
    if(error.get())
    {
      m_gda_datamodel.clear(); //So that it is 0, so we can handle it below.
    }
#endif //GLIBMM_EXCEPTIONS_ENABLED

    if(!m_gda_datamodel)
    {
      m_data_model_rows_count = 0;
      m_data_model_columns_count = m_columns_count;

      std::cerr << "DbTreeModel::refresh_from_database(): error executing SQL: " << sql_query << std::endl;
      ConnectionPool::handle_error();
      return false; //No records were found.
    }
    else
    {
      //This doesn't work with cursor-based models: const int count = m_gda_datamodel->get_n_rows();
      //because rows count is -1 until we have iterated to the last row.
      const Glib::ustring sql_query_without_sort = Utils::build_sql_select_with_where_clause(m_found_set.m_table_name, m_column_fields, m_found_set.m_where_clause, m_found_set.m_extra_join, type_sort_clause(), m_found_set.m_extra_group_by);
      const int count = Base_DB::count_rows_returned_by(sql_query_without_sort);
      if(count < 0)
      {
        std::cerr << "DbTreeModel::refresh_from_database(): count is < 0" << std::endl;
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
  //std::cout << "debug: DbTreeModel::get_value_vfunc(): column=" << column << std::endl;

  if(check_treeiter_validity(iter))
  {
    //std::cout << "  debug: DbTreeModel::get_value_vfunc() 1" << std::endl;

    if(column < (int)m_columns_count)
    {
       //std::cout << "  debug: DbTreeModel::get_value_vfunc() 1.1" << std::endl;

      //Get the correct ValueType from the Gtk::TreeModel::Column's type, so we don't have to repeat it here:
      //(This would be a custom boxed type for our Gda::Value (stored inside the TreeModel's Glib::Value just as an int or char* would be stored in it.)
      //std::cout << "  debug: DbTreeModel::get_value_vfunc(): column=" << column << ", value type=" << g_type_name(typeModelColumn::ValueType::value_type()) << std::endl;

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
         //std::cout << "  debug: DbTreeModel::get_value_vfunc() 1.2" << std::endl;

        //const typeRow& dataRow = *datamodel_row;

        //g_warning("DbTreeModel::get_value_vfunc 1: column=%d, row=%d", column, datamodel_row);

        DbValue result;

        DbTreeModelRow& row_details = m_map_rows[datamodel_row]; //Adds it if necessary.
        const int column_sql = column;
        if(column_sql < (int)m_columns_count) //TODO_Performance: Remove the checks.
        {
           //std::cout << "  debug: DbTreeModel::get_value_vfunc() 1.3" << std::endl;

          if( !(datamodel_row < (internal_rows_count - 1)))
          {
              //std::cout << "  debug: DbTreeModel::get_value_vfunc() 1.4" << std::endl;

             //std::cout << "DbTreeModel::get_value_vfunc: row " << datamodel_row << " is placeholder" << std::endl;

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
              Glib::RefPtr<Gnome::Gda::Column> column = m_gda_datamodel->describe_column(col);
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
              std::cout << "  debug: DbTreeModel::get_value_vfunc(): column_sql=" << column_sql << ", describe_column() returned GType: " << gtype_expected << " but get_value() returned GType: " << result.get_value_type() << std::endl;
            }
            */
          }
        }
        else
          g_warning("DbTreeModel::get_value_vfunc: column out of bounds: sql_col=%d, max=%d.", column_sql, m_columns_count);


        /*
        GType debug_type = result.get_value_type();
        std::cout << "  debug: DbTreeModel::get_value_vfunc(): result value type: GType=" << debug_type << std::endl;
        if(debug_type)
          std::cout << "    GType name=\"" << g_type_name(debug_type) << "\"" << std::endl; 
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
    //std::cout << "DbTreeModel::iter_next_vfunc():" << datamodel_row << std::endl;

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
  if(!check_treeiter_validity(iter))
    return false;

  return Gtk::TreeModel::iter_is_valid(iter);
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
  type_datamodel_row_index datamodel_row = get_internal_rows_count();
  //g_warning("DbTreeModel::append(): new row number=%d", datamodel_row);
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
  const bool iter_is_valid = create_iterator(datamodel_row, iter);
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
    type_datamodel_row_index datamodel_row = get_datamodel_row_index_from_tree_row_iter(row);

    //TODO: Check column against get_n_columns() too, though it could hurt performance.


    //Cast it to the specific value type.
    //It can't work with anything else anyway.
    typedef Glib::Value< DbValue > ValueDbValue;

    const ValueDbValue* pDbValue = static_cast<const ValueDbValue*>(&value);
    if(!pDbValue)
      g_warning("DbTreeModel::set_value_impl(): value is not a Value< DbValue >.");
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
    type_map_rows::iterator iterFind = m_map_rows.find(datamodel_row);
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
  type_map_rows::const_iterator iterFind = m_map_rows.find(datamodel_row);
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
    type_datamodel_row_index row = rows_count - 1;
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
    type_datamodel_row_index row = rows_count - 1;

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
    //This doesn't work with iter-only models (it returns -1): found = (gulong)m_gda_datamodel->get_n_rows();
    found = m_data_model_rows_count;

    if(m_found_set.m_where_clause.empty())
      total = found;
    else
    {
      //Ask the database how many records there are in the whole table:
      //TODO: Apparently, this is very slow:
      const Glib::ustring sql_query = "SELECT count(*) FROM \"" + m_found_set.m_table_name + "\"";
      Glib::RefPtr<Gnome::Gda::SqlParser> parser = m_connection->get_gda_connection()->create_parser();
      Glib::RefPtr<Gnome::Gda::Statement> stmt = parser->parse_string(sql_query);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
      Glib::RefPtr<Gnome::Gda::DataModel> datamodel = m_connection->get_gda_connection()->statement_execute_select(stmt);
#else
      std::auto_ptr<Glib::Error> error;
      Glib::RefPtr<Gnome::Gda::DataModel> datamodel = m_connection->get_gda_connection()->statement_execute_select(stmt, error);
      // Ignore error, datamodel presence is checked below
#endif //GLIBMM_EXCEPTIONS_ENABLED

      if(datamodel)
      {
        if(datamodel->get_n_rows())
        {
          Gnome::Gda::Value value = datamodel->get_value_at(0, 0);
          total = (gulong)value.get_int64(); //I discovered that it's a int64 by trying it.
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
