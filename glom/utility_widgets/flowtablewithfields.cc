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
 
#include "flowtablewithfields.h"

FlowTableWithFields::FlowTableWithFields()
{
  
}

FlowTableWithFields::~FlowTableWithFields()
{
}

void FlowTableWithFields::add_field(const Field& field, const Glib::ustring& group)
{
  Glib::ustring id = field.get_name();
  type_mapFields::iterator iterFind = m_mapFields.find(id);
  if(iterFind == m_mapFields.end()) //If it is not already there.
  {
    Info info;
    info.m_field = field;
    
    info.m_first = Gtk::manage(new Gtk::Alignment());

    Gtk::Label* label = Gtk::manage(new Gtk::Label(field.get_title_or_name()));
    info.m_first->add( *label );
    label->show();
    info.m_first->show();
    info.m_first->set(Gtk::ALIGN_RIGHT);
    info.m_first->show_all_children(); //This does not seem to work, so we show the label explicitly.
    
    info.m_second = Gtk::manage(new EntryGlom(field.get_glom_type()) );
    int width = get_suitable_width(field.get_glom_type());
    info.m_second->set_size_request(width, -1 /* auto */);
    info.m_second->show_all();                            

    info.m_group = group;

    m_mapFields[id] = info;

    add(*(info.m_first), *(info.m_second));

    info.m_second->signal_edited().connect( sigc::bind(sigc::mem_fun(*this, &FlowTableWithFields::on_entry_edited), id)  );
  }
  else
    g_warning("FlowTableWithFields::add_field: The ID exists already.");
}

void FlowTableWithFields::remove_field(const Glib::ustring& id)
{
  type_mapFields::iterator iterFind = m_mapFields.find(id);
  if(iterFind != m_mapFields.end())
  {
    Info info = iterFind->second;
    remove(*(info.m_first));
    
    delete info.m_first;
    delete info.m_second; //It is removed at the same time.

    m_mapFields.erase(iterFind);
  } 
}

EntryGlom* FlowTableWithFields::get_field(const Field& field)
{
  return get_field(field.get_name());
}

EntryGlom* FlowTableWithFields::get_field(const Glib::ustring& id)
{
  type_mapFields::const_iterator iterFind = m_mapFields.find(id);
  if(iterFind != m_mapFields.end())
  {
    Info info = iterFind->second;
    return info.m_second;
  }

  return 0; //Not found.
}

void FlowTableWithFields::change_group(const Glib::ustring& /* id */, const Glib::ustring& /*new_group */)
{
  //TODO.
}

void FlowTableWithFields::remove_all()
{
  m_mapFields.clear();
  FlowTable::remove_all();
}

FlowTableWithFields::type_signal_field_edited FlowTableWithFields::signal_field_edited()
{
  return m_signal_field_edited;
}

void FlowTableWithFields::on_entry_edited(const Glib::ustring& id)
{
  m_signal_field_edited.emit(id);
}

int FlowTableWithFields::get_suitable_width(Field::glom_field_type field_type)
{
  int result = 150;

  Glib::ustring example_text;
  switch(field_type)
  {
    case(Field::TYPE_DATE):
    {
      example_text = "99-99-9999"; //TODO: Get suitable text for the date as displayed in the locale.
      break;
    }
    case(Field::TYPE_TIME):
    {
      example_text = "99:99:99"; //TODO: Get suitable text for the time as displayed in the locale.
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
    Glib::RefPtr<Pango::Layout> refLayout = create_pango_layout(example_text);
    int width = 0;
    int height = 0;
    refLayout->get_pixel_size(width, height);
    result = width;
  }

  return result;
}






           


