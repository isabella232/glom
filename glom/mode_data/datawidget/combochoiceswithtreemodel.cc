/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
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

#include "combochoiceswithtreemodel.h"
#include <libglom/data_structure/glomconversions.h>
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl


namespace Glom
{

namespace DataWidgetChildren
{

ComboChoicesWithTreeModel::ComboChoicesWithTreeModel()
{
  init();
}

ComboChoicesWithTreeModel::~ComboChoicesWithTreeModel()
{
  delete_model();
}

void ComboChoicesWithTreeModel::init()
{
  ComboChoices::init();
}

void ComboChoicesWithTreeModel::create_model(guint columns_count)
{
  delete_model();

  Gtk::TreeModel::ColumnRecord record;

  //Create the TreeModelColumns, adding them to the ColumnRecord:
  m_vec_model_columns.resize(columns_count, 0);
  for(guint i = 0; i < columns_count; ++i)
  {
    type_model_column* model_column = new type_model_column();

    //Store it so we can use it and delete it later:
    m_vec_model_columns[i] = model_column;

    record.add(*model_column);
  }

  //Create the model:
  m_refModel = Gtk::ListStore::create(record);
}

void ComboChoicesWithTreeModel::delete_model()
{
  //Delete the vector's items:
  for(type_vec_model_columns::iterator iter = m_vec_model_columns.begin(); iter != m_vec_model_columns.end(); ++iter)
  {
    type_model_column* model_column = *iter;
     if(model_column)
       delete model_column;
  }
  m_vec_model_columns.clear();

  m_refModel.reset();
}

void ComboChoicesWithTreeModel::set_choices_with_second(const type_list_values_with_second& list_values)
{
  //Recreate the entire model:
  guint columns_count = 1; //For the main field.
  if(!list_values.empty())
  {
    type_list_values_with_second::const_iterator iter= list_values.begin();
    if(iter != list_values.end())
    {
      const type_list_values& second = iter->second;
      columns_count += second.size();
    }
  }
  create_model(columns_count);

  //Fill the model with data:
  //TODO: Remove duplication with ComboEntry:
  sharedptr<LayoutItem_Field> layout_item =
    sharedptr<LayoutItem_Field>::cast_dynamic(get_layout_item());
  const FieldFormatting& format = layout_item->get_formatting_used();
  sharedptr<const Relationship> choice_relationship;
  sharedptr<const LayoutItem_Field> layout_choice_first;
  sharedptr<const LayoutGroup> layout_choice_extra;
  bool choice_show_all = false;
  format.get_choices_related(choice_relationship, layout_choice_first, layout_choice_extra, choice_show_all);

  LayoutGroup::type_list_const_items extra_fields;
  if(layout_choice_extra)
    extra_fields = layout_choice_extra->get_items_recursive();

  for(type_list_values_with_second::const_iterator iter = list_values.begin(); iter != list_values.end(); ++iter)
  {
    Gtk::TreeModel::iterator iterTree = m_refModel->append();
    Gtk::TreeModel::Row row = *iterTree;

    if(layout_choice_first)
    {
      const Glib::ustring text =
        Conversions::get_text_for_gda_value(layout_choice_first->get_glom_type(), iter->first, layout_choice_first->get_formatting_used().m_numeric_format);
      row.set_value(0, text);

      const type_list_values extra_values = iter->second;
      if(layout_choice_extra && !extra_values.empty())
      {
        guint model_index = 1; //0 is for the main field.
        type_list_values::const_iterator iterValues = extra_values.begin();
        for(LayoutGroup::type_list_const_items::const_iterator iterExtra = extra_fields.begin();
          iterExtra != extra_fields.end(); ++iterExtra)
        {
          if(model_index >= columns_count)
            break;

          if(iterValues == extra_values.end())
            break;

          const sharedptr<const LayoutItem> item = *iterExtra;
          const sharedptr<const LayoutItem_Field> item_field = sharedptr<const LayoutItem_Field>::cast_dynamic(item);
          if(item_field)
          {
            const Gnome::Gda::Value value = *iterValues;
            const Glib::ustring text =
              Conversions::get_text_for_gda_value(item_field->get_glom_type(), value, item_field->get_formatting_used().m_numeric_format);
            row.set_value(model_index, text);
          }

          ++model_index;
          ++iterValues;
        }
      }
    }
  }
}


void ComboChoicesWithTreeModel::set_choices(const FieldFormatting::type_list_values& list_values)
{
  create_model(1);

  for(FieldFormatting::type_list_values::const_iterator iter = list_values.begin(); iter != list_values.end(); ++iter)
  {
    Gtk::TreeModel::iterator iterTree = m_refModel->append();
    Gtk::TreeModel::Row row = *iterTree;

    sharedptr<const LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::cast_dynamic(get_layout_item());
    if(layout_item)
    {
      const Gnome::Gda::Value value = *iter;
      const Glib::ustring text = Conversions::get_text_for_gda_value(layout_item->get_glom_type(), value, layout_item->get_formatting_used().m_numeric_format);
      row.set_value(0, text);
    }
  }
}

} //namespace DataWidetChildren
} //namespace Glom
