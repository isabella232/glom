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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include <libglom/python_embed/py_glom_ui.h>
#include <libglom/python_embed/pygdavalue_conversions.h>


namespace Glom
{

PyGlomUI::PyGlomUI()
: m_callbacks(nullptr)
{
}

PyGlomUI::PyGlomUI(const PythonUICallbacks& callbacks)
: m_callbacks(&callbacks)
{
}

PyGlomUI::~PyGlomUI()
{
}

void PyGlomUI::show_table_details(const std::string& table_name, const boost::python::object& primary_key_value)
{
  if(!m_callbacks || !(m_callbacks->m_slot_show_table_details))
    return;

  Gnome::Gda::Value gda_primary_key_value;

  GValue value = {0, {{0}}};
  const auto test = glom_pygda_value_from_pyobject(&value, primary_key_value);
  if(test && G_IS_VALUE(&value))
    gda_primary_key_value = Gnome::Gda::Value(&value);

  m_callbacks->m_slot_show_table_details(table_name, gda_primary_key_value);
}

void PyGlomUI::show_table_list(const std::string& table_name)
{
  if(m_callbacks && m_callbacks->m_slot_show_table_list)
    m_callbacks->m_slot_show_table_list(table_name);
}

void PyGlomUI::print_layout()
{
  if(m_callbacks && m_callbacks->m_slot_print_layout)
    m_callbacks->m_slot_print_layout();
}


void PyGlomUI::print_report(const std::string& report_name)
{
  if(m_callbacks && m_callbacks->m_slot_print_report)
    m_callbacks->m_slot_print_report(report_name);
}

void PyGlomUI::start_new_record()
{
  if(m_callbacks && m_callbacks->m_slot_print_report)
    m_callbacks->m_slot_start_new_record();
}


} //namespace Glom
