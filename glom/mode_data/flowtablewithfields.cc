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
#include <glom/utility_widgets/datawidget.h>
#include <glom/utility_widgets/buttonglom.h>
#include <glom/utility_widgets/notebookglom.h>
#include <glom/utility_widgets/notebooklabelglom.h>
#include <glom/utility_widgets/imageglom.h>
#include <glom/utility_widgets/labelglom.h>
#include <glom/utility_widgets/dialog_flowtable.h>
#include <glom/utility_widgets/placeholder-glom.h>
#include <glom/application.h>
#include <gtkmm/checkbutton.h>
#include <libglom/data_structure/glomconversions.h>
#include <glom/glade_utils.h>
#include <glom/mode_data/box_data_list_related.h>
#include <glom/mode_design/layout/dialog_choose_relationship.h>
#include <glom/utils_ui.h> //For bold_message()).
#include <libglom/data_structure/layout/layoutitem_placeholder.h>
#include <glom/signal_reemitter.h>

#include <glibmm/i18n.h>

namespace Glom
{
  
FlowTableWithFields::Info::Info()
: m_first(0),
  m_second(0),
  m_checkbutton(0)
{
}

FlowTableWithFields::FlowTableWithFields(const Glib::ustring& table_name)
:
#if !defined(GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED)
  // This creates a custom GType for us, to override vfuncs and default
  // signal handlers even with the reduced API (done in flowtable.cc).
  // TODO: It is necessary to do this in all derived classes which is
  // rather annoying, though I don't see another possibility at the moment. armin.
  Glib::ObjectBase("Glom_FlowTable"),
#endif // !defined(GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED)
  m_placeholder(0),
  m_table_name(table_name)
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
    sharedptr<const Field> field_details = field->get_full_field_details();
    if(field_details)
    {
      if(field_details->get_auto_increment())
        set_field_editable(field, false);
      else
        set_field_editable(field, field->get_editable_and_allowed());
    }
  }
  else
  {
    sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::cast_dynamic(item);
    if(portal)
    {
      add_layout_portal_at_position(portal, add_before);     
    }
    else
    {
      sharedptr<LayoutItem_Notebook> notebook = sharedptr<LayoutItem_Notebook>::cast_dynamic(item);
      if(notebook)
      {
        add_layout_notebook_at_position(notebook, add_before);
      }
      else
      {
        sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::cast_dynamic(item);
        if(group)
          add_layout_group_at_position(group, add_before);
        else
        {
          sharedptr<LayoutItem_Button> layout_button = sharedptr<LayoutItem_Button>::cast_dynamic(item);
          if(layout_button)
            add_button_at_position(layout_button, m_table_name, add_before);
          else
          {
            sharedptr<LayoutItem_Text> layout_textobject = sharedptr<LayoutItem_Text>::cast_dynamic(item);
            if(layout_textobject)
              add_textobject_at_position(layout_textobject, m_table_name, add_before);
            else
            {
              sharedptr<LayoutItem_Image> layout_imageobject = sharedptr<LayoutItem_Image>::cast_dynamic(item);
              if(layout_imageobject)
                add_imageobject_at_position(layout_imageobject, m_table_name, add_before);
              else
              {
                sharedptr<LayoutItem_Placeholder> layout_placeholder = 
                  sharedptr<LayoutItem_Placeholder>::cast_dynamic(item);
                if(layout_placeholder)
                  add_placeholder_at_position(layout_placeholder, m_table_name, add_before);
              }
            }
          }
        }
      }
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
    Gtk::Frame* frame = Gtk::manage( new Gtk::Frame ); //TODO_leak: This is possibly leaked, according to valgrind. 

    if(!group->get_title().empty())
    {
      Gtk::Label* label = Gtk::manage( new Gtk::Label ); //TODO: This is maybe leaked, according to valgrind, though it should be managed by GtkFrame.
      label->set_markup( Utils::bold_message(group->get_title()) );
      label->show();
      frame->set_label_widget(*label);
    }

    frame->set_shadow_type(Gtk::SHADOW_NONE); //HIG-style
    frame->show();

    Gtk::Alignment* alignment = Gtk::manage( new Gtk::Alignment ); //TODO_leak: This is possibly leaked, according to valgrind.

    if(!group->get_title().empty()) //Don't indent if it has no title, to allow use of groups just for positioning.
    {
      alignment->set_padding(Glom::Utils::DEFAULT_SPACING_SMALL, 0, 6, 0); //Use left-padding of 6 even on Maemo because indentation is important.
      #ifdef GLOM_ENABLE_MAEMO
      std::cerr << "DEBUG: Unexpected group with title causing extra spacing on Maemo." << std::endl;
      #endif
    }

    alignment->show();
    frame->add(*alignment);

    FlowTableWithFields* flow_table = Gtk::manage( new FlowTableWithFields() );
    add_view(flow_table); //Allow these sub-flowtables to access the document too.
    flow_table->set_table(m_table_name);

    flow_table->set_columns_count(group->get_columns_count());
    
    //Use the parent table's padding:
    flow_table->set_column_padding(get_column_padding());
    flow_table->set_row_padding(get_row_padding());
    flow_table->show();
    
    Gtk::EventBox* event_box = Gtk::manage( new Gtk::EventBox() ); //TODO_Leak: Valgrind says this is possibly leaked.
    event_box->add(*flow_table);
    event_box->set_visible_window(false);
#ifndef GLOM_ENABLE_CLIENT_ONLY
    event_box->signal_button_press_event().connect (sigc::mem_fun (*flow_table,
      &FlowTableWithFields::on_button_press_event));
#endif      
    event_box->show();
    
    alignment->add(*event_box);

    LayoutGroup::type_list_items items = group->get_items(); 
    for(LayoutGroup::type_list_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
    {
      sharedptr<LayoutItem> item = *iter;
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
    flow_table->signal_script_button_clicked().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_script_button_clicked) );
  }
}

Box_Data_List_Related* FlowTableWithFields::create_related(const sharedptr<LayoutItem_Portal>& portal, bool show_title)
{
  if(!portal)
    return 0;

  Document* pDocument = static_cast<Document*>(get_document());
  if(pDocument)
  {
    Box_Data_List_Related* portal_box = Gtk::manage(new Box_Data_List_Related);
    add_view(portal_box); //Give it access to the document, needed to get the layout and fields information.

    //Create the layout:
    if(portal && portal->get_has_relationship_name())
      portal_box->init_db_details(portal, show_title);
    else
      portal_box->init_db_details(m_table_name, show_title);

    Glib::ustring to_table;
    sharedptr<Relationship> relationship = pDocument->get_relationship(m_table_name, portal->get_relationship_name());
    if(relationship)
      to_table = relationship->get_to_table();

    portal_box->set_layout_item(portal, to_table);
    portal_box->show();

    m_portals.push_back(portal_box);

    //Connect signals:
    //Just reemit this object's signal when receiving the same signal from the portal:
    signal_connect_for_reemit_1arg(portal_box->signal_portal_record_changed(), signal_related_record_changed());

    portal_box->signal_user_requested_details().connect( sigc::bind( sigc::mem_fun(*this, &FlowTableWithFields::on_portal_user_requested_details), portal_box));

    return portal_box;
  }

  return 0;
}

Box_Data_Calendar_Related* FlowTableWithFields::create_related_calendar(const sharedptr<LayoutItem_CalendarPortal>& portal, bool show_title)
{
  if(!portal)
    return 0;

  Document* pDocument = static_cast<Document*>(get_document());
  if(pDocument)
  {
    Box_Data_Calendar_Related* portal_box = Gtk::manage(new Box_Data_Calendar_Related);
    add_view(portal_box); //Give it access to the document, needed to get the layout and fields information.

    //Create the layout:
    if(portal && portal->get_has_relationship_name())
      portal_box->init_db_details(portal, show_title);
    else
      portal_box->init_db_details(m_table_name, show_title);

    Glib::ustring to_table;
    sharedptr<Relationship> relationship = pDocument->get_relationship(m_table_name, portal->get_relationship_name());
    if(relationship)
      to_table = relationship->get_to_table();

    portal_box->set_layout_item(portal, to_table);
    portal_box->show();

    m_portals.push_back(portal_box);

    //Connect signals:
    //Just reemit this object's signal when receiving the same signal from the portal:
    signal_connect_for_reemit_1arg(portal_box->signal_portal_record_changed(), signal_related_record_changed());    

    portal_box->signal_user_requested_details().connect( sigc::bind( sigc::mem_fun(*this, &FlowTableWithFields::on_portal_user_requested_details), portal_box));

    return portal_box;
  }

  return 0;
}

void FlowTableWithFields::add_layout_portal_at_position(const sharedptr<LayoutItem_Portal>& portal, const type_list_layoutwidgets::iterator& add_before)
{
  Box_Data_Portal* portal_box = 0;
  sharedptr<LayoutItem_CalendarPortal> calendar_portal = sharedptr<LayoutItem_CalendarPortal>::cast_dynamic(portal);
  if(calendar_portal)
    portal_box = create_related_calendar(calendar_portal);
  else
    portal_box = create_related(portal);
  
  if(portal_box)
  {
    add(*portal_box, true /* expand */);
    add_layoutwidgetbase(portal_box, add_before);
  }
  else
    std::cerr << "FlowTableWithFields::add_layout_portal_at_position(): No portal was created." << std::endl;
}

void FlowTableWithFields::add_layout_notebook_at_position(const sharedptr<LayoutItem_Notebook>& notebook, const type_list_layoutwidgets::iterator& add_before)
{
  if(!notebook)
    return;

  //Add the widget:
  NotebookGlom* notebook_widget = Gtk::manage(new NotebookGlom());

  notebook_widget->show();
  notebook_widget->set_layout_item(notebook, m_table_name);

  for(LayoutGroup::type_list_items::iterator iter = notebook->m_list_items.begin(); iter != notebook->m_list_items.end(); ++iter)
  {
    sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::cast_dynamic(*iter);
    if(group)
    {
#ifndef GLOM_ENABLE_CLIENT_ONLY
      NotebookLabelGlom* tab_label = Gtk::manage(new NotebookLabelGlom(notebook_widget));
      tab_label->show();
#else
      Gtk::Label* tab_label = Gtk::manage(new Gtk::Label());
      tab_label->show();
#endif
      sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::cast_dynamic(group);
      if(portal)
      {
        const Glib::ustring tab_title = glom_get_sharedptr_title_or_name(portal->get_relationship());
        //tab_label->set_markup(Utils::bold_message(tab_title));
        tab_label->set_label(tab_title);

        //Add a Related Records list for this portal:
        Box_Data_List_Related* portal_box = create_related(portal, false /* no label, because it's in the tab instead. */);
        //portal_box->set_border_width(Glom::Utils::DEFAULT_SPACING_SMALL); It has "padding" around the Alignment instead.
        portal_box->show();
        notebook_widget->append_page(*portal_box, *tab_label);

        add_layoutwidgetbase(portal_box, add_before);
      }
      else
      {
        const Glib::ustring tab_title = group->get_title_or_name();
        //tab_label->set_markup(Utils::bold_message(tab_title));
        tab_label->set_label(tab_title);

        //Add a FlowTable for this group:
        FlowTableWithFields* flow_table = Gtk::manage( new FlowTableWithFields() );
        add_view(flow_table); //Allow these sub-flowtables to access the document too.
        flow_table->set_table(m_table_name);

        flow_table->set_columns_count(group->get_columns_count());
        flow_table->set_column_padding(get_column_padding());
        flow_table->set_row_padding(get_row_padding());
        flow_table->show();
        
        // Put the new flowtable in an event box to catch events
        Gtk::EventBox* event_box = Gtk::manage( new Gtk::EventBox() ); //TODO_Leak: Valgrind says this is possibly leaked.
        event_box->add(*flow_table);
        event_box->set_visible_window(false);
#ifndef GLOM_ENABLE_CLIENT_ONLY        
        event_box->signal_button_press_event().connect (sigc::mem_fun (*flow_table,
                                                                       &FlowTableWithFields::on_button_press_event));
#endif                                                                       
        event_box->show();
        //This doesn't work (probably because we haven't implmented it in our custom container),
        //so we put the flowtable in an alignment and give that a border instead.
        //flow_table->set_border_width(Glom::Utils::DEFAULT_SPACING_SMALL); //Put some space between the page child and the page edges.
        Gtk::Alignment* alignment = Gtk::manage(new Gtk::Alignment());
        alignment->set_border_width(Glom::Utils::DEFAULT_SPACING_SMALL);
        alignment->add(*event_box);
        alignment->show();

        notebook_widget->append_page(*alignment, *tab_label);

        //Add child items:
        LayoutGroup::type_list_items items = group->get_items(); 
        for(LayoutGroup::type_list_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
        {
          sharedptr<LayoutItem> item = *iter;
          if(item)
          {
            flow_table->add_layout_item(item);
          }
        }

        m_sub_flow_tables.push_back(flow_table);
        flow_table->set_layout_item(group, m_table_name);
        add_layoutwidgetbase(flow_table, add_before);

        //Connect signal:
        flow_table->signal_field_edited().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_flowtable_entry_edited) );
        flow_table->signal_field_open_details_requested().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_flowtable_entry_open_details_requested) );
        flow_table->signal_related_record_changed().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_flowtable_related_record_changed) );
        flow_table->signal_requested_related_details().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_flowtable_requested_related_details) );

        flow_table->signal_script_button_clicked().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_script_button_clicked) );
      }

    }
  }

  add_layoutwidgetbase(notebook_widget, add_before);
  //add_view(button); //So it can get the document.
  Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(*add_before);
  if(widget)
    insert_before (*notebook_widget, *widget, true /* expand */);
  else
    add(*notebook_widget, true /* expand */);
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
      alignment->set_padding(Utils::DEFAULT_SPACING_SMALL, 0, Utils::DEFAULT_SPACING_SMALL, 0);

    alignment->show();
    frame->add(*alignment);

    FlowTableWithFields* flow_table = Gtk::manage( new FlowTableWithFields() );

    flow_table->set_columns_count(1);
    flow_table->set_column_padding(get_column_padding());
    flow_table->set_row_padding(get_row_padding());
    flow_table->show();
    alignment->add(*flow_table);

    for(type_map_field_sequence::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      //Gtk::Entry* debug = Gtk::manage(new Gtk::Entry() );
      //flow_table->add(*debug);
      //debug->show();
      flow_table->add_field(*iter);
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
  DataWidget* pDataWidget = Gtk::manage(new DataWidget(layoutitem_field, table_name, get_document()) ); //TODO_Leak: Possibly leaked, according to valgrind.
  add_layoutwidgetbase(pDataWidget, add_before);
  add_view(pDataWidget); //So it can get the document.

  info.m_second = pDataWidget; 
  info.m_second->show_all();

  //Add a label, if one is necessary:
  Gtk::Label* label = info.m_second->get_label();
  info.m_first = label;
  if(label && !label->get_text().empty())
  {
    label->set_property("xalign", 0.0); //Equivalent to Gtk::ALIGN_LEFT, but we can't use that here.
    label->set_property("yalign", 0.5); //Equivalent ot Gtk::ALIGN_CENTER, but we can't use that here.; 

    label->show();
  }

  //info.m_group = layoutitem_field.m_group;

  //Expand multiline text fields to take up the maximum possible width:
  bool expand_second = false;
  if( (layoutitem_field->get_glom_type() == Field::TYPE_TEXT) && layoutitem_field->get_formatting_used().get_text_format_multiline())
  {
    expand_second = true;
    if(label)
      label->set_property("yalign", 0.0); //Equivalent to Gtk::ALIGN_TOP. Center is neater next to entries, but center is silly next to multi-line text boxes.
  }
  else if(layoutitem_field->get_glom_type() == Field::TYPE_IMAGE)
  {
    if(label)
      label->set_property("yalign", 0.0); //Equivalent to Gtk::ALIGN_TOP. Center is neater next to entries, but center is silly next to large images.
  }
  
  Gtk::EventBox* eventbox = Gtk::manage(new Gtk::EventBox());
  eventbox->add(*info.m_first);
  eventbox->set_visible_window(false);
  eventbox->set_events(Gdk::ALL_EVENTS_MASK);
  eventbox->show_all();
  
  Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(*add_before);
  if(widget)
    insert_before(*eventbox, *(info.m_second), *widget, expand_second);
  else
    add(*eventbox, *(info.m_second), expand_second);

  info.m_second->signal_edited().connect( sigc::bind(sigc::mem_fun(*this, &FlowTableWithFields::on_entry_edited), layoutitem_field)  ); //TODO:  Is it a good idea to bind the LayoutItem? sigc::bind() probably stores a copy at this point.

#ifndef GLOM_ENABLE_CLIENT_ONLY
  info.m_second->signal_layout_item_added().connect( sigc::bind(
    sigc::mem_fun(*this, &FlowTableWithFields::on_datawidget_layout_item_added), info.m_second) );
#endif // !GLOM_ENABLE_CLIENT_ONLY

  info.m_second->signal_open_details_requested().connect( sigc::bind(sigc::mem_fun(*this, &FlowTableWithFields::on_entry_open_details_requested), layoutitem_field)  );

  m_listFields.push_back(info); //This would be the wrong position, but you should only use this method directly when you expect it to be followed by a complete re-layout.
}


void FlowTableWithFields::add_button_at_position(const sharedptr<LayoutItem_Button>& layoutitem_button, const Glib::ustring& table_name, const type_list_layoutwidgets::iterator& add_before)
{
  //Add the widget
  ButtonGlom* button = Gtk::manage(new ButtonGlom());
  button->set_label(layoutitem_button->get_title_or_name());
  button->set_layout_item(layoutitem_button, table_name);
  button->signal_clicked().connect(
    sigc::bind(
      sigc::mem_fun(*this, &FlowTableWithFields::on_script_button_clicked),
      layoutitem_button) );

  button->show();
    
  add_layoutwidgetbase(button, add_before);
  //add_view(button); //So it can get the document.

  Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(*add_before);
  if(widget)
    insert_before (*button, *widget, false /* expand */);
  else
    add(*button, false /* expand */);
}

void FlowTableWithFields::add_textobject_at_position(const sharedptr<LayoutItem_Text>& layoutitem_text, const Glib::ustring& table_name , const type_list_layoutwidgets::iterator& add_before)
{
  //Add the widget:
  Gtk::Alignment* alignment_label = Gtk::manage(new Gtk::Alignment());
  alignment_label->set(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
  alignment_label->show();
  
  const Glib::ustring text = layoutitem_text->get_text();
  LabelGlom* label = Gtk::manage(new LabelGlom(text, 0.0 /* xalign */, 0.5 /* yalign */)); //The alignment here seems to be necessary as well (or instead of) the parent Gtk::Alignment.
  label->set_layout_item(layoutitem_text, table_name);
  label->show();
  alignment_label->add(*label);
  
  apply_formatting(*label, layoutitem_text->get_formatting_used());

  add_layoutwidgetbase(label, add_before);

  const Glib::ustring title = layoutitem_text->get_title();
  if(title.empty())
  {
    Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(*add_before);
    if(widget)
      insert_before(*alignment_label, *widget, false /* expand */);
    else
      add(*alignment_label, false /* expand */);
  }
  else
  {
    Gtk::Alignment* alignment_title = Gtk::manage(new Gtk::Alignment());
    alignment_title->set(Gtk::ALIGN_RIGHT, Gtk::ALIGN_CENTER);
    alignment_title->show();

    LabelGlom* title_label = Gtk::manage(new LabelGlom(title, 0, 0, false));
    title_label->set_layout_item(layoutitem_text, table_name);
    title_label->show();
    alignment_title->add(*title_label);
    add_layoutwidgetbase(title_label, add_before);
    
    Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(*add_before);
    if(widget)
      insert_before (*alignment_title, *alignment_label, *widget, false /* expand */);
    else
      add(*alignment_title, *alignment_label, false /* expand */);
  }
}

void FlowTableWithFields::add_placeholder_at_position(const sharedptr<LayoutItem_Placeholder>& /* layoutitem_placeholder */, const Glib::ustring& /* table_name */, const type_list_layoutwidgets::iterator& add_before)
{
  //Delete any existing placeholder (there can be only one):
  if(m_placeholder)
  {
    delete m_placeholder;
    m_placeholder = 0;
  }
  
  //Add the widget:
  m_placeholder = Gtk::manage(new Gtk::Alignment());
  m_placeholder->set(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
  m_placeholder->show();

  PlaceholderGlom* preview = Gtk::manage(new PlaceholderGlom);
  preview->show();

  m_placeholder->add(*preview);

  m_list_layoutwidgets.insert(add_before, preview);
  Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(*add_before);
  if(widget)
    insert_before(*m_placeholder, *widget, false /* expand */);
  else
    add(*m_placeholder, false);
}

void FlowTableWithFields::add_imageobject_at_position(const sharedptr<LayoutItem_Image>& layoutitem_image, const Glib::ustring& table_name , const type_list_layoutwidgets::iterator& add_before)
{
  //Add the widget:
  ImageGlom* image = Gtk::manage(new ImageGlom());
  image->set_size_request(200, 200);
  image->set_value(layoutitem_image->get_image());
  image->set_read_only(); //Only field images can be changed by the user when they are on a layout.
  image->set_layout_item(layoutitem_image, table_name);
  image->show();

  add_layoutwidgetbase(image, add_before);
  //add_view(button); //So it can get the document.

  const Glib::ustring title = layoutitem_image->get_title();
  if(title.empty())
  {
    Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(*add_before);
    if(widget)
      insert_before(*image, *widget, true /* expand */);
    else
      add(*image, true /* expand */);
  }
  else
  {
    Gtk::Alignment* alignment_title = Gtk::manage(new Gtk::Alignment());
    alignment_title->set(Gtk::ALIGN_RIGHT, Gtk::ALIGN_CENTER);
    alignment_title->show();

    Gtk::Label* title_label = Gtk::manage(new Gtk::Label(title));
    title_label->show();
    alignment_title->add(*title_label);
    Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(*add_before);
    if(widget)
      insert_before(*alignment_title, *image, *widget, true /* expand */);
    else
      add(*alignment_title, *image, true /* expand */);
  }
}

void FlowTableWithFields::get_layout_groups(Document::type_list_layout_groups& groups)
{
  sharedptr<LayoutGroup> group(get_layout_group());
  if(group)
  {
    groups.push_back(group);
  }
}

sharedptr<LayoutGroup> FlowTableWithFields::get_layout_group()
{
  return sharedptr<LayoutGroup>::cast_dynamic(get_layout_item());
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
  // TODO: Avoid duplication. Maybe we can call set_other_field_value plus
  // set the value for field's widget directly.
  //Set widgets which should show the value of this field:
  type_list_widgets list_widgets = get_field(field, true);
  for(type_list_widgets::iterator iter = list_widgets.begin(); iter != list_widgets.end(); ++iter)
  {
    DataWidget* datawidget = dynamic_cast<DataWidget*>(*iter);
    if(datawidget)
    {
      datawidget->set_value(value);
    }
  }

  //Refresh widgets which should show the related records for relationships that use this field:
  type_portals list_portals = get_portals(field /* from_key field name */);
  for(type_portals::iterator iter = list_portals.begin(); iter != list_portals.end(); ++iter)
  {
    Box_Data_Portal* portal = *iter;
    if(portal)
    {
      //g_warning("FlowTableWithFields::set_field_value: foreign_key_value=%s", value.to_string().c_str());
      portal->refresh_data_from_database_with_foreign_key(value /* foreign key value */);
    }
  }
}

void FlowTableWithFields::set_other_field_value(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  //Set widgets which should show the value of this field:
  type_list_widgets list_widgets = get_field(field, false);
  for(type_list_widgets::iterator iter = list_widgets.begin(); iter != list_widgets.end(); ++iter)
  {
    DataWidget* datawidget = dynamic_cast<DataWidget*>(*iter);
    if(datawidget)
    {
      datawidget->set_value(value);
    }
  }

  //Refresh widgets which should show the related records for relationships that use this field:
  type_portals list_portals = get_portals(field /* from_key field name */);
  for(type_portals::iterator iter = list_portals.begin(); iter != list_portals.end(); ++iter)
  {
    Box_Data_Portal* portal = *iter;
    if(portal)
    {
      //g_warning("FlowTableWithFields::set_field_value: foreign_key_value=%s", value.to_string().c_str());
      portal->refresh_data_from_database_with_foreign_key(value /* foreign key value */);
    }
  }
}

Gnome::Gda::Value FlowTableWithFields::get_field_value(const sharedptr<const LayoutItem_Field>& field) const
{
  type_list_const_widgets list_widgets = get_field(field, true);
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
  type_list_widgets list_widgets = get_field(field, true);
  for(type_list_widgets::iterator iter = list_widgets.begin(); iter != list_widgets.end(); ++iter)
  {
    DataWidget* datawidget = dynamic_cast<DataWidget*>(*iter);
    if(datawidget)
    {
      datawidget->set_editable(editable);
    }
  }
}


FlowTableWithFields::type_portals FlowTableWithFields::get_portals(const sharedptr<const LayoutItem_Field>& from_key)
{
  type_portals result;

  const Glib::ustring from_key_name = from_key->get_name();

  //Check the single-item widgets:
   for(type_portals::const_iterator iter = m_portals.begin(); iter != m_portals.end(); ++iter)
  {
    //*iter is a FlowTableItem.
    Box_Data_Portal* pPortalUI = *iter;
    if(pPortalUI)
    {
      sharedptr<LayoutItem_Portal> portal = pPortalUI->get_portal();
      if(portal)
      {
        sharedptr<Relationship> relationship = portal->get_relationship(); //In this case, we only care about the first relationship (not any child relationships), because that's what would trigger a change.
        if(relationship && (relationship->get_from_field() == from_key_name))
          result.push_back(pPortalUI);
      }
      else
      {
        std::cerr << "FlowTableWithFields::get_portals(): get_portal() returned NULL." << std::endl;
      }
    }
  }

  //Check the sub-flowtables:
  for(type_sub_flow_tables::const_iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    FlowTableWithFields* subtable = *iter;
    if(subtable)
    {
      type_portals sub_list = subtable->get_portals(from_key);
      if(!sub_list.empty())
      {
        //Add to the main result:
        result.insert(result.end(), sub_list.begin(), sub_list.end());
      }
    }
  }

  return result;
}

namespace
{
  // Get the direct widgets represesenting a given layout item
  // from a flowtable, without considering subflowtables:
  template<typename InputIterator, typename OutputIterator>
  void get_direct_fields(InputIterator begin, InputIterator end, OutputIterator out, const sharedptr<const LayoutItem_Field>& layout_item, bool include_item)
  {
    for(InputIterator iter = begin; iter != end; ++iter)
    {
      if(iter->m_field->is_same_field(layout_item) && (include_item || iter->m_field != layout_item))
      {
        if(iter->m_checkbutton)
          *out++ = iter->m_checkbutton;
        else
          *out++ = iter->m_second;
      }
    }
  }
}

FlowTableWithFields::type_list_const_widgets FlowTableWithFields::get_field(const sharedptr<const LayoutItem_Field>& layout_item, bool include_item) const
{
  type_list_const_widgets result;

  get_direct_fields(m_listFields.begin(), m_listFields.end(), std::back_inserter(result), layout_item, include_item);

  //Check the sub-flowtables:
  for(type_sub_flow_tables::const_iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    const FlowTableWithFields* subtable = *iter;
    if(subtable)
    {
      type_list_const_widgets sub_list = subtable->get_field(layout_item, include_item);
      if(!sub_list.empty())
      {
        //Add to the main result:
        result.insert(result.end(), sub_list.begin(), sub_list.end());
      }
    }
  }

  return result;
}

FlowTableWithFields::type_list_widgets FlowTableWithFields::get_field(const sharedptr<const LayoutItem_Field>& layout_item, bool include_item)
{
  type_list_widgets result;

  get_direct_fields(m_listFields.begin(), m_listFields.end(), std::back_inserter(result), layout_item, include_item);

  //TODO: Avoid duplication

  //Check the sub-flowtables:
  for(type_sub_flow_tables::const_iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    FlowTableWithFields* subtable = *iter;
    if(subtable)
    {
      type_list_widgets sub_list = subtable->get_field(layout_item, include_item);
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
    Box_Data_Portal* pPortal = *iter;
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

FlowTableWithFields::type_signal_script_button_clicked FlowTableWithFields::signal_script_button_clicked()
{
  return m_signal_script_button_clicked;
}

void FlowTableWithFields::on_script_button_clicked(const sharedptr< LayoutItem_Button>& layout_item)
{
  m_signal_script_button_clicked.emit(layout_item);
}

void FlowTableWithFields::on_entry_edited(const Gnome::Gda::Value& value, const sharedptr<const LayoutItem_Field> field)
{
  m_signal_field_edited.emit(field, value);
}

void FlowTableWithFields::on_entry_open_details_requested(const Gnome::Gda::Value& value, const sharedptr<const LayoutItem_Field> field)
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
#ifndef GLOM_ENABLE_CLIENT_ONLY
  FlowTableDnd::set_design_mode(value);
#else
  FlowTable::set_design_mode(value);
#endif

  //Set the mode in the sub-flowtables:
  for(type_sub_flow_tables::iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    FlowTableWithFields* subtable = *iter;
    if(subtable)
      subtable->set_design_mode(value);
  }
}

void FlowTableWithFields::add_layoutwidgetbase(LayoutWidgetBase* layout_widget)
{
  add_layoutwidgetbase(layout_widget, m_list_layoutwidgets.end());
}

void FlowTableWithFields::add_layoutwidgetbase(LayoutWidgetBase* layout_widget, const type_list_layoutwidgets::iterator& add_before)
{
  m_list_layoutwidgets.insert(add_before, layout_widget);

  //Handle layout_changed signal:
#ifndef GLOM_ENABLE_CLIENT_ONLY
  layout_widget->signal_layout_changed().connect(sigc::mem_fun(*this, &FlowTableWithFields::on_layoutwidget_changed));
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void FlowTableWithFields::on_layoutwidget_changed()
{
  //Forward the signal to the container, 
  //so it can make sure that the change is saved in the document.
  signal_layout_changed().emit();
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifndef GLOM_ENABLE_CLIENT_ONLY
void FlowTableWithFields::on_datawidget_layout_item_added(LayoutWidgetBase::enumType item_type, DataWidget* pDataWidget)
{
  //pDataWidget is the widget that asked us to add an item,
  //so the new item will be after that item, next to it.

  //Get the widget's layout item:
  sharedptr<const LayoutItem> layout_item = pDataWidget->get_layout_item();
  if(!layout_item)
  {
    std::cerr << "FlowTableWithFields::on_datawidget_layout_item_added(): layout_item is null." << std::endl;
    return;
  }

  //std::cout << "debug: layout_item name=" << layout_item->get_name() << std::endl;

  //Get the group that the widget's layout item is in:
  sharedptr<LayoutGroup> layout_group = sharedptr<LayoutGroup>::cast_dynamic(get_layout_item());
  if(!layout_group)
  {
    std::cerr << "FlowTableWithFields::on_datawidget_layout_item_added(): layout_group is null." << std::endl;
    return;
  }

  
  //Create/Choose the new layout item:
  sharedptr<LayoutItem> layout_item_new;
  if(item_type == LayoutWidgetBase::TYPE_FIELD)
  {
    sharedptr<LayoutItem_Field> layout_item_field = pDataWidget->offer_field_list(m_table_name);
    if(layout_item_field)
    {
      //TODO: privileges.
      layout_item_new = layout_item_field; 
    }
  }
  else if(item_type == LayoutWidgetBase::TYPE_GROUP)
  {
    sharedptr<LayoutGroup> layout_item = sharedptr<LayoutGroup>::create();
    layout_item->set_title(_("New Group"));
    layout_item_new = layout_item;
  }
  else if(item_type == LayoutWidgetBase::TYPE_NOTEBOOK)
  {
    sharedptr<LayoutItem_Notebook> layout_item = sharedptr<LayoutItem_Notebook>::create();
    layout_item->set_name(_("notebook"));

    //Add an example tab, so that it shows up.
    sharedptr<LayoutGroup> group_tab = sharedptr<LayoutGroup>::create();

    //Note to translators: This is the default name (not seen by most users) for a notebook tab.
    group_tab->set_name(_("tab1"));

    //Note to translators: This is the default label text for a notebook tab.
    group_tab->set_title(_("Tab One"));

    layout_item->add_item(group_tab);

    layout_item_new = layout_item;
  }
  else if(item_type == LayoutWidgetBase::TYPE_PORTAL)
  {
    layout_item_new = get_portal_relationship();
  }
  else if(item_type == LayoutWidgetBase::TYPE_BUTTON)
  {
    sharedptr<LayoutItem_Button> layout_item = sharedptr<LayoutItem_Button>::create();
    layout_item->set_name(_("button"));
    layout_item->set_title(_("New Button"));
    layout_item_new = layout_item;
  }
  else if(item_type == LayoutWidgetBase::TYPE_TEXT)
  {
    sharedptr<LayoutItem_Text> layout_item = sharedptr<LayoutItem_Text>::create();
    layout_item->set_name(_("text"));
    layout_item->set_text(_("New Text"));
    layout_item_new = layout_item;
  }


  if(layout_item_new)
  {
    layout_group->add_item(layout_item_new, layout_item);
  
    //We have changed the structure itself in the document, because we are using the same structure via sharedptr.
    //So we just tell the parent widgets to rebuild the layout from the document:
    signal_layout_changed().emit(); //This should result in a complete re-layout.
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void FlowTableWithFields::on_flowtable_related_record_changed(const Glib::ustring& relationship_name)
{
  //TODO_DoublyRelated

  //Forward it to the parent:
  signal_related_record_changed().emit(relationship_name);
}

void FlowTableWithFields::on_portal_user_requested_details(Gnome::Gda::Value primary_key_value, Box_Data_Portal* portal_box)
{
  sharedptr<const LayoutItem_Portal> portal = portal_box->get_portal();
  if(!portal)
    return;

  //Try to find a related (or doubly related) table that is not hidden, and open that,
  //based on the navigation options for the portal:
  Glib::ustring table_name;
  Gnome::Gda::Value primary_key;
  portal_box->get_suitable_record_to_view_details(primary_key_value, table_name, primary_key);

  if(!(table_name.empty()) && !Conversions::value_is_empty(primary_key))
    signal_requested_related_details().emit(table_name, primary_key);
}


void FlowTableWithFields::on_flowtable_requested_related_details(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value)
{
  //Forward it to the parent:
  signal_requested_related_details().emit(table_name, primary_key_value);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY

void FlowTableWithFields::on_dnd_add_layout_item_by_type(int item_type_num, Gtk::Widget* above_widget)
{
  LayoutWidgetBase::enumType item_type = static_cast<LayoutWidgetBase::enumType>(item_type_num);
  LayoutWidgetBase* above = dynamic_cast<LayoutWidgetBase*>(above_widget);
  if(!above)
    return;

  switch(item_type)
  {
    case LayoutWidgetBase::TYPE_FIELD:
      on_dnd_add_layout_item_field(above);
      break;
    case LayoutWidgetBase::TYPE_BUTTON:
      on_dnd_add_layout_item_button(above);
      break;
    case LayoutWidgetBase::TYPE_TEXT:
      on_dnd_add_layout_item_text(above);
      break;
    case LayoutWidgetBase::TYPE_IMAGE:
      on_dnd_add_layout_item_image(above);
      break;
    case LayoutWidgetBase::TYPE_GROUP:
      on_dnd_add_layout_group(above);
      break;
    case LayoutWidgetBase::TYPE_NOTEBOOK:
      on_dnd_add_layout_notebook(above);
      break;
    case LayoutWidgetBase::TYPE_PORTAL:
      on_dnd_add_layout_portal(above);
      break;
    default:
      std::cerr << "FlowTableWithFields::on_dnd_add_layout_item(): Unknown drop type: " << item_type << std::endl;
   }
}

void FlowTableWithFields::on_dnd_add_layout_item_field(LayoutWidgetBase* above)
{
  //Ask the user to choose the layout item
  sharedptr<LayoutItem_Field> layout_item_field = 
    DataWidget::offer_field_list(m_table_name, sharedptr<LayoutItem_Field>(), 
      get_document(), App_Glom::get_application());
  if(!layout_item_field)
  {
    realize();
    return;
  }

  sharedptr<LayoutItem> item = sharedptr<LayoutItem>::cast_dynamic(layout_item_field);
  dnd_add_to_layout_group(item, above);
  
  //Tell the parent to tell the document to save the layout
  signal_layout_changed().emit();
}

void FlowTableWithFields::on_dnd_add_layout_notebook (LayoutWidgetBase* above)
{
  sharedptr<LayoutItem_Notebook> notebook(new LayoutItem_Notebook);
  sharedptr<LayoutItem> item = sharedptr<LayoutItem>::cast_dynamic(notebook);
  notebook->set_name(_("Notebook"));
  // Add a group to the notebook
  sharedptr<LayoutGroup> group(new LayoutGroup ());
  group->set_title(_("New Group"));
  group->set_name(_("Group"));
  notebook->m_list_items.push_back(group);
  
  dnd_add_to_layout_group(item, above);
  
  //Tell the parent to tell the document to save the layout
  signal_layout_changed().emit();
}

void FlowTableWithFields::on_dnd_add_layout_portal (LayoutWidgetBase* above)
{
  sharedptr<LayoutItem_Portal> portal = get_portal_relationship();
  
  if(portal)
  {
    sharedptr<LayoutItem> item = sharedptr<LayoutItem>::cast_dynamic(portal);
    dnd_add_to_layout_group(item, above);
    
    //Tell the parent to tell the document to save the layout
    signal_layout_changed().emit();
  }
}

void FlowTableWithFields::on_dnd_add_layout_group(LayoutWidgetBase* above)
{  
  sharedptr<LayoutGroup> group(new LayoutGroup());
  group->set_title(_("New Group"));
  group->set_name(_("Group"));
  
  sharedptr<LayoutItem> item = sharedptr<LayoutItem>::cast_dynamic(group);
  dnd_add_to_layout_group (item, above);
  
  //Tell the parent to tell the document to save the layout
  signal_layout_changed().emit();
}

void FlowTableWithFields::on_dnd_add_layout_item_button(LayoutWidgetBase* above)
{
  // create the button
  sharedptr<LayoutItem_Button> layout_item_button = sharedptr<LayoutItem_Button>::create();
  layout_item_button->set_title(_("New Button")); //Give the button a default title, so it is big enough, and so people see that they should change it.
  layout_item_button->set_name("new_button");
  sharedptr<LayoutItem> layout_item = sharedptr<LayoutItem>::cast_dynamic(layout_item_button);

  dnd_add_to_layout_group(layout_item, above);
  //Tell the parent to tell the document to save the layout:
  signal_layout_changed().emit();
}

void FlowTableWithFields::on_dnd_add_layout_item_text(LayoutWidgetBase* above)
{
  // create the text label
  sharedptr<LayoutItem_Text> textobject = sharedptr<LayoutItem_Text>::create();
  textobject->set_name(_("text"));
  textobject->set_text(_("New Text"));
  sharedptr<LayoutItem> layout_item = sharedptr<LayoutItem>::cast_dynamic(textobject);

  dnd_add_to_layout_group(layout_item, above);
  //Tell the parent to tell the document to save the layout

  signal_layout_changed().emit();
}

void FlowTableWithFields::on_dnd_add_layout_item_image(LayoutWidgetBase* above)
{
  // create the text label
  sharedptr<LayoutItem_Image> image_object = sharedptr<LayoutItem_Image>::create();
  sharedptr<LayoutItem> layout_item = sharedptr<LayoutItem>::cast_dynamic(image_object);

  dnd_add_to_layout_group(layout_item, above);
  //Tell the parent to tell the document to save the layout

  signal_layout_changed().emit();
}

void FlowTableWithFields::on_dnd_add_layout_item(LayoutWidgetBase* above, const sharedptr<LayoutItem>& item)
{
  dnd_add_to_layout_group(item, above);
  
  // Don't do this here - it's done in the drag_end handler
  // signal_layout_changed().emit();
}

void FlowTableWithFields::on_dnd_add_placeholder(Gtk::Widget* above_widget)
{
  LayoutWidgetBase* above = dynamic_cast<LayoutWidgetBase*>(above_widget);
  if(!above)
    return;

  if(m_placeholder)
  {
    if(dynamic_cast<Glom::PlaceholderGlom*>(above))
      return;

    on_dnd_remove_placeholder();
  }
  type_list_layoutwidgets::iterator cur_widget = std::find (m_list_layoutwidgets.begin(),
                                                            m_list_layoutwidgets.end(),
                                                            above);
  sharedptr<LayoutItem_Placeholder> placeholder_field(new LayoutItem_Placeholder);
  sharedptr<LayoutItem> item = sharedptr<LayoutItem>::cast_dynamic(placeholder_field);  
  add_layout_item_at_position(placeholder_field, cur_widget);

  dnd_add_to_layout_group(item, above, true /* ignore error*/);
}

void FlowTableWithFields::on_dnd_remove_placeholder()
{ 
  if(m_placeholder)
  {
    //Get the layout group that the "above" widget's layout item is in
    sharedptr<LayoutGroup> layout_group = get_layout_group();
    if(layout_group)
    { 
      LayoutGroup::type_list_items items = layout_group->get_items();
      for (LayoutGroup::type_list_items::iterator item = items.begin();
           item != items.end(); ++item)
      {
        sharedptr<LayoutItem_Placeholder> placeholder = 
          sharedptr<LayoutItem_Placeholder>::cast_dynamic(*item);

        if(placeholder)
        {
          layout_group->remove_item(*item);
        }
      }   
    }

    remove(*m_placeholder);
  }
  
  m_placeholder = 0;
}

void FlowTableWithFields::dnd_notify_failed_drop()
{
  // TODO: Avoid this error message, maybe by adding a group.
  // TODO: At least avoid losing the dragged item.
  Gtk::MessageDialog dialog(_("You cannot drop anything here. Try to add a group first"),
                      false, Gtk::MESSAGE_ERROR);
  dialog.run();
}

bool FlowTableWithFields::dnd_add_to_layout_group(const sharedptr<LayoutItem>& item,  LayoutWidgetBase* layoutwidget, bool ignore_error)
{
  //Get the layout group that the "above" widget's layout item is in:
  sharedptr<LayoutGroup> layout_group = get_layout_group();
  if(!layout_group)
  {
    if(!ignore_error)
      dnd_notify_failed_drop();

    return false;
  }
      
  if(layoutwidget && layoutwidget->get_layout_item())
    layout_group->add_item(item, layoutwidget->get_layout_item());
  else
    layout_group->add_item(item);

  return true;
}

void FlowTableWithFields::on_menu_properties_activate()
{
  try
  {
    Glib::RefPtr<Gtk::Builder> refXml = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom_developer.glade"), "dialog_flowtable");

    Dialog_FlowTable* dialog = 0;
    refXml->get_widget_derived("dialog_flowtable", dialog);

    if(dialog)
    {
      dialog->set_flowtable(this);
      const int response = dialog->run();
      if(response == Gtk::RESPONSE_OK)
      {
        sharedptr<LayoutGroup> group = get_layout_group();
        group->set_columns_count( dialog->get_columns_count() );
        group->set_title(dialog->get_title());
        signal_layout_changed().emit();
      }

      delete dialog;
    }
  }
  catch(const Gtk::BuilderError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
}

void FlowTableWithFields::on_menu_delete_activate()
{
  Glib::ustring message;
  if(!get_layout_item()->get_title().empty())
  {
    //TODO: Use a real English sentence here?
    message = Glib::ustring::compose(_("Delete whole group \"%1\"?"),
                                      get_layout_item()->get_title());
  }
  else
  {
     //TODO: Use a real English sentence here:
    message = _("Delete whole group?");
  }

  Gtk::MessageDialog dlg(message, false, Gtk::MESSAGE_QUESTION,
    Gtk::BUTTONS_YES_NO, true);
  switch(dlg.run())
  {
    case Gtk::RESPONSE_YES:
      LayoutWidgetUtils::on_menu_delete_activate();
      break;
    default:
      return;
  }
}

bool FlowTableWithFields::on_button_press_event(GdkEventButton *event)
{
  App_Glom* pApp = App_Glom::get_application();
  if(pApp && pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  {
    GdkModifierType mods;
    gdk_window_get_pointer( Gtk::Widget::gobj()->window, 0, 0, &mods );
    if(mods & GDK_BUTTON3_MASK)
    {
      //Give user choices of actions on this item:
      m_pPopupMenuUtils->popup(event->button, event->time);
      return true; //We handled this event.
    }
  }
  return Gtk::Widget::on_button_press_event(event);
}

sharedptr<LayoutItem_Portal> FlowTableWithFields::get_portal_relationship()
{
  try
  {
    Glib::RefPtr<Gtk::Builder> refXml = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom_developer.glade"), "dialog_choose_relationship");
    
    Dialog_ChooseRelationship* dialog = 0;
    refXml->get_widget_derived("dialog_choose_relationship", dialog);
    
    if(dialog)
    {
      Document* pDocument = static_cast<Document*>(get_document());
      dialog->set_document(pDocument, m_table_name);
      //TODO: dialog->set_transient_for(*get_app_window());
      const int response = dialog->run();
      dialog->hide();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen relationship:
        sharedptr<Relationship> relationship  = dialog->get_relationship_chosen();
        if(relationship)
        {
          sharedptr<LayoutItem_Portal> layout_item = sharedptr<LayoutItem_Portal>::create();
          layout_item->set_relationship(relationship);
          return layout_item;
        }
      }
      
      delete dialog;
    }
  }
  catch(const Gtk::BuilderError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  return sharedptr<LayoutItem_Portal>();
}

void FlowTableWithFields::set_child_widget_dnd_in_progress(Gtk::Widget* child, bool in_progress)
{
  //To be reimplemented by derived classes.
  LayoutWidgetBase* base = dynamic_cast<LayoutWidgetBase*>(child);
  if(!child)
    return;

  base->set_dnd_in_progress(in_progress); 
}

bool FlowTableWithFields::get_child_widget_dnd_in_progress(Gtk::Widget* child) const
{
  LayoutWidgetBase* base = dynamic_cast<LayoutWidgetBase*>(child);
  if(!base)
    return false;
  else
    return base->get_dnd_in_progress();
}

#endif // !GLOM_ENABLE_CLIENT_ONLY

} //namespace Glom
