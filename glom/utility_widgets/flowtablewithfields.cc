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
#include "../mode_data/dialog_choose_relationship.h"
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>

FlowTableWithFields::Info::Info()
: m_first(0),
  m_second(0),
  m_checkbutton(0)
{
}

FlowTableWithFields::FlowTableWithFields(const Glib::ustring& table_name)
: m_table_name(table_name)
{
}

FlowTableWithFields::~FlowTableWithFields()
{
  //Remove views. The widgets are deleted automatically because they are managed.
  for(type_listFields::iterator iter = m_listFields.begin(); iter != m_listFields.end(); ++iter)
  {
    View_Composite_Glom* pViewFirst = dynamic_cast<View_Composite_Glom*>(iter->m_first);
    if(pViewFirst)
      remove_view(pViewFirst);

   View_Composite_Glom* pViewSecond = dynamic_cast<View_Composite_Glom*>(iter->m_second);
    if(pViewSecond)
      remove_view(pViewSecond);
  }
}

void FlowTableWithFields::set_table(const Glib::ustring& table_name)
{
  m_table_name = table_name;

  //Recurse:
  for(type_sub_flow_tables::iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    FlowTableWithFields* subtable = *iter;
    if(subtable)
    {
      subtable->set_table(table_name);
    }
  }
}

void FlowTableWithFields::add_layout_item(const sharedptr<LayoutItem>& item)
{
  add_layout_item_at_position(item, m_list_layoutwidgets.end()); 
}

void FlowTableWithFields::add_layout_item_at_position(const sharedptr<LayoutItem>& item, const type_list_layoutwidgets::iterator& add_before)
{
  //Get derived type and do the appropriate thing:
  sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(item);
  if(field)
  {
    add_field_at_position(field, m_table_name, add_before);

    //Do not allow editing of auto-increment fields:
    if(field->get_full_field_details()->get_auto_increment())
      set_field_editable(field, false);
    else
      set_field_editable(field, field->get_editable_and_allowed());
  }
  else
  {
    sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::cast_dynamic(item);
    if(portal)
    {
      Document_Glom* pDocument = static_cast<Document_Glom*>(get_document());
      if(pDocument)
      {
        sharedptr<Relationship> relationship = pDocument->get_relationship(m_table_name, portal->get_relationship_name());
        if(relationship)
        {
          Box_Data_List_Related* portal_box = Gtk::manage(new Box_Data_List_Related);
          add_view(portal_box); //Give it access to the document, needed to get the layout and fields information.

          portal_box->init_db_details(portal); //Create the layout

          portal_box->set_layout_item(portal, relationship->get_to_table());
          portal_box->show();
          add(*portal_box, true /* expand */);

          m_portals.push_back(portal_box);
          add_layoutwidgetbase(portal_box, add_before);

          //Connect signals:
          portal_box->signal_record_changed().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_portal_record_changed) );

          portal_box->signal_user_requested_details().connect( sigc::bind( sigc::mem_fun(*this, &FlowTableWithFields::on_portal_user_requested_details), portal_box));
        }
      }
    }
    else
    {
     sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::cast_dynamic(item);
      if(group)
        add_layout_group_at_position(group, add_before);
    }
  }
}

void FlowTableWithFields::add_layout_group(const sharedptr<LayoutGroup>& group)
{
  add_layout_group_at_position(group, m_list_layoutwidgets.end());
}

void FlowTableWithFields::add_layout_group_at_position(const sharedptr<LayoutGroup>& group, const type_list_layoutwidgets::iterator& add_before)
{
  if(!group)
    return;

  if(true)//!fields.empty() && !group_name.empty())
  {
    Gtk::Frame* frame = Gtk::manage( new Gtk::Frame );

    if(!group->get_title().empty())
    {
      Gtk::Label* label = Gtk::manage( new Gtk::Label );
      label->set_markup( Bakery::App_Gtk::util_bold_message(group->get_title()) );
      label->show();
      frame->set_label_widget(*label);
    }

    frame->set_shadow_type(Gtk::SHADOW_NONE); //HIG-style
    frame->show();

    Gtk::Alignment* alignment = Gtk::manage( new Gtk::Alignment );

    if(!group->get_title().empty()) //Don't indent if it has no title, to allow use of groups just for positioning.
      alignment->set_padding(6, 0, 6, 0);

    alignment->show();
    frame->add(*alignment);

    FlowTableWithFields* flow_table = Gtk::manage( new FlowTableWithFields() );
    add_view(flow_table); //Allow these sub-flowtables to access the document too.
    flow_table->set_table(m_table_name);

    flow_table->set_columns_count(group->m_columns_count);
    flow_table->set_padding(6);
    flow_table->show();
    alignment->add(*flow_table);

    LayoutGroup::type_map_items items = group->get_items(); 
    for(LayoutGroup::type_map_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
    {
      sharedptr<LayoutItem> item = iter->second;
      if(item)
      {
        flow_table->add_layout_item(item);
      }
    }

    add(*frame, true /* expand */);

    m_sub_flow_tables.push_back(flow_table);
    flow_table->set_layout_item(group, m_table_name);
    add_layoutwidgetbase(flow_table, add_before);

    //Connect signal:
    flow_table->signal_field_edited().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_flowtable_entry_edited) );
    flow_table->signal_field_open_details_requested().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_flowtable_entry_open_details_requested) );
    flow_table->signal_related_record_changed().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_flowtable_related_record_changed) );
    flow_table->signal_requested_related_details().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_flowtable_requested_related_details) );
  }
}

/*
void FlowTableWithFields::add_group(const Glib::ustring& group_name, const Glib::ustring& group_title, const type_map_field_sequence& fields)
{
  if(true)//!fields.empty() && !group_name.empty())
  {
    Gtk::Frame* frame = Gtk::manage( new Gtk::Frame );

    if(!group_title.empty())
    {
      Gtk::Label* label = Gtk::manage( new Gtk::Label );
      label->set_text("<b>" + group_title + "") );
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
*/

void FlowTableWithFields::add_field(const sharedptr<LayoutItem_Field>& layoutitem_field, const Glib::ustring& table_name)
{
  add_field_at_position(layoutitem_field, table_name, m_list_layoutwidgets.end());
}

void FlowTableWithFields::add_field_at_position(const sharedptr<LayoutItem_Field>& layoutitem_field, const Glib::ustring& table_name, const type_list_layoutwidgets::iterator& add_before)
{
  Info info;
  info.m_field = layoutitem_field;

  //Add the entry or checkbox (handled by the DataWidget)
  DataWidget* pDataWidget = Gtk::manage(new DataWidget(layoutitem_field, table_name, get_document()) );
  add_layoutwidgetbase(pDataWidget, add_before);
  add_view(pDataWidget); //So it can get the document.

  info.m_second = pDataWidget; 
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

  //info.m_group = layoutitem_field.m_group;

  //Expand multiline text fields to take up the maximum possible width:
  bool expand_second = false;
  if( (layoutitem_field->get_glom_type() == Field::TYPE_TEXT) && layoutitem_field->get_formatting_used().get_text_format_multiline())
    expand_second = true;

  add(*(info.m_first), *(info.m_second), expand_second);

  info.m_second->signal_edited().connect( sigc::bind(sigc::mem_fun(*this, &FlowTableWithFields::on_entry_edited), layoutitem_field)  ); //TODO:  Is it a good idea to bind the LayoutItem? sigc::bind() probably stores a copy at this point.
  info.m_second->signal_layout_item_added().connect( sigc::bind(
    sigc::mem_fun(*this, &FlowTableWithFields::on_datawidget_layout_item_added), info.m_second) );

  info.m_second->signal_open_details_requested().connect( sigc::bind(sigc::mem_fun(*this, &FlowTableWithFields::on_entry_open_details_requested), layoutitem_field)  );

  m_listFields.push_back(info); //This would be the wrong position, but you should only use this method directly when you expect it to be followed by a complete re-layout.
}

void FlowTableWithFields::remove_field(const Glib::ustring& id)
{
  for(type_listFields::iterator iter = m_listFields.begin(); iter != m_listFields.end(); ++iter)
  {
    if(iter->m_field->get_name() == id)
    {
      Info info = *iter;
      remove(*(info.m_first));

      View_Composite_Glom* pViewFirst = dynamic_cast<View_Composite_Glom*>(info.m_first);
      if(pViewFirst)
        remove_view(pViewFirst);

      delete info.m_first;

      View_Composite_Glom* pViewSecond = dynamic_cast<View_Composite_Glom*>(info.m_second);
      if(pViewSecond)
        remove_view(pViewSecond);

      delete info.m_second; //It is removed at the same time.

      iter = m_listFields.erase(iter);
    }
  } 
}

void FlowTableWithFields::set_field_value(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  //Set widgets which should show the value of this field:
  type_list_widgets list_widgets = get_field(field);
  for(type_list_widgets::iterator iter = list_widgets.begin(); iter != list_widgets.end(); ++iter)
  {
    DataWidget* datawidget = dynamic_cast<DataWidget*>(*iter);
    if(datawidget)
    {
      datawidget->set_value(value);
    }
  }

  //Refresh widgets which should show the related records for relationships that use this field:
  type_list_widgets list_portals = get_portals(field /* from_key field name */);
  for(type_list_widgets::iterator iter = list_portals.begin(); iter != list_portals.end(); ++iter)
  {
    Box_Data_List_Related* portal = dynamic_cast<Box_Data_List_Related*>(*iter);
    if(portal)
    {
      //g_warning("FlowTableWithFields::set_field_value: foreign_key_value=%s", value.to_string().c_str());
      portal->refresh_data_from_database_with_foreign_key(value /* foreign key value */);
    }
  }
}

Gnome::Gda::Value FlowTableWithFields::get_field_value(const sharedptr<const LayoutItem_Field>& field) const
{
  type_list_const_widgets list_widgets = get_field(field);
  if(!list_widgets.empty())
  {
    const DataWidget* datawidget = dynamic_cast<const DataWidget*>(*(list_widgets.begin()));

    if(datawidget)
      return datawidget->get_value();
  }

  //g_warning("FlowTableWithFields::get_field_value(): returning null");
  return Gnome::Gda::Value(); //null.
}

void FlowTableWithFields::set_field_editable(const sharedptr<const LayoutItem_Field>& field, bool editable)
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


FlowTableWithFields::type_list_widgets FlowTableWithFields::get_portals(const sharedptr<const LayoutItem_Field>& from_key)
{
  type_list_widgets result;

  const Glib::ustring from_key_name = from_key->get_name();

  //Check the single-item widgets:
   for(type_portals::const_iterator iter = m_portals.begin(); iter != m_portals.end(); ++iter)
  {
    //*iter is a FlowTableItem.
    Box_Data_List_Related* pPortal = *iter;
    if(pPortal)
    {
      sharedptr<Relationship> relationship = pPortal->get_relationship();
      if(relationship && (relationship->get_from_field() == from_key_name))
        result.push_back(pPortal);
    }
  }

  //Check the sub-flowtables:
  for(type_sub_flow_tables::const_iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    FlowTableWithFields* subtable = *iter;
    if(subtable)
    {
      type_list_widgets sub_list = subtable->get_portals(from_key);
      if(!sub_list.empty())
      {
        //Add to the main result:
        result.insert(result.end(), sub_list.begin(), sub_list.end());
      }
    }
  }

  return result;
}

FlowTableWithFields::type_list_const_widgets FlowTableWithFields::get_field(const sharedptr<const LayoutItem_Field>& layout_item) const
{
  type_list_const_widgets result;

  const Glib::ustring layout_item_name = layout_item->get_name();
  const Glib::ustring layout_item_relationship_name = layout_item->get_relationship_name();
  for(type_listFields::const_iterator iter = m_listFields.begin(); iter != m_listFields.end(); ++iter)
  {
    if( (iter->m_field->get_name() == layout_item_name) && (iter->m_field->get_relationship_name() == layout_item_relationship_name) )
    {
      const Info& info = *iter;
      if(info.m_checkbutton)
        result.push_back(info.m_checkbutton);
      else
        result.push_back(info.m_second);
    }
  }

  //Check the sub-flowtables:
  for(type_sub_flow_tables::const_iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    const FlowTableWithFields* subtable = *iter;
    if(subtable)
    {
      type_list_const_widgets sub_list = subtable->get_field(layout_item);
      if(!sub_list.empty())
      {
        //Add to the main result:
        result.insert(result.end(), sub_list.begin(), sub_list.end());
      }
    }
  }

  return result;
}

FlowTableWithFields::type_list_widgets FlowTableWithFields::get_field(const sharedptr<const LayoutItem_Field>& layout_item)
{
  //TODO: Avoid duplication
  type_list_widgets result;

  const Glib::ustring layout_item_name = layout_item->get_name();
  const Glib::ustring layout_item_relationship_name = layout_item->get_relationship_name();
  for(type_listFields::const_iterator iter = m_listFields.begin(); iter != m_listFields.end(); ++iter)
  {
    if( (iter->m_field->get_name() == layout_item_name) && (iter->m_field->get_relationship_name() == layout_item_relationship_name) )
    {
      const Info& info = *iter;
      if(info.m_checkbutton)
        result.push_back(info.m_checkbutton);
      else
        result.push_back(info.m_second);
    }
  }

  //Check the sub-flowtables:
  for(type_sub_flow_tables::const_iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    FlowTableWithFields* subtable = *iter;
    if(subtable)
    {
      type_list_widgets sub_list = subtable->get_field(layout_item);
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
  m_listFields.clear();

  for(type_sub_flow_tables::iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    FlowTableWithFields* pSub = *iter;
    if(pSub)
    {
      remove_view(*iter);
      remove(*pSub);

      delete pSub;
    }
  }
 
  m_sub_flow_tables.clear();


  for(type_portals::iterator iter = m_portals.begin(); iter != m_portals.end(); ++iter)
  {
    Box_Data_List_Related* pPortal = *iter;
    remove_view(pPortal);
    remove(*pPortal);
    delete pPortal;
  }
  m_portals.clear();

  m_list_layoutwidgets.clear();

  //Remove views. The widgets are deleted automatically because they are managed.
  for(type_listFields::iterator iter = m_listFields.begin(); iter != m_listFields.end(); ++iter)
  {
    View_Composite_Glom* pViewFirst = dynamic_cast<View_Composite_Glom*>(iter->m_first);
    if(pViewFirst)
      remove_view(pViewFirst);

   View_Composite_Glom* pViewSecond = dynamic_cast<View_Composite_Glom*>(iter->m_second);
    if(pViewSecond)
      remove_view(pViewSecond);
  }

  FlowTable::remove_all();
}

FlowTableWithFields::type_signal_field_edited FlowTableWithFields::signal_field_edited()
{
  return m_signal_field_edited;
}

FlowTableWithFields::type_signal_field_open_details_requested FlowTableWithFields::signal_field_open_details_requested()
{
  return m_signal_field_open_details_requested;
}

FlowTableWithFields::type_signal_related_record_changed FlowTableWithFields::signal_related_record_changed()
{
  return m_signal_related_record_changed;
}

FlowTableWithFields::type_signal_requested_related_details FlowTableWithFields::signal_requested_related_details()
{
  return m_signal_requested_related_details;
}

void FlowTableWithFields::on_entry_edited(const Gnome::Gda::Value& value, sharedptr<const LayoutItem_Field> field)
{
  m_signal_field_edited.emit(field, value);
}

void FlowTableWithFields::on_entry_open_details_requested(const Gnome::Gda::Value& value, sharedptr<const LayoutItem_Field> field)
{
  m_signal_field_open_details_requested.emit(field, value);
}

void FlowTableWithFields::on_flowtable_entry_edited(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  m_signal_field_edited.emit(field, value);
}

void FlowTableWithFields::on_flowtable_entry_open_details_requested(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  m_signal_field_open_details_requested.emit(field, value);
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

/*
void FlowTableWithFields::get_layout_groups(Document_Glom::type_mapLayoutGroupSequence& groups)
{
  //This is only called on the top-level FlowTable.
  //It assumes that there are only groups in this FlowTable,
  //and no other layout items:
  Document_Glom::type_mapLayoutGroupSequence::size_type sequence = 1; //0 means no position.
  for(type_list_layoutwidgets::iterator iter = m_list_layoutwidgets.begin(); iter != m_list_layoutwidgets.end(); ++iter)
  {
    LayoutWidgetBase* pLayoutWidget = *iter;
    if(pLayoutWidget)
    {
      FlowTableWithFields* pFlowTable = dynamic_cast<FlowTableWithFields*>(pLayoutWidget);
      if(pFlowTable)
      {
        //recurse:
        sharedptr<LayoutGroup> group_child = pFlowTable->get_layout_group();
        groups[sequence] = group_child;

        ++sequence;
      }
    }
  }

}
*/

/*
sharedptr<LayoutGroup> FlowTableWithFields::get_layout_group()
{
  sharedptr<LayoutGroup> group;

  //Initialize output parameter:
  //group = LayoutGroup();
  sharedptr<const LayoutGroup> pGroupExisting = sharedptr<const LayoutGroup>::cast_dynamic(get_layout_item());
  if(pGroupExisting)
  {
    //Start with the same information:
    group = glom_sharedptr_clone(pGroupExisting); //TODO_Performance: Copy without copying/clone the children.
    group->remove_all_items(); //We will readd these.
  }

  //Iterate over the data widgets:
  for(type_list_layoutwidgets::iterator iter = m_list_layoutwidgets.begin(); iter != m_list_layoutwidgets.end(); ++iter)
  {
    LayoutWidgetBase* pLayoutWidget = *iter;
    if(pLayoutWidget)
    {
      FlowTableWithFields* pFlowTable = dynamic_cast<FlowTableWithFields*>(pLayoutWidget);
      if(pFlowTable)
      {
        //recurse:
        sharedptr<LayoutGroup> group_child = pFlowTable->get_layout_group();
        group->add_item(group_child);
      }
      else
      {
        sharedptr<const LayoutItem> pLayoutItem = (*iter)->get_layout_item();
        if(pLayoutItem)
        {
          group->add_item(pLayoutItem);
        }

      }
    }
  }

  return group;
}
*/

void FlowTableWithFields::add_layoutwidgetbase(LayoutWidgetBase* layout_widget)
{
  add_layoutwidgetbase(layout_widget, m_list_layoutwidgets.end());
}

void FlowTableWithFields::add_layoutwidgetbase(LayoutWidgetBase* layout_widget, const type_list_layoutwidgets::iterator& add_before)
{
  m_list_layoutwidgets.insert(add_before, layout_widget);

  //Handle layout_changed signal:
  layout_widget->signal_layout_changed().connect(sigc::mem_fun(*this, &FlowTableWithFields::on_layoutwidget_changed));
}

void FlowTableWithFields::on_layoutwidget_changed()
{
  /*
  sharedptr<LayoutGroup> pLayoutItem = sharedptr<LayoutGroup>::cast_dynamic(get_layout_item());
  if(pLayoutItem)
  {
    //Update tha LayoutItem by looking at the child widgets:
    sharedptr<LayoutGroup> group = get_layout_group();
    set_layout_item(group, m_table_name);
  }
  */

  //Forward the signal to the container:
  signal_layout_changed().emit();
}

void FlowTableWithFields::on_datawidget_layout_item_added(TreeStore_Layout::enumType item_type, DataWidget* pDataWidget)
{
  //Get the position of the selected item:
  type_list_layoutwidgets::iterator iterAfter = m_list_layoutwidgets.end();
  type_list_layoutwidgets::iterator iterFind = std::find(m_list_layoutwidgets.begin(), m_list_layoutwidgets.end(), pDataWidget);
  if(iterFind != m_list_layoutwidgets.end())
  {
    iterAfter = iterFind;
    ++iterAfter; //std::list<>::insert() inserts before, but we want to insert after, so we increment..
  }

  if(item_type == TreeStore_Layout::TYPE_FIELD)
  {
    sharedptr<LayoutItem_Field> layout_item_field = pDataWidget->offer_field_list(m_table_name);
    if(layout_item_field)
    {
      //TODO: privileges.
      add_layout_item_at_position(layout_item_field, iterAfter);
    }
  }
  else if(item_type == TreeStore_Layout::TYPE_GROUP)
  {
    sharedptr<LayoutGroup> layout_item = sharedptr<LayoutGroup>::create();
    layout_item->set_title(_("New Group"));
    add_layout_item_at_position(layout_item, iterAfter);
  }
  else if(item_type == TreeStore_Layout::TYPE_PORTAL)
  {
    try
    {
      Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_choose_relationship");

      Dialog_ChooseRelationship* dialog = 0;
      refXml->get_widget_derived("dialog_choose_relationship", dialog);

      if(dialog)
      {
        Document_Glom* pDocument = static_cast<Document_Glom*>(get_document());
        dialog->set_document(pDocument, m_table_name);
        //TODO: dialog->set_transient_for(*get_app_window());
        int response = dialog->run();
        dialog->hide();
        if(response == Gtk::RESPONSE_OK)
        {
          //Get the chosen relationship:
          sharedptr<Relationship> relationship  = dialog->get_relationship_chosen();
          if(relationship)
          {
            sharedptr<LayoutItem_Portal> layout_item = sharedptr<LayoutItem_Portal>::create();
            layout_item->set_relationship(relationship);
            add_layout_item_at_position(layout_item, iterAfter);
          }
        }

        delete dialog;
      }
    }
    catch(const Gnome::Glade::XmlError& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }

  //TODO: Only if it has really changed:
  signal_layout_changed().emit(); //This should result in a complete re-layout.
}

void FlowTableWithFields::on_portal_record_changed(const Glib::ustring& relationship_name)
{
  signal_related_record_changed().emit(relationship_name);
}

void FlowTableWithFields::on_flowtable_related_record_changed(const Glib::ustring& relationship_name)
{
  //Forward it to the parent:
  signal_related_record_changed().emit(relationship_name);
}

void FlowTableWithFields::on_portal_user_requested_details(Gnome::Gda::Value primary_key_value, Box_Data_List_Related* portal_box)
{
  signal_requested_related_details().emit(portal_box->get_relationship()->get_to_table(), primary_key_value);
}

void FlowTableWithFields::on_flowtable_requested_related_details(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value)
{
  //Forward it to the parent:
  signal_requested_related_details().emit(table_name, primary_key_value);
}
