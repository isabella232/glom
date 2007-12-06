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
#include "buttonglom.h"
#include "notebookglom.h"
#include "imageglom.h"
#include "labelglom.h"
#include <gtkmm/checkbutton.h>
#include <glom/libglom/data_structure/glomconversions.h>
#include "../mode_data/box_data_list_related.h"
#include "../mode_data/dialog_choose_relationship.h"
#include <bakery/App/App_Gtk.h> //For util_bold_message().
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
#if !defined(GLIBMM_VFUNCS_ENABLED) || !defined(GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED)
  // This creates a custom GType for us, to override vfuncs and default
  // signal handlers even with the reduced API (done in flowtable.cc).
  // TODO: It is necessary to do this in all derived classes which is
  // rather annoying, though I don't see another possibility at the moment. armin.
  Glib::ObjectBase("Glom_FlowTable"),
#endif // !defined(GLIBMM_VFUNCS_ENABLED) || !defined(GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED)
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
      add_layout_related_at_position(portal, add_before);
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
      alignment->set_padding(Glom::Utils::DEFAULT_SPACING_SMALL, 0, 6, 0); //Use left-padding of 6 even on Maemo because indentation is important.

    alignment->show();
    frame->add(*alignment);

    FlowTableWithFields* flow_table = Gtk::manage( new FlowTableWithFields() );
    add_view(flow_table); //Allow these sub-flowtables to access the document too.
    flow_table->set_table(m_table_name);

    flow_table->set_columns_count(group->m_columns_count);
    flow_table->set_padding(Utils::DEFAULT_SPACING_SMALL);
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

    flow_table->signal_script_button_clicked().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_script_button_clicked) );
  }
}

Box_Data_List_Related* FlowTableWithFields::create_related(const sharedptr<LayoutItem_Portal>& portal, bool show_title)
{
  if(!portal)
    return 0;

  Document_Glom* pDocument = static_cast<Document_Glom*>(get_document());
  if(pDocument)
  {
    sharedptr<Relationship> relationship = pDocument->get_relationship(m_table_name, portal->get_relationship_name());
    if(relationship)
    {
      Box_Data_List_Related* portal_box = Gtk::manage(new Box_Data_List_Related);
      add_view(portal_box); //Give it access to the document, needed to get the layout and fields information.

      portal_box->init_db_details(portal, show_title); //Create the layout

      portal_box->set_layout_item(portal, relationship->get_to_table());
      portal_box->show();

      m_portals.push_back(portal_box);

      //Connect signals:
      portal_box->signal_record_changed().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_portal_record_changed) );

      portal_box->signal_user_requested_details().connect( sigc::bind( sigc::mem_fun(*this, &FlowTableWithFields::on_portal_user_requested_details), portal_box));

      return portal_box;
    }
  }

  return 0;
}

void FlowTableWithFields::add_layout_related_at_position(const sharedptr<LayoutItem_Portal>& portal, const type_list_layoutwidgets::iterator& add_before)
{
  Box_Data_List_Related* portal_box = create_related(portal);
  if(portal_box)
  {
    add(*portal_box, true /* expand */);
    add_layoutwidgetbase(portal_box, add_before);
  }
}

void FlowTableWithFields::add_layout_notebook_at_position(const sharedptr<LayoutItem_Notebook>& notebook, const type_list_layoutwidgets::iterator& add_before)
{
  if(!notebook)
    return;

  //Add the widget:
  NotebookGlom* notebook_widget = Gtk::manage(new NotebookGlom());

  notebook_widget->show();

  for(LayoutGroup::type_map_items::iterator iter = notebook->m_map_items.begin(); iter != notebook->m_map_items.end(); ++iter)
  {
    sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::cast_dynamic(iter->second);
    if(group)
    {
      Gtk::Label* tab_label = Gtk::manage(new Gtk::Label());
      tab_label->show();

      sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::cast_dynamic(group);
      if(portal)
      {
        const Glib::ustring tab_title = glom_get_sharedptr_title_or_name(portal->get_relationship());
        //tab_label->set_markup(Bakery::App_Gtk::util_bold_message(tab_title));
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
        //tab_label->set_markup(Bakery::App_Gtk::util_bold_message(tab_title));
        tab_label->set_label(tab_title);

        //Add a FlowTable for this group:
        FlowTableWithFields* flow_table = Gtk::manage( new FlowTableWithFields() );
        add_view(flow_table); //Allow these sub-flowtables to access the document too.
        flow_table->set_table(m_table_name);

        flow_table->set_columns_count(group->m_columns_count);
        flow_table->set_padding(Utils::DEFAULT_SPACING_SMALL);
        flow_table->show();

        //This doesn't work (probably because we haven't implmented it in our custom container),
        //so we put the flowtable in an alignment and give that a border instead.
        //flow_table->set_border_width(Glom::Utils::DEFAULT_SPACING_SMALL); //Put some space between the page child and the page edges.
        Gtk::Alignment* alignment = Gtk::manage(new Gtk::Alignment());
        alignment->set_border_width(Glom::Utils::DEFAULT_SPACING_SMALL);
        alignment->add(*flow_table);
        alignment->show();

        notebook_widget->append_page(*alignment, *tab_label);

        //Add child items:
        LayoutGroup::type_map_items items = group->get_items(); 
        for(LayoutGroup::type_map_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
        {
          sharedptr<LayoutItem> item = iter->second;
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
    flow_table->set_padding(Utils::DEFAULT_SPACING_SMALL);
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

  add(*(info.m_first), *(info.m_second), expand_second);

  info.m_second->signal_edited().connect( sigc::bind(sigc::mem_fun(*this, &FlowTableWithFields::on_entry_edited), layoutitem_field)  ); //TODO:  Is it a good idea to bind the LayoutItem? sigc::bind() probably stores a copy at this point.

#ifndef GLOM_ENABLE_CLIENT_ONLY
  info.m_second->signal_layout_item_added().connect( sigc::bind(
    sigc::mem_fun(*this, &FlowTableWithFields::on_datawidget_layout_item_added), info.m_second) );
#endif // !GLOM_ENABLE_CLIENT_ONLY

  info.m_second->signal_open_details_requested().connect( sigc::bind(sigc::mem_fun(*this, &FlowTableWithFields::on_entry_open_details_requested), layoutitem_field)  );

  m_listFields.push_back(info); //This would be the wrong position, but you should only use this method directly when you expect it to be followed by a complete re-layout.
}


void FlowTableWithFields::add_button_at_position(const sharedptr<LayoutItem_Button>& layoutitem_button, const Glib::ustring& /* table_name */, const type_list_layoutwidgets::iterator& add_before)
{
  //Add the widget:
  ButtonGlom* button = Gtk::manage(new ButtonGlom());
  button->set_label(layoutitem_button->get_title_or_name());

  button->signal_clicked().connect(
    sigc::bind(
      sigc::mem_fun(*this, &FlowTableWithFields::on_script_button_clicked),
      layoutitem_button) );

  button->show(), 

  add_layoutwidgetbase(button, add_before);
  //add_view(button); //So it can get the document.

  add(*button);
}

void FlowTableWithFields::add_textobject_at_position(const sharedptr<LayoutItem_Text>& layoutitem_text, const Glib::ustring& /* table_name */, const type_list_layoutwidgets::iterator& add_before)
{
  //Add the widget:
  Gtk::Alignment* alignment_label = Gtk::manage(new Gtk::Alignment());
  alignment_label->set(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
  alignment_label->show();

  LabelGlom* label = Gtk::manage(new LabelGlom(layoutitem_text->get_text(), 0.0 /* xalign */, 0.5 /* yalign */)); //The alignment here seems to be necessary as well (or instead of) the parent Gtk::Alignment.
  label->show();
  alignment_label->add(*label);

  add_layoutwidgetbase(label, add_before);
  //add_view(button); //So it can get the document.

  const Glib::ustring title = layoutitem_text->get_title();
  if(title.empty())
    add(*alignment_label, true /* expand */);
  else
  {
    Gtk::Alignment* alignment_title = Gtk::manage(new Gtk::Alignment());
    alignment_title->set(Gtk::ALIGN_RIGHT, Gtk::ALIGN_CENTER);
    alignment_title->show();

    Gtk::Label* title_label = Gtk::manage(new Gtk::Label(title));
    title_label->show();
    alignment_title->add(*title_label);

    add(*alignment_title, *alignment_label, true /* expand */);
  }
}

void FlowTableWithFields::add_imageobject_at_position(const sharedptr<LayoutItem_Image>& layoutitem_image, const Glib::ustring& /* table_name */, const type_list_layoutwidgets::iterator& add_before)
{
  //Add the widget:
  ImageGlom* image = Gtk::manage(new ImageGlom());
  image->set_size_request(200, 200);
  image->set_value(layoutitem_image->get_image());
  image->set_read_only(); //Only field images can be changed by the user when they are on a layout.
  image->show();

  add_layoutwidgetbase(image, add_before);
  //add_view(button); //So it can get the document.

  const Glib::ustring title = layoutitem_image->get_title();
  if(title.empty())
    add(*image, true /* expand */);
  else
  {
    Gtk::Alignment* alignment_title = Gtk::manage(new Gtk::Alignment());
    alignment_title->set(Gtk::ALIGN_RIGHT, Gtk::ALIGN_CENTER);
    alignment_title->show();

    Gtk::Label* title_label = Gtk::manage(new Gtk::Label(title));
    title_label->show();
    alignment_title->add(*title_label);

    add(*alignment_title, *image, true /* expand */);
  }
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
    Box_Data_List_Related* pPortalUI = *iter;
    if(pPortalUI)
    {
      sharedptr<LayoutItem_Portal> portal = pPortalUI->get_portal();
      sharedptr<Relationship> relationship = portal->get_relationship(); //In this case, we only care about the first relationship (not any child relationships), because that's what would trigger a change.
      if(relationship && (relationship->get_from_field() == from_key_name))
        result.push_back(pPortalUI);
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

  for(type_listFields::const_iterator iter = m_listFields.begin(); iter != m_listFields.end(); ++iter)
  {
    if( (iter->m_field->is_same_field(layout_item)) )
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

  for(type_listFields::const_iterator iter = m_listFields.begin(); iter != m_listFields.end(); ++iter)
  {
    if( (iter->m_field->is_same_field(layout_item)) )
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

FlowTableWithFields::type_signal_script_button_clicked FlowTableWithFields::signal_script_button_clicked()
{
  return m_signal_script_button_clicked;
}

void FlowTableWithFields::on_script_button_clicked(const sharedptr< LayoutItem_Button>& layout_item)
{
  m_signal_script_button_clicked.emit(layout_item);
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
  //Forward the signal to the container:
  signal_layout_changed().emit();
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifndef GLOM_ENABLE_CLIENT_ONLY
void FlowTableWithFields::on_datawidget_layout_item_added(LayoutWidgetBase::enumType item_type, DataWidget* pDataWidget)
{
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

  //Get the position of the selected item in its group:
  int position = -1;
  LayoutGroup::type_map_items& map_items = layout_group->m_map_items;
  for(LayoutGroup::type_map_items::const_iterator iter = map_items.begin(); iter != map_items.end(); ++iter)
  {
    sharedptr<const LayoutItem> item = iter->second;
    if(item && (item.obj() == layout_item.obj()))
    {
      position = iter->first;
 
      //std::cout << "debug: position found = " << position << ", name=" << iter->second->get_name() << std::endl;

      break;
    }
  }

  if(position == -1)
  {
    std::cerr << "FlowTableWithFields::on_datawidget_layout_item_added(): can't find layout_item in layout_group." << std::endl;
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
    group_tab->set_name(_("tab1"));
    group_tab->set_title(_("Tab One"));
    layout_item->add_item(group_tab);

    layout_item_new = layout_item;
  }
  else if(item_type == LayoutWidgetBase::TYPE_PORTAL)
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
            layout_item_new = layout_item;
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
  else if(item_type == LayoutWidgetBase::TYPE_BUTTON)
  {
    sharedptr<LayoutItem_Button> layout_item = sharedptr<LayoutItem_Button>::create();
    layout_item->set_name(_("button"));
    layout_item->set_title(_("New Button"));
    layout_item_new = layout_item;
  }
  else if(item_type == LayoutWidgetBase::TYPE_BUTTON)
  {
    sharedptr<LayoutItem_Text> layout_item = sharedptr<LayoutItem_Text>::create();
    layout_item->set_name(_("text"));
    layout_item->set_text(_("New Text"));
    layout_item_new = layout_item;
  }


  if(layout_item_new)
  {
    //Move the subsequent items forwards:
    LayoutGroup::type_map_items::iterator iterPos = map_items.find(position);
    if(iterPos == map_items.end())
    {
      std::cerr << "FlowTableWithFields::on_datawidget_layout_item_added(): Can not find iter for layout_item in layout_group with found position." << std::endl;
      return;
    }

    LayoutGroup::type_map_items::iterator iterBeforeEnd = map_items.end();
    --iterBeforeEnd;
    for(LayoutGroup::type_map_items::iterator iter = iterBeforeEnd; iter != iterPos; --iter)
    {
      const int position_next = iter->first + 1;
      iter->second->m_sequence = position_next;
      map_items[position_next] = iter->second;  
      iter->second = sharedptr<LayoutItem>();
    }

    //This position is now free for us to insert something:
    const int position_new = position + 1;
    //std::cout << "debug: position_new=" << position_new << std::endl;  

    layout_item_new->m_sequence = position_new;
    map_items[position_new] = layout_item_new;
  
    //We have changed the structure itself in the document, because we are using the same structure via sharedptr.
    //So we just tell the parent widgets to rebuild the layout from the document:
    signal_layout_changed().emit(); //This should result in a complete re-layout.
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void FlowTableWithFields::on_portal_record_changed(const Glib::ustring& relationship_name)
{
  signal_related_record_changed().emit(relationship_name);
}

void FlowTableWithFields::on_flowtable_related_record_changed(const Glib::ustring& relationship_name)
{
  //TODO_DoublyRelated

  //Forward it to the parent:
  signal_related_record_changed().emit(relationship_name);
}

void FlowTableWithFields::on_portal_user_requested_details(Gnome::Gda::Value primary_key_value, Box_Data_List_Related* portal_box)
{
  sharedptr<const LayoutItem_Portal> portal = portal_box->get_portal();
  if(!portal)
    return;

  const Glib::ustring related_table = portal->get_table_used(Glib::ustring() /* parent table - not relevant */);
  if(related_table.empty())
    return;

  if(get_document()->get_table_is_hidden(related_table))
  {
    //Try to find a doubly-related table that is not hidden, and open that instead:
    Glib::ustring doubly_related_table_name;
    Gnome::Gda::Value doubly_related_primary_key;
    portal_box->get_suitable_record_to_view_details(primary_key_value, doubly_related_table_name, doubly_related_primary_key);

    if(!(doubly_related_table_name.empty()) && !Conversions::value_is_empty(doubly_related_primary_key))
       signal_requested_related_details().emit(doubly_related_table_name, doubly_related_primary_key);
  }
  else
  {
    signal_requested_related_details().emit(related_table, primary_key_value);
  }
}

void FlowTableWithFields::on_flowtable_requested_related_details(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value)
{
  //Forward it to the parent:
  signal_requested_related_details().emit(table_name, primary_key_value);
}

} //namespace Glom
