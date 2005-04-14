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

#include "dialog_field_layout.h"
//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

Dialog_FieldLayout::Dialog_FieldLayout(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_label_field_name(0),
  m_checkbutton_editable(0),
  m_frame_numeric_format(0),
  m_frame_text_format(0),
  m_checkbox_format_text_multiline(0)
{
  refGlade->get_widget("label_field_name", m_label_field_name);
  refGlade->get_widget("checkbutton_editable", m_checkbutton_editable);

  //Numeric formatting:
  refGlade->get_widget("frame_numeric_format", m_frame_numeric_format);
  refGlade->get_widget("checkbutton_format_thousands", m_checkbox_format_use_thousands);
  refGlade->get_widget("checkbutton_format_use_decimal_places", m_checkbox_format_use_decimal_places);
  refGlade->get_widget("entry_format_decimal_places", m_entry_format_decimal_places);
  refGlade->get_widget_derived("entry_currency_symbol", m_entry_currency_symbol);

  //Text formatting:
  refGlade->get_widget("frame_text_format", m_frame_text_format);
  refGlade->get_widget("checkbutton_format_text_multiline", m_checkbox_format_text_multiline);

  show_all_children();
}

Dialog_FieldLayout::~Dialog_FieldLayout()
{
}

void Dialog_FieldLayout::set_field(const LayoutItem_Field& field)
{
  m_layout_item = field;

  m_label_field_name->set_text( field.get_layout_display_name() );

  m_checkbutton_editable->set_active( field.get_editable() );

  m_checkbox_format_use_thousands->set_active( field.m_numeric_format.m_use_thousands_separator );
  m_checkbox_format_use_decimal_places->set_active( field.m_numeric_format.m_decimal_places_restricted );

  char pchText[10] = {0};
  sprintf(pchText, "%d", field.m_numeric_format.m_decimal_places);
  m_entry_format_decimal_places->set_text(Glib::ustring(pchText));

  m_entry_currency_symbol->get_entry()->set_text(field.m_numeric_format.m_currency_symbol);

  //Hide inappropriate UI:
  const Field& the_field = field.m_field;
  const bool is_numeric = (the_field.get_glom_type() == Field::TYPE_NUMERIC);
  if(is_numeric)
    m_frame_numeric_format->show();
  else
    m_frame_numeric_format->hide();

  const bool is_text = (the_field.get_glom_type() == Field::TYPE_TEXT);
  if(is_text)
    m_frame_text_format->show();
  else
    m_frame_text_format->hide();

  g_warning("glom_type=%d, name=%s", the_field.get_glom_type(), the_field.get_name().c_str());
}

bool Dialog_FieldLayout::get_field_chosen(LayoutItem_Field& field) const
{
  m_layout_item.set_editable( m_checkbutton_editable->get_active() );

  //Numeric Formatting:
  m_layout_item.m_numeric_format.m_use_thousands_separator = m_checkbox_format_use_thousands->get_active();
  m_layout_item.m_numeric_format.m_decimal_places_restricted = m_checkbox_format_use_decimal_places->get_active();

  const Glib::ustring strDecPlaces = m_entry_format_decimal_places->get_text();
  m_layout_item.m_numeric_format.m_decimal_places = atoi(strDecPlaces.c_str());

  m_layout_item.m_numeric_format.m_currency_symbol = m_entry_currency_symbol->get_entry()->get_text();


  field = m_layout_item;
  return true;
}

