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

#include "comboglomchoicesbase.h"
#include "../data_structure/glomconversions.h"
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl


ComboGlomChoicesBase::ComboGlomChoicesBase()
: m_with_second(false)
{
  init();
}

ComboGlomChoicesBase::ComboGlomChoicesBase(const LayoutItem_Field& field_second)
: m_with_second(true),
  m_layoutitem_second(field_second)
{
  init();
}

void ComboGlomChoicesBase::init()
{
  m_refModel = Gtk::ListStore::create(m_Columns);
}

ComboGlomChoicesBase::~ComboGlomChoicesBase()
{
}

ComboGlomChoicesBase::type_signal_edited ComboGlomChoicesBase::signal_edited()
{
  return m_signal_edited;
}

void ComboGlomChoicesBase::set_choices_with_second(const type_list_values_with_second& list_values)
{
  m_refModel->clear();

  for(type_list_values_with_second::const_iterator iter = list_values.begin(); iter != list_values.end(); ++iter)
  {
    Gtk::TreeModel::iterator iterTree = m_refModel->append();
    Gtk::TreeModel::Row row = *iterTree;

    LayoutItem_Field* layout_item = dynamic_cast<LayoutItem_Field*>(get_layout_item());
    if(layout_item)
    {
      row[m_Columns.m_col_first] = GlomConversions::get_text_for_gda_value(layout_item->m_field.get_glom_type(), iter->first, layout_item->m_numeric_format);;

      if(m_with_second)
        row[m_Columns.m_col_second] = GlomConversions::get_text_for_gda_value(m_layoutitem_second.m_field.get_glom_type(), iter->second, layout_item->m_numeric_format);;
    }
  }
}


void ComboGlomChoicesBase::set_choices(const LayoutItem_Field::type_list_values& list_values)
{
  m_refModel->clear();

  for(LayoutItem_Field::type_list_values::const_iterator iter = list_values.begin(); iter != list_values.end(); ++iter)
  {
    Gtk::TreeModel::iterator iterTree = m_refModel->append();
    Gtk::TreeModel::Row row = *iterTree;

    LayoutItem_Field* layout_item = dynamic_cast<LayoutItem_Field*>(get_layout_item());
    if(layout_item)
    {
      const Gnome::Gda::Value value = *iter;
      const Glib::ustring text = GlomConversions::get_text_for_gda_value(layout_item->m_field.get_glom_type(), value, layout_item->m_numeric_format);

      row[m_Columns.m_col_first] = text;
    }
  }
}

