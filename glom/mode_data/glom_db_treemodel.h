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
#include <glom/data_structure/field.h>

class DbTreeModel
  : public Glib::Object,
    public Gtk::TreeModel
{
public:
  typedef unsigned int size_type;
  
protected:
  //Create a TreeModel with @a columns_count number of columns, each of type Glib::ustring.
  DbTreeModel(size_type columns_count);
  virtual ~DbTreeModel();
  
public:
  static Glib::RefPtr<DbTreeModel> create(size_type columns_count);
  
  typedef Gnome::Gda::Value DbValue;

  Gtk::TreeModelColumn< DbValue >& get_model_column(int column);
  
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
   typedef std::vector< DbValue > typeRow; //X columns, all of type Value.
   typedef std::vector< typeRow > typeListOfRows; //Y rows.

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
     GlueItem(int row_number);
     int get_row_number() const;
     
   protected:
     int m_row_number;
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

     //This is just a list of stuff to delete later:
     typedef std::list<GlueItem*> type_listOfGlue;
     type_listOfGlue m_list;
   };

   typeListOfRows::iterator get_data_row_iter_from_tree_row_iter(const iterator& iter);
   typeListOfRows::const_iterator get_data_row_iter_from_tree_row_iter(const iterator& iter) const;
   bool check_treeiter_validity(const iterator& iter) const;
   void remember_glue_item(GlueItem* item) const;

   //The data:
   typeListOfRows m_rows;

   //Column information:
   ColumnRecord m_column_record;

   typedef Gtk::TreeModelColumn< DbValue > typeModelColumn;
   // Usually you would have different types for each column -
   // then you would want a vector of pointers to the model columns.
   typedef std::vector< typeModelColumn > typeListOfModelColumns;
   typeListOfModelColumns m_listModelColumns;

   int m_stamp; //When the model's stamp and the TreeIter's stamp are equal, the TreeIter is valid.
   mutable GlueList* m_pGlueList;

};

#endif //GLOM_MODE_DATA_DB_TREEMODEL_H

