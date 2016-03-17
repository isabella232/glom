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

#include "dialog_field_summary.h"

namespace Glom
{

const char* Dialog_FieldSummary::glade_id("dialog_field_summary");
const bool Dialog_FieldSummary::glade_developer(true);

Dialog_FieldSummary::Dialog_FieldSummary(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_label_field(nullptr),
  m_combo_summarytype(nullptr),
  m_button_field(nullptr)
{
  builder->get_widget("label_field", m_label_field);
  builder->get_widget_derived("combobox_summarytype", m_combo_summarytype);

  builder->get_widget("button_field", m_button_field);

  //Connect signals:
  m_button_field->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_FieldSummary::on_button_field));

  show_all_children();
}

void Dialog_FieldSummary::set_item(const std::shared_ptr<const LayoutItem_FieldSummary>& item, const Glib::ustring& table_name)
{
  m_layout_item = glom_sharedptr_clone(item);
  m_table_name = table_name;

  m_label_field->set_text( item->get_layout_display_name_field() );
  m_combo_summarytype->set_summary_type( item->get_summary_type() );
}

std::shared_ptr<LayoutItem_FieldSummary> Dialog_FieldSummary::get_item() const
{
  auto result = glom_sharedptr_clone(m_layout_item);
  result->set_summary_type( m_combo_summarytype->get_summary_type() );

  return result;
}

void Dialog_FieldSummary::on_button_field()
{
  auto field = offer_field_list_select_one_field(m_layout_item, m_table_name, this);
  if(field)
  {
    m_layout_item->set_field(field);
    set_item(m_layout_item, m_table_name); //Update the UI.
  }
}

} //namespace Glom
