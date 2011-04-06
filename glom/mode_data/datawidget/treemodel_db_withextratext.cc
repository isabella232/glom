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
#include "treemodel_db_withextratext.h"

#include <libglom/connectionpool.h>
#include <libglom/data_structure/glomconversions.h> //For util_build_sql
#include <libglom/utils.h>
#include <libglom/db_utils.h>

#include "glom/application.h"

namespace Glom
{

typedef Glib::Value<Glib::ustring> type_value_string;

DbTreeModelWithExtraText::DbTreeModelWithExtraText(const FoundSet& found_set, const type_vec_const_layout_items& layout_items, bool get_records, bool find_mode, Base_DB::type_vecConstLayoutFields& fields_shown)
: Glib::ObjectBase( typeid(DbTreeModel) ), //register a custom GType.
  DbTreeModel(found_set, layout_items, get_records, find_mode, fields_shown)
{
  //Remember the key field details so we can use it later to get a text representation.
  if(m_column_index_key != -1 && (guint)m_column_index_key < fields_shown.size())
    m_item_key = fields_shown[m_column_index_key];
  else
  {
    std::cerr << G_STRFUNC << ": m_column_index_key is not set, or is out of range. m_column_index_key=" << m_column_index_key << ", size=" << fields_shown.size() << std::endl;
  }
}

DbTreeModelWithExtraText::~DbTreeModelWithExtraText()
{
  clear();
}

Glib::RefPtr<DbTreeModelWithExtraText> DbTreeModelWithExtraText::create(const FoundSet& found_set, const type_vec_layout_items& layout_items, bool get_records, bool find_mode, Base_DB::type_vecConstLayoutFields& fields_shown)
{
  //Create a const version of the input, because C++ can't convert it automatically:
  type_vec_const_layout_items const_items;
  const_items.insert(const_items.end(), layout_items.begin(), layout_items.end());

  return create(found_set, const_items, get_records, find_mode, fields_shown);
}

Glib::RefPtr<DbTreeModelWithExtraText> DbTreeModelWithExtraText::create(const FoundSet& found_set, const type_vec_const_layout_items& layout_items, bool get_records, bool find_mode, Base_DB::type_vecConstLayoutFields& fields_shown)
{
  return Glib::RefPtr<DbTreeModelWithExtraText>( new DbTreeModelWithExtraText(found_set, layout_items, get_records, find_mode, fields_shown) );
}

int DbTreeModelWithExtraText::get_n_columns_vfunc() const
{
  return DbTreeModel::get_n_columns_vfunc() + 1;
}

GType DbTreeModelWithExtraText::get_column_type_vfunc(int index) const
{
  if(index == get_text_column())
    return type_value_string::value_type();
  else
    return DbTreeModel::get_column_type_vfunc(index);
}

void DbTreeModelWithExtraText::get_value_vfunc(const TreeModel::iterator& iter, int column, Glib::ValueBase& value) const
{
  //std::cout << G_STRFUNC << ": Debug: column=" << column << std::endl;
  if(column == get_text_column())
  {
    Glib::ustring text;
    
    if(m_column_index_key == -1)
    {
      std::cerr << G_STRFUNC << ": m_column_index_key is not set." << std::endl;
    }
    else
    {
      Glib::Value<Gnome::Gda::Value> value_db;
      get_value_vfunc(iter, m_column_index_key, value_db);
      const DbValue dbvalue = value_db.get();
      
      text =
        Conversions::get_text_for_gda_value(m_item_key->get_glom_type(), dbvalue, m_item_key->get_formatting_used().m_numeric_format);
      //std::cout << "debug: text=" << text << std::endl;
      //std::cout << "  debug: m_item_key name=" << m_item_key->get_name() << std::endl;
      //std::cout << "  debug: dbvalue=" << dbvalue.to_string() << std::endl;
    }
  
    type_value_string value_specific;
    value_specific.init( type_value_string::value_type() );  //TODO: Is there any way to avoid this step?
    value_specific.set(text);
    value.init( type_value_string::value_type() );  //TODO: Is there any way to avoid this step?
    value = value_specific;
  }
  else
  {
    DbTreeModel::get_value_vfunc(iter, column, value);
  }
}

int DbTreeModelWithExtraText::get_text_column() const
{
  return get_n_columns_vfunc() - 1;
}


} //namespace Glom