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
#include "../mode_data/box_data_list_related.h"

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

void FlowTableWithFields::add_layout_item(const LayoutItem& item)
{
  const LayoutItem* pItem = &item;

  //Get derived type and do the appropriate thing:
  const LayoutGroup* group = dynamic_cast<const LayoutGroup*>(pItem);
  if(group)
    add_layout_group(*group);
  else
  {
    const LayoutItem_Field* field = dynamic_cast<const LayoutItem_Field*>(pItem);
    if(field)
    {                          
      add_field(field->m_field);

      //Do not allow editing of auto-increment fields:
      if(field->m_field.get_field_info().get_auto_increment())
        set_field_editable(field->m_field, false);
    }
    else
    {
      const LayoutItem_Portal* portal = dynamic_cast<const LayoutItem_Portal*>(pItem);
      if(portal)
      {
        Gtk::Widget* portal_box = Gtk::manage(new Gtk::Label("TODO: " + portal->get_relationship()));//Gtk::manage(new Box_Data_List_Related);
        portal_box->show();
        add(*portal_box);
      }
    }
  }
}

void FlowTableWithFields::add_layout_group(const LayoutGroup& group)
{
  if(true)//!fields.empty() && !group_name.empty())
  {
    Gtk::Frame* frame = Gtk::manage( new Gtk::Frame );

    if(!group.m_title.empty())
    {
      Gtk::Label* label = Gtk::manage( new Gtk::Label );
      label->set_text("<b>" + group.m_title + "</b>" );
      label->set_use_markup();
      label->show();
      frame->set_label_widget(*label);
    }

    frame->set_shadow_type(Gtk::SHADOW_NONE); //HIG-style
    frame->show();

    Gtk::Alignment* alignment = Gtk::manage( new Gtk::Alignment );

    if(!group.m_title.empty()) //Don't indent if it has no title, to allow use of groups just for positioning.
      alignment->set_padding(6, 0, 6, 0);

    alignment->show();
    frame->add(*alignment);

    FlowTableWithFields* flow_table = Gtk::manage( new FlowTableWithFields() );

    flow_table->set_columns_count(group.m_columns_count);
    flow_table->set_padding(6);
    flow_table->show();
    alignment->add(*flow_table);

    LayoutGroup::type_map_const_items items = group.get_items(); 
    for(LayoutGroup::type_map_const_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
    {
      const LayoutItem* item = iter->second;
      if(item)
      {
        flow_table->add_layout_item(*item);
      }
    }

    add(*frame);

    m_sub_flow_tables.push_back(flow_table);

    //Connect signal:
    flow_table->signal_field_edited().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_flowtable_entry_edited) );
  }
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
    flow_table->signal_field_edited().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_flowtable_entry_edited) );
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
    if(label && !label->get_text().empty())
    {
      info.m_first->add( *label );
      label->set_alignment(Gtk::ALIGN_RIGHT, Gtk::ALIGN_CENTER); //Align right.
      label->show();
      
      info.m_first->show();
      info.m_first->set(Gtk::ALIGN_RIGHT, Gtk::ALIGN_CENTER);
      info.m_first->show_all_children(); //This does not seem to work, so we show the label explicitly.
    }

    info.m_group = group;

    add(*(info.m_first), *(info.m_second));

    info.m_second->signal_edited().connect( sigc::bind(sigc::mem_fun(*this, &FlowTableWithFields::on_entry_edited), id)  );
  
    m_mapFields[id] = info;
  }
  else
    g_warning("FlowTableWithFields::add_field: The ID exists already: %s", id.c_str());
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
  type_list_widgets list_widgets = get_field(id);
  for(type_list_widgets::iterator iter = list_widgets.begin(); iter != list_widgets.end(); ++iter)
  {
    DataWidget* datawidget = dynamic_cast<DataWidget*>(*iter);
    if(datawidget)
    {
      datawidget->set_value(value);
    }
  }
}

Gnome::Gda::Value FlowTableWithFields::get_field_value(const Field& field) const
{
  return get_field_value(field.get_name());
}

Gnome::Gda::Value FlowTableWithFields::get_field_value(const Glib::ustring& id) const
{
  type_list_const_widgets list_widgets = get_field(id);
  if(!list_widgets.empty())
  {
    const DataWidget* datawidget = dynamic_cast<const DataWidget*>(*(list_widgets.begin()));
    
    if(datawidget)
      return datawidget->get_value();
  }
    
  return Gnome::Gda::Value(); //null.
}
 
void FlowTableWithFields::set_field_editable(const Field& field, bool editable)
{
  type_list_widgets list_widgets = get_field(field);
  for(type_list_widgets::iterator iter = list_widgets.begin(); iter != list_widgets.end(); ++iter)
  {
    DataWidget* datawidget = dynamic_cast<DataWidget*>(*iter);
    if(datawidget)
    {
      datawidget->set_editable(editable);
    }
  }
}


FlowTableWithFields::type_list_widgets FlowTableWithFields::get_field(const Field& field)
{
  return get_field(field.get_name());
}

FlowTableWithFields::type_list_const_widgets FlowTableWithFields::get_field(const Field& field) const
{
  return get_field(field.get_name());
}

FlowTableWithFields::type_list_const_widgets FlowTableWithFields::get_field(const Glib::ustring& id) const
{
  type_list_const_widgets result;
  
  type_mapFields::const_iterator iterFind = m_mapFields.find(id);
  if(iterFind != m_mapFields.end())
  {
    const Info& info = iterFind->second;
    if(info.m_checkbutton)
      result.push_back(info.m_checkbutton);
    else
      result.push_back(info.m_second);
  }

  //Check the sub-flowtables:
  for(type_sub_flow_tables::const_iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    const FlowTableWithFields* subtable = *iter;
    if(subtable)
    {
      type_list_const_widgets sub_list = subtable->get_field(id);
      if(!sub_list.empty())
      {
        //Add to the main result:
        result.insert(result.end(), sub_list.begin(), sub_list.end());
      }
    }
  }

  return result;
}

FlowTableWithFields::type_list_widgets FlowTableWithFields::get_field(const Glib::ustring& id)
{
  //TODO: Avoid duplication
  type_list_widgets result;

  type_mapFields::const_iterator iterFind = m_mapFields.find(id);
  if(iterFind != m_mapFields.end())
  {
    const Info& info = iterFind->second;
    if(info.m_checkbutton)
      result.push_back(info.m_checkbutton);
    else
      result.push_back(info.m_second);
  }

  //Check the sub-flowtables:
  for(type_sub_flow_tables::const_iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    FlowTableWithFields* subtable = *iter;
    if(subtable)
    {
      type_list_widgets sub_list = subtable->get_field(id);
      if(!sub_list.empty())
      {
        //Add to the main result:
        result.insert(result.end(), sub_list.begin(), sub_list.end());
      }
    }
  }

  return result;
}

void FlowTableWithFields::change_group(const Glib::ustring& /* id */, const Glib::ustring& /*new_group */)
{
  //TODO.
}

void FlowTableWithFields::remove_all()
{
  //TODO: Release the fields memory, and the portal memory.
  m_mapFields.clear();
  m_sub_flow_tables.clear();
  FlowTable::remove_all();
}

FlowTableWithFields::type_signal_field_edited FlowTableWithFields::signal_field_edited()
{
  return m_signal_field_edited;
}

void FlowTableWithFields::on_entry_edited(const Gnome::Gda::Value& value, Glib::ustring id)
{
  m_signal_field_edited.emit(id, value); 
}

void FlowTableWithFields::on_flowtable_entry_edited(const Glib::ustring& id, const Gnome::Gda::Value& value)
{
  m_signal_field_edited.emit(id, value);
}

void FlowTableWithFields::set_design_mode(bool value)
{
  FlowTable::set_design_mode(value);

  //Set the mode in the sub-flowtables:
  for(type_sub_flow_tables::iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    FlowTableWithFields* subtable = *iter;
    if(subtable)
      subtable->set_design_mode(value);
  }
}





           


