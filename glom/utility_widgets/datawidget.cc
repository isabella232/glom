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

#include "datawidget.h"
#include "entryglom.h"
#include "../data_structure/glomconversions.h"



DataWidget::DataWidget(Field::glom_field_type glom_type, const Glib::ustring& title)
: m_glom_type(glom_type)
{
  Gtk::Widget* child = 0;
  if(glom_type == Field::TYPE_BOOLEAN)
  {
    Gtk::CheckButton* checkbutton = Gtk::manage( new Gtk::CheckButton( title ) );
    checkbutton->show();
    checkbutton->signal_toggled().connect( sigc::mem_fun(*this, &DataWidget::on_widget_edited)  );
    child = checkbutton;

    m_label.set_text( Glib::ustring() ); //It is not used.
  }
  else
  {
    m_label.set_label(title);
    m_label.set_alignment(0);
    m_label.show();

    EntryGlom* entry = Gtk::manage(new EntryGlom(glom_type));
    int width = get_suitable_width(glom_type);
    entry->set_size_request(width, -1 /* auto */);
    entry->show_all();
    entry->signal_edited().connect( sigc::mem_fun(*this, &DataWidget::on_widget_edited)  );
    child = entry;
  }

  if(child)
    add(*child);
}

DataWidget::~DataWidget()
{
}

void DataWidget::on_widget_edited()
{
  m_signal_edited.emit(get_value());
}

DataWidget::type_signal_edited DataWidget::signal_edited()
{
  return m_signal_edited;
}

void DataWidget::set_value(const Gnome::Gda::Value& value)
{
  Gtk::Widget* widget = get_child();
  EntryGlom* entry = dynamic_cast<EntryGlom*>(widget);
  if(entry)
    entry->set_value(value);
  else
  {
    Gtk::CheckButton* checkbutton = dynamic_cast<Gtk::CheckButton*>(widget);
    if(checkbutton)
    {
      bool bValue = false;
      if(!value.is_null())
        bValue = value.get_bool();
        
      checkbutton->set_active( bValue );
    }
  }
}

Gnome::Gda::Value DataWidget::get_value() const
{
  const Gtk::Widget* widget = get_child();
  const EntryGlom* entry = dynamic_cast<const EntryGlom*>(widget);
  if(entry)
    return entry->get_value();
  else
  {
    const Gtk::CheckButton* checkbutton = dynamic_cast<const Gtk::CheckButton*>(widget);
    if(checkbutton)
    {
      return Gnome::Gda::Value(checkbutton->get_active());
    }
    else
      return Gnome::Gda::Value(); //null.
  }
}

Gtk::Label* DataWidget::get_label()
{
  return &m_label;
}

const Gtk::Label* DataWidget::get_label() const
{
  return &m_label;
}

int DataWidget::get_suitable_width(Field::glom_field_type field_type)
{
  int result = 150;

  Glib::ustring example_text;
  switch(field_type)
  {
    case(Field::TYPE_DATE):
    {
      Gnome::Gda::Date date = {0, 0, 0};
      date.day = 31;
      date.month = 12;
      date.year = 2000;
      example_text = GlomConversions::get_text_for_gda_value(field_type, Gnome::Gda::Value(date));
      break;
    }
    case(Field::TYPE_TIME):
    {
      Gnome::Gda::Time time = {0, 0, 0, 0};
      time.hour = 24;
      time.minute = 59;
      time.second = 59;
      example_text = GlomConversions::get_text_for_gda_value(field_type, Gnome::Gda::Value(time));
      break;
    }
    case(Field::TYPE_NUMERIC):
    {
      example_text = "9999999999";
      break;
    }
    case(Field::TYPE_TEXT):
    {
      example_text = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
      break;
    }
    default:
    {
      break;
    }
  }


  if(!example_text.empty())
  {
    //Get the width required for this string in the current font:
    Glib::RefPtr<Pango::Layout> refLayout = create_pango_layout(example_text);
    int width = 0;
    int height = 0;
    refLayout->get_pixel_size(width, height);
    result = width;

    //Add a bit more:
    result += 10;
  }

  return result;
}

void DataWidget::set_editable(bool editable)
{
  Gtk::Widget* child = get_child();
  Gtk::Entry* entry = dynamic_cast<Gtk::Entry*>(child);
  if(entry)
    entry->set_editable(editable);
  else
  {
    Gtk::CheckButton* checkbutton = dynamic_cast<Gtk::CheckButton*>(child);
    if(checkbutton)
      checkbutton->set_sensitive(editable);
  }
}




  
