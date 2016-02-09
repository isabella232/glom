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

#include "combo_summarytype.h"
#include <glibmm/i18n.h>

#include <iostream>

namespace Glom
{

Combo_SummaryType::Combo_SummaryType(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& /* builder */)
: Gtk::ComboBox(cobject)
{
  m_model = Gtk::ListStore::create(m_model_columns);

  //Fill the model:

  auto iter = m_model->append();
  (*iter)[m_model_columns.m_summary_type] = LayoutItem_FieldSummary::summaryType::SUM;
  (*iter)[m_model_columns.m_name] = LayoutItem_FieldSummary::get_summary_type_name(LayoutItem_FieldSummary::summaryType::SUM);
  iter = m_model->append();
  (*iter)[m_model_columns.m_summary_type] = LayoutItem_FieldSummary::summaryType::AVERAGE;
  (*iter)[m_model_columns.m_name] = LayoutItem_FieldSummary::get_summary_type_name(LayoutItem_FieldSummary::summaryType::AVERAGE);
  iter = m_model->append();
  (*iter)[m_model_columns.m_summary_type] = LayoutItem_FieldSummary::summaryType::COUNT;
  (*iter)[m_model_columns.m_name] = LayoutItem_FieldSummary::get_summary_type_name(LayoutItem_FieldSummary::summaryType::COUNT);

  set_model(m_model);

  //Show this column.
  pack_start(m_model_columns.m_name);
}

void Combo_SummaryType::set_summary_type(LayoutItem_FieldSummary::summaryType summary_type)
{
  for(const auto& row : m_model->children())
  {
    const LayoutItem_FieldSummary::summaryType this_value = row[m_model_columns.m_summary_type];

    if(this_value == summary_type)
    {
      set_active(row);
      return; //success
    }
  }

  std::cerr << G_STRFUNC << ": no item found\n";

  //Not found, so mark it as blank:
  unset_active();
}

LayoutItem_FieldSummary::summaryType Combo_SummaryType::get_summary_type() const
{
  //Get the active row:
  auto active_row = get_active();
  if(active_row)
  {
    Gtk::TreeModel::Row row = *active_row;
    return row[m_model_columns.m_summary_type];
  }

  return LayoutItem_FieldSummary::summaryType::INVALID;
}

} //namespace Glom




