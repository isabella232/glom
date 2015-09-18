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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "combobox_locale.h"
#include <glom/mode_design/iso_codes.h>

#include <iostream>

namespace Glom
{

ComboBox_Locale::ComboBox_Locale(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& /* builder */)
: Gtk::ComboBox(cobject)
{
  m_model = Gtk::ListStore::create(m_model_columns);

  //Fill the model:
  for(const auto& the_locale : IsoCodes::get_list_of_locales())
  {
    auto tree_iter = m_model->append();
    Gtk::TreeModel::Row row = *tree_iter;

    row[m_model_columns.m_identifier] = the_locale.m_identifier;
    row[m_model_columns.m_name] = the_locale.m_name;
  }

  m_model->set_sort_column(m_model_columns.m_name, Gtk::SORT_ASCENDING);

  set_model(m_model);

  //Do not show the non-human-readable ID: pack_start(m_model_columns.m_identifier);

  //Show this too.
  //Create the cell renderer manually, so we can specify the alignment:
  auto cell = Gtk::manage(new Gtk::CellRendererText());
  cell->property_xalign() = 0.0f;
  pack_start(*cell);
  add_attribute(cell->property_text(), m_model_columns.m_name);
}


ComboBox_Locale::~ComboBox_Locale()
{

}

Glib::ustring ComboBox_Locale::get_selected_locale() const
{
  auto iter = get_active();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    return row[m_model_columns.m_identifier];
  }
  else
    return Glib::ustring();
}

void ComboBox_Locale::set_selected_locale(const Glib::ustring& locale)
{
  //Look for the row with this text, and activate it:
  Glib::RefPtr<Gtk::TreeModel> model = get_model();
  if(model)
  {
    for(const auto& row : model->children())
    {
      const Glib::ustring& this_text = row[m_model_columns.m_identifier];
      //std::cout << G_STRFUNC << ": DEBUG: locale=" << locale << ", this_text=" << this_text << "." << std::endl;
 
      if(this_text == locale)
      {
        set_active(row);
        return; //success
      }
    }

    //Not found, so mark it as blank:
    std::cerr << G_STRFUNC << ": locale not found in list: " << locale << ", list size=" << model->children().size() << std::endl;
  }
  else
  {
    std::cerr << G_STRFUNC << ": locale not found in list: " << locale << ". The model is empty." << std::endl;
  }

  unset_active();
}

} //namespace Glom

