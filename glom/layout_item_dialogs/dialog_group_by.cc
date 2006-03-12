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
#include <sstream> //For stringstream
#include <glibmm/i18n.h>

Dialog_GroupBy::Dialog_GroupBy(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_label_group_by(0),
  m_label_sort_by(0),
  m_label_secondary_fields(0),
  m_button_field_group_by(0),
  m_button_field_sort_by(0),
  m_button_secondary_fields(0),
  m_comboboxentry_border_width(0),
  m_dialog_choose_secondary_fields(0)
{
  refGlade->get_widget("label_group_by", m_label_group_by);
  refGlade->get_widget("label_sort_by", m_label_sort_by);
  refGlade->get_widget("label_secondary_fields", m_label_secondary_fields);

  refGlade->get_widget("button_select_group_by", m_button_field_group_by);
  refGlade->get_widget("button_select_sort_by", m_button_field_sort_by);
  refGlade->get_widget("button_secondary_edit", m_button_secondary_fields);

  refGlade->get_widget_derived("comboboxentry_border_width", m_comboboxentry_border_width);

  //Connect signals:
  m_button_field_group_by->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_GroupBy::on_button_field_group_by));
  m_button_field_sort_by->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_GroupBy::on_button_field_sort_by));
  m_button_secondary_fields->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_GroupBy::on_button_secondary_fields));

  show_all_children();
}

Dialog_GroupBy::~Dialog_GroupBy()
{
  if(m_dialog_choose_secondary_fields)
  {
    remove_view(m_dialog_choose_secondary_fields);
    delete m_dialog_choose_secondary_fields;
  }
}

void Dialog_GroupBy::set_item(const sharedptr<const LayoutItem_GroupBy>& item, const Glib::ustring& table_name)
{
  m_layout_item = glom_sharedptr_clone(item);
  m_table_name = table_name;

  if(item->get_has_field_group_by())
    m_label_group_by->set_text( item->get_field_group_by()->get_layout_display_name() );
  else
    m_label_group_by->set_text( Glib::ustring() );

  if(item->get_has_field_sort_by())
    m_label_sort_by->set_text( item->get_field_sort_by()->get_layout_display_name() );
  else
    m_label_sort_by->set_text( Glib::ustring() );

  Glib::ustring border_width_as_text;
  std::stringstream the_stream;
  the_stream.imbue(std::locale("")); //Current locale.
  the_stream << m_layout_item->get_border_width();
  border_width_as_text = the_stream.str();
  std::cout << "set_item: border_width_as_text=" << border_width_as_text << std::endl;
  m_comboboxentry_border_width->get_entry()->set_text(border_width_as_text);
}

sharedptr<LayoutItem_GroupBy> Dialog_GroupBy::get_item() const
{
  std::stringstream the_stream;
  the_stream.imbue(std::locale("")); //Current locale.
  the_stream << m_comboboxentry_border_width->get_entry()->get_text();

  double border_width_as_number = 0;
  the_stream >> border_width_as_number;
   std::cout << "get_item: border_width_as_number=" << border_width_as_number << std::endl;
  m_layout_item->set_border_width(border_width_as_number);
std::cout << "get_item: set_border_width()=" << m_layout_item->get_border_width() << std::endl;

  return glom_sharedptr_clone(m_layout_item);
}

void Dialog_GroupBy::on_button_field_group_by()
{
  sharedptr<LayoutItem_Field> field = offer_field_list(m_table_name, this);
  if(field)
  {
    m_layout_item->set_field_group_by(field);
    set_item(m_layout_item, m_table_name); //Update the UI.
  }
}

void Dialog_GroupBy::on_button_field_sort_by()
{
  sharedptr<LayoutItem_Field> field = offer_field_list(m_table_name, this);
  if(field)
  {
    m_layout_item->set_field_sort_by(field);
    set_item(m_layout_item, m_table_name); //Update the UI.
  }
}

void Dialog_GroupBy::on_button_secondary_fields()
{
  if(!m_dialog_choose_secondary_fields)
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_groupby_secondary_fields");
    if(refXml)
    {
      refXml->get_widget_derived("dialog_groupby_secondary_fields", m_dialog_choose_secondary_fields);
      if(m_dialog_choose_secondary_fields)
      {
        add_view(m_dialog_choose_secondary_fields); //Give it access to the document.
        m_dialog_choose_secondary_fields->signal_hide().connect( sigc::mem_fun(*this, &Dialog_GroupBy::on_dialog_secondary_fields_hide) );
      }
    }
  }

  if(m_dialog_choose_secondary_fields)
  {
    m_dialog_choose_secondary_fields->set_fields(m_table_name, m_layout_item->m_group_secondary_fields->m_map_items);

    int response = m_dialog_choose_secondary_fields->run();
    m_dialog_choose_secondary_fields->hide();
    if(response == Gtk::RESPONSE_OK && m_dialog_choose_secondary_fields->get_modified())
    {
      m_layout_item->m_group_secondary_fields->remove_all_items(); //Free the existing member items.
      m_layout_item->m_group_secondary_fields->m_map_items = m_dialog_choose_secondary_fields->get_fields();
    }
  }
}

void Dialog_GroupBy::on_dialog_secondary_fields_hide()
{

}
