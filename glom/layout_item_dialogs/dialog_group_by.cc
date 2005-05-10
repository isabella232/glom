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

#include "dialog_group_by.h"
#include "../data_structure/glomconversions.h"
#include <glibmm/i18n.h>

Dialog_GroupBy::Dialog_GroupBy(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_label_group_by(0),
  m_label_sort_by(0),
  m_button_field_group_by(0),
  m_button_field_sort_by(0)
{
  refGlade->get_widget("label_group_by", m_label_group_by);
  refGlade->get_widget("label_sort_by", m_label_sort_by);

  refGlade->get_widget("button_select_group_by", m_button_field_group_by);
  refGlade->get_widget("button_select_sort_by", m_button_field_sort_by);

  //Connect signals:
  m_button_field_group_by->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_GroupBy::on_button_field_group_by));
  m_button_field_sort_by->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_GroupBy::on_button_field_sort_by));

  show_all_children();
}

Dialog_GroupBy::~Dialog_GroupBy()
{
}

void Dialog_GroupBy::set_item(const LayoutItem_GroupBy& item, const Glib::ustring& table_name)
{
  m_layout_item = item;
  m_table_name = table_name;

  m_label_group_by->set_text( item.get_field_group_by()->get_layout_display_name() );
  m_label_sort_by->set_text( item.get_field_sort_by()->get_layout_display_name() );
}

bool Dialog_GroupBy::get_item(LayoutItem_GroupBy& item) const
{
  item = m_layout_item;

  return true;
}

void Dialog_GroupBy::on_button_field_group_by()
{
  LayoutItem_Field field;
  bool test = offer_field_list(field, m_table_name);
  if(test)
  {
    m_layout_item.set_field_group_by(field);
    set_item(m_layout_item, m_table_name); //Update the UI.
  }
}

void Dialog_GroupBy::on_button_field_sort_by()
{
  LayoutItem_Field field;
  bool test = offer_field_list(field, m_table_name);
  if(test)
  {
    m_layout_item.set_field_sort_by(field);
    set_item(m_layout_item, m_table_name); //Update the UI.
  }
}
