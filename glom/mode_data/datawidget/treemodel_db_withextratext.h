/* Glom
 *
 * Copyright (C) 2001-2010 Murray Cumming
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

#ifndef GLOM_UTILITY_WIDGETS_DB_TREEMODEL_WITHEXTRATEXTH
#define GLOM_UTILITY_WIDGETS_DB_TREEMODEL_WITHEXTRATEXTH

#include <glom/mode_data/datawidget/treemodel_db.h>

namespace Glom
{

/** This awkward class is just a version of DbTreeModel that has an 
 * extra text column that is a text representation of the primary key,
 * for use in a GtkCombo with has_entry, which requires a text column in the model.
 */
class DbTreeModelWithExtraText
  : public DbTreeModel
{
public:
private:

  DbTreeModelWithExtraText(const FoundSet& found_set, const type_vec_const_layout_items& layout_items, bool get_records, bool find_mode, Base_DB::type_vecConstLayoutFields& fields_shown);
  virtual ~DbTreeModelWithExtraText();

public:

  /** A convenience method, creating the model from a list of LayoutItems,
   * instead of a list of LayoutItem_Fields.
   */
  static Glib::RefPtr<DbTreeModelWithExtraText> create(const FoundSet& found_set, const type_vec_layout_items& layout_items, bool get_records, bool find_mode, Base_DB::type_vecConstLayoutFields& fields_shown);

  /** A convenience method, creating the model from a list of LayoutItems,
   * instead of a list of LayoutItem_Fields.
   * Any LayoutItem_Fields should already have their full field details.
   */
  static Glib::RefPtr<DbTreeModelWithExtraText> create(const FoundSet& found_set, const type_vec_const_layout_items& layout_items, bool get_records, bool find_mode, Base_DB::type_vecConstLayoutFields& fields_shown);
  
  /** This column is a text representation of the first field column.
   */
  int get_text_column() const;

private:
  virtual int get_n_columns_vfunc() const;
  virtual GType get_column_type_vfunc(int index) const;
  virtual void get_value_vfunc(const TreeModel::iterator& iter, int column, Glib::ValueBase& value) const;
  
  
  int m_column_index_first; //The index of the first field in the TreeModel.
  std::shared_ptr<const LayoutItem_Field> m_item_first;
};

} //namespace Glom

#endif // GLOM_UTILITY_WIDGETS_DB_TREEMODEL_WITHEXTRATEXTH
