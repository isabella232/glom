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
#include "datawidget.h"
#include <gtkmm/checkbutton.h>
#include "../data_structure/glomconversions.h"

FlowTableWithFields::Info::Info()
: m_first(0),
  m_second(0),
  m_checkbutton(0)
{
}

FlowTableWithFields::FlowTableWithFields()
{
  
}

FlowTableWithFields::~FlowTableWithFields()
{
}

void FlowTableWithFields::add_group(const Glib::ustring& /* group_name */, const Glib::ustring& group_title, const type_map_field_sequence& fields)
{
  if(true)//!fields.empty() && !group_name.empty())
  {
    Gtk::Frame* frame = Gtk::manage( new Gtk::Frame );
   
    if(!group_title.empty())
    {
      Gtk::Label* label = Gtk::manage( new Gtk::Label );
      label->set_text("<b>" + group_title + "</b>" );
      label->set_use_markup();
      label->show();
      frame->set_label_widget(*label);
    }

    frame->set_shadow_type(Gtk::SHADOW_NONE); //HIG-style 
    frame->show();

    Gtk::Alignment* alignment = Gtk::manage( new Gtk::Alignment );

    if(!group_title.empty()) //Don't indent if it has no title, to allow use of groups just for positioning.
      alignment->set_padding(6, 0, 6, 0);
      
    alignment->show();
    frame->add(*alignment);
    
    FlowTableWithFields* flow_table = Gtk::manage( new FlowTableWithFields() );
    
    flow_table->set_columns_count(1);
    flow_table->set_padding(6);
    flow_table->show();
    alignment->add(*flow_table);
    
    for(type_map_field_sequence::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      //Gtk::Entry* debug = Gtk::manage(new Gtk::Entry() );
      //flow_table->add(*debug);
      //debug->show();
      flow_table->add_field(iter->second);
    }
   
    add(*frame);

    m_sub_flow_tables.push_back(flow_table);

    //Connect signal:
    flow_table->signal_field_edited().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_entry_edited) );
  }
}
  
void FlowTableWithFields::add_field(const Field& field, const Glib::ustring& group)
{
  const Glib::ustring id = field.get_name();
  type_mapFields::iterator iterFind = m_mapFields.find(id);
  if(iterFind == m_mapFields.end()) //If it is not already there.
  {
    Info info;
    info.m_field = field;

    //Add the entry or checkbox (handled by the DataWidget)
    info.m_second = Gtk::manage(new DataWidget(field.get_glom_type(), field.get_title_or_name()) );
    info.m_second->show_all();

    info.m_first = Gtk::manage(new Gtk::Alignment());

    //Add a label, if one is necessary:
    Gtk::Label* label = info.m_second->get_label();
    if(!label->get_text().empty())
    {
      info.m_first->add( *label );
      label->show();
      info.m_first->show();
      info.m_first->set(Gtk::ALIGN_RIGHT);
      info.m_first->show_all_children(); //This does not seem to work, so we show the label explicitly.
    }

    info.m_group = group;

    add(*(info.m_first), *(info.m_second));

    info.m_second->signal_edited().connect( sigc::bind(sigc::mem_fun(*this, &FlowTableWithFields::on_entry_edited), id)  );
  
    m_mapFields[id] = info;
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

void FlowTableWithFields::set_field_value(const Field& field, const Gnome::Gda::Value& value)
{
  set_field_value(field.get_name(), value);
}

void FlowTableWithFields::set_field_value(const Glib::ustring& id, const Gnome::Gda::Value& value)
{
  Gtk::Widget* widget = get_field(id);
  DataWidget* datawidget = dynamic_cast<DataWidget*>(widget);
  if(datawidget)
    datawidget->set_value(value);
}

Gnome::Gda::Value FlowTableWithFields::get_field_value(const Field& field) const
{
  return get_field_value(field.get_name());
}

Gnome::Gda::Value FlowTableWithFields::get_field_value(const Glib::ustring& id) const
{
  const Gtk::Widget* widget = get_field(id);
  const DataWidget* datawidget = dynamic_cast<const DataWidget*>(widget);
  if(datawidget)
    return datawidget->get_value();
  else
  {
    return Gnome::Gda::Value(); //null.
  }
}
 
void FlowTableWithFields::set_field_editable(const Field& field, bool editable)
{
  Gtk::Widget* widget = get_field(field);
  DataWidget* datawidget = dynamic_cast<DataWidget*>(widget);
  if(datawidget)
    datawidget->set_editable(editable);
}


Gtk::Widget* FlowTableWithFields::get_field(const Field& field)
{
  return get_field(field.get_name());
}

const Gtk::Widget* FlowTableWithFields::get_field(const Field& field) const
{
  return get_field(field.get_name());
}

const Gtk::Widget* FlowTableWithFields::get_field(const Glib::ustring& id) const
{
  const Gtk::Widget* widget = 0;

  type_mapFields::const_iterator iterFind = m_mapFields.find(id);
  if(iterFind != m_mapFields.end())
  {
    const Info& info = iterFind->second;
    if(info.m_checkbutton)
      widget = info.m_checkbutton;
    else
      widget = info.m_second;
  }

  if(widget)
    return widget;
  else
  {
    //Check the sub-flowtables:
    for(type_sub_flow_tables::const_iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
    {
      FlowTableWithFields* subtable = *iter;
      if(subtable)
        widget = subtable->get_field(id);

      if(widget)
        return widget;
    }
  }

  return 0; //Not found.
}

Gtk::Widget* FlowTableWithFields::get_field(const Glib::ustring& id)
{
  const FlowTableWithFields* pThis = this;
  return const_cast<Gtk::Widget*>(pThis->get_field(id)); 
}

void FlowTableWithFields::change_group(const Glib::ustring& /* id */, const Glib::ustring& /*new_group */)
{
  //TODO.
}

void FlowTableWithFields::remove_all()
{
  m_mapFields.clear();
  m_sub_flow_tables.clear();
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

void FlowTableWithFields::on_checkbutton_toggled(const Glib::ustring& id)
{
  m_signal_field_edited.emit(id);
}






           


