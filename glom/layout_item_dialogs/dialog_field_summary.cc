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

#include "dialog_field_summary.h"
#include "../data_structure/glomconversions.h"
#include <glibmm/i18n.h>

Dialog_FieldSummary::Dialog_FieldSummary(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_label_field(0),
  m_combo_summarytype(0),
  m_button_field(0)
{
  refGlade->get_widget("label_field", m_label_field);
  refGlade->get_widget_derived("combobox_summarytype", m_combo_summarytype);

  refGlade->get_widget("button_field", m_button_field);

  //Connect signals:
  m_button_field->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_FieldSummary::on_button_field));

  show_all_children();
}

Dialog_FieldSummary::~Dialog_FieldSummary()
{
}

void Dialog_FieldSummary::set_item(const LayoutItem_FieldSummary& item, const Glib::ustring& table_name)
{
  m_layout_item = item;
  m_table_name = table_name;

  m_label_field->set_text( item.get_layout_display_name() );
  m_combo_summarytype->set_summary_type( item.get_summary_type() );
}

bool Dialog_FieldSummary::get_item(LayoutItem_FieldSummary& item) const
{
  item = m_layout_item;
  item.set_summary_type( m_combo_summarytype->get_summary_type() );

  return true;
}

void Dialog_FieldSummary::on_button_field()
{
  LayoutItem_Field field;
  bool test = offer_field_list(field, m_table_name);
  if(test)
  {
    m_layout_item.set_field(field);
    set_item(m_layout_item, m_table_name); //Update the UI.
  }
}
