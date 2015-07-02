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

#include "comboentry_borderwidth.h"
#include <sstream> //For stringstream.

namespace Glom
{

ComboEntry_BorderWidth::ComboEntry_BorderWidth(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& /* builder */)
: Gtk::ComboBox(cobject)
{
  m_model = Gtk::ListStore::create(m_model_columns);

  //Fill the model:
  auto iter = m_model->append();
  (*iter)[m_model_columns.m_value] = string_for_number(0.05);
  iter = m_model->append();
  (*iter)[m_model_columns.m_value] = string_for_number(0.1);
  iter = m_model->append();
  (*iter)[m_model_columns.m_value] = string_for_number(0.2);
  iter = m_model->append();
  (*iter)[m_model_columns.m_value] = string_for_number(0.5);
  iter = m_model->append();
  (*iter)[m_model_columns.m_value] = string_for_number(1.0);

  set_model(m_model);
  set_entry_text_column(m_model_columns.m_value);
}


ComboEntry_BorderWidth::~ComboEntry_BorderWidth()
{

}

Glib::ustring ComboEntry_BorderWidth::string_for_number(double number)
{
  std::stringstream the_stream;
  the_stream.imbue(std::locale("")); //The current locale.
  the_stream << number;
  return the_stream.str();
}

} //namespace Glom
