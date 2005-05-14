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

#ifndef GLOM_MODE_DATA_DB_TREEMODEL_H
#define GLOM_MODE_DATA_DB_TREEMODEL_H

#include <gtkmm/treemodel.h>
#include <gtkmm/treepath.h>
#include "../../data_structure/layout/layoutitem_field.h"
#include "../../connectionpool.h"
#include "../../base_db.h"

class DbTreeModel;

class DbTreeModelRow
{
public:
  DbTreeModelRow();

  bool m_placeholder;

  typedef Gnome::Gda::Value DbValue;

  //Field Values. We store them here after reading them from the database,
  //so that we can change them without changing them in the database immediately.
  //This is a duplication of data but at least we are still only getting the _rows_ that will be displayed. //TODO_Performance?
  typedef std::map<int, DbValue > type_vec_values;
  type_vec_values m_db_values;

  ///Gets the values from the database if necessary.
  DbValue get_value(DbTreeModel& model, int column, int row);
  void set_value(DbTreeModel& model, int column, int row, const DbValue& value);

  void fill_values_if_necessary(DbTreeModel& model, int row);

  bool m_values_retrieved; //Whether the values have been read from the datamodel.
  //int m_data_model_row_number; //The row in the data model from which the values were read.

  DbValue m_key;

  bool m_removed; //If it should not be shown anymore.
};

class DbTreeModel
  : public Glib::Object,
    public Gtk::TreeModel
{
public:
  typedef unsigned int size_type;

  //typedef std::vector<LayoutItem_Field> type_vec_fields;
  typedef Base_DB::type_vecLayoutFields type_vec_fields;

  friend class DbTreeModelRow;

protected:
  //Create a TreeModel with @a columns_count number of columns, each of type Glib::ustring.
  DbTreeModel(const Gtk::TreeModelColumnRecord& columns, const Glib::ustring& table_name, const type_vec_fields& column_fields, int column_index_key, bool get_records = true, const Glib::ustring& where_clause = Glib::ustring());
  virtual ~DbTreeModel();

public:
  static Glib::RefPtr<DbTreeModel> create(const Gtk::TreeModelColumnRecord& columns, const Glib::ustring& table_name, const type_vec_fields& column_fields, int column_index_key, bool get_records = true, const Glib::ustring& where_clause = Glib::ustring());

  virtual bool refresh_from_database(const Glib::ustring& where_clause = Glib::ustring());

  typedef DbTreeModelRow::DbValue DbValue;

  virtual void set_is_placeholder(const TreeModel::iterator& iter, bool val);
  virtual bool get_is_placeholder(const TreeModel::iterator& iter) const;

  virtual void set_key_value(const TreeModel::iterator& iter, const DbValue& value);
  virtual DbValue get_key_value(const TreeModel::iterator& iter) const;

  //Convenience method:
  TreeModel::iterator get_last_row();

  /** Removes the given row from the list store.
   * @param iter The iterator to the row to be removed.
   * @result An iterator to the next row, or end() if there is none.
   */
  iterator erase(const iterator& iter);

  void clear();

  /** Creates a new row at the end.
   * The row will be empty - to fill in values, you need to dereference the returned iterator and use Row::operator[] or Row::set_value().
   *
   * @result An iterator to the new row.
   */
  iterator append();

protected:

   // Overrides:
   virtual Gtk::TreeModelFlags get_flags_vfunc() const;
   virtual int get_n_columns_vfunc() const;
   virtual GType get_column_type_vfunc(int index) const;
   virtual void get_value_vfunc(const TreeModel::iterator& iter, int column, Glib::ValueBase& value) const;

   bool iter_next_vfunc(const iterator& iter, iterator& iter_next) const;

   //TODO: Make sure that we make all of these const when we have made them all const in the TreeModel:
   virtual bool iter_children_vfunc(const iterator& parent, iterator& iter) const;
   virtual bool iter_has_child_vfunc(const iterator& iter) const;
   virtual int iter_n_children_vfunc(const iterator& iter) const;
   virtual int iter_n_root_children_vfunc() const;
   virtual bool iter_nth_child_vfunc(const iterator& parent, int n, iterator& iter) const;
   virtual bool iter_nth_root_child_vfunc(int n, iterator& iter) const;
   virtual bool iter_parent_vfunc(const iterator& child, iterator& iter) const;
   virtual Path get_path_vfunc(const iterator& iter) const;
   virtual bool get_iter_vfunc(const Path& path, iterator& iter) const;

   virtual bool iter_is_valid(const iterator& iter) const;

   virtual void set_value_impl(const iterator& row, int column, const Glib::ValueBase& value);

private:
   typedef DbTreeModelRow typeRow; //X columns, all of type Value.

   //We use a std::list instead of a std::vector, though it is slower to access via an index,
   //because std::list iterators are not all invalidated when we erase an element from the middle.
   //typedef std::list< typeRow > typeListOfRows; //Y rows.

   typedef unsigned int type_datamodel_iter;
   typedef const unsigned int type_datamodel_const_iter;

   bool create_iterator(const type_datamodel_iter& row_iter, DbTreeModel::iterator& iter) const;
   void invalidate_iter(iterator& iter) const;
   bool row_was_removed(const type_datamodel_iter& row_iter) const;


   //This maps the GtkTreeIters to potential paths:
   //Each GlueItem might be stored in more than one GtkTreeIter,
   //but it will be deleted only once, because it is stored
   //only once in the GlueList.
   //GtkTreeIter::user_data might contain orphaned GlueList pointers,
   //but nobody will access them because GtkTreeIter::stamp will have the
   //wrong value, marking the user_data as invalid.
   class GlueItem
   {
   public:
     GlueItem(const type_datamodel_iter& row_iter);
     //type_datamodel_iter get_row_iter() const;

     type_datamodel_iter get_datamodel_row_iter() const;

   protected:
     //type_datamodel_iter m_row_iter;
     type_datamodel_iter m_datamodel_row_iter;
   };

   //Allow the GlueList inner class to access the declaration of the GlueItem inner class.
   //SUN's Forte compiler complains about this.
   class GlueList;
   friend class GlueList; 

   class GlueList
   {
   public:
     GlueList();
     ~GlueList();

     //We must reuse GlueItems instead of having 2 that contain equal iterators,
     //because Gtk::TreeIter::iterator::operator() unfortunately does only
     //a pointer comparison, without allowing us to implement specific logic.
     GlueItem* get_existing_item(const type_datamodel_iter& row_iter);

     //This is just a list of stuff to delete later:
     typedef std::list<GlueItem*> type_listOfGlue;
     type_listOfGlue m_list;
   };


   type_datamodel_iter get_datamodel_row_iter_from_tree_row_iter(const iterator& iter) const;
   //type_datamodel_iter get_data_row_iter_from_tree_row_iter(const iterator& iter) const;
   //typeListOfRows::const_iterator get_data_row_iter_from_tree_row_iter(const iterator& iter) const;
   bool check_treeiter_validity(const iterator& iter) const;
   void remember_glue_item(GlueItem* item) const;

   //Structure:
   unsigned int m_columns_count;
   Glib::ustring m_table_name, m_where_clause;
   type_vec_fields m_column_fields;
   int m_column_index_key;

   //Data:
   sharedptr<SharedConnection> m_connection;
   Glib::RefPtr<Gnome::Gda::DataModel> m_gda_datamodel;
   guint m_data_model_rows_count; //TODO: TODO_Performance: GdaDataModel probably needs an on-demand iterator. murrayc.
   guint m_data_model_columns_count; //1 less than m_columns_count, which also has a model column for the key.

   //TODO: Performance:
   typedef std::map<type_datamodel_iter, DbTreeModelRow> type_map_rows;
   mutable type_map_rows m_map_rows; //mutable because getting fills the internal cache.
   int m_count_extra_rows; //Rows that are not from the database.
   int m_count_removed_rows; //A cache, instead of searching through the map.
   int get_internal_rows_count() const;

   //mutable typeListOfRows m_rows;

   //Column information:
   ColumnRecord m_column_record;

   typedef Gtk::TreeModelColumn< DbValue > typeModelColumn;
   bool m_get_records;

   int m_stamp; //When the model's stamp and the TreeIter's stamp are equal, the TreeIter is valid.
   mutable GlueList* m_pGlueList;

   static bool m_iface_initialized;
};

#endif //GLOM_MODE_DATA_DB_TREEMODEL_H

