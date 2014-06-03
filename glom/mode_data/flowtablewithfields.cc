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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "flowtablewithfields.h"
#include <glom/mode_data/datawidget/datawidget.h>
#include <glom/mode_data/buttonglom.h>
#include <glom/mode_data/datawidget/combochoices.h>
#include <glom/utility_widgets/notebookglom.h>
#include <glom/utility_widgets/notebooklabelglom.h>
#include <glom/utility_widgets/imageglom.h>
#include <glom/mode_data/datawidget/label.h>
#include <glom/utility_widgets/dialog_flowtable.h>
#include <glom/appwindow.h>
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
  m_first_eventbox(0),
  m_second(0),
  m_checkbutton(0)
{
}

FlowTableWithFields::FlowTableWithFields(const Glib::ustring& table_name)
:
  m_placeholder(0),
  m_table_name(table_name),
  m_find_mode(false)
{
  setup_util_menu(this);
}

FlowTableWithFields::~FlowTableWithFields()
{
  //Remove views. The widgets are deleted automatically because they are managed.
  for(type_listFields::iterator iter = m_listFields.begin(); iter != m_listFields.end(); ++iter)
  {
    View_Composite_Glom* pViewFirst = dynamic_cast<View_Composite_Glom*>(iter->m_first);
    if(pViewFirst)
    {
      remove_view(pViewFirst);
    }

    View_Composite_Glom* pViewSecond = dynamic_cast<View_Composite_Glom*>(iter->m_second);
    if(pViewSecond)
    {
      remove_view(pViewSecond);
    }
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
  //Get derived type and do the appropriate thing:
  sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(item);
  if(field)
  {
    add_field(field, m_table_name);

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
      add_layout_portal(portal);
    }
    else
    {
      sharedptr<LayoutItem_Notebook> notebook = sharedptr<LayoutItem_Notebook>::cast_dynamic(item);
      if(notebook)
      {
        add_layout_notebook(notebook);
      }
      else
      {
        sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::cast_dynamic(item);
        if(group)
          add_layout_group(group);
        else
        {
          sharedptr<LayoutItem_Button> layout_button = sharedptr<LayoutItem_Button>::cast_dynamic(item);
          if(layout_button)
            add_button(layout_button, m_table_name);
          else
          {
            sharedptr<LayoutItem_Text> layout_textobject = sharedptr<LayoutItem_Text>::cast_dynamic(item);
            if(layout_textobject)
              add_textobject(layout_textobject, m_table_name);
            else
            {
              sharedptr<LayoutItem_Image> layout_imageobject = sharedptr<LayoutItem_Image>::cast_dynamic(item);
              if(layout_imageobject)
                add_imageobject(layout_imageobject, m_table_name);
            }
          }
        }
      }
    }
  }
}

void FlowTableWithFields::add_layout_group(const sharedptr<LayoutGroup>& group, bool with_indent)
{
  if(!group)
    return;

  if(true)//!fields.empty() && !group_name.empty())
  {
    Gtk::Frame* frame = Gtk::manage( new Gtk::Frame ); //TODO_leak: This is possibly leaked, according to valgrind.

    const Glib::ustring group_title = item_get_title(group);
    if(!group_title.empty())
    {
      Gtk::Label* label = Gtk::manage( new Gtk::Label ); //TODO: This is maybe leaked, according to valgrind, though it should be managed by GtkFrame.
      label->set_markup( UiUtils::bold_message(group_title) );
      label->show();
      frame->set_label_widget(*label);
    }

    frame->set_shadow_type(Gtk::SHADOW_NONE); //HIG-style
    frame->show();

    FlowTableWithFields* flow_table = Gtk::manage( new FlowTableWithFields() );
    flow_table->set_find_mode(m_find_mode);
    add_view(flow_table); //Allow these sub-flowtables to access the document too.
    flow_table->set_table(m_table_name);

    flow_table->set_lines(group->get_columns_count());

    //Use the parent table's padding:
    flow_table->set_horizontal_spacing(get_horizontal_spacing());
    flow_table->set_vertical_spacing(get_vertical_spacing());
    flow_table->show();

    Gtk::EventBox* event_box = Gtk::manage( new Gtk::EventBox() ); //TODO_Leak: Valgrind says this is possibly leaked.
    event_box->add(*flow_table);
    event_box->set_visible_window(false);
#ifndef GLOM_ENABLE_CLIENT_ONLY
    event_box->signal_button_press_event().connect (sigc::mem_fun (*flow_table,
      &FlowTableWithFields::on_button_press_event));
#endif
    event_box->show();

    frame->add(*event_box);

    if(!group_title.empty()) //Don't indent if it has no title, to allow use of groups just for positioning.
    {
      //Add some indenting just to avoid the out-denting caused by this GtkFrame bug:
      //https://bugzilla.gnome.org/show_bug.cgi?id=644199
      const int BASE_INDENT = 3;
      
      //std::cout << "title= " << group_title << ", with_indent=" << with_indent << std::endl;
      event_box->set_margin_top(Glom::UiUtils::DEFAULT_SPACING_SMALL);

      if(with_indent) 
      {
        event_box->set_margin_start(Glom::UiUtils::DEFAULT_SPACING_SMALL + BASE_INDENT);
      }
      else
      {
        event_box->set_margin_start(BASE_INDENT);
      }
    }


    LayoutGroup::type_list_items items = group->get_items();
    for(LayoutGroup::type_list_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
    {
      sharedptr<LayoutItem> item = *iter;
      if(item)
      {
        flow_table->add_layout_item(item);
      }
    }

    add_widgets(*frame, true /* expand */);

    m_sub_flow_tables.push_back(flow_table);
    flow_table->set_layout_item(group, m_table_name);
    add_layoutwidgetbase(flow_table);

    //Connect signals:
    signal_connect_for_reemit_2args(flow_table->signal_field_edited(), m_signal_field_edited);
    signal_connect_for_reemit_1arg(flow_table->signal_field_choices_changed(), m_signal_field_choices_changed);
    signal_connect_for_reemit_2args(flow_table->signal_field_open_details_requested(), m_signal_field_open_details_requested);
    signal_connect_for_reemit_1arg(flow_table->signal_related_record_changed(), m_signal_related_record_changed);
    signal_connect_for_reemit_2args(flow_table->signal_requested_related_details(), m_signal_requested_related_details);

    flow_table->signal_script_button_clicked().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_script_button_clicked) );

    flow_table->align_child_group_labels();
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
    portal_box->set_find_mode(m_find_mode);
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
    portal_box->set_find_mode(m_find_mode); //TODO: Implement this in the class
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

void FlowTableWithFields::add_layout_portal(const sharedptr<LayoutItem_Portal>& portal)
{
  Box_Data_Portal* portal_box = 0;
  sharedptr<LayoutItem_CalendarPortal> calendar_portal = sharedptr<LayoutItem_CalendarPortal>::cast_dynamic(portal);
  if(calendar_portal)
    portal_box = create_related_calendar(calendar_portal);
  else
    portal_box = create_related(portal);

  if(portal_box)
  {
    add_widgets(*portal_box, true /* expand */);
    add_layoutwidgetbase(portal_box);
  }
  else
    std::cerr << G_STRFUNC << ": No portal was created." << std::endl;
}

void FlowTableWithFields::add_layout_notebook(const sharedptr<LayoutItem_Notebook>& notebook)
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
      NotebookLabel* tab_label = Gtk::manage(new NotebookLabel(notebook_widget));
      tab_label->show();
#else
      Gtk::Label* tab_label = Gtk::manage(new Gtk::Label());
      tab_label->show();
#endif

      tab_label->set_label(item_get_title_or_name(group));

      sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::cast_dynamic(group);
      if(portal)
      {
        //Add a Related Records list for this portal:
        Box_Data_List_Related* portal_box = create_related(portal, false /* no label, because it's in the tab instead. */);
        //portal_box->set_border_width(Glom::UiUtils::DEFAULT_SPACING_SMALL); It has margins around the frame's child widget instead.
        portal_box->show();
        notebook_widget->append_page(*portal_box, *tab_label);

        add_layoutwidgetbase(portal_box);
      }
      else
      {
        //Add a FlowTable for this group:
        FlowTableWithFields* flow_table = Gtk::manage( new FlowTableWithFields() );
        flow_table->set_find_mode(m_find_mode);
        add_view(flow_table); //Allow these sub-flowtables to access the document too.
        flow_table->set_table(m_table_name);

        flow_table->set_lines(group->get_columns_count());
        flow_table->set_horizontal_spacing(get_horizontal_spacing());
        flow_table->set_vertical_spacing(get_vertical_spacing());
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
        //TODO: Make it work (with margins) so we can remove the deprecate Gtk::Alignment.
        //flow_table->set_border_width(Glom::UiUtils::DEFAULT_SPACING_SMALL); //Put some space between the page child and the page edges.
        Gtk::Alignment* alignment = Gtk::manage(new Gtk::Alignment());
        alignment->set_border_width(Glom::UiUtils::DEFAULT_SPACING_SMALL);
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
        add_layoutwidgetbase(flow_table);

        //Connect signal:
        signal_connect_for_reemit_2args(flow_table->signal_field_edited(), m_signal_field_edited);
        signal_connect_for_reemit_2args(flow_table->signal_field_open_details_requested(), m_signal_field_open_details_requested);
        signal_connect_for_reemit_1arg(flow_table->signal_related_record_changed(), signal_related_record_changed());
        signal_connect_for_reemit_2args(flow_table->signal_requested_related_details(), signal_requested_related_details());


        flow_table->signal_script_button_clicked().connect( sigc::mem_fun(*this, &FlowTableWithFields::on_script_button_clicked) );
      }

    }
  }

  add_layoutwidgetbase(notebook_widget);
  //add_view(button); //So it can get the document.
  add_widgets(*notebook_widget, true /* expand */);
}

void FlowTableWithFields::add_field(const sharedptr<LayoutItem_Field>& layoutitem_field, const Glib::ustring& table_name)
{
  Info info;
  info.m_field = layoutitem_field;

  //Add the entry or checkbox (handled by the DataWidget)
  DataWidget* pDataWidget = Gtk::manage(new DataWidget(layoutitem_field, table_name, get_document()) ); //TODO_Leak: Possibly leaked, according to valgrind.
  add_layoutwidgetbase(pDataWidget);
  add_view(pDataWidget); //So it can get the document.

  info.m_second = pDataWidget;
  info.m_second->show_all();

  //Add a label, if one is necessary:
  Gtk::Label* label = info.m_second->get_label();
  info.m_first = label;
  if(label && !label->get_text().empty())
  {
    label->set_property("xalign", 0.0); //Equivalent to Gtk::ALIGN_START, but we can't use that here.
    label->set_property("yalign", 0.5); //Equivalent ot Gtk::ALIGN_CENTER, but we can't use that here.;

    label->show();
  }

  //info.m_group = layoutitem_field.m_group;

  //Expand multiline text fields to take up the maximum possible width:
  if( (layoutitem_field->get_glom_type() == Field::TYPE_TEXT) && layoutitem_field->get_formatting_used().get_text_format_multiline())
  {
    if(label)
      label->set_property("yalign", 0.0); //Equivalent to Gtk::ALIGN_START. Center is neater next to entries, but center is silly next to multi-line text boxes.
  }
  else if(layoutitem_field->get_glom_type() == Field::TYPE_IMAGE)
  {
    if(label)
      label->set_property("yalign", 0.0); //Equivalent to Gtk::ALIGN_START. Center is neater next to entries, but center is silly next to large images.
  }

  Gtk::EventBox* eventbox = Gtk::manage(new Gtk::EventBox());
  eventbox->add(*info.m_first);
  info.m_first_eventbox = eventbox; //Remember it so we can retrieve the column number later from FlowTable.
  eventbox->set_visible_window(false);
  eventbox->set_events(Gdk::ALL_EVENTS_MASK);
  eventbox->show_all();

  add_widgets(*eventbox, *(info.m_second), true);

  info.m_second->signal_edited().connect( sigc::bind(sigc::mem_fun(*this, &FlowTableWithFields::on_entry_edited), layoutitem_field)  ); //TODO:  Is it a good idea to bind the LayoutItem? sigc::bind() probably stores a copy at this point.

  info.m_second->signal_choices_changed().connect( sigc::bind(sigc::mem_fun(*this, &FlowTableWithFields::on_entry_choices_changed), layoutitem_field)  );

#ifndef GLOM_ENABLE_CLIENT_ONLY
  info.m_second->signal_layout_item_added().connect( sigc::bind(
    sigc::mem_fun(*this, &FlowTableWithFields::on_datawidget_layout_item_added), info.m_second) );
#endif // !GLOM_ENABLE_CLIENT_ONLY

  info.m_second->signal_open_details_requested().connect( sigc::bind(sigc::mem_fun(*this, &FlowTableWithFields::on_entry_open_details_requested), layoutitem_field)  );

  m_listFields.push_back(info); //This would be the wrong position, but you should only use this method directly when you expect it to be followed by a complete re-layout.
}


void FlowTableWithFields::add_button(const sharedptr<LayoutItem_Button>& layoutitem_button, const Glib::ustring& table_name)
{
  //Add the widget
  ButtonGlom* button = Gtk::manage(new ButtonGlom());
  button->set_label(item_get_title_or_name(layoutitem_button));
  button->set_layout_item(layoutitem_button, table_name);
  button->signal_clicked().connect(
    sigc::bind(
      sigc::mem_fun(*this, &FlowTableWithFields::on_script_button_clicked),
      layoutitem_button) );

  button->show();

  add_layoutwidgetbase(button);
  //add_view(button); //So it can get the document.

  const Formatting::HorizontalAlignment alignment =
    layoutitem_button->get_formatting_used_horizontal_alignment();
  Gtk::Widget* widget_to_add = button;
  bool expand = false;
  if(alignment != Formatting::HORIZONTAL_ALIGNMENT_LEFT)
  {
    //Put the button in a Gtk::Box so we can have non-default alignment in
    //its space. Note that we will need a different technique if we ever
    //support center alignment.
    Gtk::Box* box_button = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
    box_button->show();
    if(alignment == Formatting::HORIZONTAL_ALIGNMENT_RIGHT)
      box_button->pack_end(*button, Gtk::PACK_SHRINK);
    else
      box_button->pack_start(*button, Gtk::PACK_SHRINK);

    widget_to_add = box_button;
    expand = true;
  }

  add_widgets(*widget_to_add, expand);

  apply_formatting(*button, layoutitem_button);
}

void FlowTableWithFields::add_textobject(const sharedptr<LayoutItem_Text>& layoutitem_text, const Glib::ustring& table_name)
{
  //Add the widget:
  const Glib::ustring text = layoutitem_text->get_text(AppWindow::get_current_locale());
  DataWidgetChildren::Label* label = Gtk::manage(new DataWidgetChildren::Label(text));
  label->set_layout_item(layoutitem_text, table_name);

  const Formatting::HorizontalAlignment alignment =
    layoutitem_text->get_formatting_used_horizontal_alignment();
  const Gtk::Align x_align = (alignment == Formatting::HORIZONTAL_ALIGNMENT_LEFT ? Gtk::ALIGN_START : Gtk::ALIGN_END);
  label->set_halign(x_align);
  label->set_valign(Gtk::ALIGN_CENTER);
  label->show();

  apply_formatting(*label, layoutitem_text);

  add_layoutwidgetbase(label);

  const Glib::ustring title = item_get_title(layoutitem_text);
  if(title.empty())
  {
    add_widgets(*label, true /* expand */);
  }
  else
  {
    DataWidgetChildren::Label* title_label = Gtk::manage(new DataWidgetChildren::Label(title, 0, 0, false));
    title_label->set_layout_item(layoutitem_text, table_name);
    title_label->set_halign(Gtk::ALIGN_END);
    title_label->set_valign(Gtk::ALIGN_CENTER);
    title_label->show();
    add_layoutwidgetbase(title_label);

    add_widgets(*title_label, *label, true /* expand */);
  }
}

void FlowTableWithFields::add_imageobject(const sharedptr<LayoutItem_Image>& layoutitem_image, const Glib::ustring& table_name)
{
  //Add the widget:
  ImageGlom* image = Gtk::manage(new ImageGlom());
  image->set_size_request(200, 200);
  image->set_value(layoutitem_image->get_image());
  image->set_read_only(); //Only field images can be changed by the user when they are on a layout.
  image->set_layout_item(layoutitem_image, table_name);
  image->show();

  add_layoutwidgetbase(image);
  //add_view(button); //So it can get the document.

  const Glib::ustring title = item_get_title(layoutitem_image);
  if(title.empty())
  {
    add_widgets(*image, true /* expand */);
  }
  else
  {
    Gtk::Label* title_label = Gtk::manage(new Gtk::Label(title));
    title_label->set_halign(Gtk::ALIGN_END);
    title_label->set_valign(Gtk::ALIGN_CENTER);
    title_label->show();
    add_widgets(*title_label, *image, true /* expand */);
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
  set_field_value(field, value, true);
}

void FlowTableWithFields::set_field_value(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value, bool set_specified_field_layout)
{
  //Set widgets which should show the value of this field:
  type_list_widgets list_widgets = get_field(field, set_specified_field_layout);
  for(type_list_widgets::iterator iter = list_widgets.begin(); iter != list_widgets.end(); ++iter)
  {
    DataWidget* datawidget = dynamic_cast<DataWidget*>(*iter);
    if(datawidget)
    {
      datawidget->set_value(value);
    }
  }

  //Refresh portal widgets which should show the related records for relationships that use this field:
  type_portals list_portals = get_portals(field /* from_key field name */);
  for(type_portals::iterator iter = list_portals.begin(); iter != list_portals.end(); ++iter)
  {
    Box_Data_Portal* portal = *iter;
    if(portal)
    {
      //std::cerr << G_STRFUNC << ": foreign_key_value=" << value.to_string() << std::endl;
      portal->refresh_data_from_database_with_foreign_key(value /* foreign key value */);
    }
  }

  //Refresh choices widgets which should show the related records for relationships that use this field:
  type_choice_widgets list_choice_widgets = get_choice_widgets(field /* from_key field name */);
  for(type_choice_widgets::iterator iter = list_choice_widgets.begin(); iter != list_choice_widgets.end(); ++iter)
  {
    DataWidgetChildren::ComboChoices* widget = *iter;
    if(widget)
    {
      //std::cerr << G_STRFUNC << ": foreign_key_value=" << value.to_string() << std::endl;
      widget->refresh_data_from_database_with_foreign_key(get_document(), value /* foreign key value */);
    }
  }
}

void FlowTableWithFields::set_other_field_value(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  set_field_value(field, value, false);
}

Gnome::Gda::Value FlowTableWithFields::get_field_value(const sharedptr<const LayoutItem_Field>& field) const
{
  type_list_const_widgets list_widgets = get_field(field, true);
  for(type_list_const_widgets::const_iterator iter = list_widgets.begin();
    iter != list_widgets.end(); ++iter)
  {
    const DataWidget* datawidget = dynamic_cast<const DataWidget*>(*iter);
    if(!datawidget)
      continue;

    const Gnome::Gda::Value value = datawidget->get_value();
    if(!Conversions::value_is_empty(value))
      return value;
  }

  //std::cerr << G_STRFUNC << ": returning null" << std::endl;
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

void FlowTableWithFields::update_choices(const sharedptr<const LayoutItem_Field>& field)
{
  type_list_widgets list_widgets = get_field(field, true);
  for(type_list_widgets::iterator iter = list_widgets.begin(); iter != list_widgets.end(); ++iter)
  {
    DataWidget* datawidget = dynamic_cast<DataWidget*>(*iter);
    if(!datawidget)
      continue;

    DataWidgetChildren::ComboChoices* combo = 
      dynamic_cast<DataWidgetChildren::ComboChoices*>(datawidget->get_data_child_widget());
    if(!combo)
      continue;

    const sharedptr<const LayoutItem> layout_item = combo->get_layout_item();
    const sharedptr<const LayoutItem_Field> layout_item_field = 
      sharedptr<const LayoutItem_Field>::cast_dynamic(layout_item);
    if(!layout_item_field || !layout_item_field->get_formatting_used().get_has_related_choices())
      continue;

    //TODO: Handle not-all related choices, too.
    combo->set_choices_related(get_document(), field, Gnome::Gda::Value() /* no ID means show all related records */);
  }

  //TODO: See also "Refresh choices widgets which should show the related records for relationships that use this field"
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
        sharedptr<const Relationship> relationship = portal->get_relationship(); //In this case, we only care about the first relationship (not any child relationships), because that's what would trigger a change.
        if(relationship && (relationship->get_from_field() == from_key_name))
          result.push_back(pPortalUI);
      }
      else
      {
        std::cerr << G_STRFUNC << ": get_portal() returned NULL." << std::endl;
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

FlowTableWithFields::type_choice_widgets FlowTableWithFields::get_choice_widgets(const sharedptr<const LayoutItem_Field>& from_key)
{
  type_choice_widgets result;
  if(!from_key)
    return result;

  const Glib::ustring from_key_name = from_key->get_name();

  //Check the single-item widgets:
  for(type_listFields::iterator iter = m_listFields.begin(); iter != m_listFields.end(); ++iter)
  {
    DataWidget* widget = iter->m_second;
    if(!widget)
      continue;

    Gtk::Widget* child_widget = widget->get_data_child_widget();
    DataWidgetChildren::ComboChoices* combochoices = dynamic_cast<DataWidgetChildren::ComboChoices*>(child_widget);
    if(!combochoices)
      continue;

    const sharedptr<const LayoutItem> layout_item =
      combochoices->get_layout_item();
    const sharedptr<const LayoutItem_Field> field =
       sharedptr<const LayoutItem_Field>::cast_dynamic(layout_item);
    if(!field)
      continue;

    const Formatting& format = field->get_formatting_used();

    bool choice_show_all = false;
    const sharedptr<const Relationship> choice_relationship =
      format.get_choices_related_relationship(choice_show_all);
    if(choice_show_all)
      continue; //"Show All" choices don't use the ID field values.

    if(choice_relationship->get_from_field() == from_key_name)
      result.push_back(combochoices);
  }

  //Check the sub-flowtables:
  for(type_sub_flow_tables::const_iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    FlowTableWithFields* subtable = *iter;
    if(subtable)
    {
      const type_choice_widgets sub_list = subtable->get_choice_widgets(from_key);
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
  static void get_direct_fields(InputIterator begin, InputIterator end, OutputIterator out, const sharedptr<const LayoutItem_Field>& layout_item, bool include_item)
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
  FlowTable::remove_all();

  m_listFields.clear();

  for(type_sub_flow_tables::iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    FlowTableWithFields* pSub = *iter;
    if(pSub)
    {
      remove_view(*iter);

      delete pSub;
    }
  }

  m_sub_flow_tables.clear();


  for(type_portals::iterator iter = m_portals.begin(); iter != m_portals.end(); ++iter)
  {
    Box_Data_Portal* pPortal = *iter;
    remove_view(pPortal);
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

FlowTableWithFields::type_signal_field_choices_changed FlowTableWithFields::signal_field_choices_changed()
{
  return m_signal_field_choices_changed;
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

void FlowTableWithFields::on_entry_choices_changed(const sharedptr<const LayoutItem_Field> field)
{
  m_signal_field_choices_changed.emit(field);
}

void FlowTableWithFields::on_entry_open_details_requested(const Gnome::Gda::Value& value, const sharedptr<const LayoutItem_Field> field)
{
  m_signal_field_open_details_requested.emit(field, value);
}

void FlowTableWithFields::set_design_mode(bool value)
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  FlowTable::set_design_mode(value);
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
  m_list_layoutwidgets.push_back(layout_widget);

  //Handle layout_changed signal:
#ifndef GLOM_ENABLE_CLIENT_ONLY
  layout_widget->signal_layout_changed().connect(signal_layout_changed().make_slot());
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void FlowTableWithFields::on_datawidget_layout_item_added(LayoutWidgetBase::enumType item_type, DataWidget* pDataWidget)
{
  //pDataWidget is the widget that asked us to add an item,
  //so the new item will be after that item, next to it.

  //Get the widget's layout item:
  sharedptr<const LayoutItem> layout_item = pDataWidget->get_layout_item();
  if(!layout_item)
  {
    std::cerr << G_STRFUNC << ": layout_item is null." << std::endl;
    return;
  }

  //std::cout << "debug: layout_item name=" << layout_item->get_name() << std::endl;

  //Get the group that the widget's layout item is in:
  sharedptr<LayoutGroup> layout_group = sharedptr<LayoutGroup>::cast_dynamic(get_layout_item());
  if(!layout_group)
  {
    std::cerr << G_STRFUNC << ": layout_group is null." << std::endl;
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
    layout_item->set_title_original(_("New Group"));
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
    group_tab->set_title_original(_("Tab One"));

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
    layout_item->set_title_original(_("New Button"));
    layout_item_new = layout_item;
  }
  else if(item_type == LayoutWidgetBase::TYPE_TEXT)
  {
    sharedptr<LayoutItem_Text> layout_item = sharedptr<LayoutItem_Text>::create();
    layout_item->set_name(_("text"));
    layout_item->set_text_original(_("New Text"));
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

//TODO: Use Value by const &
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

void FlowTableWithFields::apply_size_groups_to_labels(const type_vec_sizegroups& size_groups)
{
  //Remove widgets from any existing size group:
  for(type_listFields::iterator iter = m_listFields.begin(); iter != m_listFields.end(); ++iter)
  {
    Info info = *iter;
    Gtk::Label* label = info.m_first;
    Glib::RefPtr<Gtk::SizeGroup> previous_size_group = info.m_first_in_sizegroup;
    if(!label || !previous_size_group)
      continue;

    previous_size_group->remove_widget(*label);
    info.m_first_in_sizegroup.reset();
  }

  m_vec_size_groups = size_groups;

  if(m_vec_size_groups.empty())
    return;

  for(type_listFields::iterator iter = m_listFields.begin(); iter != m_listFields.end(); ++iter)
  {
    Info info = *iter;
    Gtk::Label* label = info.m_first;
    if(!label)
      continue;

    Gtk::EventBox* label_parent = info.m_first_eventbox;
    if(!label_parent)
      continue;

    //Only align labels in the first column, because items in separate columns
    //couldn't be aligned vertically anyway, and this would cause extra space.
    //TODO: Use a different SizeGroup for items in 2nd columns?
    guint column = 0;
    const bool ready = get_column_for_first_widget(*label_parent, column);
    if(!ready)
      continue;

    if(column >= m_vec_size_groups.size())
      continue;

    Glib::RefPtr<Gtk::SizeGroup> size_group = m_vec_size_groups[column];
    if(size_group && (info.m_first_in_sizegroup != size_group))
    {
      size_group->add_widget(*label);
      info.m_first_in_sizegroup = size_group; //Remember it so we can remove it later.
    }
  }
}

void FlowTableWithFields::align_child_group_labels()
{
  //Don't bother if there are not >1 groups to align:
  if(m_sub_flow_tables.size() < 2)
    return;

  //Create a size group for each column and tell all groups to use them for their labels:
  const guint max_columns = get_sub_flowtables_max_columns();
  type_vec_sizegroups vec_sizegroups(max_columns);
  for(guint i = 0; i < max_columns; ++i)
  {
    vec_sizegroups[i] = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
  }

  for(type_sub_flow_tables::iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    FlowTableWithFields* subtable = *iter;
    if(subtable)
      subtable->apply_size_groups_to_labels(vec_sizegroups);
  }
}

guint FlowTableWithFields::get_sub_flowtables_max_columns() const
{
  guint result = get_lines();

  for(type_sub_flow_tables::const_iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    const FlowTableWithFields* subtable = *iter;
    if(subtable)
    {
      const guint count = subtable->get_lines();
      if(count > result)
          result = count;
    }
  }

  return result;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY

void FlowTableWithFields::on_menu_properties_activate()
{
  Dialog_FlowTable* dialog = 0;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;

  dialog->set_flowtable(this);
  const int response = dialog->run();
  if(response == Gtk::RESPONSE_OK)
  {
    sharedptr<LayoutGroup> group = get_layout_group();
    group->set_columns_count( dialog->get_columns_count() );
    group->set_title(dialog->get_title(), AppWindow::get_current_locale());
    signal_layout_changed().emit();
  }

  delete dialog;

}

void FlowTableWithFields::on_menu_delete_activate()
{
  Glib::ustring message;
  if(!item_get_title(get_layout_item()).empty())
  {
    //TODO: Use a real English sentence here?
    message = Glib::ustring::compose(_("Delete whole group \"%1\"?"),
                                      item_get_title(get_layout_item()));
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
  AppWindow* pApp = AppWindow::get_appwindow();
  if(pApp && pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  {
    GdkModifierType mods;
    gdk_window_get_device_position( gtk_widget_get_window (Gtk::Widget::gobj()), event->device, 0, 0, &mods );
    if(mods & GDK_BUTTON3_MASK)
    {
      //Give user choices of actions on this item:
      m_pPopupMenuUtils->popup(event->button, event->time);
      return true; //We handled this event.
    }
  }
  return Gtk::Widget::on_button_press_event(event);
}

//TODO: Rename this? It's not a simpler getter. It does UI.
sharedptr<LayoutItem_Portal> FlowTableWithFields::get_portal_relationship()
{
  Dialog_ChooseRelationship* dialog = 0;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return sharedptr<LayoutItem_Portal>();

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
      delete dialog;
      return layout_item;
    }
  }

  delete dialog;
  return sharedptr<LayoutItem_Portal>();
}

void FlowTableWithFields::set_find_mode(bool val)
{
  m_find_mode = val;

  //Set find mode in all portals:
  for(type_portals::const_iterator iter = m_portals.begin(); iter != m_portals.end(); ++iter)
  {
    //*iter is a FlowTableItem.
    Box_Data_Portal* portal = *iter;
    if(portal)
      portal->set_find_mode(m_find_mode);
  }

  //Set find mode in all the child flowtables, recursively:
  for(type_sub_flow_tables::iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    FlowTableWithFields* subtable = *iter;
    if(subtable)
    {
      subtable->set_find_mode(m_find_mode);
    }
  }
}

void FlowTableWithFields::set_enable_drag_and_drop(bool enabled)
{
  const EggDragEnableMode drag_mode = 
    (enabled ? EGG_DRAG_FULL : EGG_DRAG_DISABLED);

  //Only enable dragging of the sub-tables.
  //Otherwise just the whole thing will be dragged,
  //though there would be nowhere to drop it:
  set_drag_enabled(EGG_DRAG_DISABLED);
  set_drop_enabled(enabled);
  
  for(type_sub_flow_tables::iterator iter = m_sub_flow_tables.begin(); iter != m_sub_flow_tables.end(); ++iter)
  {
    FlowTableWithFields* child = *iter;
    if(child)
    {
      //std::cout << G_STRFUNC << ": child" << std::endl;
      child->set_drag_enabled(drag_mode);
      child->set_drop_enabled(enabled);  
    }
  }
}
  
#endif // !GLOM_ENABLE_CLIENT_ONLY

} //namespace Glom
