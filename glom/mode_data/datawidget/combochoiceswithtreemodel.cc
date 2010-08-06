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

void ComboChoicesWithTreeModel::init()
{
  ComboChoices::init();
  m_refModel = Gtk::ListStore::create(m_Columns);
}

ComboChoicesWithTreeModel::~ComboChoicesWithTreeModel()
{
}

void ComboChoicesWithTreeModel::set_choices_with_second(const type_list_values_with_second& list_values)
{
  m_refModel->clear();

  //TODO: Remove duplication with ComboEntry:
  sharedptr<LayoutItem_Field> layout_item = 
    sharedptr<LayoutItem_Field>::cast_dynamic(get_layout_item());
  const FieldFormatting& format = layout_item->get_formatting_used();
  sharedptr<const Relationship> choice_relationship;
  sharedptr<const LayoutItem_Field> layout_choice_first, layout_choice_second;
  bool choice_show_all = false;
  format.get_choices_related(choice_relationship, layout_choice_first, layout_choice_second, choice_show_all);
  
  for(type_list_values_with_second::const_iterator iter = list_values.begin(); iter != list_values.end(); ++iter)
  {
    Gtk::TreeModel::iterator iterTree = m_refModel->append();
    Gtk::TreeModel::Row row = *iterTree;

    if(layout_choice_first)
    {
      row[m_Columns.m_col_first] = Conversions::get_text_for_gda_value(layout_choice_first->get_glom_type(), iter->first, layout_choice_first->get_formatting_used().m_numeric_format);

      if(layout_choice_second)
      {
        row[m_Columns.m_col_second] = Conversions::get_text_for_gda_value(layout_choice_second->get_glom_type(), iter->second, layout_choice_second->get_formatting_used().m_numeric_format);
      }
    }
  }
}


void ComboChoicesWithTreeModel::set_choices(const FieldFormatting::type_list_values& list_values)
{
  m_refModel->clear();

  for(FieldFormatting::type_list_values::const_iterator iter = list_values.begin(); iter != list_values.end(); ++iter)
  {
    Gtk::TreeModel::iterator iterTree = m_refModel->append();
    Gtk::TreeModel::Row row = *iterTree;

    sharedptr<const LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::cast_dynamic(get_layout_item());
    if(layout_item)
    {
      const Gnome::Gda::Value value = *iter;
      const Glib::ustring text = Conversions::get_text_for_gda_value(layout_item->get_glom_type(), value, layout_item->get_formatting_used().m_numeric_format);

      row[m_Columns.m_col_first] = text;
    }
  }
}

} //namespace DataWidetChildren
} //namespace Glom
