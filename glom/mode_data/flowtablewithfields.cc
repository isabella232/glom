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
: m_first(nullptr),
  m_first_eventbox(nullptr),
  m_second(nullptr),
  m_checkbutton(nullptr)
{
}

FlowTableWithFields::FlowTableWithFields(const Glib::ustring& table_name)
:
  m_placeholder(nullptr),
  m_table_name(table_name),
  m_find_mode(false)
{
  setup_util_menu(this);
}

FlowTableWithFields::~FlowTableWithFields()
{
  //Remove views. The widgets are deleted automatically because they are managed.
  for(const auto& the_pair : m_listFields)
  {
    auto pViewFirst = dynamic_cast<View_Composite_Glom*>(the_pair.m_first);
    if(pViewFirst)
    {
      remove_view(pViewFirst);
    }

    auto pViewSecond = dynamic_cast<View_Composite_Glom*>(the_pair.m_second);
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
  for(const auto& subtable : m_sub_flow_tables)
  {
    if(subtable)
    {
      subtable->set_table(table_name);
    }
  }
}

void FlowTableWithFields::add_layout_item(const std::shared_ptr<LayoutItem>& item)
{
  //Get derived type and do the appropriate thing:
  std::shared_ptr<LayoutItem_Field> field = std::dynamic_pointer_cast<LayoutItem_Field>(item);
  if(field)
  {
    add_field(field, m_table_name);

    //Do not allow editing of auto-increment fields:
    std::shared_ptr<const Field> field_details = field->get_full_field_details();
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
    auto group = std::dynamic_pointer_cast<LayoutGroup>(item);
    if(group)
    {
      add_layout_group_or_derived(group);
    }
    else
    {
      auto layout_button = std::dynamic_pointer_cast<LayoutItem_Button>(item);
      if(layout_button)
        add_button(layout_button, m_table_name);
      else
      {
        auto layout_textobject = std::dynamic_pointer_cast<LayoutItem_Text>(item);
        if(layout_textobject)
          add_textobject(layout_textobject, m_table_name);
        else
        {
          auto layout_imageobject = std::dynamic_pointer_cast<LayoutItem_Image>(item);
          if(layout_imageobject)
            add_imageobject(layout_imageobject, m_table_name);
        }
      }
    }
  }
}

void FlowTableWithFields::add_layout_group_or_derived(const std::shared_ptr<LayoutGroup>& group, bool with_indent)
{
  auto portal = std::dynamic_pointer_cast<LayoutItem_Portal>(group);
  if(portal)
  {
    add_layout_portal(portal);
  }
  else
  {
    auto notebook = std::dynamic_pointer_cast<LayoutItem_Notebook>(group);
    if(notebook)
    {
      add_layout_notebook(notebook);
    }
    else
    {
      add_layout_group(group, with_indent);
    }
  }
}
          
void FlowTableWithFields::add_layout_group(const std::shared_ptr<LayoutGroup>& group, bool with_indent)
{
  if(!group)
    return;

  if(true)//!fields.empty() && !group_name.empty())
  {
    auto frame = Gtk::manage( new Gtk::Frame ); //TODO_leak: This is possibly leaked, according to valgrind.

    const auto group_title = item_get_title(group);
    if(!group_title.empty())
    {
      auto label = Gtk::manage( new Gtk::Label ); //TODO: This is maybe leaked, according to valgrind, though it should be managed by GtkFrame.
      label->set_markup( UiUtils::bold_message(group_title) );
      label->show();
      frame->set_label_widget(*label);
    }

    frame->set_shadow_type(Gtk::SHADOW_NONE); //HIG-style
    frame->show();

    auto flow_table = Gtk::manage( new FlowTableWithFields() );
    flow_table->set_find_mode(m_find_mode);
    add_view(flow_table); //Allow these sub-flowtables to access the document too.
    flow_table->set_table(m_table_name);

    flow_table->set_lines(group->get_columns_count());

    //Use the parent table's padding:
    flow_table->set_horizontal_spacing(get_horizontal_spacing());
    flow_table->set_vertical_spacing(get_vertical_spacing());
    flow_table->show();

    auto event_box = Gtk::manage( new Gtk::EventBox() ); //TODO_Leak: Valgrind says this is possibly leaked.
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
      event_box->set_margin_top(Utils::to_utype(Glom::UiUtils::DefaultSpacings::SMALL));

      if(with_indent) 
      {
        event_box->set_margin_start(Utils::to_utype(Glom::UiUtils::DefaultSpacings::SMALL) + BASE_INDENT);
      }
      else
      {
        event_box->set_margin_start(BASE_INDENT);
      }
    }


    LayoutGroup::type_list_items items = group->get_items();
    for(const auto& item : items)
    {
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

Box_Data_List_Related* FlowTableWithFields::create_related(const std::shared_ptr<LayoutItem_Portal>& portal, bool show_title)
{
  if(!portal)
    return 0;

  auto pDocument = static_cast<Document*>(get_document());
  if(pDocument)
  {
    auto portal_box = Gtk::manage(new Box_Data_List_Related);
    portal_box->set_find_mode(m_find_mode);
    add_view(portal_box); //Give it access to the document, needed to get the layout and fields information.

    //Create the layout:
    if(portal && portal->get_has_relationship_name())
      portal_box->init_db_details(portal, show_title);
    else
      portal_box->init_db_details(m_table_name, show_title);

    Glib::ustring to_table;
    std::shared_ptr<Relationship> relationship = pDocument->get_relationship(m_table_name, portal->get_relationship_name());
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

Box_Data_Calendar_Related* FlowTableWithFields::create_related_calendar(const std::shared_ptr<LayoutItem_CalendarPortal>& portal, bool show_title)
{
  if(!portal)
    return 0;

  auto pDocument = static_cast<Document*>(get_document());
  if(pDocument)
  {
    auto portal_box = Gtk::manage(new Box_Data_Calendar_Related);
    portal_box->set_find_mode(m_find_mode); //TODO: Implement this in the class
    add_view(portal_box); //Give it access to the document, needed to get the layout and fields information.

    //Create the layout:
    if(portal && portal->get_has_relationship_name())
      portal_box->init_db_details(portal, show_title);
    else
      portal_box->init_db_details(m_table_name, show_title);

    Glib::ustring to_table;
    std::shared_ptr<Relationship> relationship = pDocument->get_relationship(m_table_name, portal->get_relationship_name());
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

void FlowTableWithFields::add_layout_portal(const std::shared_ptr<LayoutItem_Portal>& portal)
{
  Box_Data_Portal* portal_box = nullptr;
  std::shared_ptr<LayoutItem_CalendarPortal> calendar_portal = std::dynamic_pointer_cast<LayoutItem_CalendarPortal>(portal);
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

void FlowTableWithFields::add_layout_notebook(const std::shared_ptr<LayoutItem_Notebook>& notebook)
{
  if(!notebook)
    return;

  //Add the widget:
  auto notebook_widget = Gtk::manage(new NotebookGlom());

  notebook_widget->show();
  notebook_widget->set_layout_item(notebook, m_table_name);

  for(const auto& item : notebook->m_list_items)
  {
    auto group = std::dynamic_pointer_cast<LayoutGroup>(item);
    if(group)
    {
#ifndef GLOM_ENABLE_CLIENT_ONLY
      auto tab_label = Gtk::manage(new NotebookLabel(notebook_widget));
      tab_label->show();
#else
      auto tab_label = Gtk::manage(new Gtk::Label());
      tab_label->show();
#endif

      tab_label->set_label(item_get_title_or_name(group));

      std::shared_ptr<LayoutItem_Portal> portal = std::dynamic_pointer_cast<LayoutItem_Portal>(group);
      if(portal)
      {
        //Add a Related Records list for this portal:
        auto portal_box = create_related(portal, false /* no label, because it's in the tab instead. */);
        //portal_box->set_border_width(Utils::to_utype(Glom::UiUtils::DefaultSpacings::SMALL)); It has margins around the frame's child widget instead.
        portal_box->show();
        notebook_widget->append_page(*portal_box, *tab_label);

        add_layoutwidgetbase(portal_box);
      }
      else
      {
        //Add a FlowTable for this group:
        auto flow_table = Gtk::manage( new FlowTableWithFields() );
        flow_table->set_find_mode(m_find_mode);
        add_view(flow_table); //Allow these sub-flowtables to access the document too.
        flow_table->set_table(m_table_name);

        flow_table->set_lines(group->get_columns_count());
        flow_table->set_horizontal_spacing(get_horizontal_spacing());
        flow_table->set_vertical_spacing(get_vertical_spacing());
        flow_table->show();

        // Put the new flowtable in an event box to catch events
        auto event_box = Gtk::manage( new Gtk::EventBox() ); //TODO_Leak: Valgrind says this is possibly leaked.
        event_box->add(*flow_table);
        event_box->set_visible_window(false);
#ifndef GLOM_ENABLE_CLIENT_ONLY
        event_box->signal_button_press_event().connect (sigc::mem_fun (*flow_table,
                                                                       &FlowTableWithFields::on_button_press_event));
#endif
        event_box->show();

        //Put some space between the page child and the page edges.
        //This doesn't work (probably because we haven't implemented it in our custom container),
        //so we use GtkWidget margins instead. TODO: What's the difference.
        event_box->set_margin_start(Utils::to_utype(Glom::UiUtils::DefaultSpacings::SMALL));
        event_box->set_margin_end(Utils::to_utype(Glom::UiUtils::DefaultSpacings::SMALL));
        event_box->set_margin_top(Utils::to_utype(Glom::UiUtils::DefaultSpacings::SMALL));
        event_box->set_margin_bottom(Utils::to_utype(Glom::UiUtils::DefaultSpacings::SMALL));

        notebook_widget->append_page(*event_box, *tab_label);

        //Add child items:
        for(const auto& child_item : group->get_items())
        {
          if(child_item)
          {
            flow_table->add_layout_item(child_item);
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

void FlowTableWithFields::add_field(const std::shared_ptr<LayoutItem_Field>& layoutitem_field, const Glib::ustring& table_name)
{
  Info info;
  info.m_field = layoutitem_field;

  //Add the entry or checkbox (handled by the DataWidget)
  auto pDataWidget = Gtk::manage(new DataWidget(layoutitem_field, table_name, get_document()) ); //TODO_Leak: Possibly leaked, according to valgrind.
  add_layoutwidgetbase(pDataWidget);
  add_view(pDataWidget); //So it can get the document.

  info.m_second = pDataWidget;
  info.m_second->show_all();

  //Add a label, if one is necessary:
  //We put it inside a box so that halign works as we want.
  //See https://mail.gnome.org/archives/gtk-devel-list/2014-July/msg00005.html

  auto label = info.m_second->get_label();
  if(label && !label->get_text().empty())
  {
    auto boxLabel = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
    boxLabel->show();
    boxLabel->pack_start(*label);
    info.m_first = boxLabel;

    label->set_halign(Gtk::ALIGN_START);
    label->set_valign(Gtk::ALIGN_CENTER);
    label->show();
  }
  else
  {
    info.m_first = nullptr;
  }

  //info.m_group = layoutitem_field.m_group;

  //Expand multiline text fields to take up the maximum possible width:
  if( (layoutitem_field->get_glom_type() == Field::glom_field_type::TEXT) && layoutitem_field->get_formatting_used().get_text_format_multiline())
  {
    if(label)
      label->set_valign(Gtk::ALIGN_START); //Center is neater next to entries, but center is silly next to multi-line text boxes.
  }
  else if(layoutitem_field->get_glom_type() == Field::glom_field_type::IMAGE)
  {
    if(label)
      label->set_valign(Gtk::ALIGN_START); //Center is neater next to entries, but center is silly next to large images.
  }

  auto eventbox = Gtk::manage(new Gtk::EventBox());
  if(info.m_first)
      eventbox->add(*info.m_first);

  eventbox->set_halign(Gtk::ALIGN_START);
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


void FlowTableWithFields::add_button(const std::shared_ptr<LayoutItem_Button>& layoutitem_button, const Glib::ustring& table_name)
{
  //Add the widget
  auto button = Gtk::manage(new ButtonGlom());
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
  Gtk::Widget *widget_to_add = button;
  bool expand = false;
  if(alignment != Formatting::HorizontalAlignment::LEFT)
  {
    //Put the button in a Gtk::Box so we can have non-default alignment in
    //its space. Note that we will need a different technique if we ever
    //support center alignment.
    auto box_button = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
    box_button->show();
    if(alignment == Formatting::HorizontalAlignment::RIGHT)
      box_button->pack_end(*button, Gtk::PACK_SHRINK);
    else
      box_button->pack_start(*button, Gtk::PACK_SHRINK);

    widget_to_add = box_button;
    expand = true;
  }

  add_widgets(*widget_to_add, expand);

  apply_formatting(*button, layoutitem_button);
}

void FlowTableWithFields::add_textobject(const std::shared_ptr<LayoutItem_Text>& layoutitem_text, const Glib::ustring& table_name)
{
  //Add the widget:
  const auto text = layoutitem_text->get_text(AppWindow::get_current_locale());
  auto label = Gtk::manage(new DataWidgetChildren::Label(text));
  label->set_layout_item(layoutitem_text, table_name);

  const Formatting::HorizontalAlignment alignment =
    layoutitem_text->get_formatting_used_horizontal_alignment();
  const Gtk::Align x_align = (alignment == Formatting::HorizontalAlignment::LEFT ? Gtk::ALIGN_START : Gtk::ALIGN_END);
  label->set_halign(x_align);
  label->set_valign(Gtk::ALIGN_CENTER);
  label->show();

  apply_formatting(*label, layoutitem_text);

  add_layoutwidgetbase(label);

  const auto title = item_get_title(layoutitem_text);
  if(title.empty())
  {
    add_widgets(*label, true /* expand */);
  }
  else
  {
    auto title_label = Gtk::manage(new DataWidgetChildren::Label(title, Gtk::ALIGN_START, Gtk::ALIGN_START, false));
    title_label->set_layout_item(layoutitem_text, table_name);
    title_label->set_halign(Gtk::ALIGN_END);
    title_label->set_valign(Gtk::ALIGN_CENTER);
    title_label->show();
    add_layoutwidgetbase(title_label);

    add_widgets(*title_label, *label, true /* expand */);
  }
}

void FlowTableWithFields::add_imageobject(const std::shared_ptr<LayoutItem_Image>& layoutitem_image, const Glib::ustring& table_name)
{
  //Add the widget:
  auto image = Gtk::manage(new ImageGlom());
  image->set_size_request(200, 200);
  image->set_value(layoutitem_image->get_image());
  image->set_read_only(); //Only field images can be changed by the user when they are on a layout.
  image->set_layout_item(layoutitem_image, table_name);
  image->show();

  add_layoutwidgetbase(image);
  //add_view(button); //So it can get the document.

  const auto title = item_get_title(layoutitem_image);
  if(title.empty())
  {
    add_widgets(*image, true /* expand */);
  }
  else
  {
    auto title_label = Gtk::manage(new Gtk::Label(title));
    title_label->set_halign(Gtk::ALIGN_END);
    title_label->set_valign(Gtk::ALIGN_CENTER);
    title_label->show();
    add_widgets(*title_label, *image, true /* expand */);
  }
}

void FlowTableWithFields::get_layout_groups(Document::type_list_layout_groups& groups)
{
  std::shared_ptr<LayoutGroup> group(get_layout_group());
  if(group)
  {
    groups.push_back(group);
  }
}

std::shared_ptr<LayoutGroup> FlowTableWithFields::get_layout_group()
{
  return std::dynamic_pointer_cast<LayoutGroup>(get_layout_item());
}


void FlowTableWithFields::remove_field(const Glib::ustring& id)
{
  for(type_listFields::iterator iter = m_listFields.begin(); iter != m_listFields.end(); ++iter)
  {
    Info info = *iter;
    if(info.m_field->get_name() == id)
    {
      remove(*(info.m_first));

      auto pViewFirst = dynamic_cast<View_Composite_Glom*>(info.m_first);
      if(pViewFirst)
        remove_view(pViewFirst);

      delete info.m_first;

      auto pViewSecond = dynamic_cast<View_Composite_Glom*>(info.m_second);
      if(pViewSecond)
        remove_view(pViewSecond);

      delete info.m_second; //It is removed at the same time.

      iter = m_listFields.erase(iter);
    }
  }
}

void FlowTableWithFields::set_field_value(const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  set_field_value(field, value, true);
}

void FlowTableWithFields::set_field_value(const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value, bool set_specified_field_layout)
{
  //Set widgets which should show the value of this field:
  for(const auto& item : get_field(field, set_specified_field_layout))
  {
    auto datawidget = dynamic_cast<DataWidget*>(item);
    if(datawidget)
    {
      datawidget->set_value(value);
    }
  }

  //Refresh portal widgets which should show the related records for relationships that use this field:
  for(const auto& portal : get_portals(field /* from_key field name */))
  {
    if(portal)
    {
      //std::cerr << G_STRFUNC << ": foreign_key_value=" << value.to_string() << std::endl;
      portal->refresh_data_from_database_with_foreign_key(value /* foreign key value */);
    }
  }

  //Refresh choices widgets which should show the related records for relationships that use this field:
  for(const auto& widget : get_choice_widgets(field /* from_key field name */))
  {
    if(widget)
    {
      //std::cerr << G_STRFUNC << ": foreign_key_value=" << value.to_string() << std::endl;
      widget->refresh_data_from_database_with_foreign_key(get_document(), value /* foreign key value */);
    }
  }
}

void FlowTableWithFields::set_other_field_value(const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  set_field_value(field, value, false);
}

Gnome::Gda::Value FlowTableWithFields::get_field_value(const std::shared_ptr<const LayoutItem_Field>& field) const
{
  type_list_const_widgets list_widgets = get_field(field, true);
  for(type_list_const_widgets::const_iterator iter = list_widgets.begin();
    iter != list_widgets.end(); ++iter)
  {
    const auto datawidget = dynamic_cast<const DataWidget*>(*iter);
    if(!datawidget)
      continue;

    const auto value = datawidget->get_value();
    if(!Conversions::value_is_empty(value))
      return value;
  }

  //std::cerr << G_STRFUNC << ": returning null" << std::endl;
  return Gnome::Gda::Value(); //null.
}

void FlowTableWithFields::set_field_editable(const std::shared_ptr<const LayoutItem_Field>& field, bool editable)
{
  for(const auto& item : get_field(field, true))
  {
    auto datawidget = dynamic_cast<DataWidget*>(item);
    if(datawidget)
    {
      datawidget->set_editable(editable);
    }
  }
}

void FlowTableWithFields::update_choices(const std::shared_ptr<const LayoutItem_Field>& field)
{
  for(const auto& item : get_field(field, true))
  {
    auto datawidget = dynamic_cast<DataWidget*>(item);
    if(!datawidget)
      continue;

    auto combo =
      dynamic_cast<DataWidgetChildren::ComboChoices*>(datawidget->get_data_child_widget());
    if(!combo)
      continue;

    const std::shared_ptr<const LayoutItem> layout_item = combo->get_layout_item();
    const std::shared_ptr<const LayoutItem_Field> layout_item_field = 
      std::dynamic_pointer_cast<const LayoutItem_Field>(layout_item);
    if(!layout_item_field || !layout_item_field->get_formatting_used().get_has_related_choices())
      continue;

    //TODO: Handle not-all related choices, too.
    combo->set_choices_related(get_document(), field, Gnome::Gda::Value() /* no ID means show all related records */);
  }

  //TODO: See also "Refresh choices widgets which should show the related records for relationships that use this field"
}


FlowTableWithFields::type_portals FlowTableWithFields::get_portals(const std::shared_ptr<const LayoutItem_Field>& from_key)
{
  type_portals result;

  const auto from_key_name = from_key->get_name();

  //Check the single-item widgets:
  for(const auto& pPortalUI : m_portals)
  {
    //*iter is a FlowTableItem.
    if(pPortalUI)
    {
      std::shared_ptr<LayoutItem_Portal> portal = pPortalUI->get_portal();
      if(portal)
      {
        std::shared_ptr<const Relationship> relationship = portal->get_relationship(); //In this case, we only care about the first relationship (not any child relationships), because that's what would trigger a change.
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
  for(const auto& subtable : m_sub_flow_tables)
  {
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

FlowTableWithFields::type_choice_widgets FlowTableWithFields::get_choice_widgets(const std::shared_ptr<const LayoutItem_Field>& from_key)
{
  type_choice_widgets result;
  if(!from_key)
    return result;

  const auto from_key_name = from_key->get_name();

  //Check the single-item widgets:
  for(const auto& the_pair : m_listFields)
  {
    auto widget = the_pair.m_second;
    if(!widget)
      continue;

    auto child_widget = widget->get_data_child_widget();
    auto combochoices = dynamic_cast<DataWidgetChildren::ComboChoices*>(child_widget);
    if(!combochoices)
      continue;

    const std::shared_ptr<const LayoutItem> layout_item =
      combochoices->get_layout_item();
    const std::shared_ptr<const LayoutItem_Field> field =
       std::dynamic_pointer_cast<const LayoutItem_Field>(layout_item);
    if(!field)
      continue;

    const auto format = field->get_formatting_used();

    bool choice_show_all = false;
    const std::shared_ptr<const Relationship> choice_relationship =
      format.get_choices_related_relationship(choice_show_all);
    if(choice_show_all)
      continue; //"Show All" choices don't use the ID field values.

    if(choice_relationship->get_from_field() == from_key_name)
      result.push_back(combochoices);
  }

  //Check the sub-flowtables:
  for(const auto& subtable : m_sub_flow_tables)
  {
    if(subtable)
    {
      const auto sub_list = subtable->get_choice_widgets(from_key);
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
  static void get_direct_fields(InputIterator begin, InputIterator end, OutputIterator out, const std::shared_ptr<const LayoutItem_Field>& layout_item, bool include_item)
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

FlowTableWithFields::type_list_const_widgets FlowTableWithFields::get_field(const std::shared_ptr<const LayoutItem_Field>& layout_item, bool include_item) const
{
  type_list_const_widgets result;

  get_direct_fields(m_listFields.begin(), m_listFields.end(), std::back_inserter(result), layout_item, include_item);

  //Check the sub-flowtables:
  for(const auto& subtable : m_sub_flow_tables)
  {
    if(subtable)
    {
      auto sub_list = subtable->get_field(layout_item, include_item);
      if(!sub_list.empty())
      {
        //Add to the main result:
        result.insert(result.end(), sub_list.begin(), sub_list.end());
      }
    }
  }

  return result;
}

FlowTableWithFields::type_list_widgets FlowTableWithFields::get_field(const std::shared_ptr<const LayoutItem_Field>& layout_item, bool include_item)
{
  type_list_widgets result;

  get_direct_fields(m_listFields.begin(), m_listFields.end(), std::back_inserter(result), layout_item, include_item);

  //TODO: Avoid duplication

  //Check the sub-flowtables:
  for(const auto& subtable : m_sub_flow_tables)
  {
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

  for(auto pSub : m_sub_flow_tables)
  {
    if(pSub)
    {
      remove_view(pSub);

      delete pSub;
    }
  }

  m_sub_flow_tables.clear();


  for(const auto& pPortal : m_portals)
  {
    remove_view(pPortal);
    delete pPortal;
  }
  m_portals.clear();

  m_list_layoutwidgets.clear();

  //Remove views. The widgets are deleted automatically because they are managed.
  for(const auto& the_pair : m_listFields)
  {
    auto pViewFirst = dynamic_cast<View_Composite_Glom*>(the_pair.m_first);
    if(pViewFirst)
      remove_view(pViewFirst);

   auto pViewSecond = dynamic_cast<View_Composite_Glom*>(the_pair.m_second);
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

void FlowTableWithFields::on_script_button_clicked(const std::shared_ptr< LayoutItem_Button>& layout_item)
{
  m_signal_script_button_clicked.emit(layout_item);
}

void FlowTableWithFields::on_entry_edited(const Gnome::Gda::Value& value, const std::shared_ptr<const LayoutItem_Field> field)
{
  m_signal_field_edited.emit(field, value);
}

void FlowTableWithFields::on_entry_choices_changed(const std::shared_ptr<const LayoutItem_Field> field)
{
  m_signal_field_choices_changed.emit(field);
}

void FlowTableWithFields::on_entry_open_details_requested(const Gnome::Gda::Value& value, const std::shared_ptr<const LayoutItem_Field> field)
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
  for(const auto& subtable : m_sub_flow_tables)
  {
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
  std::shared_ptr<const LayoutItem> layout_item_parent = pDataWidget->get_layout_item();
  if(!layout_item_parent)
  {
    std::cerr << G_STRFUNC << ": layout_item_parent is null." << std::endl;
    return;
  }

  //std::cout << "debug: layout_item_parent name=" << layout_item_parent->get_name() << std::endl;

  //Get the group that the widget's layout item is in:
  std::shared_ptr<LayoutGroup> layout_group = std::dynamic_pointer_cast<LayoutGroup>(get_layout_item());
  if(!layout_group)
  {
    std::cerr << G_STRFUNC << ": layout_group is null." << std::endl;
    return;
  }


  //Create/Choose the new layout item:
  std::shared_ptr<LayoutItem> layout_item_new;
  if(item_type == LayoutWidgetBase::enumType::FIELD)
  {
    std::shared_ptr<LayoutItem_Field> layout_item_field = pDataWidget->offer_field_list(m_table_name);
    if(layout_item_field)
    {
      //TODO: privileges.
      layout_item_new = layout_item_field;
    }
  }
  else if(item_type == LayoutWidgetBase::enumType::GROUP)
  {
    std::shared_ptr<LayoutGroup> layout_item = std::make_shared<LayoutGroup>();
    layout_item->set_title_original(_("New Group"));
    layout_item_new = layout_item;
  }
  else if(item_type == LayoutWidgetBase::enumType::NOTEBOOK)
  {
    std::shared_ptr<LayoutItem_Notebook> layout_item = std::make_shared<LayoutItem_Notebook>();
    layout_item->set_name(_("notebook"));

    //Add an example tab, so that it shows up.
    std::shared_ptr<LayoutGroup> group_tab = std::make_shared<LayoutGroup>();

    //Note to translators: This is the default name (not seen by most users) for a notebook tab.
    group_tab->set_name(_("tab1"));

    //Note to translators: This is the default label text for a notebook tab.
    group_tab->set_title_original(_("Tab One"));

    layout_item->add_item(group_tab);

    layout_item_new = layout_item;
  }
  else if(item_type == LayoutWidgetBase::enumType::PORTAL)
  {
    layout_item_new = get_portal_relationship();
  }
  else if(item_type == LayoutWidgetBase::enumType::BUTTON)
  {
    std::shared_ptr<LayoutItem_Button> layout_item = std::make_shared<LayoutItem_Button>();
    layout_item->set_name(_("button"));
    layout_item->set_title_original(_("New Button"));
    layout_item_new = layout_item;
  }
  else if(item_type == LayoutWidgetBase::enumType::TEXT)
  {
    std::shared_ptr<LayoutItem_Text> layout_item = std::make_shared<LayoutItem_Text>();
    layout_item->set_name(_("text"));
    layout_item->set_text_original(_("New Text"));
    layout_item_new = layout_item;
  }


  if(layout_item_new)
  {
    layout_group->add_item(layout_item_new, layout_item_parent);

    //We have changed the structure itself in the document, because we are using the same structure via std::shared_ptr.
    //So we just tell the parent widgets to rebuild the layout from the document:
    signal_layout_changed().emit(); //This should result in a complete re-layout.
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

//TODO: Use Value by const &
void FlowTableWithFields::on_portal_user_requested_details(Gnome::Gda::Value primary_key_value, Box_Data_Portal* portal_box)
{
  std::shared_ptr<const LayoutItem_Portal> portal = portal_box->get_portal();
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
  for(auto info : m_listFields)
  {
    auto widget = info.m_first;
    Glib::RefPtr<Gtk::SizeGroup> previous_size_group = info.m_first_in_sizegroup;
    if(!widget || !previous_size_group)
      continue;

    previous_size_group->remove_widget(*widget);
    info.m_first_in_sizegroup.reset();
  }

  m_vec_size_groups = size_groups;

  if(m_vec_size_groups.empty())
    return;

  for(auto info : m_listFields)
  {
    auto label = info.m_first;
    if(!label)
      continue;

    auto label_parent = info.m_first_eventbox;
    if(!label_parent)
      continue;

    //Only align labels in the first column, because items in separate columns
    //couldn't be aligned vertically anyway, and this would cause extra space.
    //TODO: Use a different SizeGroup for items in 2nd columns?
    guint column = 0;
    const auto ready = get_column_for_first_widget(*label_parent, column);
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
  const auto max_columns = get_sub_flowtables_max_columns();
  type_vec_sizegroups vec_sizegroups(max_columns);
  for(guint i = 0; i < max_columns; ++i)
  {
    vec_sizegroups[i] = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
  }

  for(const auto& subtable : m_sub_flow_tables)
  {
    if(subtable)
      subtable->apply_size_groups_to_labels(vec_sizegroups);
  }
}

guint FlowTableWithFields::get_sub_flowtables_max_columns() const
{
  guint result = get_lines();

  for(const auto& subtable : m_sub_flow_tables)
  {
    if(subtable)
    {
      const auto count = subtable->get_lines();
      if(count > result)
          result = count;
    }
  }

  return result;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY

void FlowTableWithFields::on_menu_properties_activate()
{
  Dialog_FlowTable* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;

  dialog->set_flowtable(this);
  const auto response = dialog->run();
  if(response == Gtk::RESPONSE_OK)
  {
    std::shared_ptr<LayoutGroup> group = get_layout_group();
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

bool FlowTableWithFields::on_button_press_event(GdkEventButton *button_event)
{
  auto pApp = AppWindow::get_appwindow();
  if(pApp && pApp->get_userlevel() == AppState::userlevels::DEVELOPER)
  {
    GdkModifierType mods;
    gdk_window_get_device_position( gtk_widget_get_window (Gtk::Widget::gobj()), button_event->device, 0, 0, &mods );
    if(mods & GDK_BUTTON3_MASK)
    {
      //Give user choices of actions on this item:
      m_pPopupMenuUtils->popup(button_event->button, button_event->time);
      return true; //We handled this event.
    }
  }
  return Gtk::Widget::on_button_press_event(button_event);
}

//TODO: Rename this? It's not a simpler getter. It does UI.
std::shared_ptr<LayoutItem_Portal> FlowTableWithFields::get_portal_relationship()
{
  Dialog_ChooseRelationship* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return std::shared_ptr<LayoutItem_Portal>();

  Document* pDocument = static_cast<Document*>(get_document());
  dialog->set_document(pDocument, m_table_name);
  //TODO: dialog->set_transient_for(*get_app_window());
  const auto response = dialog->run();
  dialog->hide();
  if(response == Gtk::RESPONSE_OK)
  {
    //Get the chosen relationship:
    std::shared_ptr<Relationship> relationship  = dialog->get_relationship_chosen();
    if(relationship)
    {
      std::shared_ptr<LayoutItem_Portal> layout_item = std::make_shared<LayoutItem_Portal>();
      layout_item->set_relationship(relationship);
      delete dialog;
      return layout_item;
    }
  }

  delete dialog;
  return std::shared_ptr<LayoutItem_Portal>();
}

void FlowTableWithFields::set_find_mode(bool val)
{
  m_find_mode = val;

  //Set find mode in all portals:
  for(const auto& portal : m_portals)
  {
    if(portal)
      portal->set_find_mode(m_find_mode);
  }

  //Set find mode in all the child flowtables, recursively:
  for(const auto& subtable : m_sub_flow_tables)
  {
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
  
  for(const auto& child : m_sub_flow_tables)
  {
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
